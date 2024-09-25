#include "../include/TreeModule.hh"

TreeModule::TreeModule(const char *filename) {
  aFilename = filename;
  aFile = new TFile(filename, "READ");
  if (aFile->IsOpen()) {
    hitsTree = static_cast<TTree *>(aFile->Get("Hits;"));
    energyTreeCZT = static_cast<TTree *>(aFile->Get("EnergyCZT;"));
    energyTreeHPGe = static_cast<TTree *>(aFile->Get("EnergyHPGe;"));
    branchEnergyDepCZT = energyTreeCZT->GetBranch("fEdepCZT");
    branchEnergyDepHPGe = energyTreeHPGe->GetBranch("fEdepHPGe");
    branchEvents = hitsTree->GetBranch("fEvents");
  } else {
    // Handle the error if file did not open correctly
    std::cerr << "Failed to open the file: " << filename << std::endl;
    aFile = nullptr;
    hitsTree = nullptr;
    energyTreeCZT = nullptr;
    energyTreeHPGe = nullptr;
    branchEnergyDepCZT = nullptr;
    branchEnergyDepHPGe = nullptr;
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

TString TreeModule::getFormattedFilename() {
  std::string filename = this->getFilename();
  // Remove .root file extension
  std::size_t pos = filename.find(".root");
  if (pos != std::string::npos) {
    filename.erase(pos, 5); // 5 is the length of ".root"
  }

  // Get the part after the last '/'
  pos = filename.find_last_of('/');
  std::string output;
  if (pos != std::string::npos) {
    output = filename.substr(pos + 1);
  } else {
    output = filename;
  } // Concatenating strings
  std::string fileoutput = output;

  // this works because TString is not a char* so when fileoutput is
  // dereferenced after the function ends, it doesn't matter
  TString rootStr = TString(fileoutput.c_str());

  return rootStr;
}

TH1D *TreeModule::energySpectrumHist(const TString detectorName,
                                     const char *fileExtension = ".png",
                                     bool isBroadened = false) {
  double eDep;

  double etemp;
  int entries = 0;
  TH1D *hist = nullptr;

  if (detectorName == "CZT") {
    int entries = branchEnergyDepCZT->GetEntries();
    hist = new TH1D(this->getFormattedFilename(), "; Energy (keV);Entries", 256,
                    -100, 6e3);
    branchEnergyDepCZT->SetAddress(&eDep);

    for (int i = 0; i < entries; i++) {
      branchEnergyDepCZT->GetEntry(i);
      etemp = eDep * 1000;
      hist->Fill(etemp);
    }
    hist->SetStats(0);
  } else if (detectorName == "HPGe") {

    int entries = branchEnergyDepHPGe->GetEntries();
    hist = new TH1D(this->getFormattedFilename(), "; Energy (keV);Entries", 256,
                    -100, 6e3);
    branchEnergyDepHPGe->SetAddress(&eDep);

    for (int i = 0; i < entries; i++) {
      branchEnergyDepHPGe->GetEntry(i);
      etemp = eDep * 1000;
      hist->Fill(etemp);
    }
    hist->SetStats(0);
  }
  return hist;
}

TH1D *TreeModule::partialEnergySpectrumHist(const TString detectorName,
                                            double lowerBound,
                                            double upperBound,
                                            const char *fileExtension = ".png",
                                            bool isBroadened = false) {
  double eDep;
  double etemp;
  TH1D *hist = nullptr;
  int entries = 0;

  double diff = upperBound - lowerBound;

  if (detectorName == "CZT") {
    int entries = branchEnergyDepCZT->GetEntries();

    if (diff >= 500) {
      hist = new TH1D(this->getFormattedFilename(), "; Energy (keV);Entries",
                      256, lowerBound, upperBound);
    } else {
      hist = new TH1D(this->getFormattedFilename(), "; Energy (keV);Entries",
                      128, lowerBound, upperBound);
    }

    branchEnergyDepCZT->SetAddress(&eDep);

    for (int i = 0; i < entries; i++) {
      branchEnergyDepCZT->GetEntry(i);
      etemp = eDep * 1000;
      if (lowerBound <= etemp && etemp <= upperBound) {
        hist->Fill(etemp);
      }
    }
    hist->SetStats(0);
  } else if (detectorName == "HPGe") {

    int entries = branchEnergyDepHPGe->GetEntries();

    if (diff >= 500) {
      hist = new TH1D(this->getFormattedFilename(), "; Energy (keV);Entries",
                      256, lowerBound, upperBound);
    } else {
      hist = new TH1D(this->getFormattedFilename(), "; Energy (keV);Entries",
                      128, lowerBound, upperBound);
    }

    branchEnergyDepHPGe->SetAddress(&eDep);

    for (int i = 0; i < entries; i++) {
      branchEnergyDepHPGe->GetEntry(i);
      etemp = eDep * 1000;
      if (lowerBound <= etemp && etemp <= upperBound) {
        hist->Fill(etemp);
      }
    }
    hist->SetStats(0);
  }

  return hist;
}
