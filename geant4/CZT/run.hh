#ifndef RUN_HH
#define RUN_HH

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4UserRunAction.hh"

class RunAction : public G4UserRunAction {
public:
  RunAction();
  ~RunAction();

  virtual void BeginOfRunAction(const G4Run *);
  virtual void EndOfRunAction(const G4Run *);
};

#endif
