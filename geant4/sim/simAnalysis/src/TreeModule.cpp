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
                                     double lowerBound = 0,
                                     double upperBound = 1500,
                                     bool isBroadened = false) {
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
  if (lowerBound == 0 && upperBound == 1500) {
    hist = new TH1D(generateRandomString(), ";Energy (keV);Entries", 10000,
                    lowerBound, upperBound);
    for (int i = 0; i < entries; i++) {
      branchEnergyDep->GetEntry(i);
      etemp = eDep * 1000;
      hist->Fill(etemp);
    }
    hist->SetStats(0);

    if (isBroadened) {
      hist = broadenedHist(hist, detectorName);
    }
  } else if (lowerBound != 0 || upperBound != 1500) {
    int nbins = 10000;

    hist = new TH1D(generateRandomString(), ";Energy (keV);Entries", nbins, 0,
                    1500);
    for (int i = 0; i < entries; i++) {
      branchEnergyDep->GetEntry(i);
      etemp = eDep * 1000;
      hist->Fill(etemp);
    }

    if (isBroadened) {
      hist = broadenedHist(hist, detectorName);
    }

    int bin_min = hist->GetXaxis()->FindBin(lowerBound);
    int bin_max = hist->GetXaxis()->FindBin(upperBound);

    // Find the maximum bin content in the specified x range
    double max_content = 0;
    for (int bin = bin_min; bin <= bin_max; ++bin) {
      double content = hist->GetBinContent(bin);
      if (content > max_content) {
        max_content = content;
      }
    }

    // Set reasonable minimum and maximum values for the y-axis
    double y_min = 0; // Assuming a baseline at 0
    double y_max =
        max_content * 1.1; // Add a 10% margin for better visual distinction

    hist->GetXaxis()->SetRangeUser(lowerBound, upperBound); // Set x-axis range
    hist->GetYaxis()->SetRangeUser(y_min, y_max);           // Set y-axis range
    hist->SetStats(0);
  }

  return hist;
}
