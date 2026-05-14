#include "Constants.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include "RooFitUtils.hpp"
#include <TF1.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>
#include <cmath>
#include <iomanip>
#include <vector>

const Float_t E_AM241 = 59.5409;
const Float_t E_BA133_53 = 53.16;
const Float_t E_BA133_79 = 79.6142;
const Float_t E_BA133_81 = 80.9979;
const Float_t E_PB_KA1 = 72.8042;
const Float_t E_PB_KA2 = 74.9694;
const Float_t E_GE_73M = 68.752;
const Float_t E_BKG_73KEV = 73.0;

const Float_t CAL_RANGE_LOW = -10;
const Float_t CAL_RANGE_HIGH = 110;

const TString OUT_SUBDIR = "calibrated_low";

struct CalibrationData {
  std::vector<Float_t> mu, mu_errors, calibration_values_keV, reduced_chi2;
  std::vector<TString> run_names;
};

struct BkgGeSimResult {
  FitResult bkg_channel;
  FitResult sig_channel;
  Bool_t valid;
};

struct DayResult {
  std::vector<TString> cal_labels;
  std::vector<TF1 *> cal_funcs;
  std::vector<TString> ge_labels;
  std::vector<Float_t> ge_mus, ge_errs, ge_chi2s;
};

struct PairCalResult {
  TF1 *cal_func = nullptr;
  BkgGeSimResult postcal;
};

std::vector<Double_t> LoadEventsFromFiltered(const TString &input_name) {
  std::vector<Double_t> events;
  TFile *file = IO::OpenForReading("filtered/" + input_name + ".root");
  if (!file || file->IsZombie()) {
    std::cerr << "ERROR: Cannot open filtered/" << input_name << ".root"
              << std::endl;
    return events;
  }
  for (Int_t c = 0; c < Constants::N_CRYSTALS; c++) {
    TTree *tree =
        static_cast<TTree *>(file->Get(Form("crystal%d_filtered_tree", c)));
    if (!tree)
      continue;
    std::vector<Double_t> ch =
        RooFitUtils::LoadEventsFromTree(tree, "energykeV");
    events.insert(events.end(), ch.begin(), ch.end());
  }
  file->Close();
  delete file;
  return events;
}

std::vector<Double_t> LoadEventsFromCalibratedLow(const TString &input_name) {
  std::vector<Double_t> events;
  TFile *file = IO::OpenForReading(OUT_SUBDIR + "/" + input_name + ".root");
  if (!file || file->IsZombie()) {
    std::cerr << "ERROR: Cannot open " << OUT_SUBDIR << "/" << input_name
              << ".root" << std::endl;
    return events;
  }
  TTree *tree = static_cast<TTree *>(file->Get("calibrated_tree"));
  if (!tree) {
    std::cerr << "ERROR: calibrated_tree not found in " << OUT_SUBDIR << "/"
              << input_name << ".root" << std::endl;
    file->Close();
    delete file;
    return events;
  }
  events = RooFitUtils::LoadEventsFromTree(tree, "depositedEnergykeV");
  file->Close();
  delete file;
  return events;
}

FitResult FitCalPeak(const std::vector<Double_t> &events,
                     const TString &input_name, const TString &peak_name,
                     Float_t fit_low, Float_t fit_high, Bool_t double_peak,
                     Double_t mu1, Double_t mu2, Bool_t interactive) {
  if (events.empty())
    return {};

  RooFitUtils fitter(events, fit_low, fit_high, Constants::BIN_WIDTH_KEV, kTRUE,
                     kFALSE, kTRUE, kTRUE, kTRUE);
  if (interactive)
    fitter.SetInteractive();
  if (double_peak)
    return fitter.FitDoublePeak(input_name, peak_name, mu1, mu2);
  return fitter.FitSinglePeak(input_name, peak_name);
}

BkgGeSimResult RunBkgGeSim(const std::vector<Double_t> &bkg_events,
                           const std::vector<Double_t> &sig_events,
                           Float_t bkg_lo, Float_t bkg_hi, Float_t sig_lo,
                           Float_t sig_hi,
                           const std::vector<Double_t> &bkg_peak_mus,
                           Double_t ge_mu_init, Bool_t bkg_flat,
                           Bool_t sig_flat, const TString &bkg_label,
                           const TString &sig_label, const TString &fit_label,
                           Bool_t interactive) {
  BkgGeSimResult out;
  out.valid = kFALSE;

  Int_t n_bkg_peaks = (Int_t)bkg_peak_mus.size();
  if (n_bkg_peaks < 1 || n_bkg_peaks > 2) {
    std::cerr << "ERROR: bkg_peak_mus must have 1 or 2 entries" << std::endl;
    return out;
  }
  if (bkg_events.empty() || sig_events.empty())
    return out;

  Bool_t use_step = kFALSE;
  Bool_t use_low_exp = kTRUE;
  Bool_t use_low_lin = kTRUE;
  Bool_t use_high_exp = kTRUE;

  RooFitUtils bkg_fitter(bkg_events, bkg_lo, bkg_hi, Constants::BIN_WIDTH_KEV,
                         bkg_flat, use_step, use_low_exp, use_low_lin,
                         use_high_exp);
  if (interactive)
    bkg_fitter.SetInteractive();

  FitResult seed;
  TString seed_label = fit_label + "_BkgSeed";
  if (n_bkg_peaks == 1)
    seed = bkg_fitter.FitSinglePeak(bkg_label, seed_label);
  else
    seed = bkg_fitter.FitDoublePeak(bkg_label, seed_label, bkg_peak_mus[0],
                                    bkg_peak_mus[1]);
  if (!seed.valid) {
    std::cerr << "ERROR: bkg-only seed fit failed for " << bkg_label
              << std::endl;
    return out;
  }

  RooFitUtils sim;
  if (interactive)
    sim.SetInteractive();
  std::vector<Double_t> sig_mus = bkg_peak_mus;
  sig_mus.push_back(ge_mu_init);

  sim.AddChannel("bkg", bkg_events, bkg_lo, bkg_hi, Constants::BIN_WIDTH_KEV,
                 n_bkg_peaks, bkg_peak_mus, bkg_flat, use_step, use_low_exp,
                 use_low_lin, use_high_exp);
  sim.AddChannel("sig", sig_events, sig_lo, sig_hi, Constants::BIN_WIDTH_KEV,
                 n_bkg_peaks + 1, sig_mus, sig_flat, use_step, use_low_exp,
                 use_low_lin, use_high_exp);
  for (Int_t i = 0; i < n_bkg_peaks; i++)
    sim.LinkPeakShape("sig", i, "bkg", i);
  sim.SeedChannel("bkg", seed);

  std::vector<FitResult> results = sim.FitSimultaneous(sig_label, fit_label);
  if (results.size() < 2)
    return out;

  out.bkg_channel = results[0];
  out.sig_channel = results[1];
  out.valid = results[1].valid;
  return out;
}

void AddZeroPoint(CalibrationData &cal_data) {
  cal_data.run_names.push_back("Zero Point");
  cal_data.mu.push_back(0);
  cal_data.mu_errors.push_back(0);
  cal_data.calibration_values_keV.push_back(0);
  cal_data.reduced_chi2.push_back(0);
}

void AddCalPoint(CalibrationData &cal_data, const TString &name, Float_t mu,
                 Float_t mu_err, Float_t true_e, Float_t chi2) {
  cal_data.run_names.push_back(name);
  cal_data.mu.push_back(mu);
  cal_data.mu_errors.push_back(mu_err);
  cal_data.calibration_values_keV.push_back(true_e);
  cal_data.reduced_chi2.push_back(chi2);
}

void PrintCalSummary(const CalibrationData &cal_data,
                     const TString &date_label) {
  std::cout << "Calibration points for " << date_label << std::endl;
  for (size_t i = 0; i < cal_data.mu.size(); i++) {
    std::cout << std::left << std::setw(45) << cal_data.run_names[i] << ": "
              << std::fixed << std::setprecision(4) << cal_data.mu[i] << " +/- "
              << cal_data.mu_errors[i] << " keV";
    if (cal_data.reduced_chi2[i] > 0)
      std::cout << " (chi2/ndf = " << std::setprecision(3)
                << cal_data.reduced_chi2[i] << ")";
    std::cout << std::endl;
  }
}

TF1 *CreateAndSavePol1Cal(const CalibrationData &cal_data,
                          const TString &date_label) {
  Int_t n = (Int_t)cal_data.mu.size();
  TGraph *graph =
      new TGraph(n, cal_data.mu.data(), cal_data.calibration_values_keV.data());
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureGraph(
      graph, kBlue, "; Precalibrated Energy [keV]; Deposited Energy [keV]");
  graph->GetXaxis()->SetRangeUser(-5, CAL_RANGE_HIGH);
  graph->GetYaxis()->SetRangeUser(-5, CAL_RANGE_HIGH);
  graph->GetXaxis()->SetNdivisions(506);
  graph->SetMarkerStyle(5);
  graph->SetMarkerSize(2);
  graph->Draw("AP");

  TF1 *cal =
      new TF1("cal_low_" + date_label, "pol1", CAL_RANGE_LOW, CAL_RANGE_HIGH);
  cal->SetParameter(0, 0);
  cal->SetParameter(1, 1);
  cal->SetNpx(1000);
  graph->Fit(cal, "LRE");
  cal->Draw("SAME");

  PlottingUtils::SaveFigure(canvas, "calibration_low_" + date_label, "",
                            PlotSaveOptions::kLINEAR);
  return cal;
}

void ApplyPol1Cal(const std::vector<TString> &input_names, TF1 *cal,
                  const TString &date_label) {
  TFile *cal_file = IO::OpenForWriting(
      OUT_SUBDIR + "/calibration_function_low_" + date_label + ".root");
  cal->Write("calibration", TObject::kOverwrite);
  cal_file->Close();
  delete cal_file;

  for (size_t i = 0; i < input_names.size(); i++) {
    TString input_name = input_names[i];
    TFile *in_file = IO::OpenForReading("filtered/" + input_name + ".root");
    if (!in_file || in_file->IsZombie()) {
      std::cerr << "ERROR: Cannot open filtered/" << input_name << ".root"
                << std::endl;
      delete in_file;
      continue;
    }
    TFile *out_file =
        IO::OpenForWriting(OUT_SUBDIR + "/" + input_name + ".root");

    Float_t deposited_energy = 0;
    TTree *cal_tree = new TTree("calibrated_tree", "Calibrated events");
    cal_tree->SetDirectory(out_file);
    cal_tree->Branch("depositedEnergykeV", &deposited_energy,
                     "depositedEnergykeV/F");

    for (Int_t c = 0; c < Constants::N_CRYSTALS; c++) {
      TTree *tree = static_cast<TTree *>(
          in_file->Get(Form("crystal%d_filtered_tree", c)));
      if (!tree)
        continue;
      Float_t energy = 0;
      tree->SetBranchAddress("energykeV", &energy);
      Int_t n_entries = tree->GetEntries();
      for (Int_t j = 0; j < n_entries; j++) {
        tree->GetEntry(j);
        deposited_energy = cal->Eval(energy);
        cal_tree->Fill();
      }
    }

    out_file->cd();
    cal_tree->Write("calibrated_tree", TObject::kOverwrite);
    std::cout << "Calibrated " << input_name << " -> " << OUT_SUBDIR << "/"
              << input_name << ".root" << std::endl;
    out_file->Close();
    delete out_file;
    in_file->Close();
    delete in_file;
  }
}

void AddGeResult(DayResult &result, const BkgGeSimResult &r,
                 const TString &label) {
  if (!r.valid || r.sig_channel.peaks.empty())
    return;
  const PeakFitResult &ge = r.sig_channel.peaks.back();
  result.ge_labels.push_back(label);
  result.ge_mus.push_back(ge.mu);
  result.ge_errs.push_back(ge.mu_error);
  result.ge_chi2s.push_back(r.sig_channel.reduced_chi2);
}

PairCalResult ProcessPbAnchoredPair(
    const TString &bkg_run, const TString &sig_run, Float_t bkg_lo,
    Float_t bkg_hi, Float_t sig_lo, Float_t sig_hi, Bool_t bkg_flat,
    const TString &date_label, const TString &pair_tag, Bool_t interactive) {
  PairCalResult out;
  std::vector<Double_t> bkg_events = LoadEventsFromFiltered(bkg_run);
  std::vector<Double_t> sig_events = LoadEventsFromFiltered(sig_run);
  std::vector<Double_t> pb_mus = {(Double_t)E_PB_KA1, (Double_t)E_PB_KA2};

  TString cal_label = date_label + "_" + pair_tag;

  BkgGeSimResult pre = RunBkgGeSim(
      bkg_events, sig_events, bkg_lo, bkg_hi, sig_lo, sig_hi, pb_mus, E_GE_73M,
      bkg_flat, kTRUE, bkg_run, sig_run, "PreCal_" + pair_tag, interactive);
  if (!pre.valid)
    return out;

  CalibrationData cal_data;
  AddZeroPoint(cal_data);
  AddCalPoint(cal_data, pair_tag + " Pb-Ka1", pre.bkg_channel.peaks.at(0).mu,
              pre.bkg_channel.peaks.at(0).mu_error, E_PB_KA1,
              pre.bkg_channel.reduced_chi2);
  AddCalPoint(cal_data, pair_tag + " Pb-Ka2", pre.bkg_channel.peaks.at(1).mu,
              pre.bkg_channel.peaks.at(1).mu_error, E_PB_KA2, -1);

  PrintCalSummary(cal_data, cal_label);
  TF1 *cal = CreateAndSavePol1Cal(cal_data, cal_label);
  out.cal_func = cal;

  std::vector<TString> pair_runs = {bkg_run, sig_run};
  ApplyPol1Cal(pair_runs, cal, cal_label);

  std::vector<Double_t> bc = LoadEventsFromCalibratedLow(bkg_run);
  std::vector<Double_t> sc = LoadEventsFromCalibratedLow(sig_run);
  out.postcal = RunBkgGeSim(
      bc, sc, bkg_lo, bkg_hi, sig_lo, sig_hi, pb_mus, E_GE_73M, bkg_flat, kTRUE,
      bkg_run + "_postcal_" + pair_tag, sig_run + "_postcal_" + pair_tag,
      "PostCal_" + pair_tag, interactive);
  return out;
}

DayResult ProcessDay_20260113(Bool_t interactive) {
  DayResult result;

  PairCalResult cu = ProcessPbAnchoredPair(
      Constants::CUSHIELDBACKGROUND_10PERCENT_20260113,
      Constants::CUSHIELDSIGNAL_10PERCENT_20260113, 65, 82, 62, 80, kTRUE,
      "20260113", "CuShield10", interactive);
  if (cu.cal_func) {
    result.cal_labels.push_back("20260113 CuShield10");
    result.cal_funcs.push_back(cu.cal_func);
  }
  AddGeResult(result, cu.postcal, "Cu Shield Signal 10% (01/13)");

  PairCalResult cd10 = ProcessPbAnchoredPair(
      Constants::CDSHIELDBACKGROUND_10PERCENT_20260113,
      Constants::CDSHIELDSIGNAL_10PERCENT_20260113, 65, 81, 64, 80, kTRUE,
      "20260113", "CdShield10", interactive);
  if (cd10.cal_func) {
    result.cal_labels.push_back("20260113 CdShield10");
    result.cal_funcs.push_back(cd10.cal_func);
  }
  AddGeResult(result, cd10.postcal, "Cd Shield Signal 10% (01/13)");

  PairCalResult cd25 = ProcessPbAnchoredPair(
      Constants::CDSHIELDBACKGROUND_25PERCENT_20260113,
      Constants::CDSHIELDSIGNAL_25PERCENT_20260113, 66, 81, 65, 81, kTRUE,
      "20260113", "CdShield25", interactive);
  if (cd25.cal_func) {
    result.cal_labels.push_back("20260113 CdShield25");
    result.cal_funcs.push_back(cd25.cal_func);
  }
  AddGeResult(result, cd25.postcal, "Cd Shield Signal 25% (01/13)");

  return result;
}

DayResult ProcessDay_20260114(Bool_t interactive) {
  DayResult result;

  PairCalResult cu = ProcessPbAnchoredPair(
      Constants::CUSHIELDBACKGROUND_10PERCENT_20260114,
      Constants::CUSHIELDSIGNAL_10PERCENT_20260114, 66, 82, 63, 80, kTRUE,
      "20260114", "CuShield10", interactive);
  if (cu.cal_func) {
    result.cal_labels.push_back("20260114 CuShield10");
    result.cal_funcs.push_back(cu.cal_func);
  }
  AddGeResult(result, cu.postcal, "Cu Shield Signal 10% (01/14)");

  return result;
}

DayResult ProcessDay_20260115(Bool_t interactive) {
  CalibrationData cal_data;
  AddZeroPoint(cal_data);

  std::vector<Double_t> am_events =
      LoadEventsFromFiltered(Constants::POSTREACTOR_AM241_20260115);
  FitResult am = FitCalPeak(am_events, Constants::POSTREACTOR_AM241_20260115,
                            "Am_59.5keV", 51, 71, kFALSE, 0, 0, interactive);
  if (am.valid)
    AddCalPoint(cal_data, "Am-241 59.5", am.peaks.at(0).mu,
                am.peaks.at(0).mu_error, E_AM241, am.reduced_chi2);

  std::vector<Double_t> ba_events =
      LoadEventsFromFiltered(Constants::POSTREACTOR_BA133_20260115);
  FitResult ba53 = FitCalPeak(ba_events, Constants::POSTREACTOR_BA133_20260115,
                              "Ba_53.16keV", 48, 58, kFALSE, 0, 0, interactive);
  if (ba53.valid)
    AddCalPoint(cal_data, "Ba-133 53.16", ba53.peaks.at(0).mu,
                ba53.peaks.at(0).mu_error, E_BA133_53, ba53.reduced_chi2);

  FitResult ba81 = FitCalPeak(ba_events, Constants::POSTREACTOR_BA133_20260115,
                              "Ba_80.98keV", 75, 90, kTRUE, E_BA133_79,
                              E_BA133_81, interactive);
  if (ba81.valid) {
    AddCalPoint(cal_data, "Ba-133 79.61", ba81.peaks.at(0).mu,
                ba81.peaks.at(0).mu_error, E_BA133_79, ba81.reduced_chi2);
    AddCalPoint(cal_data, "Ba-133 80.98", ba81.peaks.at(1).mu,
                ba81.peaks.at(1).mu_error, E_BA133_81, -1);
  }

  PrintCalSummary(cal_data, "20260115");
  TF1 *cal = CreateAndSavePol1Cal(cal_data, "20260115");

  std::vector<TString> day_datasets = {
      Constants::NOSHIELDBACKGROUND_5PERCENT_20260115,
      Constants::NOSHIELDSIGNAL_5PERCENT_20260115,
      Constants::POSTREACTOR_AM241_20260115,
      Constants::POSTREACTOR_BA133_20260115, Constants::SHUTTERCLOSED_20260115};
  ApplyPol1Cal(day_datasets, cal, "20260115");

  DayResult result;
  result.cal_labels.push_back("20260115");
  result.cal_funcs.push_back(cal);

  std::vector<Double_t> one_bkg_mus = {(Double_t)E_BKG_73KEV};
  std::vector<Double_t> b = LoadEventsFromCalibratedLow(
      Constants::NOSHIELDBACKGROUND_5PERCENT_20260115);
  std::vector<Double_t> s =
      LoadEventsFromCalibratedLow(Constants::NOSHIELDSIGNAL_5PERCENT_20260115);
  BkgGeSimResult post = RunBkgGeSim(
      b, s, 67, 77, 64, 77, one_bkg_mus, E_GE_73M, kTRUE, kTRUE,
      TString(Constants::NOSHIELDBACKGROUND_5PERCENT_20260115) + "_postcal",
      TString(Constants::NOSHIELDSIGNAL_5PERCENT_20260115) + "_postcal",
      "PostCal_BkgGeSim", interactive);
  AddGeResult(result, post, "No Shield Signal 5% (01/15)");

  return result;
}

DayResult ProcessDay_20260116(Bool_t interactive) {
  CalibrationData cal_data;
  AddZeroPoint(cal_data);

  std::vector<Double_t> ab_events =
      LoadEventsFromFiltered(Constants::POSTREACTOR_AM241_BA133_20260116);
  FitResult am =
      FitCalPeak(ab_events, Constants::POSTREACTOR_AM241_BA133_20260116,
                 "Am_59.5keV", 55, 70, kFALSE, 0, 0, interactive);
  if (am.valid)
    AddCalPoint(cal_data, "Am-241 59.5", am.peaks.at(0).mu,
                am.peaks.at(0).mu_error, E_AM241, am.reduced_chi2);

  FitResult ba53 =
      FitCalPeak(ab_events, Constants::POSTREACTOR_AM241_BA133_20260116,
                 "Ba_53.16keV", 48, 58, kFALSE, 0, 0, interactive);
  if (ba53.valid)
    AddCalPoint(cal_data, "Ba-133 53.16", ba53.peaks.at(0).mu,
                ba53.peaks.at(0).mu_error, E_BA133_53, ba53.reduced_chi2);

  FitResult ba81 = FitCalPeak(
      ab_events, Constants::POSTREACTOR_AM241_BA133_20260116, "Ba_80.98keV", 75,
      90, kTRUE, E_BA133_79, E_BA133_81, interactive);
  if (ba81.valid) {
    AddCalPoint(cal_data, "Ba-133 79.61", ba81.peaks.at(0).mu,
                ba81.peaks.at(0).mu_error, E_BA133_79, ba81.reduced_chi2);
    AddCalPoint(cal_data, "Ba-133 80.98", ba81.peaks.at(1).mu,
                ba81.peaks.at(1).mu_error, E_BA133_81, -1);
  }

  PrintCalSummary(cal_data, "20260116");
  TF1 *cal = CreateAndSavePol1Cal(cal_data, "20260116");

  std::vector<TString> day_datasets = {
      Constants::NOSHIELD_GEONCZT_0_5PERCENT_20260116,
      Constants::NOSHIELD_ACTIVEBACKGROUND_0_5PERCENT_20260116,
      Constants::NOSHIELD_GRAPHITECASTLESIGNAL_10PERCENT_20260116,
      Constants::NOSHIELD_GRAPHITECASTLEBACKGROUND_10PERCENT_20260116,
      Constants::POSTREACTOR_AM241_BA133_20260116};
  ApplyPol1Cal(day_datasets, cal, "20260116");

  DayResult result;
  result.cal_labels.push_back("20260116");
  result.cal_funcs.push_back(cal);

  std::vector<Double_t> one_bkg_mus = {(Double_t)E_BKG_73KEV};
  std::vector<Double_t> b = LoadEventsFromCalibratedLow(
      Constants::NOSHIELD_GRAPHITECASTLEBACKGROUND_10PERCENT_20260116);
  std::vector<Double_t> s = LoadEventsFromCalibratedLow(
      Constants::NOSHIELD_GRAPHITECASTLESIGNAL_10PERCENT_20260116);
  BkgGeSimResult post = RunBkgGeSim(
      b, s, 67, 80, 60, 77, one_bkg_mus, E_GE_73M, kTRUE, kTRUE,
      TString(Constants::NOSHIELD_GRAPHITECASTLEBACKGROUND_10PERCENT_20260116) +
          "_postcal",
      TString(Constants::NOSHIELD_GRAPHITECASTLESIGNAL_10PERCENT_20260116) +
          "_postcal",
      "PostCal_BkgGeSim", interactive);
  AddGeResult(result, post, "Graphite Castle Signal 10% (01/16)");

  return result;
}

void PrintCalParams(TF1 *cal, const TString &date_label) {
  if (!cal)
    return;
  std::cout << "Cal pol1 " << date_label << ": p0 = " << std::fixed
            << std::setprecision(6) << cal->GetParameter(0) << " +/- "
            << cal->GetParError(0) << ", p1 = " << cal->GetParameter(1)
            << " +/- " << cal->GetParError(1) << std::endl;
}

void PrintDayCals(const DayResult &d) {
  for (size_t i = 0; i < d.cal_funcs.size(); i++)
    PrintCalParams(d.cal_funcs[i], d.cal_labels[i]);
}

void AppendDayGe(const DayResult &d, std::vector<TString> &labels,
                 std::vector<Float_t> &mus, std::vector<Float_t> &errs,
                 std::vector<Float_t> &chi2s) {
  for (size_t i = 0; i < d.ge_labels.size(); i++) {
    labels.push_back(d.ge_labels[i]);
    mus.push_back(d.ge_mus[i]);
    errs.push_back(d.ge_errs[i]);
    chi2s.push_back(d.ge_chi2s[i]);
  }
}

void CalibrationLow() {
  const TString project_root = Paths::ProjectRootOf(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG,
                                project_root + "/plots/calibration_low",
                                project_root + "/root_files");
  Bool_t interactive = kTRUE;

  DayResult d13 = ProcessDay_20260113(interactive);
  DayResult d14 = ProcessDay_20260114(interactive);
  DayResult d15 = ProcessDay_20260115(interactive);
  DayResult d16 = ProcessDay_20260116(interactive);

  PrintDayCals(d13);
  PrintDayCals(d14);
  PrintDayCals(d15);
  PrintDayCals(d16);

  std::vector<TString> all_labels;
  std::vector<Float_t> all_mus, all_errs, all_chi2s;
  AppendDayGe(d13, all_labels, all_mus, all_errs, all_chi2s);
  AppendDayGe(d14, all_labels, all_mus, all_errs, all_chi2s);
  AppendDayGe(d15, all_labels, all_mus, all_errs, all_chi2s);
  AppendDayGe(d16, all_labels, all_mus, all_errs, all_chi2s);

  std::cout << "Individual Run Results (Ge Peak mu, post-cal):" << std::endl;
  for (size_t i = 0; i < all_mus.size(); i++) {
    std::cout << std::left << std::setw(50) << all_labels[i] << ": "
              << std::fixed << std::setprecision(4) << all_mus[i] << " +/- "
              << all_errs[i] << " keV"
              << " (chi2/ndf = " << std::setprecision(3) << all_chi2s[i] << ")"
              << std::endl;
  }

  Float_t sum_w = 0, w_sum = 0;
  for (size_t i = 0; i < all_mus.size(); i++) {
    if (all_errs[i] > 0) {
      Float_t w = 1.0 / (all_errs[i] * all_errs[i]);
      w_sum += all_mus[i] * w;
      sum_w += w;
    }
  }
  if (sum_w > 0) {
    Float_t cmu = w_sum / sum_w;
    Float_t cerr = std::sqrt(1.0 / sum_w);
    std::cout << "Combined Ge mu: " << std::fixed << std::setprecision(4) << cmu
              << " +/- " << cerr << " keV" << std::endl;
  }
}
