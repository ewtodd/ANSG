#include "../include/TreeModule.hh"

TreeModule::TreeModule(const char *filename, const char *broadenedFilename) {
  aFilename = filename;
  aFile = new TFile(filename, "READ");
  if (aFile->IsOpen()) {
    hitsTree = static_cast<TTree *>(aFile->Get("Hits"));
    energyTreeSiLi = static_cast<TTree *>(aFile->Get("EnergySiLi"));
    branchEnergyDepSiLi = energyTreeSiLi->GetBranch("fEdepSiLi");
  } else {
    std::cerr << "Failed to open the file: " << filename << std::endl;
    aFile = nullptr;
    hitsTree = nullptr;
    energyTreeSiLi = nullptr;
    branchEnergyDepSiLi = nullptr;
  }

  TString broadenedFileName;
  if (broadenedFilename) {
    broadenedFileName = broadenedFilename;
  } else {
    broadenedFileName = TString::Format(
        "%s_broadened.root", TString(filename).ReplaceAll(".root", "").Data());
  }

  broadenedFile = new TFile(broadenedFileName, "RECREATE");
  if (!broadenedFile->IsOpen()) {
    std::cerr << "Failed to open the broadened file: " << broadenedFileName
              << std::endl;
    broadenedFile = nullptr;
  }
}

TreeModule::~TreeModule() {
  if (aFile) {
    aFile->Close();
    delete aFile;
  }
  if (broadenedFile) {
    broadenedFile->Close();
    delete broadenedFile;
  }
  std::cout << "Done." << std::endl;
}

TH1D *TreeModule::createHistogram(TBranch *branch, const char *histName) {
  double eDep;
  branch->SetAddress(&eDep);

  double FWHM = 0.;
  double res = 0.;

  res = 0.165 / 5.9;
  std::cout << "Creating SiLi histogram..." << std::endl;

  FWHM = res * 68.752;
  int nbins = 1.3 * (10400) / (FWHM / 15);

  TString histTitle =
      TString::Format("%s Spectrum;Energy (keV);Entries", histName);
  TH1D *hist = new TH1D(histName, histTitle, nbins, 0, 10400);
  int entries = branch->GetEntries();
  std::cout << "Creating histogram " << histName << " with " << entries
            << " entries." << std::endl;

  for (int i = 0; i < entries; ++i) {
    branch->GetEntry(i);
    hist->Fill(eDep * 1000);
  }
  std::cout << "Filled histogram " << histName << " with " << hist->GetEntries()
            << " entries." << std::endl;
  return hist;
}

TH1D *TreeModule::broadenedHist(TH1D *hist, const TString detectorName) {
  double res = 0.;

  if (detectorName == "CZT") {
    res = 1.8 / 59.5;
  } else if (detectorName == "HPGe") {
    res = 0.430 / 68.75;
  } else if (detectorName == "SiLi") {
    res = 0.165 / 5.9;
  }

  TH1D *broadSpectrum = (TH1D *)hist->Clone();
  broadSpectrum->Reset("ICES");

  for (int i = 1; i <= hist->GetNbinsX(); ++i) {
    double eCenter = hist->GetBinCenter(i);
    double binContent = hist->GetBinContent(i);

    for (int j = 1; j <= broadSpectrum->GetNbinsX(); ++j) {
      double eLow = broadSpectrum->GetBinLowEdge(j);
      double eHigh = broadSpectrum->GetBinLowEdge(j + 1);
      double simBinSigma = (eCenter * res) / 2.35;

      double contribution =
          binContent *
          (TMath::Erf((eHigh - eCenter) / ((TMath::Sqrt(2.0) * simBinSigma))) -
           TMath::Erf((eLow - eCenter) / ((TMath::Sqrt(2.0) * simBinSigma)))) /
          2.0;
      broadSpectrum->AddBinContent(j, contribution);
    }
  }

  broadSpectrum->SetStats(0);
  return broadSpectrum;
}

void TreeModule::writeHistogramToTree(TTree *tree, TH1D *hist,
                                      const char *branchName) {
  double energy;
  TBranch *branch = tree->Branch(branchName, &energy, "energy/D");
  std::cout << "Writing broadened histogram to tree with branch " << branchName
            << std::endl;

  for (int i = 1; i <= hist->GetNbinsX(); ++i) {
    energy = hist->GetBinCenter(i);
    int entries = hist->GetBinContent(i);
    for (int j = 0; j < entries; ++j) {
      tree->Fill();
    }
  }
  std::cout << "Wrote " << branch->GetEntries() << " entries to branch "
            << branchName << std::endl;
}

void TreeModule::broadenAndStoreEnergy() {
  if (!broadenedFile || !broadenedFile->IsOpen()) {
    std::cerr << "Broadened file is not open or valid." << std::endl;
    return;
  }
  broadenedFile
      ->cd(); // Ensure that we are working in the broadened file context

  if (branchEnergyDepSiLi) {
    TTree *broadenedTreeSiLi =
        new TTree("BroadenedEnergySiLi", "Broadened Energy Spectrum for SiLi");
    TH1D *histSiLi = createHistogram(branchEnergyDepSiLi, "SiLi");
    TH1D *broadHistSiLi = broadenedHist(histSiLi, "SiLi");
    writeHistogramToTree(broadenedTreeSiLi, broadHistSiLi, "fEdepSiLi");
    broadenedTreeSiLi->Write(); // Write the tree to the file
    delete histSiLi;
    delete broadHistSiLi;
  }
}
