// include detector construction files
#include "construction.hh"

// include physics files
#include "physics.hh"

// include actions
#include "action.hh"

// skeleton files
#include "G4MTRunManager.hh"
#include "G4RunManager.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4VisManager.hh"

int main(int argc, char **argv) {

#ifdef G4MULTITHREADED
  G4MTRunManager *runManager = new G4MTRunManager();
#else
  G4RunManager *runManager = new G4RunManager();
#endif
  runManager->SetUserInitialization(new DetectorConstruction());
  runManager->SetUserInitialization(new PhysicsList());
  runManager->SetUserInitialization(new ActionInitialization());

  G4UIExecutive *ui = 0;

  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }

  G4VisManager *visManager = nullptr;
  G4UImanager *uiManager = G4UImanager::GetUIpointer();

  if (ui) {
    runManager->Initialize();
    visManager = new G4VisExecutive();
    visManager->Initialize();
    uiManager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    delete ui;
  } else {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    uiManager->ApplyCommand(command + fileName);
  }

  delete visManager;
  delete runManager;

  return 0;
}
