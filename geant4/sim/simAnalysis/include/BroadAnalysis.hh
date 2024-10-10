#ifndef BROADANALYSIS_HH
#define BROADANALYSIS_HH

#include "../src/TreeModule.cpp"
#include "TreeModule.hh"
#include <TCanvas.h>
#include <TF1.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TString.h>

class BroadAnalysis {
public:
  BroadAnalysis();
  ~BroadAnalysis();
  void loadFiles();
  void drawFullHists(const TString detectorName);
  void drawPartialHists(const TString detectorName, double lowerBound,
                        double upperBound);
  void analysis();
  TLegend *fitGaussianToPeak(TH1D *hist, double peak, double range);

private:
  TreeModule *tmSim;
};

#endif
