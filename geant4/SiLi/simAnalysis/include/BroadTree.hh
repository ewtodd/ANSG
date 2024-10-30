#ifndef BROADTREE_MODULE_H_INCLUDED
#define BROADTREE_MODULE_H_INCLUDED
#include <TBranch.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TLine.h>
#include <TMath.h>
#include <TString.h>
#include <TTree.h>
#include <cstring>
#include <iostream>
#include <random>

class BroadTree {

private:
  TFile *aFile;
  TTree *energyTreeSiLi;
  TBranch *branchEnergyDepSiLi;
  const char *aFilename;

public:
  BroadTree(const char *filename);
  ~BroadTree();
  TFile *getAFile() const { return aFile; };
  TTree *getEnergyTreeSiLi() const { return energyTreeSiLi; };
  TBranch *getBranchEnergySiLi() const { return branchEnergyDepSiLi; };
  const char *getFilename() { return aFilename; };
  TString getFormattedFilename();
  TH1D *energySpectrumHist(const TString detectorName, double lowerBound,
                           double upperBound, int nbins);
  TString generateRandomString();
};

#endif
