#ifndef ANALYSIS_HH
#define ANALYSIS_HH

#include "../src/BroadTree.cpp"
#include "../src/TreeModule.cpp"
#include "BroadTree.hh"
#include "TreeModule.hh"
#include <TCanvas.h>
#include <TColor.h>
#include <TF1.h>
#include <THStack.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TString.h>

class Analysis {
public:
  Analysis();
  ~Analysis();
  void loadFiles(bool firstTime);
  void testHists(const TString detectorName);
  void drawHists(const TString detectorName, bool isNormed);
  void drawPartialHists(const TString detectorName, double lowerBound,
                        double upperBound, bool isNormed, int nbins);

  void fitGaussianToPeak(TLegend *legend, TH1D *hist, double peak, double range,
                         TString measurementTime);
  void ClearTHStack(THStack *stack);

private:
  TreeModule *tmSim0;
  TreeModule *tmSim1;
  TreeModule *tmSim2;
  TreeModule *tmSim3;
  TreeModule *tmSim4;
  TreeModule *tmSim5;

  BroadTree *tmBroad0;
  BroadTree *tmBroad1;
  BroadTree *tmBroad2;
  BroadTree *tmBroad3;
  BroadTree *tmBroad4;
  BroadTree *tmBroad5;

  THStack *CZTStack;
  THStack *HPGeStack;
  THStack *SiLiStack;

  THStack *CZTStackPartial;
  THStack *HPGeStackPartial;
  THStack *SiLiStackPartial;
};

#endif
