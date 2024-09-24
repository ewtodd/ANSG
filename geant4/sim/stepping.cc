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

  G4LogicalVolume *fScoringVolumeCZT =
      detectorConstruction->GetScoringVolumeCZT();
  G4LogicalVolume *fScoringVolumeHPGe =
      detectorConstruction->GetScoringVolumeHPGe();

  if (volume != fScoringVolumeCZT && volume != fScoringVolumeHPGe) {
    return;
  }

  if (volume == fScoringVolumeCZT) {
    G4double edepCZT = step->GetTotalEnergyDeposit();
    fEventAction->AddEdepCZT(edepCZT);
  }
  if (volume == fScoringVolumeHPGe) {
    G4double edepHPGe = step->GetTotalEnergyDeposit();
    fEventAction->AddEdepHPGe(edepHPGe);
  }
}
