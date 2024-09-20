#include "../include/TreeModule.hh"

TreeModule::TreeModule(const char *filename) {
  aFilename = filename;
  aFile = new TFile(filename, "READ");
  if (aFile->IsOpen()) {
    hitsTree = static_cast<TTree *>(aFile->Get("Hits;"));
    energyTree = static_cast<TTree *>(aFile->Get("Energy;"));
    branchEnergyDep = energyTree->GetBranch("fEdep");
    branchEvents = hitsTree->GetBranch("fEvents");
  } else {
    // Handle the error if file did not open correctly
    std::cerr << "Failed to open the file: " << filename << std::endl;
    aFile = nullptr;
    hitsTree = nullptr;
    energyTree = nullptr;
    branchEnergyDep = nullptr;
    branchEvents = nullptr;
  }
}

TreeModule::~TreeModule() {
  std::cout << "Deleting TreeModule..." << std::endl;
  // check and delete ROOT Objects
  if (aFile) {
    aFile->Close(); // close file if it is open
    delete aFile;   // then delete it
  }
  // TTree, TBranch etc. are deleted when TFile is deleted
  std::cout << "Done." << std::endl;
}

TH1D *TreeModule::energySpectrumHist(const char *fileExtension = ".png") {
  double eDep;

  double etemp;

  int entries = branchEnergyDep->GetEntries();
  TH1D *hist = new TH1D("hist", "; Energy (keV);Entries", 256, 0, 6e3);

  branchEnergyDep->SetAddress(&eDep);

  for (int i = 0; i < entries; i++) {
    branchEnergyDep->GetEntry(i);
    etemp = eDep * 1000;
    hist->Fill(etemp);
  }

  hist->SetStats(0);

  return hist;
}

TH1D *
TreeModule::partialEnergySpectrumHist(int lowerBound, int upperBound,
                                      const char *fileExtension = ".png") {
  double eDep;

  double etemp;

  int entries = branchEnergyDep->GetEntries();
  TH1D *hist =
      new TH1D("hist", "; Energy (keV);Entries", 256, lowerBound, upperBound);

  branchEnergyDep->SetAddress(&eDep);

  for (int i = 0; i < entries; i++) {
    branchEnergyDep->GetEntry(i);
    etemp = eDep * 1000;
    hist->Fill(etemp);
  }

  hist->SetStats(0);

  return hist;
}
