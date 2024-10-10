#include "include/BroadAnalysis.hh"
#include "src/BroadAnalysis.cpp"

void macro() {
  gROOT->SetBatch(kTRUE);
  BroadAnalysis *broadAnalysis = new BroadAnalysis();
  broadAnalysis->loadFiles();
  broadAnalysis->drawFullHists("CZT");
  broadAnalysis->drawPartialHists("CZT", 55, 80);
  broadAnalysis->drawFullHists("HPGe");
  broadAnalysis->drawPartialHists("HPGe", 55, 80);
  broadAnalysis->drawFullHists("SiLi");
  broadAnalysis->drawPartialHists("SiLi", 55, 80);
  delete broadAnalysis;
}
