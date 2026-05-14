#include "Constants.hpp"
#include "FittingUtils.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include "RooFitUtils.hpp"
#include <TFile.h>
#include <TH1.h>
#include <TTree.h>
#include <iomanip>
#include <iostream>

TH1F *LoadHistogram(const TString input_name) {
  TFile *file = IO::OpenForReading("filtered/" + input_name + ".root");
  if (!file || file->IsZombie()) {
    std::cerr << "ERROR: Cannot open filtered/" << input_name << ".root"
              << std::endl;
    return nullptr;
  }
  TH1F *hist = static_cast<TH1F *>(file->Get("zoomedHist"));
  if (!hist) {
    std::cerr << "ERROR: Cannot find zoomedHist" << std::endl;
    file->Close();
    delete file;
    return nullptr;
  }
  hist->SetDirectory(0);
  file->Close();
  delete file;
  return hist;
}

std::vector<Double_t> LoadEvents(const TString input_name) {
  std::vector<Double_t> events;
  TFile *file = IO::OpenForReading("filtered/" + input_name + ".root");
  if (!file || file->IsZombie()) {
    std::cerr << "ERROR: Cannot open filtered/" << input_name << ".root"
              << std::endl;
    return events;
  }
  for (Int_t c = 0; c < Constants::N_CRYSTALS; c++) {
    TString tree_name = Form("crystal%d_filtered_tree", c);
    TTree *tree = static_cast<TTree *>(file->Get(tree_name));
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

void PrintPeak(const TString backend, const TString label,
               const PeakFitResult &peak, Float_t reduced_chi2) {
  std::cout << "  [" << backend << "] " << label << ": mu = " << std::fixed
            << std::setprecision(4) << peak.mu << " +/- " << peak.mu_error
            << ", sigma = " << peak.sigma << " +/- " << peak.sigma_error
            << ", chi2/ndf = " << std::setprecision(3) << reduced_chi2
            << std::endl;
}

void CompareSingle(const TString input_name, const TString peak_name,
                   Float_t fit_lo, Float_t fit_hi) {
  std::cout << "Single peak: " << peak_name << " on " << input_name
            << std::endl;

  TH1F *hist_tf1 = LoadHistogram(input_name);
  std::vector<Double_t> events = LoadEvents(input_name);
  if (!hist_tf1 || events.empty()) {
    delete hist_tf1;
    return;
  }

  Bool_t use_flat_bkg = kTRUE;
  Bool_t use_step = kFALSE;
  Bool_t use_low_exp = kTRUE;
  Bool_t use_low_lin = kTRUE;
  Bool_t use_high_exp = kTRUE;

  FittingUtils tf1_fitter(hist_tf1, fit_lo, fit_hi, use_flat_bkg, use_step,
                          use_low_exp, use_low_lin, use_high_exp);
  tf1_fitter.SetInteractive();
  FitResult tf1_result =
      tf1_fitter.FitSinglePeak(input_name + "_tf1", peak_name);

  RooFitUtils rf_fitter(events, fit_lo, fit_hi, Constants::BIN_WIDTH_KEV,
                        use_flat_bkg, use_step, use_low_exp, use_low_lin,
                        use_high_exp);
  rf_fitter.SetInteractive();
  FitResult rf_result =
      rf_fitter.FitSinglePeak(input_name + "_roofit", peak_name);

  if (tf1_result.valid)
    PrintPeak("TF1   ", peak_name, tf1_result.peaks[0],
              tf1_result.reduced_chi2);
  if (rf_result.valid)
    PrintPeak("RooFit", peak_name, rf_result.peaks[0], rf_result.reduced_chi2);

  delete hist_tf1;
}

void CompareDoubleFree(const TString input_name, const TString peak_name,
                       Float_t fit_lo, Float_t fit_hi, Float_t mu1_init,
                       Float_t mu2_init) {
  std::cout << "Double peak (free): " << peak_name << " on " << input_name
            << std::endl;

  TH1F *hist_tf1 = LoadHistogram(input_name);
  std::vector<Double_t> events = LoadEvents(input_name);
  if (!hist_tf1 || events.empty()) {
    delete hist_tf1;
    return;
  }

  Bool_t use_flat_bkg = kTRUE;
  Bool_t use_step = kFALSE;
  Bool_t use_low_exp = kTRUE;
  Bool_t use_low_lin = kTRUE;
  Bool_t use_high_exp = kTRUE;

  FittingUtils tf1_fitter(hist_tf1, fit_lo, fit_hi, use_flat_bkg, use_step,
                          use_low_exp, use_low_lin, use_high_exp);
  tf1_fitter.SetInteractive();
  FitResult tf1_result = tf1_fitter.FitDoublePeak(
      input_name + "_tf1", peak_name, mu1_init, mu2_init);

  RooFitUtils rf_fitter(events, fit_lo, fit_hi, Constants::BIN_WIDTH_KEV,
                        use_flat_bkg, use_step, use_low_exp, use_low_lin,
                        use_high_exp);
  rf_fitter.SetInteractive();
  FitResult rf_result = rf_fitter.FitDoublePeak(input_name + "_roofit",
                                                peak_name, mu1_init, mu2_init);

  for (Int_t i = 0; i < 2; i++) {
    TString label = TString::Format("peak%d", i + 1);
    if (tf1_result.valid)
      PrintPeak("TF1   ", label, tf1_result.peaks[i], tf1_result.reduced_chi2);
    if (rf_result.valid)
      PrintPeak("RooFit", label, rf_result.peaks[i], rf_result.reduced_chi2);
  }

  delete hist_tf1;
}

void RooFitTest() {
  const TString project_root = Paths::ProjectRootOf(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG,
                                project_root + "/plots/roofit_test",
                                project_root + "/root_files");

  CompareSingle(Constants::POSTREACTOR_AM241_20260113, "Am_59.5keV", 50, 70);

  CompareDoubleFree(Constants::POSTREACTOR_AM241_BA133_20260116, "Ba_80.98keV",
                    75, 90, 79.6142, 80.9979);
}
