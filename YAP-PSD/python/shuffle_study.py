"""Shuffle study: train CNN on input-permuted waveforms.

Tests whether the CNN's convolutional inductive bias actually exploits
adjacency in the time samples. If shuffled-input AUC matches raw-input AUC
across multiple fixed permutations (with same permutation applied to train
and test), the conv layers aren't extracting extra signal from local
correlations -- which closes the rebuttal that trees/MLP miss performance
because they're order-blind.
"""
import os
import pickle
import numpy as np
import ROOT
from sklearn.metrics import roc_auc_score

from analysis_utilities.io import load_tree_data
from psd_utils import ROOT_FILES_DIR
from torch_models import TorchCNNRegressor
import analysis_utilities

analysis_utilities.load_cpp_library()
ROOT.gROOT.SetBatch(True)
ROOT.PlottingUtils.SetStylePreferences(ROOT.PlotSaveFormat.kPNG)

CACHE_DIR = "shuffle_study_cache"
DATA_CACHE = os.path.join("sweep_cache", "prepared_data.npz")

N_PERMUTATIONS = 5
N_SEEDS_PER_PERM = 2
BASELINE_SEEDS = [42, 123, 256]
PERM_BASE_SEED = 1000
TRAIN_PER_CLASS = 10000


def _load_or_prepare_data():
    if os.path.exists(DATA_CACHE):
        print(f"Loading prepared data from {DATA_CACHE}")
        d = np.load(DATA_CACHE)
        return d["x_train"], d["y_train"], d["x_test"], d["y_test"]
    print(f"{DATA_CACHE} not found, preparing from scratch")
    from parameter_study import _prepare_data
    print("Loading alpha data (Am-241)...")
    alpha_features, alpha_waveforms = load_tree_data(ROOT_FILES_DIR +
                                                     "Am241.root",
                                                     array_branch="Samples")
    print("Loading gamma data (Na-22)...")
    gamma_features, gamma_waveforms = load_tree_data(ROOT_FILES_DIR +
                                                     "Na22.root",
                                                     array_branch="Samples")
    return _prepare_data(alpha_waveforms,
                         gamma_waveforms,
                         alpha_features,
                         gamma_features,
                         n_train_per_class=TRAIN_PER_CLASS)


def _subsample_balanced(x, y, n_per_class, seed=42):
    rng = np.random.RandomState(seed)
    idx0 = np.where(y == 0)[0]
    idx1 = np.where(y == 1)[0]
    take0 = rng.choice(idx0, size=min(n_per_class, len(idx0)), replace=False)
    take1 = rng.choice(idx1, size=min(n_per_class, len(idx1)), replace=False)
    keep = np.concatenate([take0, take1])
    return x[keep], y[keep]


def _make_cnn(seed):
    return TorchCNNRegressor(conv_channels=(16, 32, 64),
                             kernel_size=5,
                             fc_sizes=(64, ),
                             dropout=0.2,
                             max_iter=50,
                             random_state=seed,
                             verbose=False)


def _train_and_score(x_train, y_train, x_test, y_test, seed):
    model = _make_cnn(seed)
    model.fit(x_train, y_train)
    return float(roc_auc_score(y_test, model.predict(x_test)))


def _summarize(label, aucs):
    a = np.array(aucs)
    return (f"  {label:>18}  mean={a.mean():.4f}  "
            f"std={a.std():.4f}  n={len(a)}")


def _plot_results(results, output_name):
    n_perms = len(results["shuffled_groups"])
    n_points = 1 + n_perms

    baseline = np.array(results["baseline"])
    b_mean = float(baseline.mean())
    b_std = float(baseline.std())

    means = [b_mean] + [float(np.mean(g)) for g in results["shuffled_groups"]]
    stds = [b_std] + [float(np.std(g)) for g in results["shuffled_groups"]]

    y_lo = min(means[i] - stds[i] for i in range(n_points))
    y_hi = max(means[i] + stds[i] for i in range(n_points))
    pad = max(0.005, 0.25 * (y_hi - y_lo))

    canvas = ROOT.PlottingUtils.GetConfiguredCanvas()

    frame = ROOT.TH1F(str(ROOT.PlottingUtils.GetRandomName()), "", n_points,
                      -0.5, n_points - 0.5)
    frame.SetStats(0)
    frame.GetXaxis().SetBinLabel(1, "Raw")
    for i in range(n_perms):
        frame.GetXaxis().SetBinLabel(i + 2, f"Perm {i + 1}")
    frame.GetXaxis().SetLabelSize(0.05)
    frame.GetYaxis().SetTitle("ROC AUC")
    frame.GetYaxis().SetRangeUser(y_lo - pad, y_hi + pad)
    frame.Draw()

    band = ROOT.TBox(-0.5, b_mean - b_std, n_points - 0.5, b_mean + b_std)
    band.SetFillColorAlpha(ROOT.kGray, 0.4)
    band.SetLineWidth(0)
    band.Draw("SAME")

    line = ROOT.TLine(-0.5, b_mean, n_points - 0.5, b_mean)
    line.SetLineColor(ROOT.kGray + 2)
    line.SetLineStyle(2)
    line.Draw()

    x_arr = np.arange(n_points, dtype=np.float64)
    y_arr = np.array(means, dtype=np.float64)
    ex_arr = np.zeros(n_points, dtype=np.float64)
    ey_arr = np.array(stds, dtype=np.float64)

    graph = ROOT.TGraphErrors(n_points, x_arr, y_arr, ex_arr, ey_arr)
    graph.SetMarkerStyle(20)
    graph.SetMarkerSize(1.4)
    graph.SetLineWidth(ROOT.PlottingUtils.GetLineWidth())
    graph.Draw("PE SAME")

    ROOT.PlottingUtils.SaveFigure(canvas, output_name, "",
                                  ROOT.PlotSaveOptions.kLINEAR)
    canvas.Close()
    print(f"Saved {output_name}")


def _save_latex_table(results, output_path):
    baseline = np.array(results["baseline"])
    pooled = np.concatenate(
        [np.array(g) for g in results["shuffled_groups"]])

    lines = []
    lines.append(r"\begin{table}[htbp]")
    lines.append(r"  \centering")
    lines.append(r"  \caption{CNN test ROC AUC on raw vs. permuted "
                 r"waveform inputs. Each permutation is a fixed shuffle "
                 r"of the sample axis applied identically to train and "
                 r"test data. The `Raw' row gives the seed-only variance.}")
    lines.append(r"  \label{tab:shuffle}")
    lines.append(r"  \begin{tabular}{lccc}")
    lines.append(r"    \toprule")
    lines.append(r"    Input & Mean AUC & Std & Seeds \\")
    lines.append(r"    \midrule")
    lines.append(f"    Raw & {baseline.mean():.4f} & {baseline.std():.4f} & "
                 f"{len(baseline)} \\\\")
    for i, g in enumerate(results["shuffled_groups"]):
        arr = np.array(g)
        lines.append(f"    Perm {i + 1} & {arr.mean():.4f} & "
                     f"{arr.std():.4f} & {len(arr)} \\\\")
    lines.append(r"    \midrule")
    lines.append(f"    Shuffled (pooled) & {pooled.mean():.4f} & "
                 f"{pooled.std():.4f} & {len(pooled)} \\\\")
    lines.append(r"    \bottomrule")
    lines.append(r"  \end{tabular}")
    lines.append(r"\end{table}")
    table = "\n".join(lines)
    with open(output_path, "w") as f:
        f.write(table + "\n")
    print(f"LaTeX table written to {output_path}")
    print(table)


def main():
    os.makedirs(CACHE_DIR, exist_ok=True)
    os.makedirs("plots", exist_ok=True)
    results_cache = os.path.join(CACHE_DIR, "results.pkl")

    if os.path.exists(results_cache):
        print(f"Loading cached results from {results_cache}")
        with open(results_cache, "rb") as fh:
            results = pickle.load(fh)
    else:
        x_train, y_train, x_test, y_test = _load_or_prepare_data()
        if (y_train == 0).sum() > TRAIN_PER_CLASS:
            x_train, y_train = _subsample_balanced(x_train, y_train,
                                                   TRAIN_PER_CLASS)
        n_features = x_train.shape[1]
        print(f"x_train: {x_train.shape}  x_test: {x_test.shape}  "
              f"n_features: {n_features}")

        results = {"baseline": [], "shuffled_groups": []}

        print("Baseline (raw waveforms, varying seed):")
        for seed in BASELINE_SEEDS:
            auc = _train_and_score(x_train, y_train, x_test, y_test, seed)
            print(f"  seed={seed}  AUC = {auc:.4f}")
            results["baseline"].append(auc)

        print("Shuffled inputs:")
        for p in range(N_PERMUTATIONS):
            perm_rng = np.random.RandomState(PERM_BASE_SEED + p)
            perm = perm_rng.permutation(n_features)
            xs_train = x_train[:, perm]
            xs_test = x_test[:, perm]
            group = []
            for s in range(N_SEEDS_PER_PERM):
                seed = BASELINE_SEEDS[s]
                auc = _train_and_score(xs_train, y_train, xs_test, y_test,
                                       seed)
                print(f"  perm={p + 1} seed={seed}  AUC = {auc:.4f}")
                group.append(auc)
            results["shuffled_groups"].append(group)

        with open(results_cache, "wb") as fh:
            pickle.dump(results, fh)
        print(f"Results cached to {results_cache}")

    print()
    print(_summarize("Raw", results["baseline"]))
    for i, g in enumerate(results["shuffled_groups"]):
        print(_summarize(f"Perm {i + 1}", g))
    pooled = [a for g in results["shuffled_groups"] for a in g]
    print(_summarize("Shuffled pooled", pooled))

    _plot_results(results, "shuffle_study_auc")
    _save_latex_table(results, os.path.join(CACHE_DIR, "shuffle_table.txt"))


if __name__ == "__main__":
    main()
