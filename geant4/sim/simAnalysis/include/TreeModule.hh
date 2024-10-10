#ifndef TREE_MODULE_H_INCLUDED
#define TREE_MODULE_H_INCLUDED
#include <TBranch.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMath.h>
#include <TString.h>
#include <TTree.h>
#include <cstring>
#include <iostream>
#include <random>

class TreeModule {

private:
  TFile *aFile;
  TTree *hitsTree;
  TTree *energyTreeCZT;
  TTree *energyTreeHPGe;
  TTree *energyTreeSiLi;
  TBranch *branchEnergyDepCZT;
  TBranch *branchEnergyDepHPGe;
  TBranch *branchEnergyDepSiLi;
  TBranch *branchEvents;
  const char *aFilename;

public:
  TreeModule(const char *filename);
  ~TreeModule();
  TFile *getAFile() const { return aFile; };
  TTree *getHitsTree() const { return hitsTree; };
  TTree *getEnergyTreeCZT() const { return energyTreeCZT; };
  TTree *getEnergyTreeHPGe() const { return energyTreeHPGe; };
  TTree *getEnergyTreeSiLi() const { return energyTreeSiLi; };
  TBranch *getBranchEnergyCZT() const { return branchEnergyDepCZT; };
  TBranch *getBranchEnergyHPGe() const { return branchEnergyDepHPGe; };
  TBranch *getBranchEnergySiLi() const { return branchEnergyDepSiLi; };
  TBranch *getBranchEvents() const { return branchEvents; };
  const char *getFilename() { return aFilename; };
  TString getFormattedFilename();
  TH1D *broadenedHist(TH1D *hist, const TString detectorName);
  TH1D *energySpectrumHist(const TString detectorName,
                           const char *fileExtension, bool isBroadened);
  void partialEnergySpectrumHist(const TString detectorName, double lowerBound,
                                 double upperBound, const char *fileExtension,
                                 bool isBroadened);
  TString generateRandomString();
  TLegend *fitGaussianToPeak(TH1D *hist);
};

#endif
