#include "include/Analysis.hh"
#include "src/Analysis.cpp"

void macro() {
  gROOT->SetBatch(kTRUE);
  Analysis *analysis = new Analysis();
  analysis->loadFiles();
  analysis->drawHists("SiLi", false);
  analysis->drawPartialHists("SiLi", 66, 71.5, false, 28);
  delete analysis;
}
