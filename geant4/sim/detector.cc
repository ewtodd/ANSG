#include "detector.hh"

SensitiveDetector::SensitiveDetector(G4String name)
    : G4VSensitiveDetector(name) {}
SensitiveDetector::~SensitiveDetector() {}
G4bool SensitiveDetector::ProcessHits(G4Step *aStep,
                                      G4TouchableHistory *ROhist) {
  G4Track *track = aStep->GetTrack();

  G4StepPoint *preStepPoint = aStep->GetPreStepPoint();
  G4StepPoint *postStepPoint = aStep->GetPostStepPoint();

  G4ThreeVector posParticle = preStepPoint->GetPosition();

  const G4VTouchable *touchable = aStep->GetPreStepPoint()->GetTouchable();

  G4VPhysicalVolume *physVol = touchable->GetVolume();
  G4ThreeVector posDetector = physVol->GetTranslation();

  G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

  // G4String particleName = track->GetParticleDefinition()->GetParticleName();
  // G4cout << "Particle name: " << particleName << G4endl;

  G4AnalysisManager *man = G4AnalysisManager::Instance();
  man->FillNtupleIColumn(0, 0, evt);
  // man->FillNtupleDColumn(0, 1, posDetector[0]);
  // man->FillNtupleDColumn(0, 2, posDetector[1]);
  // man->FillNtupleDColumn(0, 3, posDetector[2]);
  man->AddNtupleRow(0);

  return true;
}
