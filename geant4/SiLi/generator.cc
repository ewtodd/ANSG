#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {

  // constructor takes number of particles per event
  // fParticleGun = new G4ParticleGun(1);
  fParticleSource = new G4GeneralParticleSource();

  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName = "neutron";
  G4ParticleDefinition *particle = particleTable->FindParticle(particleName);
  fParticleSource->SetParticleDefinition(particle);
}

PrimaryGenerator::~PrimaryGenerator() { delete fParticleSource; }

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {

  fParticleSource->GeneratePrimaryVertex(anEvent);
}
