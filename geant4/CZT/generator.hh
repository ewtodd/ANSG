#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4GeneralParticleSource.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGenerator();
  ~PrimaryGenerator();

  virtual void GeneratePrimaries(G4Event *);

private:
  G4GeneralParticleSource *fParticleSource;
};

#endif
