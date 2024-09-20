#include "event.hh"

EventAction::EventAction(RunAction *) { fEdep = 0.; }
EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) { fEdep = 0.; }

void EventAction::EndOfEventAction(const G4Event *) {

  G4AnalysisManager *man = G4AnalysisManager::Instance();

  if (fEdep / keV > 0.1) {
    man->FillNtupleDColumn(1, 0, fEdep);
    man->AddNtupleRow(1);
  }
}
