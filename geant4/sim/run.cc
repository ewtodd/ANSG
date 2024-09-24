#include "run.hh"

RunAction::RunAction() {
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  man->CreateNtuple("Hits", "Hits");
  man->CreateNtupleIColumn("fEvent");
  // man->CreateNtupleDColumn("fX");
  // man->CreateNtupleDColumn("fY");
  // man->CreateNtupleDColumn("fZ");
  man->FinishNtuple(0);

  man->CreateNtuple("EnergyCZT", "EnergyCZT");
  man->CreateNtupleDColumn("fEdepCZT");
  man->FinishNtuple(1);

  man->CreateNtuple("EnergyHPGe", "EnergyHPGe");
  man->CreateNtupleDColumn("fEdepHPGe");
  man->FinishNtuple(2);
}
RunAction::~RunAction() {}
void RunAction::BeginOfRunAction(const G4Run *run) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  G4int runNumber = run->GetRunID();
  std::stringstream strRunID;
  strRunID << runNumber;
  man->OpenFile("output" + strRunID.str() + ".root");
}
void RunAction::EndOfRunAction(const G4Run *) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->Write();
  man->CloseFile("output.root");
}
