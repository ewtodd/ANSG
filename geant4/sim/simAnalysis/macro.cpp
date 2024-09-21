#include "include/Analysis.hh"
#include "src/Analysis.cpp"

void macro() {
  gROOT->SetBatch(kTRUE);
  Analysis *analysis = new Analysis();
  analysis->loadFiles();
  analysis->drawFullHists();
  analysis->drawPartialHists(68.7, 68.8);
  delete analysis;
}
