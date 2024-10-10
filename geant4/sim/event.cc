#include "event.hh"

EventAction::EventAction(RunAction *) {
  fEdepCZT = 0.;
  fEdepHPGe = 0.;
}
EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) {
  fEdepCZT = 0.;
  fEdepHPGe = 0.;
  fEdepSiLi = 0.;
}

void EventAction::EndOfEventAction(const G4Event *) {

  G4AnalysisManager *man = G4AnalysisManager::Instance();
  if (fEdepCZT > 0.0000001) {
    man->FillNtupleDColumn(1, 0, fEdepCZT);
    man->AddNtupleRow(1);
  }
  if (fEdepHPGe > 0.0000001) {
    man->FillNtupleDColumn(2, 0, fEdepHPGe);
    man->AddNtupleRow(2);
  }
  if (fEdepSiLi > 0.0000001) {
    man->FillNtupleDColumn(3, 0, fEdepSiLi);
    man->AddNtupleRow(3);
  }
}
