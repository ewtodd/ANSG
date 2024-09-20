#ifndef TREE_MODULE_H_INCLUDED
#define TREE_MODULE_H_INCLUDED
#include <TBranch.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TString.h>
#include <TTree.h>
#include <cstring>
#include <iostream>

class TreeModule {

private:
  TFile *aFile;
  TTree *hitsTree;
  TTree *energyTree;
  TBranch *branchEnergyDep;
  TBranch *branchEvents;
  const char *aFilename;

public:
  TreeModule(const char *filename);
  ~TreeModule();
  TFile *getAFile() const { return aFile; };
  TTree *getHitsTree() const { return hitsTree; };
  TTree *getEnergyTree() const { return energyTree; };
  TBranch *getBranchEnergy() const { return branchEnergyDep; };
  TBranch *getBranchEvents() const { return branchEvents; };
  const char *getFilename() { return aFilename; };
  TString getFormattedFilename(std::string filePrefix,
                               const char *fileExtension);
  TH1D *energySpectrumHist(const char *fileExtension);
  TH1D *partialEnergySpectrumHist(int lowerBound, int upperBound,
                                  const char *fileExtension);
};

#endif
