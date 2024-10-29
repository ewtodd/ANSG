#include "../include/BroadTree.hh"

BroadTree::BroadTree(const char *filename) {
  aFilename = filename;
  aFile = new TFile(filename, "READ");
  if (aFile->IsOpen()) {
    energyTreeCZT = static_cast<TTree *>(aFile->Get("BroadenedEnergyCZT;"));
    energyTreeHPGe = static_cast<TTree *>(aFile->Get("BroadenedEnergyHPGe;"));
    energyTreeSiLi = static_cast<TTree *>(aFile->Get("BroadenedEnergySiLi"));
    branchEnergyDepCZT = energyTreeCZT->GetBranch("fEdepCZT");
    branchEnergyDepHPGe = energyTreeHPGe->GetBranch("fEdepHPGe");
    branchEnergyDepSiLi = energyTreeSiLi->GetBranch("fEdepSiLi");
  } else {
    std::cerr << "Failed to open the file: " << filename << std::endl;
    aFile = nullptr;
    energyTreeCZT = nullptr;
    energyTreeHPGe = nullptr;
    energyTreeSiLi = nullptr;
    branchEnergyDepCZT = nullptr;
    branchEnergyDepHPGe = nullptr;
    branchEnergyDepSiLi = nullptr;
  }
}

BroadTree::~BroadTree() {
  std::cout << "Deleting BroadTree..." << std::endl;
  if (aFile) {
    aFile->Close();
    delete aFile;
  }
  std::cout << "Done." << std::endl;
}

TString BroadTree::getFormattedFilename() {
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

// avoid histogram naming related memory leaks
TString BroadTree::generateRandomString() {
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

TH1D *BroadTree::energySpectrumHist(const TString detectorName,
                                    double lowerBound = 0,
                                    double upperBound = 100, int nbins = 829) {

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
  TString histName = generateRandomString();
  hist = new TH1D(histName, ";Energy (keV);Entries", nbins, lowerBound,
                  upperBound);
  for (int i = 0; i < entries; i++) {
    branchEnergyDep->GetEntry(i);
    etemp = eDep;
    if (etemp >= lowerBound && etemp <= upperBound) {
      hist->Fill(etemp);
    }
  }

  hist->SetStats(0);
  return hist;
}
