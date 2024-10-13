#include "include/Analysis.hh"
#include "src/Analysis.cpp"

void macro() {
  gROOT->SetBatch(kTRUE);
  Analysis *analysis = new Analysis();
  analysis->loadFiles(true);

  // analysis->drawHists("CZT", false);
  // analysis->drawHists("CZT", true);
  // analysis->drawPartialHists("CZT", 64.75, 72.75, false);
  // analysis->drawPartialHists("CZT", 64.75, 72.75, true);
  //  analysis->drawHists("HPGe");
  //  analysis->drawPartialHists("HPGe", 68, 69.5);
  //  analysis->drawHists("SiLi");
  //  analysis->drawPartialHists("SiLi", 64.75, 72.75);
  delete analysis;
}
