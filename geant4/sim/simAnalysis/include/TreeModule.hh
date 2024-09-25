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
  TTree *energyTreeCZT;
  TTree *energyTreeHPGe;
  TBranch *branchEnergyDepCZT;
  TBranch *branchEnergyDepHPGe;
  TBranch *branchEvents;
  const char *aFilename;

public:
  TreeModule(const char *filename);
  ~TreeModule();
  TFile *getAFile() const { return aFile; };
  TTree *getHitsTree() const { return hitsTree; };
  TTree *getEnergyTreeCZT() const { return energyTreeCZT; };
  TTree *getEnergyTreeHPGe() const { return energyTreeHPGe; };
  TBranch *getBranchEnergyCZT() const { return branchEnergyDepCZT; };
  TBranch *getBranchEnergyHPGe() const { return branchEnergyDepHPGe; };
  TBranch *getBranchEvents() const { return branchEvents; };
  const char *getFilename() { return aFilename; };
  TString getFormattedFilename();
  TH1D *energySpectrumHist(const TString detectorName,
                           const char *fileExtension, bool isBroadened);
  TH1D *partialEnergySpectrumHist(const TString detectorName, double lowerBound,
                                  double upperBound, const char *fileExtension,
                                  bool isBroadened);
};

#endif
