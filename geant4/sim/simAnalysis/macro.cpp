#include "include/Analysis.hh"
#include "src/Analysis.cpp"

void macro() {
  gROOT->SetBatch(kTRUE);
  Analysis *analysis = new Analysis();
  analysis->loadFiles();
  analysis->drawFullHists();
  analysis->drawPartialHists(50, 80);
  analysis->drawPartialHists(68.7, 68.8);
  analysis->drawPartialHists(5840, 5885);
  delete analysis;
}
