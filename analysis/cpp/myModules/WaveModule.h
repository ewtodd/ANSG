#ifndef WAVE_MODULE_H_INCLUDED
#define WAVE_MODULE_H_INCLUDED

#include "TreeModule.h"
#include <TArrayS.h>
#include <TBranch.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TTree.h>
#include <iostream>

class WaveModule {
private:
  int waveLength;
  Double_t *wave;
  int numSampleBaseline;
  int triggerThreshold;
  int trigger;
  int longGate;
  int shortGate;
  int polarity;

  int numTriggered;
  bool isFixTrigger;

  bool isSmooth;

  bool isConfigured;

  struct energyEstimateModule {
    Double_t energy, energyShort, psp;
  };

  energyEstimateModule energyEstimate;

  int baselineEstimate;
  Short_t *waveRaw;
  Double_t *waveSmooth;
  TArrayS *arraySample;

public:
  WaveModule(TreeModule &tm);
  ~WaveModule();
  int GetWaveLength() const { return waveLength; };
  int GetNumSampleBaseline() const { return numSampleBaseline; };
  int GetBaselineEstimate() const { return baselineEstimate; };
  void EstimateBaseline(int numSampleBaseline);
  bool FindTrigger();
  void Configure(int numSampleBaselineArg, int triggerThresholdArg,
                 int triggerArg, int longGateArg, int shortGateArg,
                 int polarityArg, bool isSmoothArg, bool isVerboseArg);
  TGraph *GetWaveformGraph(TreeModule &tm, int index);
  void MovingAverage(Double_t *outputSignal, Short_t *inputSignal,
                     int waveLength, int windowSize);
  void Estimate();
  void Optimize(TreeModule &tm);
  void ProcessEJ309(TreeModule &tm);
  void DrawEJ309(TreeModule &tm, bool save, bool rand);
};

#endif
