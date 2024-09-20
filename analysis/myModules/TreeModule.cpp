#include "TreeModule.h"
// #include "PlotFunctions.cpp"
#include <TBranch.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TString.h>
#include <TTree.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <string>

// define a constructor that reads in a CoMPASS ROOT file and creates
// a TreeModule with the branches of those we care about
TreeModule::TreeModule(const char *filename, bool isCompassRawArg = true) {
  aFilename = filename;
  isCompassRaw = isCompassRawArg;
  aFile = new TFile(filename, "READ");
  if (aFile->IsOpen()) {
    // static_cast because we know that this will always produce a
    // TTree
    aTree = static_cast<TTree *>(aFile->Get("Data_R"));
    branchEnergy = aTree->GetBranch("Energy");
    branchEnergyShort = aTree->GetBranch("EnergyShort");
    branchTimestamp = isCompassRaw ? aTree->GetBranch("Timestamp") : nullptr;
    branchSamples = isCompassRaw ? aTree->GetBranch("Samples") : nullptr;
    branchPSP = isCompassRaw ? nullptr : aTree->GetBranch("PSP");
  } else {
    // Handle the error if file did not open correctly
    std::cerr << "Failed to open the file: " << filename << std::endl;
    aFile = nullptr;
    aTree = nullptr;
    branchPSP = nullptr;
    branchEnergy = nullptr;
    branchEnergyShort = nullptr;
    branchTimestamp = nullptr;
    branchSamples = nullptr;
  }
}

// define a desctructor for the TreeMmodule
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

TString TreeModule::getFormattedFilename(std::string filePrefix,
                                         const char *fileExtension = ".png") {
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
  }

  // Concatenating strings
  std::string fileoutput = filePrefix + output + fileExtension;

  // this works because TString is not a char* so when fileoutput is
  // dereferenced after the function ends, it doesn't matter
  TString rootStr = TString(fileoutput.c_str());

  return rootStr;
}

// define a function to calculate and save a histogram showing
// the distribution of time between measurements
void TreeModule::TimeDistTM(const char *fileExtension = ".png") {

  // make variable for time difference
  Long64_t timediff;

  int entries = branchTimestamp->GetEntries();
  TCanvas *c1 = new TCanvas("c1", "c1");
  TH1D *hist = new TH1D("hist", "Time Between Measurements; Time (ps);Entries",
                        256, 0, 1e10);
  Long64_t ts1;

  Long64_t temp_ts1, temp_ts2;
  branchTimestamp->SetAddress(&ts1);

  for (int i = 0; i < entries; i++) {
    branchTimestamp->GetEntry(i);
    temp_ts1 = ts1;
    branchTimestamp->GetEntry(i + 1);
    temp_ts2 = ts1;
    timediff = (temp_ts2 - temp_ts1);
    hist->Fill(timediff);
  }
  hist->Draw();
  std::string pref = "timediff_";
  c1->Print(this->getFormattedFilename(pref, fileExtension));

  // make sure to delete hist and c1
  delete hist;
  delete c1;
}

// TO DO: fix this for case of CoMPASS raw vs processed
// define a function that makes the PSD vs. Light Output (ADC Channel) plot
void TreeModule::PSD_E_TM(const char *fileExtension = ".png") {
  // TO DO: filename formatting to name histogram properly
  TH2D *hist = new TH2D("hist", "; Light Output (a.u.); PSP; Counts", 256, 0,
                        4099, 256, 0, 1);
  int entries = branchEnergy->GetEntries();

  // define variables which we will store the branch values in
  UShort_t energy, energyShort;

  // define psp variable
  double psp;

  branchEnergy->SetAddress(&energy);
  branchEnergyShort->SetAddress(&energyShort);

  // define temp variables
  double temp_energy, temp_energyShort;

  // loop through all entries and calculate psp, fill histogram
  for (int i = 0; i < entries; i++) {
    branchEnergy->GetEntry(i);
    temp_energy = energy;
    branchEnergyShort->GetEntry(i);
    temp_energyShort = energyShort;
    psp = (temp_energy - temp_energyShort) / temp_energy;
    hist->Fill(temp_energy, psp);
  }

  TCanvas *c1 = new TCanvas();
  hist->Draw("colz");
  std::string pref = "PSP_LO_";
  c1->Print(this->getFormattedFilename(pref, fileExtension));

  // make sure to delete hist and c1
  delete hist;
  delete c1;
}

void TreeModule::PSPHist(const char *fileExtension = ".png",
                         const int drawRaw = 0) {
  switch (drawRaw) {
  case 0:
    break;
  case 1:
    std::cout << "Drawing the first three raw waveforms..." << std::endl;
    break;
  case 2:
    std::cout << "Drawing a random selection of three raw waveforms..."
              << std::endl;
    break;
  default:
    std::cerr << "Error: Choose a valid value for drawRaw - 0, 1, or 2."
              << std::endl;
    break;
  }
  TCanvas *c1 = new TCanvas();
  // SetCanvasMargins(c1);

  std::string pref = "PSP_HIST_";
  c1->Print(this->getFormattedFilename(pref, fileExtension));
}
