"""Noise robustness study for the trained XGBoost regressor.

Adds white (Gaussian) and shot (sqrt(signal) Gaussian-approximated) noise to
the held-out test waveforms before normalization and reports AUC. Runs two
experimental conditions:

1. **Test-only noisy** -- the production XGBoost model (trained on the
   unmodified waveforms) is evaluated on test data with extra noise added.
   Tests robustness of an already-deployed model to unforeseen experimental
   noise.
2. **Matched noisy** -- a fresh XGBoost is trained on noisy training data
   and evaluated on noisy test data at the same level. Tests the ceiling on
   what's achievable if the noise conditions were known at training time.

White-noise sigma is expressed in ADC counts (1 count = 1 LSB) so sub-LSB
and super-LSB regimes are directly readable. Shot-noise sigma at each
sample = ``multiplier * sqrt(|sample_ADC|)``, the Gaussian approximation to
per-sample Poisson statistics; the multiplier isolates the noise amplitude
from the natural sqrt(signal) shape.
"""
import os
import pickle
import numpy as np
import pandas as pd
import ROOT
from scipy.signal import butter, sosfiltfilt
from sklearn.base import clone
from sklearn.metrics import roc_auc_score

from analysis_utilities.io import load_tree_data
from psd_utils import process_waveforms, ANALYSIS_CACHE_DIR, ROOT_FILES_DIR
from psd_utils import _get_training_indices
from regressors import get_default_regressors
import analysis_utilities

analysis_utilities.load_cpp_library()
ROOT.gROOT.SetBatch(True)
ROOT.PlottingUtils.SetStylePreferences(ROOT.PlotSaveFormat.kPNG)

CACHE_DIR = "noise_study_cache"
MODEL_PATH = os.path.join(ANALYSIS_CACHE_DIR, "xgb_regressor.pkl")

# Cached XGBoost SHAP feature-importance arrays from parameter_study.py
# (one numpy array per random seed, mean |SHAP| per input feature).
SWEEP_CACHE_DIR = "sweep_cache"
SHAP_SEEDS = [42, 123, 256]

WHITE_SIGMAS_ADC = [1.0, 2.0, 5.0, 10.0, 15.0, 20.0]
SHOT_MULTIPLIERS = [0.5, 1.0, 2.0, 3.0, 4.0, 5.0]
NOISE_SEED = 42

# Balanced subsampling to avoid class-imbalance effects in AUC computation.
N_TRAIN_PER_CLASS = 500
N_TEST_PER_CLASS = 100000
SUBSAMPLE_SEED = 123

# Butterworth low-pass filter applied via zero-phase sosfiltfilt. Cutoff is
# normalized to Nyquist (so 0.2 == 0.2 * 250 MHz == 50 MHz at the 500 MHz
# sample rate / 2 ns per sample). Order 4 gives a clean rolloff without
# excessive ringing on the pulse shape.
LPF_ORDER = 4
LPF_CUTOFF = 0.2

# Long-gate integral parameters matching the C++ pipeline
# (Analysis-Utilities/src/WaveformProcessingUtils.cpp + YAP-PSD Constants.hpp).
# Stored waveforms are already baseline-subtracted and polarity-inverted, so
# the long_integral feature is reproduced by summing samples from
# INTEGRATION_START to INTEGRATION_START+LONG_GATE clamped to the array end.
PRE_SAMPLES = 20
PRE_GATE = 5
LONG_GATE = 250
INTEGRATION_START = PRE_SAMPLES - PRE_GATE

# Restrict both classes to a common light-output window so the alpha/gamma
# samples have comparable peak amplitudes. This isolates the noise effects
# from the peak-amplitude-distribution confound introduced by max-normalization
# (without this restriction, low-LO gammas get noise amplified relative to
# alphas simply because they get divided by a smaller peak during
# process_waveforms).
LO_LOWER = {"alpha": 750, "gamma": 750}
LO_UPPER = {"alpha": 1500, "gamma": 1500}
LO_TAG = f"{LO_LOWER['alpha']}_{LO_UPPER['alpha']}"

RAW_DATA_CACHE = os.path.join(CACHE_DIR, f"raw_split_{LO_TAG}.npz")


def _add_white_noise(wf, sigma_adc, rng):
    if sigma_adc <= 0:
        return wf
    return wf + rng.normal(0.0, sigma_adc, size=wf.shape)


def _add_shot_noise(wf, multiplier, rng):
    if multiplier <= 0:
        return wf
    sigma = multiplier * np.sqrt(np.abs(wf))
    return wf + rng.normal(size=wf.shape) * sigma


def _balanced_subsample(arr, n, seed):
    """Pick ``n`` events uniformly at random from ``arr`` without replacement.
    Returns the full array unchanged if it already has <= n events."""
    if n >= len(arr):
        return arr
    rng = np.random.RandomState(seed)
    idx = rng.choice(len(arr), size=n, replace=False)
    return arr[idx]


def _apply_lowpass(waveforms, order=LPF_ORDER, cutoff=LPF_CUTOFF):
    """Zero-phase Butterworth low-pass filter along the sample axis (axis=1).

    Uses second-order-sections form via ``sosfiltfilt`` for numerical
    stability and to apply the filter forward + backward, eliminating
    phase distortion that would otherwise shift pulse positions and break
    direct comparability with the unfiltered waveform.
    """
    sos = butter(order, cutoff, btype="low", output="sos")
    return sosfiltfilt(sos, waveforms, axis=1)


def _make_xgb():
    """Return an unfitted XGBoost regressor matching regressors.py."""
    for r in get_default_regressors():
        if r["name"] == "XGBoost":
            return clone(r["model"])
    raise KeyError("XGBoost not found in get_default_regressors()")


def _load_or_build_split():
    """Load raw alpha/gamma waveforms split into the same train/test sets as
    the main analysis. Caches to disk after the first call."""
    if os.path.exists(RAW_DATA_CACHE):
        print(f"Loading raw split from {RAW_DATA_CACHE}")
        d = np.load(RAW_DATA_CACHE)
        return (d["alpha_train"], d["gamma_train"], d["alpha_test"],
                d["gamma_test"])

    print("Loading alpha data (Am-241)...")
    alpha_feat, alpha_wf = load_tree_data(ROOT_FILES_DIR + "Am241.root",
                                          array_branch="Samples")
    print("Loading gamma data (Na-22)...")
    gamma_feat, gamma_wf = load_tree_data(ROOT_FILES_DIR + "Na22.root",
                                          array_branch="Samples")

    a_train_idx, g_train_idx = _get_training_indices(alpha_feat, gamma_feat)

    a_all = np.arange(len(alpha_wf))
    g_all = np.arange(len(gamma_wf))
    a_test_idx = np.setdiff1d(a_all, a_train_idx)
    g_test_idx = np.setdiff1d(g_all, g_train_idx)

    # Light-output filter on both training and test sets so the alpha and
    # gamma populations have matched peak-amplitude distributions.
    a_test_lo = (
        (alpha_feat["light_output"].iloc[a_test_idx] >= LO_LOWER["alpha"])
        & (alpha_feat["light_output"].iloc[a_test_idx]
           <= LO_UPPER["alpha"])).values
    g_test_lo = (
        (gamma_feat["light_output"].iloc[g_test_idx] >= LO_LOWER["gamma"])
        & (gamma_feat["light_output"].iloc[g_test_idx]
           <= LO_UPPER["gamma"])).values
    a_train_lo = (
        (alpha_feat["light_output"].iloc[a_train_idx] >= LO_LOWER["alpha"])
        & (alpha_feat["light_output"].iloc[a_train_idx]
           <= LO_UPPER["alpha"])).values
    g_train_lo = (
        (gamma_feat["light_output"].iloc[g_train_idx] >= LO_LOWER["gamma"])
        & (gamma_feat["light_output"].iloc[g_train_idx]
           <= LO_UPPER["gamma"])).values

    a_test = alpha_wf[a_test_idx][a_test_lo]
    g_test = gamma_wf[g_test_idx][g_test_lo]

    a_train = alpha_wf[a_train_idx][a_train_lo]
    g_train = gamma_wf[g_train_idx][g_train_lo]

    np.savez(RAW_DATA_CACHE,
             alpha_train=a_train,
             gamma_train=g_train,
             alpha_test=a_test,
             gamma_test=g_test)
    print(f"Raw split cached to {RAW_DATA_CACHE}")
    return a_train, g_train, a_test, g_test


def _score_clean_trained(model, alpha_test_noisy, gamma_test_noisy):
    X = np.vstack([
        process_waveforms(alpha_test_noisy),
        process_waveforms(gamma_test_noisy)
    ])
    y = np.array([0] * len(alpha_test_noisy) + [1] * len(gamma_test_noisy))
    return float(roc_auc_score(y, model.predict(X)))


def _train_and_score(alpha_train, gamma_train, alpha_test, gamma_test):
    X_train = np.vstack(
        [process_waveforms(alpha_train),
         process_waveforms(gamma_train)])
    y_train = np.array([0] * len(alpha_train) + [1] * len(gamma_train))
    model = _make_xgb()
    model.fit(X_train, y_train)
    return _score_clean_trained(model, alpha_test, gamma_test)


def _sweep_test_only(clean_model, alpha_test, gamma_test, levels, noise_fn,
                     label):
    aucs = []
    for level in levels:
        rng = np.random.RandomState(NOISE_SEED)
        a = noise_fn(alpha_test, level, rng)
        g = noise_fn(gamma_test, level, rng)
        auc = _score_clean_trained(clean_model, a, g)
        aucs.append(auc)
        print(f"  test-only  {label}={level}  AUC = {auc:.4f}")
    return aucs


def _sweep_matched(alpha_train, gamma_train, alpha_test, gamma_test, levels,
                   noise_fn, label):
    aucs = []
    for level in levels:
        rng = np.random.RandomState(NOISE_SEED)
        a_tr = noise_fn(alpha_train, level, rng)
        g_tr = noise_fn(gamma_train, level, rng)
        a_te = noise_fn(alpha_test, level, rng)
        g_te = noise_fn(gamma_test, level, rng)
        auc = _train_and_score(a_tr, g_tr, a_te, g_te)
        aucs.append(auc)
        print(f"  matched    {label}={level}  AUC = {auc:.4f}")
    return aucs


def _plot_waveform_panel(waveforms,
                         level_label,
                         output_name,
                         output_subdir,
                         n_samples=5):
    """Overlay n_samples normalized waveforms on one canvas."""
    n_points = waveforms.shape[1]
    x_values = np.arange(n_points, dtype=np.float64) * 2

    canvas = ROOT.PlottingUtils.GetConfiguredCanvas()

    colors = [
        ROOT.kRed + 2, ROOT.kBlue + 2, ROOT.kGreen + 2, ROOT.kOrange + 2,
        ROOT.kMagenta + 2
    ]

    graphs = []
    for i in range(min(n_samples, len(waveforms))):
        graph = ROOT.TGraph(n_points, x_values,
                            waveforms[i].astype(np.float64))
        graph.SetLineColor(colors[i % len(colors)])
        graph.SetLineWidth(ROOT.PlottingUtils.GetLineWidth())
        if i == 0:
            graph.SetTitle("")
            graph.GetXaxis().SetTitle("Time [ns]")
            graph.GetYaxis().SetTitle("Normalized Amplitude [a.u.]")
            graph.GetXaxis().SetRangeUser(0, x_values[-1])
            graph.GetYaxis().SetRangeUser(-0.2, 1.2)
            graph.Draw("AL")
        else:
            graph.Draw("L SAME")
        graphs.append(graph)

    _ = ROOT.PlottingUtils.AddText(level_label, 0.78, 0.85)

    ROOT.PlottingUtils.SaveFigure(canvas, output_name, output_subdir,
                                  ROOT.PlotSaveOptions.kLINEAR)
    canvas.Close()


def _plot_noise_sample_waveforms(alpha_test,
                                 gamma_test,
                                 noise_levels,
                                 noise_fn,
                                 level_name,
                                 level_unit,
                                 output_subdir,
                                 n_samples=5,
                                 pick_seed=7):
    """Plot 5 normalized waveforms each for alpha and gamma at every noise
    level (including 0 = unmodified baseline). The same events and the same
    noise-realization seed are used at every level, so visual comparison
    isolates the effect of changing the noise magnitude.
    """
    pick_rng = np.random.RandomState(pick_seed)
    a_idx = pick_rng.choice(len(alpha_test), size=n_samples, replace=False)
    g_idx = pick_rng.choice(len(gamma_test), size=n_samples, replace=False)
    a_sample = alpha_test[a_idx]
    g_sample = gamma_test[g_idx]

    for level in [0.0] + list(noise_levels):
        rng = np.random.RandomState(NOISE_SEED)
        a_noisy = noise_fn(a_sample, level, rng)
        g_noisy = noise_fn(g_sample, level, rng)
        a_norm = process_waveforms(a_noisy)
        g_norm = process_waveforms(g_noisy)

        level_tag = f"{level:g}".replace(".", "p")
        unit_str = f" {level_unit}" if level_unit else ""
        alpha_label = f"Am-241 (#alpha), {level_name}={level:g}{unit_str}"
        gamma_label = f"Na-22 (#gamma), {level_name}={level:g}{unit_str}"

        _plot_waveform_panel(a_norm, alpha_label,
                             f"sample_alpha_{level_name}_{level_tag}",
                             output_subdir, n_samples)
        _plot_waveform_panel(g_norm, gamma_label,
                             f"sample_gamma_{level_name}_{level_tag}",
                             output_subdir, n_samples)

    print(f"Saved {2 * (1 + len(noise_levels))} waveform plots to "
          f"plots/{output_subdir}/")


def _plot_baseline_lpf_waveforms(alpha_test,
                                 gamma_test,
                                 output_subdir,
                                 n_samples=5,
                                 pick_seed=7,
                                 order=LPF_ORDER,
                                 cutoff=LPF_CUTOFF):
    """Plot the same 5 baseline events (alpha + gamma) before and after a
    Butterworth low-pass filter. Used to visualize how much high-frequency
    content the LPF strips from the unmodified data."""
    pick_rng = np.random.RandomState(pick_seed)
    a_idx = pick_rng.choice(len(alpha_test), size=n_samples, replace=False)
    g_idx = pick_rng.choice(len(gamma_test), size=n_samples, replace=False)
    a_sample = alpha_test[a_idx]
    g_sample = gamma_test[g_idx]

    a_raw = process_waveforms(a_sample)
    g_raw = process_waveforms(g_sample)
    a_lpf = process_waveforms(_apply_lowpass(a_sample, order, cutoff))
    g_lpf = process_waveforms(_apply_lowpass(g_sample, order, cutoff))

    lpf_text = f"LPF (order={order}, cutoff={cutoff:g})"
    _plot_waveform_panel(a_raw, "Am-241 (#alpha), unfiltered",
                         "sample_alpha_unfiltered", output_subdir, n_samples)
    _plot_waveform_panel(a_lpf, f"Am-241 (#alpha), {lpf_text}",
                         "sample_alpha_lpf", output_subdir, n_samples)
    _plot_waveform_panel(g_raw, "Na-22 (#gamma), unfiltered",
                         "sample_gamma_unfiltered", output_subdir, n_samples)
    _plot_waveform_panel(g_lpf, f"Na-22 (#gamma), {lpf_text}",
                         "sample_gamma_lpf", output_subdir, n_samples)

    print(f"Saved 4 baseline LPF-comparison waveform plots to "
          f"plots/{output_subdir}/")


def _compute_long_integral(waveforms):
    """Reproduce the long_integral feature from the C++ pipeline.

    Matches WaveformProcessingUtils::ExtractFeatures: sum of samples from
    INTEGRATION_START to INTEGRATION_START+LONG_GATE, clamped to the array
    end. Waveforms are already baseline-subtracted and polarity-inverted
    upstream, so this is a direct port.
    """
    n = waveforms.shape[1]
    long_end = min(INTEGRATION_START + LONG_GATE, n)
    return np.sum(waveforms[:, INTEGRATION_START:long_end], axis=1)


def _make_hist(values, n_bins, lo, hi):
    """Create and fill a TH1F using per-value Fill, matching the pattern
    used elsewhere in the codebase (e.g. _plot_classified_spectra in
    proof_of_concept.py)."""
    h = ROOT.TH1F(str(ROOT.PlottingUtils.GetRandomName()), "", n_bins, lo, hi)
    for v in values:
        h.Fill(v)
    return h


def _plot_lo_overlay(am_unmod,
                     am_noisy,
                     na_unmod,
                     na_noisy,
                     level_label,
                     output_name,
                     output_subdir,
                     lo_range,
                     n_bins=200):
    """Four overlaid LO histograms on one canvas:
        Am-241 unmodified (dashed red) vs noisy (solid red),
        Na-22  unmodified (dashed blue) vs noisy (solid blue).
    """
    canvas = ROOT.TCanvas(str(ROOT.PlottingUtils.GetRandomName()), "", 1200,
                          600)
    pad_plot = ROOT.TPad("pad_plot", "", 0.0, 0.0, 0.72, 1.0)
    pad_plot.SetLogy()
    pad_plot.SetLeftMargin(0.12)
    pad_plot.SetRightMargin(0.05)
    pad_plot.Draw()
    pad_leg = ROOT.TPad("pad_leg", "", 0.72, 0.0, 1.0, 1.0)
    pad_leg.SetLeftMargin(0.0)
    pad_leg.SetRightMargin(0.05)
    pad_leg.Draw()
    pad_plot.cd()

    lo_min, lo_max = lo_range
    h_am_u = _make_hist(am_unmod, n_bins, lo_min, lo_max)
    h_am_n = _make_hist(am_noisy, n_bins, lo_min, lo_max)
    h_na_u = _make_hist(na_unmod, n_bins, lo_min, lo_max)
    h_na_n = _make_hist(na_noisy, n_bins, lo_min, lo_max)

    ROOT.PlottingUtils.ConfigureHistogram(h_am_u, ROOT.kRed + 2)
    ROOT.PlottingUtils.ConfigureHistogram(h_am_n, ROOT.kRed + 2)
    ROOT.PlottingUtils.ConfigureHistogram(h_na_u, ROOT.kBlue + 2)
    ROOT.PlottingUtils.ConfigureHistogram(h_na_n, ROOT.kBlue + 2)
    h_am_u.SetLineStyle(2)
    h_na_u.SetLineStyle(2)

    h_am_u.GetXaxis().SetTitle("Long integral [ADC]")
    h_am_u.GetYaxis().SetTitle("Counts")
    h_am_u.GetYaxis().SetTitleOffset(1)
    h_am_u.SetTitle("")

    max_val = max(h_am_u.GetMaximum(), h_am_n.GetMaximum(),
                  h_na_u.GetMaximum(), h_na_n.GetMaximum())
    h_am_u.SetMaximum(max_val * 1.4)

    h_am_u.Draw("HIST")
    h_am_n.Draw("HIST SAME")
    h_na_u.Draw("HIST SAME")
    h_na_n.Draw("HIST SAME")

    _ = ROOT.PlottingUtils.AddText(level_label, 0.92, 0.85)

    pad_leg.cd()
    leg = ROOT.PlottingUtils.AddLegend(0.0, 0.95, 0.55, 0.85)
    leg.SetMargin(0.15)
    leg.AddEntry(h_am_u, "Am-241 unmodified", "l")
    leg.AddEntry(h_am_n, "Am-241 noisy", "l")
    leg.AddEntry(h_na_u, "Na-22 unmodified", "l")
    leg.AddEntry(h_na_n, "Na-22 noisy", "l")
    leg.Draw()

    ROOT.PlottingUtils.SaveFigure(canvas, output_name, output_subdir,
                                  ROOT.PlotSaveOptions.kLOG)
    canvas.Close()
    h_am_u.Delete()
    h_am_n.Delete()
    h_na_u.Delete()
    h_na_n.Delete()


def _plot_noise_light_output(alpha_test, gamma_test, noise_levels, noise_fn,
                             level_name, level_unit, output_subdir):
    """For each noise level (plus the unmodified baseline), plot LO spectra
    overlaying the unmodified data against the same data with noise added.

    Noise is applied directly to the full waveform array; the long_integral
    is then recomputed using the same algorithm as the C++ pipeline.
    """
    am_unmod = _compute_long_integral(alpha_test)
    na_unmod = _compute_long_integral(gamma_test)

    all_lo = np.concatenate([am_unmod, na_unmod])
    lo_lo, lo_hi = np.percentile(all_lo, [0.1, 99.9])
    span = lo_hi - lo_lo
    lo_range = (lo_lo - 0.3 * span, lo_hi + 0.5 * span)

    for level in [0.0] + list(noise_levels):
        rng = np.random.RandomState(NOISE_SEED)
        am_noisy = _compute_long_integral(noise_fn(alpha_test, level, rng))
        na_noisy = _compute_long_integral(noise_fn(gamma_test, level, rng))

        level_tag = f"{level:g}".replace(".", "p")
        unit_str = f" {level_unit}" if level_unit else ""
        level_label = f"{level_name}={level:g}{unit_str}"
        output_name = f"lo_{level_name}_{level_tag}"
        _plot_lo_overlay(am_unmod, am_noisy, na_unmod, na_noisy, level_label,
                         output_name, output_subdir, lo_range)

    print(f"Saved {1 + len(noise_levels)} LO spectra to "
          f"plots/{output_subdir}/")


def _plot_variance_vs_mean(alpha_test,
                           gamma_test,
                           output_name,
                           min_amplitude_frac=0.03):
    """Per-sample sigma^2(t) vs mu(t) on a log-log plot, one point per
    sample and per class. Pure Poisson shot noise predicts sigma^2 ~ mu
    (slope 1 in log-log); flat regions indicate an electronic noise floor;
    slope-2 regions indicate multiplicative noise. The TGraphs are saved
    to a ROOT file in CACHE_DIR so they can be opened in TBrowser and
    fitted interactively with the FitPanel.

    Samples with mean amplitude below ``min_amplitude_frac`` * (per-class
    peak mean) are excluded, since pre-trigger baseline and far-decay-tail
    samples are noise-floor-dominated and scatter on the log-log plot
    without informing the noise-budget question.
    """
    mean_a = np.mean(alpha_test, axis=0)
    var_a = np.var(alpha_test, axis=0)
    mean_g = np.mean(gamma_test, axis=0)
    var_g = np.var(gamma_test, axis=0)

    thresh_a = min_amplitude_frac * float(mean_a.max())
    thresh_g = min_amplitude_frac * float(mean_g.max())
    mask_a = (mean_a > thresh_a) & (var_a > 0)
    mask_g = (mean_g > thresh_g) & (var_g > 0)
    print(f"  Keeping {int(mask_a.sum())}/{len(mean_a)} alpha samples and "
          f"{int(mask_g.sum())}/{len(mean_g)} gamma samples "
          f"(>= {min_amplitude_frac * 100:.1f}% of peak)")
    mu_a = mean_a[mask_a].astype(np.float64)
    v_a = var_a[mask_a].astype(np.float64)
    mu_g = mean_g[mask_g].astype(np.float64)
    v_g = var_g[mask_g].astype(np.float64)

    canvas = ROOT.PlottingUtils.GetConfiguredCanvas()
    canvas.SetLogx(True)
    canvas.SetLogy(True)
    canvas.SetLeftMargin(0.14)
    canvas.SetRightMargin(0.04)

    g_alpha = ROOT.TGraph(len(mu_a), mu_a, v_a)
    g_alpha.SetName("alpha_variance_vs_mean")
    g_alpha.SetTitle(";#mu(t) [ADC];#sigma^{2}(t) [ADC^{2}]")
    g_alpha.SetMarkerColor(ROOT.kRed + 2)
    g_alpha.SetMarkerStyle(20)
    g_alpha.SetMarkerSize(0.9)
    g_alpha.GetXaxis().SetTitle("Per-sample mean amplitude #mu(t) [ADC]")
    g_alpha.GetYaxis().SetTitle("Per-sample variance #sigma^{2}(t) [ADC^{2}]")
    g_alpha.GetYaxis().SetTitleOffset(1.2)

    all_mu = np.concatenate([mu_a, mu_g])
    all_v = np.concatenate([v_a, v_g])
    mu_lo = float(all_mu.min()) * 0.5
    mu_hi = float(all_mu.max()) * 2.0
    v_lo = float(all_v.min()) * 0.5
    v_hi = float(all_v.max()) * 2.0
    g_alpha.GetXaxis().SetLimits(mu_lo, mu_hi)
    g_alpha.GetYaxis().SetRangeUser(v_lo, v_hi)
    g_alpha.Draw("AP")

    g_gamma = ROOT.TGraph(len(mu_g), mu_g, v_g)
    g_gamma.SetName("gamma_variance_vs_mean")
    g_gamma.SetTitle(";#mu(t) [ADC];#sigma^{2}(t) [ADC^{2}]")
    g_gamma.SetMarkerColor(ROOT.kBlue + 2)
    g_gamma.SetMarkerStyle(21)
    g_gamma.SetMarkerSize(0.9)
    g_gamma.Draw("P SAME")

    # Identify outliers via residuals from a global power-law fit (in log
    # space). Points more than `outlier_n_sigma` * stddev off the trend get
    # labeled with their original time-sample index so they can be matched
    # back to a specific position on the waveform.
    label_objs = []
    outlier_n_sigma = 1.8
    for class_name, color, mu_arr, v_arr, mask_arr in (
        ("alpha", ROOT.kRed + 2, mu_a, v_a, mask_a),
        ("gamma", ROOT.kBlue + 2, mu_g, v_g, mask_g),
    ):
        log_mu = np.log(mu_arr)
        log_v = np.log(v_arr)
        slope, intercept = np.polyfit(log_mu, log_v, 1)
        residuals = log_v - (intercept + slope * log_mu)
        thresh = outlier_n_sigma * float(np.std(residuals))
        outlier_mask = np.abs(residuals) > thresh
        kept_sample_indices = np.where(mask_arr)[0]
        n_out = int(outlier_mask.sum())
        print(f"  {class_name}: bulk power-law slope = {slope:.3f}, "
              f"{n_out} outliers (>{outlier_n_sigma:.1f}sigma off-trend):")
        for i in np.where(outlier_mask)[0]:
            sample_idx = int(kept_sample_indices[i])
            t_ns = sample_idx * 2
            print(f"    sample {sample_idx:3d} (t = {t_ns} ns): "
                  f"mu = {mu_arr[i]:.2f}, var = {v_arr[i]:.2f}, "
                  f"residual = {residuals[i]:+.2f}")
            lbl = ROOT.TLatex(mu_arr[i], v_arr[i], f" {sample_idx}")
            lbl.SetTextSize(0.022)
            lbl.SetTextColor(color)
            lbl.Draw()
            label_objs.append(lbl)

    leg = ROOT.PlottingUtils.AddLegend(0.17, 0.35, 0.7, 0.88)
    leg.AddEntry(g_alpha, "Am-241 (#alpha)", "p")
    leg.AddEntry(g_gamma, "Na-22 (#gamma)", "p")
    leg.Draw()

    ROOT.PlottingUtils.SaveFigure(canvas, output_name, "",
                                  ROOT.PlotSaveOptions.kLOG)
    canvas.Close()
    print(f"Saved {output_name}")

    # Dump the full per-sample table (alpha + gamma, indexed by sample) to a
    # text file alongside the ROOT file so non-outliers can also be inspected.
    txt_path = os.path.join(CACHE_DIR, f"{output_name}.txt")
    with open(txt_path, "w") as fh:
        fh.write(
            "# sample_idx  t_ns  mu_alpha  var_alpha  mu_gamma  var_gamma\n")
        for s in range(len(mean_a)):
            fh.write(f"{s:4d}  {s * 2:5d}  "
                     f"{mean_a[s]:12.4f}  {var_a[s]:14.4f}  "
                     f"{mean_g[s]:12.4f}  {var_g[s]:14.4f}\n")
    print(f"Saved {txt_path}")

    root_path = os.path.join(CACHE_DIR, f"{output_name}.root")
    out_file = ROOT.TFile(root_path, "RECREATE")
    g_alpha.Write()
    g_gamma.Write()
    out_file.Close()
    print(f"Saved {root_path} (open in TBrowser and right-click "
          f"-> FitPanel to fit interactively)")


def _load_xgb_shap_mean():
    """Average XGBoost SHAP importance across seeds from parameter_study.py.
    Returns None if no cached SHAP files are found."""
    arrs = []
    for seed in SHAP_SEEDS:
        path = os.path.join(SWEEP_CACHE_DIR, f"xgb_shap_values_seed{seed}.npy")
        if os.path.exists(path):
            arrs.append(np.load(path))
    if not arrs:
        return None
    return np.mean(np.stack(arrs, axis=0), axis=0)


def _plot_average_waveform_band(alpha_test, gamma_test, output_name):
    """Single-pad plot of per-sample class-mean separation vs. within-class
    spread, with the XGBoost SHAP attribution overlaid.

    Statistics are computed on per-event-normalized waveforms (the same
    process_waveforms output the model receives), so sigma_pool reflects
    only the shape variation that the model actually sees -- not the LO
    variation across events, which per-event peak normalization removes.

      |Delta mu(t)| = |mean_alpha(t) - mean_gamma(t)|              (red)
      sigma_pool(t) = sqrt(0.5 * (sigma_alpha^2 + sigma_gamma^2))  (blue)
      XGBoost SHAP(t), rescaled to share the y-axis                (black, dashed)

    |Delta mu| and sigma_pool are in the same normalized-amplitude units so
    they are directly comparable; their ratio is the per-sample d'
    discriminability. SHAP is rescaled to max(|Delta mu|, sigma_pool) so its
    peak location can be visually compared.
    """
    a_norm = process_waveforms(alpha_test)
    g_norm = process_waveforms(gamma_test)

    mean_a = np.mean(a_norm, axis=0)
    std_a = np.std(a_norm, axis=0)
    mean_g = np.mean(g_norm, axis=0)
    std_g = np.std(g_norm, axis=0)

    mean_diff = np.abs(mean_a - mean_g)
    sigma_pool = np.sqrt(0.5 * (std_a**2 + std_g**2))

    n_points = len(mean_a)
    x_values = np.arange(n_points, dtype=np.float64) * 2

    canvas = ROOT.PlottingUtils.GetConfiguredCanvas()

    g_mean_diff = ROOT.TGraph(n_points, x_values, mean_diff.astype(np.float64))
    g_mean_diff.SetLineColor(ROOT.kRed + 2)
    g_mean_diff.SetLineWidth(ROOT.PlottingUtils.GetLineWidth())
    g_mean_diff.SetTitle("")
    g_mean_diff.GetXaxis().SetTitle("Time [ns]")
    g_mean_diff.GetYaxis().SetTitle("Amplitude [a.u.]")
    g_mean_diff.GetYaxis().SetTitleOffset(1)
    g_mean_diff.GetXaxis().SetRangeUser(0, x_values[-1])

    g_sigma_pool = ROOT.TGraph(n_points, x_values,
                               sigma_pool.astype(np.float64))
    g_sigma_pool.SetLineColor(ROOT.kBlue + 2)
    g_sigma_pool.SetLineWidth(ROOT.PlottingUtils.GetLineWidth())

    plot_max = max(float(mean_diff.max()), float(sigma_pool.max()))

    shap_mean = _load_xgb_shap_mean()
    g_shap = None
    if shap_mean is not None and len(shap_mean) == n_points:
        shap_scaled = shap_mean * (plot_max / float(shap_mean.max()))
        g_shap = ROOT.TGraph(n_points, x_values,
                             shap_scaled.astype(np.float64))
        g_shap.SetLineColor(ROOT.kBlack)
        g_shap.SetLineStyle(2)
        g_shap.SetLineWidth(ROOT.PlottingUtils.GetLineWidth())
    elif shap_mean is not None:
        print(f"  Skipping SHAP overlay: SHAP length {len(shap_mean)} != "
              f"waveform length {n_points}")
    else:
        print(f"  Skipping SHAP overlay: no cached SHAP values found in "
              f"{SWEEP_CACHE_DIR}")

    g_mean_diff.GetYaxis().SetRangeUser(0, plot_max * 1.15)
    g_mean_diff.Draw("AL")
    g_sigma_pool.Draw("L SAME")
    if g_shap is not None:
        g_shap.Draw("L SAME")

    leg = ROOT.PlottingUtils.AddLegend(0.6, 0.95, 0.65, 0.78)
    leg.AddEntry(g_mean_diff, "|#Delta#mu(t)|", "l")
    leg.AddEntry(g_sigma_pool, "#sigma_{pool}(t)", "l")
    if g_shap is not None:
        leg.AddEntry(g_shap, "XGBoost SHAP (rescaled)", "l")
    leg.Draw()

    ROOT.PlottingUtils.SaveFigure(canvas, output_name, "",
                                  ROOT.PlotSaveOptions.kLINEAR)
    canvas.Close()
    print(f"Saved {output_name}")


def _plot_two_curves(levels, aucs_test_only, aucs_matched, baseline_auc,
                     baseline_lpf_auc, x_title, output_name, x_log):
    canvas = ROOT.TCanvas(str(ROOT.PlottingUtils.GetRandomName()), "", 1200,
                          600)

    pad_plot = ROOT.TPad("pad_plot", "", 0.0, 0.0, 0.72, 1.0)
    pad_plot.SetRightMargin(0.02)
    if x_log:
        pad_plot.SetLogx(True)
    pad_plot.Draw()

    pad_leg = ROOT.TPad("pad_leg", "", 0.72, 0.0, 1.0, 1.0)
    pad_leg.SetLeftMargin(0.0)
    pad_leg.SetRightMargin(0.05)
    pad_leg.Draw()

    pad_plot.cd()

    n = len(levels)
    x_arr = np.array(levels, dtype=np.float64)

    g_test = ROOT.TGraph(n, x_arr, np.array(aucs_test_only, dtype=np.float64))
    g_test.SetLineColor(ROOT.kBlue + 2)
    g_test.SetLineWidth(ROOT.PlottingUtils.GetLineWidth())
    g_test.SetMarkerColor(ROOT.kBlue + 2)
    g_test.SetMarkerStyle(20)
    g_test.SetMarkerSize(1.2)

    g_match = ROOT.TGraph(n, x_arr, np.array(aucs_matched, dtype=np.float64))
    g_match.SetLineColor(ROOT.kRed + 2)
    g_match.SetLineWidth(ROOT.PlottingUtils.GetLineWidth())
    g_match.SetMarkerColor(ROOT.kRed + 2)
    g_match.SetMarkerStyle(21)
    g_match.SetMarkerSize(1.2)

    all_y = (list(aucs_test_only) + list(aucs_matched) +
             [baseline_auc, baseline_lpf_auc])
    y_min = min(all_y)
    y_max = max(all_y)
    pad = max(0.005, 0.2 * (y_max - y_min))

    g_test.SetTitle("")
    g_test.GetXaxis().SetTitle(x_title)
    g_test.GetYaxis().SetTitle("ROC AUC")
    g_test.GetYaxis().SetRangeUser(y_min - pad, y_max + pad)
    g_test.Draw("APL")
    g_match.Draw("PL SAME")

    base_line = ROOT.TLine(x_arr[0], baseline_auc, x_arr[-1], baseline_auc)
    base_line.SetLineColor(ROOT.kGray + 2)
    base_line.SetLineStyle(2)
    base_line.Draw()

    lpf_line = ROOT.TLine(x_arr[0], baseline_lpf_auc, x_arr[-1],
                          baseline_lpf_auc)
    lpf_line.SetLineColor(ROOT.kGreen + 2)
    lpf_line.SetLineStyle(7)
    lpf_line.SetLineWidth(ROOT.PlottingUtils.GetLineWidth())
    lpf_line.Draw()

    pad_leg.cd()
    leg = ROOT.PlottingUtils.AddLegend(0.0, 0.95, 0.35, 0.85)
    leg.SetMargin(0.15)
    leg.AddEntry(g_test, "#splitline{Trained unmodified}{/ tested noisy}",
                 "lp")
    leg.AddEntry(g_match, "#splitline{Trained noisy}{/ tested noisy}", "lp")
    leg.AddEntry(base_line, "#splitline{Unmodified}{baseline}", "l")
    leg.AddEntry(lpf_line, "#splitline{Low-pass filter}{baseline}", "l")
    leg.Draw()

    ROOT.PlottingUtils.SaveFigure(canvas, output_name, "",
                                  ROOT.PlotSaveOptions.kLINEAR)
    canvas.Close()
    print(f"Saved {output_name}")


def _save_latex_table(results, output_path):
    lines = []
    lines.append(r"\begin{table}[htbp]")
    lines.append(r"  \centering")
    lines.append(
        r"  \caption{XGBoost test ROC AUC under added noise. White "
        r"noise $\sigma$ is in ADC counts (= LSB). Shot noise "
        r"$\sigma$ per sample $= m\sqrt{|\mathrm{signal}_{\mathrm{ADC}}|}$. "
        r"`Test-only' uses the production model trained on the "
        r"unmodified waveforms. `Matched' retrains XGBoost on "
        r"training data with the same added noise.}")
    lines.append(r"  \label{tab:noise}")
    lines.append(r"  \begin{tabular}{lccc}")
    lines.append(r"    \toprule")
    lines.append(r"    Noise & Level & Test-only AUC & Matched AUC \\")
    lines.append(r"    \midrule")
    lines.append(f"    Unmodified & --- & {results['baseline_auc']:.4f} & "
                 f"{results['baseline_auc']:.4f} \\\\")
    lines.append(f"    Unmodified, LPF(order={LPF_ORDER}, "
                 f"cutoff={LPF_CUTOFF:g}) & --- & "
                 f"{results['baseline_lpf_auc']:.4f} & "
                 f"{results['baseline_lpf_auc']:.4f} \\\\")
    lines.append(r"    \midrule")
    for s, a1, a2 in zip(results["white_sigmas"],
                         results["white_aucs_test_only"],
                         results["white_aucs_matched"]):
        lines.append(f"    White, $\\sigma={s}$ LSB & {s} & {a1:.4f} & "
                     f"{a2:.4f} \\\\")
    lines.append(r"    \midrule")
    for m, a1, a2 in zip(results["shot_multipliers"],
                         results["shot_aucs_test_only"],
                         results["shot_aucs_matched"]):
        lines.append(f"    Shot, $m={m}$ & {m} & {a1:.4f} & {a2:.4f} \\\\")
    lines.append(r"    \bottomrule")
    lines.append(r"  \end{tabular}")
    lines.append(r"\end{table}")
    table = "\n".join(lines)
    with open(output_path, "w") as fh:
        fh.write(table + "\n")
    print(f"LaTeX table written to {output_path}")
    print(table)


def main():
    os.makedirs(CACHE_DIR, exist_ok=True)
    os.makedirs("plots", exist_ok=True)
    results_cache = os.path.join(CACHE_DIR, f"results_{LO_TAG}.pkl")

    # Raw split is always loaded so sample-waveform plots can run regardless
    # of whether AUC results are cached. Loading from raw_split.npz is fast.
    a_train, g_train, a_test, g_test = _load_or_build_split()
    print(f"Loaded (LO {LO_LOWER['alpha']}-{LO_UPPER['alpha']}): "
          f"{len(a_train)} alpha + {len(g_train)} gamma train  |  "
          f"{len(a_test)} alpha + {len(g_test)} gamma test")

    a_train = _balanced_subsample(a_train, N_TRAIN_PER_CLASS, SUBSAMPLE_SEED)
    g_train = _balanced_subsample(g_train, N_TRAIN_PER_CLASS,
                                  SUBSAMPLE_SEED + 1)
    a_test = _balanced_subsample(a_test, N_TEST_PER_CLASS, SUBSAMPLE_SEED + 2)
    g_test = _balanced_subsample(g_test, N_TEST_PER_CLASS, SUBSAMPLE_SEED + 3)
    print(f"Balanced: {len(a_train)} alpha + {len(g_train)} gamma train  |  "
          f"{len(a_test)} alpha + {len(g_test)} gamma test")

    if os.path.exists(results_cache):
        print(f"Loading cached results from {results_cache}")
        with open(results_cache, "rb") as fh:
            results = pickle.load(fh)
    else:
        if not os.path.exists(MODEL_PATH):
            raise FileNotFoundError(
                f"{MODEL_PATH} not found. Run analysis.py first to train "
                f"and cache the XGBoost regressor.")
        with open(MODEL_PATH, "rb") as fh:
            clean_model = pickle.load(fh)

        baseline_auc = _score_clean_trained(clean_model, a_test, g_test)
        print(f"Baseline (unmodified, clean-trained) AUC = {baseline_auc:.4f}")

        print(f"Baseline LPF (Butterworth order={LPF_ORDER}, "
              f"cutoff={LPF_CUTOFF:g}): training fresh XGBoost...")
        baseline_lpf_auc = _train_and_score(
            _apply_lowpass(a_train),
            _apply_lowpass(g_train),
            _apply_lowpass(a_test),
            _apply_lowpass(g_test),
        )
        print(f"Baseline LPF AUC = {baseline_lpf_auc:.4f}")

        print("White noise sweeps (sigma in ADC counts / LSB):")
        white_test_only = _sweep_test_only(clean_model, a_test, g_test,
                                           WHITE_SIGMAS_ADC, _add_white_noise,
                                           "sigma_LSB")
        white_matched = _sweep_matched(a_train, g_train, a_test, g_test,
                                       WHITE_SIGMAS_ADC, _add_white_noise,
                                       "sigma_LSB")

        print("Shot noise sweeps (sigma = m * sqrt(|signal|)):")
        shot_test_only = _sweep_test_only(clean_model, a_test, g_test,
                                          SHOT_MULTIPLIERS, _add_shot_noise,
                                          "multiplier")
        shot_matched = _sweep_matched(a_train, g_train, a_test, g_test,
                                      SHOT_MULTIPLIERS, _add_shot_noise,
                                      "multiplier")

        results = {
            "baseline_auc": baseline_auc,
            "baseline_lpf_auc": baseline_lpf_auc,
            "white_sigmas": WHITE_SIGMAS_ADC,
            "white_aucs_test_only": white_test_only,
            "white_aucs_matched": white_matched,
            "shot_multipliers": SHOT_MULTIPLIERS,
            "shot_aucs_test_only": shot_test_only,
            "shot_aucs_matched": shot_matched,
        }
        with open(results_cache, "wb") as fh:
            pickle.dump(results, fh)
        print(f"Results cached to {results_cache}")

    _plot_two_curves(results["white_sigmas"],
                     results["white_aucs_test_only"],
                     results["white_aucs_matched"],
                     results["baseline_auc"],
                     results["baseline_lpf_auc"],
                     "White noise #sigma [ADC counts = LSB]",
                     "noise_white_auc",
                     x_log=True)

    _plot_two_curves(
        results["shot_multipliers"],
        results["shot_aucs_test_only"],
        results["shot_aucs_matched"],
        results["baseline_auc"],
        results["baseline_lpf_auc"],
        "Shot noise multiplier #it{m} (#sigma = #it{m}#sqrt{|s|})",
        "noise_shot_auc",
        x_log=False)

    print("Plotting sample waveforms at each white-noise level...")
    _plot_noise_sample_waveforms(a_test, g_test, results["white_sigmas"],
                                 _add_white_noise, "sigma", "LSB",
                                 "noise_waveforms_white")

    print("Plotting sample waveforms at each shot-noise level...")
    _plot_noise_sample_waveforms(a_test, g_test, results["shot_multipliers"],
                                 _add_shot_noise, "m", "",
                                 "noise_waveforms_shot")

    print("Plotting baseline LPF waveform comparison...")
    _plot_baseline_lpf_waveforms(a_test, g_test,
                                 "noise_waveforms_baseline_lpf")

    print("Plotting LO spectra at each white-noise level...")
    _plot_noise_light_output(a_test, g_test, results["white_sigmas"],
                             _add_white_noise, "sigma", "LSB",
                             "noise_lo_white")

    print("Plotting LO spectra at each shot-noise level...")
    _plot_noise_light_output(a_test, g_test, results["shot_multipliers"],
                             _add_shot_noise, "m", "", "noise_lo_shot")

    print("Plotting per-sample mean +/- sigma waveform band per class...")
    _plot_average_waveform_band(a_test, g_test, "average_waveform_band")

    print("Plotting per-sample variance vs mean (shot-noise diagnostic)...")
    _plot_variance_vs_mean(a_test, g_test, "variance_vs_mean")

    _save_latex_table(results, os.path.join(CACHE_DIR, "noise_table.txt"))


if __name__ == "__main__":
    main()
