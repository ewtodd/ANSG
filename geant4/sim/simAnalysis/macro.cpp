#include "include/Analysis.hh"
#include "src/Analysis.cpp"

void macro() {
  Analysis *analysis = new Analysis();
  analysis->loadFiles();
  analysis->drawFullHists();
  analysis->drawPartialHists(0, 100);

  delete analysis;
}
