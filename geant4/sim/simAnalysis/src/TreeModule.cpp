#include "../include/TreeModule.hh"

TreeModule::TreeModule(const char *filename) {
  aFilename = filename;
  aFile = new TFile(filename, "READ");
  if (aFile->IsOpen()) {
    hitsTree = static_cast<TTree *>(aFile->Get("Hits;"));
    energyTreeCZT = static_cast<TTree *>(aFile->Get("EnergyCZT;"));
    energyTreeHPGe = static_cast<TTree *>(aFile->Get("EnergyHPGe;"));
    energyTreeSiLi = static_cast<TTree *>(aFile->Get("EnergySiLi"));
    branchEnergyDepCZT = energyTreeCZT->GetBranch("fEdepCZT");
    branchEnergyDepHPGe = energyTreeHPGe->GetBranch("fEdepHPGe");
    branchEnergyDepSiLi = energyTreeSiLi->GetBranch("fEdepSiLi");
    branchEvents = hitsTree->GetBranch("fEvents");
  } else {
    std::cerr << "Failed to open the file: " << filename << std::endl;
    aFile = nullptr;
    hitsTree = nullptr;
    energyTreeCZT = nullptr;
    energyTreeHPGe = nullptr;
    energyTreeSiLi = nullptr;
    branchEnergyDepCZT = nullptr;
    branchEnergyDepHPGe = nullptr;
    branchEnergyDepSiLi = nullptr;
    branchEvents = nullptr;
  }
}

TreeModule::~TreeModule() {
  std::cout << "Deleting TreeModule..." << std::endl;
  if (aFile) {
    aFile->Close();
    delete aFile;
  }
  std::cout << "Done." << std::endl;
}

TString TreeModule::getFormattedFilename() {
  std::string filename = this->getFilename();
  std::size_t pos = filename.find(".root");
  if (pos != std::string::npos) {
    filename.erase(pos, 5);
  }
  pos = filename.find_last_of('/');
  std::string output;
  if (pos != std::string::npos) {
    output = filename.substr(pos + 1);
  } else {
    output = filename;
  }
  return TString(output.c_str());
}

TH1D *TreeModule::broadenedHist(TH1D *hist, const TString detectorName) {
  double res = 0.;
  TBranch *branchEnergyDep = nullptr;

  if (detectorName == "CZT") {
    branchEnergyDep = branchEnergyDepCZT;
    res = 1.8 / 59.5;
  } else if (detectorName == "HPGe") {
    branchEnergyDep = branchEnergyDepHPGe;
    res = 0.430 / 68.75;
  } else if (detectorName == "SiLi") {
    branchEnergyDep = branchEnergyDepSiLi;
    res = 0.165 / 5.9;
  }

  double eDep;
  branchEnergyDep->SetAddress(&eDep);

  TH1D *broadSpectrum = new TH1D(
      this->getFormattedFilename(), ";Energy (keV);Entries", hist->GetNbinsX(),
      hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());

  for (int i = 1; i <= hist->GetNbinsX(); ++i) {
    double eCenter = hist->GetBinCenter(i);
    double binContent = hist->GetBinContent(i);

    for (int j = 1; j <= broadSpectrum->GetNbinsX(); ++j) {
      double eLow = broadSpectrum->GetBinLowEdge(j);
      double eHigh = broadSpectrum->GetBinLowEdge(j + 1);
      double simBinSigma = (eCenter * res) / 2.35;

      double contribution =
          binContent *
          (TMath::Erf((eHigh - eCenter) / (TMath::Sqrt(2.0) * simBinSigma)) -
           TMath::Erf((eLow - eCenter) / (TMath::Sqrt(2.0) * simBinSigma))) /
          2.0;
      broadSpectrum->AddBinContent(i, contribution);
    }
  }

  broadSpectrum->SetStats(0);
  return broadSpectrum;
}

// avoid histogram naming related memory leaks
TString TreeModule::generateRandomString() {
  const int length = 3; // Length of the random string
  const char charset[] = "abcdefghijklmnopqrstuvwxyz"; // Possible characters
  const size_t max_index = sizeof(charset) - 1;
  std::default_random_engine generator(
      std::random_device{}()); // Seed with random device
  std::uniform_int_distribution<int> distribution(0, max_index - 1);

  TString randomString;
  for (int i = 0; i < length; ++i) {
    randomString += charset[distribution(generator)];
  }

  return randomString;
}

TH1D *TreeModule::energySpectrumHist(const TString detectorName,
                                     const char *fileExtension,
                                     bool isBroadened) {
  double eDep;
  double etemp;
  int entries = 0;
  TH1D *hist = nullptr;
  TBranch *branchEnergyDep = nullptr;

  if (detectorName == "CZT") {
    entries = branchEnergyDepCZT->GetEntries();
    branchEnergyDep = branchEnergyDepCZT;
  } else if (detectorName == "HPGe") {
    entries = branchEnergyDepHPGe->GetEntries();
    branchEnergyDep = branchEnergyDepHPGe;
  } else if (detectorName == "SiLi") {
    entries = branchEnergyDepSiLi->GetEntries();
    branchEnergyDep = branchEnergyDepSiLi;
  }

  branchEnergyDep->SetAddress(&eDep);

  hist =
      new TH1D(generateRandomString(), ";Energy (keV);Entries", 256, -100, 6e3);
  for (int i = 0; i < entries; i++) {
    branchEnergyDep->GetEntry(i);
    etemp = eDep * 1000;
    hist->Fill(etemp);
  }
  hist->SetStats(0);

  if (isBroadened) {
    TH1D *broadHist = broadenedHist(hist, detectorName);
    delete hist; // Free the original histogram
    return broadHist;
  }
  return hist;
}
void TreeModule::partialEnergySpectrumHist(const TString detectorName,
                                           double lowerBound, double upperBound,
                                           const char *fileExtension,
                                           bool isBroadened) {
  double eDep;
  double etemp;
  TH1D *hist = nullptr;
  int entries = 0;

  TBranch *branchEnergyDep = nullptr;

  if (detectorName == "CZT") {
    entries = branchEnergyDepCZT->GetEntries();
    branchEnergyDep = branchEnergyDepCZT;
  } else if (detectorName == "HPGe") {
    entries = branchEnergyDepHPGe->GetEntries();
    branchEnergyDep = branchEnergyDepHPGe;
  } else if (detectorName == "SiLi") {
    entries = branchEnergyDepSiLi->GetEntries();
    branchEnergyDep = branchEnergyDepSiLi;
  }
  branchEnergyDep->SetAddress(&eDep);

  double diff = upperBound - lowerBound;
  hist = new TH1D(generateRandomString(), ";Energy (keV);Entries",
                  diff >= 500 ? 256 : 128, lowerBound, upperBound);

  for (int i = 0; i < entries; i++) {
    branchEnergyDep->GetEntry(i);
    etemp = eDep * 1000;
    if (lowerBound <= etemp && etemp <= upperBound) {
      hist->Fill(etemp);
    }
  }

  hist->SetStats(0);
  TCanvas *c1 = new TCanvas("c1", "c1", 2000, 1500);
  // Set the margins
  c1->SetLeftMargin(0.15);  // Set the left margin to 15% of the canvas width
  c1->SetRightMargin(0.05); // Set the right margin to 5% of the canvas width
  c1->SetTopMargin(0.1);    // Set the top margin to 10% of the canvas height
  c1->SetBottomMargin(
      0.15); // Set the bottom margin to 15% of the canvas height
  // Set fill colors and
  // styles
  Short_t lineWidth = 2;
  hist->SetLineColor(kRed);
  hist->SetLineWidth(lineWidth);

  if (isBroadened) {
    TH1D *broadHist = broadenedHist(hist, detectorName);
    delete hist; // Free the original histogram

    // Draw the broadened histogram
    broadHist->SetLineWidth(lineWidth);
    broadHist->Draw();

    // Perform Gaussian fit and legend creation
    TLegend *legend = fitGaussianToPeak(broadHist);
    legend->Draw(); // Draw the legend after the histogram

    // Add the title using TLatex
    TLatex *latex = new TLatex();
    latex->SetNDC();
    latex->SetTextSize(0.04);
    latex->DrawLatex(0.5, 0.95, detectorName);
    c1->Update();

    TString fileName = Form("partialHist_%.2f_%.2f", lowerBound, upperBound);
    c1->Print(fileName + detectorName + "_broad.png");

    delete c1;
    delete broadHist;
    delete legend;
  } else {
    hist->Draw();

    // Add the title using TLatex
    TLatex *latex = new TLatex();
    latex->SetNDC();
    latex->SetTextSize(0.04);
    latex->DrawLatex(0.5, 0.95, detectorName);
    c1->Update();

    TString fileName = Form("partialHist_%.2f_%.2f", lowerBound, upperBound);
    c1->Print(fileName + detectorName + "_.png");

    delete c1;
    delete hist;
  }
}

TLegend *TreeModule::fitGaussianToPeak(TH1D *hist) {
  // Define the fit range around the peak position
  double fitRangeMin = 68.75 - 5;
  double fitRangeMax = 68.75 + 5;
  // Define the Gaussian function
  TF1 *gaussFit = new TF1("gaussFit", "gaus + [3]", fitRangeMin, fitRangeMax);

  // Initial parameters: [0] = height, [1] = mean, [2] = sigma
  gaussFit->SetParameters(hist->GetMaximum(), 68.75, 1.0);

  // Perform the fit
  hist->Fit(gaussFit, "RQ");

  // Retrieve fit parameters and their uncertainties
  double peakMean = gaussFit->GetParameter(1);
  double peakMeanError = gaussFit->GetParError(1);
  // Create the legend
  TLegend *legend = new TLegend(0.65, 0.75, 0.95, 0.9);
  legend->SetTextSize(0.025);
  legend->SetFillColor(0);
  legend->AddEntry(
      gaussFit,
      TString::Format("Mean: %.3f #pm %.3f keV", peakMean, peakMeanError), "l");

  // Output fit results to console
  std::cout << "Peak position (mean): " << peakMean << " ± " << peakMeanError 
            << " keV" << std::endl;

  return legend;
}
