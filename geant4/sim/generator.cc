#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {

  // constructor takes number of particles per event
  fParticleGun = new G4ParticleGun(1);
  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName = "neutron";
  G4ParticleDefinition *particle = particleTable->FindParticle(particleName);

  G4ThreeVector pos(0., 0., 0.);
  G4ThreeVector mom(0., 0., 1.);

  fParticleGun->SetParticlePosition(pos);
  fParticleGun->SetParticleMomentumDirection(mom);
  // I think this is correct?
  fParticleGun->SetParticleEnergy(0.025 * eV);
  fParticleGun->SetParticleDefinition(particle);
}

PrimaryGenerator::~PrimaryGenerator() { delete fParticleGun; }

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
