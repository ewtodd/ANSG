#include "event.hh"

EventAction::EventAction(RunAction *) { fEdepCZT = 0.; }
EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) { fEdepCZT = 0.; }

void EventAction::EndOfEventAction(const G4Event *) {

  G4AnalysisManager *man = G4AnalysisManager::Instance();
  if (fEdepCZT > 0.0000001) {
    man->FillNtupleDColumn(1, 0, fEdepCZT);
    man->AddNtupleRow(1);
  }
}
