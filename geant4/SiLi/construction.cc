#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  G4Material *Ge = nist->FindOrBuildMaterial("G4_Ge");
  // Silicon for SiLi
  G4Material *Si = nist->FindOrBuildMaterial("G4_Si");

  // constructor arguments are name and half of x,y,z length (in m)
  G4Box *solidWorld = new G4Box("solidWorld", 0.25 * m, 0.25 * m, 0.25 * m);

  G4double radius = 5 * mm;
  G4double halfHeight = 2.5 * mm;
  // make SiLi  - size is same as Mirion Technologies detector
  G4Tubs *solidSi =
      new G4Tubs("solidSi", 0., radius, halfHeight, 0 * deg, 360 * deg);

  G4double len_wid = (1.25 / 2) * cm;
  G4double thick = (0.05 / 2) * cm;
  // G4double size = (cbrt(2) / 2) * cm;
  // G4double xsize0 = (cbrt(2) / 2) * cm;
  // G4double xsize1 = (cbrt(1. / 256) / 2) * cm;
  // G4double xsize2 = (0.069296 / 2) * cm;
  // G4double xsize3 = (0.0239385 / 2) * cm; // too thin?

  G4cout << "Thickness of Germanium target: " << thick * 2 / mm << " mm."
         << G4endl;

  G4cout << "Crosssectional area of Germanium target: "
         << len_wid * thick * 4 / cm2 << "cm2." << G4endl;

  // Make the Germanium target:
  G4Box *solidGe = new G4Box("solidGe", thick, len_wid, len_wid);
  G4cout << "Volume of Germanium target: " << solidGe->GetCubicVolume() / cm3
         << " cm3." << G4endl;

  // Make logical world
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  // constructor arguments are rotation, origin location as G4ThreeVector,
  // G4LogicalVolume, name, whether to place in mother volume or not (0 for no),
  // boolean arguments, number of times to create and whether to check for
  // overlaps
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  // make the logical volume for germanium sample
  G4LogicalVolume *logicGe = new G4LogicalVolume(solidGe, Ge, "logicGe");

  // detector logical volumes
  logicDetectorSiLi = new G4LogicalVolume(solidSi, Si, "logicDetectorSiLi");

  fScoringVolumeSiLi = logicDetectorSiLi;

  // place Germanium sample
  G4VPhysicalVolume *physGe = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 10 * cm), logicGe, "physGe", logicWorld, false,
      0, true); // 0 for Thin and HCyl, rotationMatrix for VCyl

  // place SiLi Detector
  G4RotationMatrix *rotationMatrix = new G4RotationMatrix();
  rotationMatrix->rotateY(90. * deg);

  G4double offsetSi = thick + 2.5 + halfHeight; // offset in y direction in mm

  G4VPhysicalVolume *physSi = new G4PVPlacement(
      rotationMatrix, G4ThreeVector(offsetSi * mm, 0., 10 * cm),
      logicDetectorSiLi, "physSi", logicWorld, false, 0, true);

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
  SensitiveDetector *sensDetSiLi = new SensitiveDetector("Si Det");

  logicDetectorSiLi->SetSensitiveDetector(sensDetSiLi);
}
