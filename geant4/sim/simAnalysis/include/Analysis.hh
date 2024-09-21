#ifndef ANALYSIS_HH
#define ANALYSIS_HH

#include "../src/TreeModule.cpp"
#include "TLegend.h"
#include "TLine.h"
#include "TreeModule.hh"

class Analysis {
public:
  Analysis();
  ~Analysis();
  void loadFiles();
  void drawFullHists();
  void drawPartialHists(double lowerBound, double upperBound);
  void analysis();

private:
  TreeModule *tmGeo0;
  TreeModule *tmGeo1;
  TreeModule *tmGeo2;
  TreeModule *tmGeo3;
  // TreeModule *tmGeo4;
  // TreeModule *tmGeo5;
  // TreeModule *tmGeo6;
};

#endif
