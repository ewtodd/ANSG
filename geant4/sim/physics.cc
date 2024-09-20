#include "physics.hh"

PhysicsList::PhysicsList() {
  RegisterPhysics(new G4EmStandardPhysics());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4RadioactiveDecayPhysics());
  RegisterPhysics(new G4HadronPhysicsQGSP_BIC_AllHP);
  RegisterPhysics(new G4ThermalNeutrons());
}
PhysicsList::~PhysicsList() {}
