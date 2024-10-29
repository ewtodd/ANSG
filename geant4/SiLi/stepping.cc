#include "stepping.hh"

SteppingAction::SteppingAction(EventAction *eventAction) {
  fEventAction = eventAction;
}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step *step) {
  G4LogicalVolume *volume = step->GetPreStepPoint()
                                ->GetTouchableHandle()
                                ->GetVolume()
                                ->GetLogicalVolume();

  const DetectorConstruction *detectorConstruction =
      static_cast<const DetectorConstruction *>(
          G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  G4LogicalVolume *fScoringVolumeSiLi =
      detectorConstruction->GetScoringVolumeSiLi();

  if (volume != fScoringVolumeSiLi) {
    return;
  }

  if (volume == fScoringVolumeSiLi) {
    G4double edepSiLi = step->GetTotalEnergyDeposit();
    fEventAction->AddEdepSiLi(edepSiLi);
  }
}
