#ifndef PHYSICS_HH
#define PHYSICS_HH

#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4HadronPhysicsQGSP_BIC_AllHP.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4ThermalNeutrons.hh"
#include "G4VModularPhysicsList.hh"

class PhysicsList : public G4VModularPhysicsList {
public:
  PhysicsList();
  ~PhysicsList();
};

#endif
