#ifndef EVENT_HH
#define EVENT_HH

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserEventAction.hh"
#include "Randomize.hh"
#include "run.hh"

class EventAction : public G4UserEventAction {
public:
  EventAction(RunAction *);
  ~EventAction();

  virtual void BeginOfEventAction(const G4Event *);
  virtual void EndOfEventAction(const G4Event *);

  void AddEdepSiLi(G4double edep) { fEdepSiLi += edep; }

private:
  G4double fEdepSiLi;
};

#endif
