#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  G4Material *Ge = nist->FindOrBuildMaterial("G4_Ge");

  // density value taken from Properties of Narrow Gap Cadmium-Based Compounds
  G4Material *CZT = new G4Material("CdZnTe", 5.811 * g / cm3, 3);
  CZT->AddElement(nist->FindOrBuildElement("Cd"), 1);
  CZT->AddElement(nist->FindOrBuildElement("Zn"), 1);
  CZT->AddElement(nist->FindOrBuildElement("Te"), 1);

  // constructor arguments are name and half of x,y,z length (in m)
  G4Box *solidWorld = new G4Box("solidWorld", 0.25 * m, 0.25 * m, 0.25 * m);

  // make it same size as from the thesis?
  G4Box *solidCZT = new G4Box("solidCZT", 1. * cm, 1. * cm, 0.75 * cm);

  G4double size = sqrt(2) / 2;
  // G4double xsize0 = sqrt(2) / 2;
  // G4double xsize1 = sqrt(2) / 4;
  // G4double xsize2 = sqrt(2) / 8;
  // G4double xsize3 = sqrt(2) / 16;
  // G4double xsize4 = sqrt(2) / 32;
  // G4double xsize5 = sqrt(2) / 64;
  G4double xsize6 = sqrt(2) / 128;
  // make the germanium!
  G4Box *solidGe = new G4Box("solidGe", xsize6 * cm, size * cm, size * cm);

  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  // constructor arguments are rotation, origin location as G4ThreeVector,
  // G4LogicalVolume, name, whether to place in mother volume or not (0 for no),
  // boolean arguments, number of times to create and whether to check for
  // overlaps
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  G4double offset = xsize6 + 1;
  logicDetectorHPGe = new G4LogicalVolume(solidGe, Ge, "logicDetectorHPGe");
  logicDetectorCZT = new G4LogicalVolume(solidCZT, CZT, "logicDetectorCZT");

  fScoringVolume = logicDetectorCZT;

  G4VPhysicalVolume *physCZT = new G4PVPlacement(
      0, G4ThreeVector(offset * cm, 0., 10 * cm), logicDetectorCZT, "physCZT",
      logicWorld, false, 0, true);

  G4VPhysicalVolume *physGe =
      new G4PVPlacement(0, G4ThreeVector(0., 0., 10 * cm), logicDetectorHPGe,
                        "physGe", logicWorld, false, 0, true);

  //
  // Germanium lattice information
  //

  // G4LatticeManager gives physics processes access to lattices by volume
  G4LatticeManager *LM = G4LatticeManager::GetLatticeManager();
  G4LatticeLogical *GeLogical = LM->LoadLattice(Ge, "Ge");

  // G4LatticePhysical assigns G4LatticeLogical a physical orientation
  G4LatticePhysical *GePhysical =
      new G4LatticePhysical(GeLogical, physGe->GetFrameRotation());
  LM->RegisterLattice(physGe, GePhysical);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  // SensitiveDetector *sensDetHPGe = new SensitiveDetector("HPGe Det");
  SensitiveDetector *sensDetCZT = new SensitiveDetector("CZT Det");

  // logicDetectorHPGe->SetSensitiveDetector(sensDetHPGe);
  logicDetectorCZT->SetSensitiveDetector(sensDetCZT);
}
