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
  G4double resCZT = 1.8 / 59.5;
  G4double resSiLi = 0.165 / 5.9;
  G4double resHPGe = 0.430 / 68.75;

  G4double FWHMCZT = resCZT * fEdepCZT;
  G4double FWHMHPGe = resHPGe * fEdepHPGe;
  G4double FWHMSiLi = resSiLi * fEdepSiLi;

  G4double fEdepCZTBroad =
      fEdepCZT * G4RandGauss::shoot(fEdepCZT, FWHMCZT / 2.35);
  G4double fEdepHPGeBroad =
      fEdepHPGe * G4RandGauss::shoot(fEdepHPGe, FWHMHPGe / 2.35);
  G4double fEdepSiLiBroad =
      fEdepSiLi * G4RandGauss::shoot(fEdepSiLi, FWHMSiLi / 2.35);

  if (fEdepCZT > 0.0000001) {
    man->FillNtupleDColumn(1, 0, fEdepCZT);
    man->FillNtupleDColumn(1, 1, fEdepCZTBroad);
    man->AddNtupleRow(1);
  }
  if (fEdepHPGe > 0.0000001) {
    man->FillNtupleDColumn(2, 0, fEdepHPGe);
    man->FillNtupleDColumn(2, 1, fEdepHPGeBroad);
    man->AddNtupleRow(2);
  }
  if (fEdepSiLi > 0.0000001) {
    man->FillNtupleDColumn(3, 0, fEdepSiLi);
    man->FillNtupleDColumn(3, 1, fEdepSiLiBroad);
    man->AddNtupleRow(3);
  }
}
