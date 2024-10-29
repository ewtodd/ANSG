#include "event.hh"

EventAction::EventAction(RunAction *) { fEdepSiLi = 0.; }
EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) { fEdepSiLi = 0.; }

void EventAction::EndOfEventAction(const G4Event *) {

  G4AnalysisManager *man = G4AnalysisManager::Instance();
  if (fEdepSiLi > 0.0000001) {
    man->FillNtupleDColumn(1, 0, fEdepSiLi);
    man->AddNtupleRow(1);
  }
}
