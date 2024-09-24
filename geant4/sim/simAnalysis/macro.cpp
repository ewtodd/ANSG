#include "include/Analysis.hh"
#include "src/Analysis.cpp"

void macro() {
  gROOT->SetBatch(kTRUE);
  Analysis *analysis = new Analysis();
  analysis->loadFiles();
  analysis->drawFullHists("CZT");
  analysis->drawPartialHists("CZT", 68.7, 68.8);
  delete analysis;
}
