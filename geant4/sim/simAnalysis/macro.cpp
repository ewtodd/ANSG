#include "include/Analysis.hh"
#include "src/Analysis.cpp"

void macro() {
  gROOT->SetBatch(kTRUE);
  Analysis *analysis = new Analysis();
  analysis->loadFiles();
  analysis->drawHists("CZT", false);
  analysis->drawPartialHists("CZT", 64.75, 72.75, false, 66);
  // analysis->drawPartialHists("CZT", 64.75, 72.75, true, 66);
  //    analysis->drawHists("HPGe");
  //    analysis->drawPartialHists("HPGe", 68, 69.5);
  //    analysis->drawHists("SiLi");
  //    analysis->drawPartialHists("SiLi", 64.75, 72.75);
  delete analysis;
}
