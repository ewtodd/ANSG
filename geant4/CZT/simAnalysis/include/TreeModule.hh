#ifndef TREE_MODULE_H_INCLUDED
#define TREE_MODULE_H_INCLUDED

#include <TBranch.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TMath.h>
#include <TString.h>
#include <TTree.h>
#include <iostream>
#include <random>

class TreeModule {
private:
  TFile *aFile;
  TFile *broadenedFile;
  TTree *hitsTree;
  TTree *energyTreeCZT;
  TTree *energyTreeHPGe;
  TTree *energyTreeSiLi;
  TBranch *branchEnergyDepCZT;
  TBranch *branchEnergyDepHPGe;
  TBranch *branchEnergyDepSiLi;
  const char *aFilename;

  TH1D *createHistogram(TBranch *branch, const char *histName);
  TH1D *broadenedHist(TH1D *hist, const TString detectorName);
  void writeHistogramToTree(TTree *tree, TH1D *hist, const char *branchName);

public:
  TreeModule(const char *filename, const char *broadenedFilename = nullptr);
  ~TreeModule();
  void broadenAndStoreEnergy();
  const char *getFilename() { return aFilename; };
};

#endif // TREE_MODULE_H_INCLUDED
