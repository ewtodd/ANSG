#include "myModules/TreeModule.cpp"
#include "myModules/TreeModule.h"
#include "myModules/WaveModule.cpp"
#include "myModules/WaveModule.h"
#include <TLine.h>
#include <iostream>

void analysis() {
  // make graphics not show up
  gROOT->SetBatch(kTRUE);
  TreeModule *tmCH1 = new TreeModule(
      "07302024_Cf_50cm_1/RAW/SDataR_CH1@DT5730B_1090_07302024_Cf_50cm_1.root");
  WaveModule *wmCH1 = new WaveModule(*tmCH1);
  // tmCH1->TimeDistTM(".png");
  //tmCH1->PSD_E_TM(".png");
TreeModule *tmCH2 = new TreeModule(
      "07302024_Cf_100cm_0/RAW/SDataR_CH1@DT5730B_1090_07302024_Cf_100cm_0.root");
  WaveModule *wmCH2 = new WaveModule(*tmCH2);

  //wmCH1->Configure(64, 20, 10, 75, 15, -1, false);
  //    wmCH1->Optimize(*tmCH1);
  //wmCH1->ProcessEJ309(*tmCH1);
    wmCH1->DrawEJ309(*tmCH1);
  //TreeModule *tmCH1Processed = new TreeModule(
     // "SDataR_CH1@DT5730B_1090_07302024_Cf_50cm_1_PROCESSED.root", false);
 wmCH2->DrawEJ309(*tmCH2);

  //tmCH1Processed->PSD_E_TM(".png");

  delete wmCH1;
  delete tmCH1;
}
