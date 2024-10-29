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

  // make the CZT 1x1x1 cm3
  G4Box *solidCZT = new G4Box("solidCZT", 0.5 * cm, 0.5 * cm, 0.5 * cm);

  G4double len_wid = (1.25 / 2) * cm;
  G4double thick = (0.05 / 2) * cm;
  // G4double size = (cbrt(2) / 2) * cm;
  // G4double xsize0 = (cbrt(2) / 2) * cm;
  // G4double xsize1 = (cbrt(1. / 256) / 2) * cm;
  // G4double xsize2 = (0.069296 / 2) * cm;
  // G4double xsize3 = (0.0239385 / 2) * cm; // too thin?

  G4cout << "Thickness of Germanium target: " << thick * 2 / mm << " mm."
         << G4endl;

  G4cout << "Cross-sectional area of Germanium target: "
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

  logicDetectorCZT = new G4LogicalVolume(solidCZT, CZT, "logicDetectorCZT");

  fScoringVolumeCZT = logicDetectorCZT;

  // place CZT detector
  G4double offsetCZT =
      .75 * cm; // offset is 0.25 cm + 0.5 cm for half side length
  G4double posCZT =
      thick + offsetCZT; //+ radiusX for VCyl/HCyl, + xsizeX for Thin

  G4VPhysicalVolume *physCZT =
      new G4PVPlacement(0, G4ThreeVector(posCZT, 0., 10 * cm), logicDetectorCZT,
                        "physCZT", logicWorld, false, 0, true);

  // place Germanium sample
  G4VPhysicalVolume *physGe = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 10 * cm), logicGe, "physGe", logicWorld, false,
      0, true); // 0 for Thin and HCyl, rotationMatrix for VCyl

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
  SensitiveDetector *sensDetCZT = new SensitiveDetector("CZT Det");

  logicDetectorCZT->SetSensitiveDetector(sensDetCZT);
}
