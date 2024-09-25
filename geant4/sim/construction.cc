#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  G4Material *Ge = nist->FindOrBuildMaterial("G4_Ge");

  // Cadmium for shielding!
  G4Material *Cd = nist->FindOrBuildMaterial("G4_Cd");

  // density value taken from Properties of Narrow Gap Cadmium-Based Compounds
  G4Material *CZT = new G4Material("CdZnTe", 5.811 * g / cm3, 3);
  CZT->AddElement(nist->FindOrBuildElement("Cd"), 1);
  CZT->AddElement(nist->FindOrBuildElement("Zn"), 1);
  CZT->AddElement(nist->FindOrBuildElement("Te"), 1);

  // constructor arguments are name and half of x,y,z length (in m)
  G4Box *solidWorld = new G4Box("solidWorld", 0.25 * m, 0.25 * m, 0.25 * m);

  // make it same size as from the thesis?
  G4Box *solidCZT = new G4Box("solidCZT", 1. * cm, 1. * cm, 0.75 * cm);

  //  make the germanium!
  G4double radius = 8 * mm;
  G4double halfHeight = 5 * mm;
  // G4double radius0 = 8 * mm;
  // G4double halfHeight0 = 5 * mm;
  G4double radius1 = 4 * mm;
  G4double halfHeight1 = 2.5 * mm;
  // G4double radius2 = 3 * mm;
  // G4double halfHeight2 = 2 * mm;
  // G4double radius3 = 2 * mm;
  // G4double halfHeight3 = 1.5 * mm;

  G4Tubs *solidGe =
      new G4Tubs("solidGe", 0., radius1, halfHeight1, 0 * deg, 360 * deg);
  G4Tubs *solidGeDet =
      new G4Tubs("solidGeDet", 0., radius, halfHeight, 0 * deg, 360 * deg);
  G4cout << solidGe->GetCubicVolume() / cm3 << G4endl;

  G4double CdThickness = 1 * mm;
  G4double CdRadius = radius + CdThickness;
  G4double CdHalfHeight = halfHeight + CdThickness;
  G4Tubs *solidCd =
      new G4Tubs("solidCd", radius, CdRadius, CdHalfHeight, 0 * deg, 360 * deg);

  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  // constructor arguments are rotation, origin location as G4ThreeVector,
  // G4LogicalVolume, name, whether to place in mother volume or not (0 for no),
  // boolean arguments, number of times to create and whether to check for
  // overlaps
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  G4LogicalVolume *logicGe = new G4LogicalVolume(solidGe, Ge, "logicGe");
  G4LogicalVolume *logicCd = new G4LogicalVolume(solidCd, Cd, "logicCd");

  logicDetectorHPGe = new G4LogicalVolume(solidGeDet, Ge, "logicDetectorHPGe");
  logicDetectorCZT = new G4LogicalVolume(solidCZT, CZT, "logicDetectorCZT");

  fScoringVolumeCZT = logicDetectorCZT;
  fScoringVolumeHPGe = logicDetectorHPGe;

  G4double offsetCZT = 1.25 * cm;
  G4double posCZT = radius1 + offsetCZT;
  G4VPhysicalVolume *physCZT =
      new G4PVPlacement(0, G4ThreeVector(posCZT, 0., 10 * cm), logicDetectorCZT,
                        "physCZT", logicWorld, false, 0, true);

  G4RotationMatrix *rotationMatrix = new G4RotationMatrix();
  rotationMatrix->rotateX(90. * deg);

  G4VPhysicalVolume *physGe =
      new G4PVPlacement(rotationMatrix, G4ThreeVector(0., 0., 10 * cm), logicGe,
                        "physGe", logicWorld, false, 0, true);

  G4double offsetHPGe = radius1 + CdRadius + 2.5;
  G4VPhysicalVolume *physCd = new G4PVPlacement(
      rotationMatrix, G4ThreeVector(-offsetHPGe * mm, 0., 10 * cm), logicCd,
      "physGe", logicWorld, false, 0, true);
  G4VPhysicalVolume *physGeDet = new G4PVPlacement(
      rotationMatrix, G4ThreeVector(-offsetHPGe * mm, 0., 10 * cm),
      logicDetectorHPGe, "physGe", logicWorld, false, 0, true);

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
  LM->RegisterLattice(physGeDet, GePhysical);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *sensDetHPGe = new SensitiveDetector("HPGe Det");
  SensitiveDetector *sensDetCZT = new SensitiveDetector("CZT Det");

  logicDetectorHPGe->SetSensitiveDetector(sensDetHPGe);
  logicDetectorCZT->SetSensitiveDetector(sensDetCZT);
}
