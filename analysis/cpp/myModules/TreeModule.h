#ifndef TREE_MODULE_H_INCLUDED
#define TREE_MODULE_H_INCLUDED
#include <TBranch.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>

class TreeModule {

private:
  bool isCompassRaw;
  TFile *aFile;
  TTree *aTree;
  TBranch *branchPSP;
  TBranch *branchEnergy;
  TBranch *branchEnergyShort;
  TBranch *branchTimestamp;
  TBranch *branchSamples;
  const char *aFilename;
  struct histogramModule {
    TH1D *energyHist, pspHist;
    TH2D *pspLOHist;
  };

  histogramModule hists;

public:
  TreeModule(const char *filename, bool isCompassRawArg);
  ~TreeModule();
  TFile *getAFile() const { return aFile; };
  TTree *getATree() const { return aTree; };
  TBranch *getBranchPSP() const { return branchPSP; };
  TBranch *getBranchEnergy() const { return branchEnergy; };
  TBranch *getBranchEnergyShort() const { return branchEnergyShort; };
  TBranch *getBranchTimestamp() const { return branchTimestamp; };
  TBranch *getBranchSamples() const { return branchSamples; };
  const char *getFilename() { return aFilename; };
  TString getFormattedFilename(std::string filePrefix,
                               const char *fileExtension);
  void TimeDistTM(const char *fileExtension);
  void PSD_E_TM(const char *fileExtension);
  void PSPHist(const char *fileExtension, const int drawRaw);
};

#endif
