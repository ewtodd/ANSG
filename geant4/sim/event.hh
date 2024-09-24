#ifndef EVENT_HH
#define EVENT_HH

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserEventAction.hh"
#include "run.hh"

class EventAction : public G4UserEventAction {
public:
  EventAction(RunAction *);
  ~EventAction();

  virtual void BeginOfEventAction(const G4Event *);
  virtual void EndOfEventAction(const G4Event *);

  void AddEdepCZT(G4double edep) { fEdepCZT += edep; }
  void AddEdepHPGe(G4double edep) { fEdepHPGe += edep; }

private:
  G4double fEdepCZT;
  G4double fEdepHPGe;
};

#endif
