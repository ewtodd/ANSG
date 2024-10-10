#ifndef BROADANALYSIS_HH
#define BROADANALYSIS_HH

#include "../src/TreeModule.cpp"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TreeModule.hh"

class BroadAnalysis {
public:
  BroadAnalysis();
  ~BroadAnalysis();
  void loadFiles();
  void drawFullHists(const TString detectorName);
  void drawPartialHists(const TString detectorName, double lowerBound,
                        double upperBound);
  void analysis();

private:
  TreeModule *tmSim;
  TreeModule *tmBroad;
};

#endif
