#include "WaveModule.h"
#include "PlotFunctions.cpp"
#include "TreeModule.h"
#include <TArrayS.h>
#include <TBranch.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TF1.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TLine.h>
#include <TRandom2.h>
#include <TString.h>
#include <TTree.h>
#include <iostream>
#include <ostream>

// create constructor for the WaveModule
WaveModule::WaveModule(TreeModule &tm)
    : waveLength(0), wave(nullptr), numSampleBaseline(0), triggerThreshold(0),
      trigger(0), longGate(0), shortGate(0), polarity(0), numTriggered(0),
      isFixTrigger(false), isSmooth(false), baselineEstimate(0),
      waveRaw(nullptr), arraySample(nullptr) {
  energyEstimateModule energyEstimate;
  energyEstimate.energy = 0.0;
  energyEstimate.energyShort = 0.0;
  energyEstimate.psp = 0.0;
  tm.getBranchSamples()->SetAddress(&arraySample);
  tm.getBranchSamples()->GetEntry(0);
  waveLength = arraySample->GetSize();
  waveRaw = new Short_t[waveLength];
  arraySample->fArray = waveRaw;
  wave = new Double_t[waveLength];
  waveSmooth = new Double_t[waveLength];
}

// create destructor for the WaveModule
WaveModule::~WaveModule() {
  std::cout << "Deleting WaveModule..." << std::endl;
  delete[] wave;
  delete[] waveRaw;
  std::cout << "Done." << std::endl;
}

// function to estimate background for a given nunmber of samples
void WaveModule::EstimateBaseline(int numSampleBaselineArg) {
  numSampleBaseline = numSampleBaselineArg;
  baselineEstimate = 0;
  // loop to add to background estimate
  for (int i = 0; i < numSampleBaseline; i++) {
    baselineEstimate += waveRaw[i];
  }
  // divide by numSamples so we are getting an average
  baselineEstimate /= numSampleBaseline;
}

bool WaveModule::FindTrigger() {
  if (!isFixTrigger) {
    if (isSmooth) {
      MovingAverage(waveSmooth, waveRaw, waveLength, 7);
      for (int i = numSampleBaseline; i < waveLength; i++) {
        if (polarity == -1) {
          // Check to see if waveRaw at this index is large enough over baseline
          // to be considered an event. If so, trigger for that index.
          if (waveSmooth[i] < baselineEstimate - triggerThreshold) {
            trigger = i;
            numTriggered++;
            return true;
          }
        } else if (polarity == 1) {
          if (waveSmooth[i] > baselineEstimate + triggerThreshold) {
            trigger = i;
            numTriggered++;
            return true;
          }
        } else {
          std::cerr << "Set proper polarity." << std::endl;
          return false;
        }
      }
      trigger = -1;
      return false;
    } else {
      for (int i = numSampleBaseline; i < waveLength; i++) {
        if (polarity == -1) {
          // Check to see if waveRaw at this index is large enough over baseline
          // to be considered an event. If so, trigger for that index.
          if (waveRaw[i] < baselineEstimate - triggerThreshold) {
            trigger = i;
            numTriggered++;
            return true;
          }
        } else if (polarity == 1) {
          if (waveRaw[i] > baselineEstimate + triggerThreshold) {
            trigger = i;
            numTriggered++;
            return true;
          }
        } else {
          std::cerr << "Set proper polarity." << std::endl;
          return false;
        }
      }
      trigger = -1;
      return false;
    }
  } else {
    numTriggered++;
    return true;
  }
}

// set many parameters for the WaveModule at once
void WaveModule::Configure(int numSampleBaselineArg, int triggerThresholdArg,
                           int triggerArg, int longGateArg, int shortGateArg,
                           int polarityArg, bool isSmoothArg = false,
                           bool isVerboseArg = true) {
  numSampleBaseline = numSampleBaselineArg;
  triggerThreshold = triggerThresholdArg;
  trigger = triggerArg;
  longGate = longGateArg;
  shortGate = shortGateArg;
  isSmooth = isSmoothArg;
  if (!(polarityArg == 1 || polarityArg == -1)) {
    std::cerr << "Polarity must be either 1 or -1." << std::endl;
  } else {
    polarity = polarityArg;
  }
  if (isVerboseArg) {
    std::cout << "Configuration is complete." << std::endl;
  }
  isConfigured = true;
}

// function to return a TGraph containing raw waveform at given index
TGraph *WaveModule::GetWaveformGraph(TreeModule &tm, int index = 0) {
  tm.getBranchSamples()->GetEntry(index);
  Double_t *time = new Double_t[waveLength];
  for (int i = 0; i < waveLength; i++) {
    wave[i] = isSmooth ? waveSmooth[i] : waveRaw[i];

    // time is 2 times index because of sampling rate
    time[i] = i * 2;
  }
  TGraph *g = new TGraph(waveLength, time, wave);
  delete[] time;
  return g;
}

// function to smooth the waveform using a moving average
void WaveModule::MovingAverage(Double_t *outputSignal, Short_t *inputSignal,
                               int waveLength, int windowSize) {
  for (int i = 0; i < waveLength; i++) {
    double sum = 0.0;
    int count = 0;

    for (int j = i - windowSize / 2; j <= i + windowSize / 2; j++) {
      if (j >= 0 && j < waveLength) {
        sum += inputSignal[j];
        count++;
      }
    }

    outputSignal[i] = sum / count;
  }
}

// function to estimate psp/light output
void WaveModule::Estimate() {
  EstimateBaseline(numSampleBaseline);
  energyEstimate.energy = 0.0;
  energyEstimate.energyShort = 0.0;
  energyEstimate.psp = 0.0;
  int end, endShort;
  if (FindTrigger()) {
    end = (trigger + longGate) < waveLength ? trigger + longGate : waveLength;
    endShort = trigger + shortGate;
    for (int i = trigger; i < end; i++) {
      if (polarity == -1) {
        energyEstimate.energy += !isSmooth ? baselineEstimate - waveRaw[i]
                                           : baselineEstimate - waveSmooth[i];
        if (i < endShort)
          energyEstimate.energyShort += !isSmooth
                                            ? baselineEstimate - waveRaw[i]
                                            : baselineEstimate - waveSmooth[i];
      } else if (polarity == 1) {
        energyEstimate.energy += !isSmooth ? waveRaw[i] - baselineEstimate
                                           : waveSmooth[i] - baselineEstimate;
        if (i < endShort)
          energyEstimate.energyShort += !isSmooth
                                            ? waveRaw[i] - baselineEstimate
                                            : waveSmooth[i] - baselineEstimate;
      }
    }
    energyEstimate.psp = (energyEstimate.energy - energyEstimate.energyShort) /
                         energyEstimate.energy;
  }
}

// Find parameters of WM that maximize the FOM
// in the PSP histogram
void WaveModule::Optimize(TreeModule &tm) {
  // TO DO: Store stuff in processed file
  // TFile* processed = new TFile(tm.getFormattedFilename("","_PROCESSED.root"),
  // "RECREATE"); TTree* data = new
  // TTree(tm.getFormattedFilename("",""),tm.getFormattedFilename("",""));
  int numEntries = tm.getBranchSamples()->GetEntries() / 10;
  double bestFOM = 0.0;
  double currentFOM = 0.0;
  int bestShort = shortGate, bestLong = longGate;
  int iterations = 0;
  double bestGammaConst = 0.0;
  double bestGammaMu = 0.0;
  double bestGammaSigma = 0.0;
  double bestNeutronConst = 0.0;
  double bestNeutronMu = 0.0;
  double bestNeutronSigma = 0.0;
  double gammaConst = 0.0;
  double gammaMu = 0.0;
  double gammaSigma = 0.0;
  double neutronConst = 0.0;
  double neutronMu = 0.0;
  double neutronSigma = 0.0;
  TCanvas *c1 = new TCanvas("c1", "Canvas", 1600, 1200);
  SetCanvasMargins(c1);
  TH1D *pspHist =
      new TH1D("pspHist", "Pulse Shape Parameter; PSP; Entries", 1e4, 0, 1);
  // define our own functions
  TF1 *gamma = new TF1("gamma", "gaus", 0, 0.25);
  TF1 *neutron = new TF1("neutron", "gaus", 0.175, 0.4);
  // gamma->SetParameter(0, 4400);
  // gamma->SetParLimits(0, 3950, 4500);
  gamma->SetParLimits(1, 0, 0.25);
  // neutron->SetParameter(0,475);
  // neutron->SetParLimits(0, 200, 525);
  neutron->SetParLimits(1, 0.175, 0.4);
  int nShort = shortGate;
  int nLong = longGate;
  for (int s = (nShort - 4); s < (nShort + 5); s++) {
    for (int l = (nLong - 7); l < (nLong + 8); l++) {
      if (iterations % 10 == 0) {
        std::cout << "Iteration number: " << iterations << std::endl;
      }
      longGate = l;
      shortGate = s;
      pspHist->Reset();
      for (int i = 0; i < numEntries; i++) {
        tm.getBranchSamples()->GetEntry(i);
        Estimate();
        pspHist->Fill(energyEstimate.psp);
      }
      if (iterations != 0) {
        gamma->SetParameter(0, gammaConst);
        gamma->SetParameter(1, gammaMu);
        gamma->SetParameter(2, gammaSigma);
        neutron->SetParameter(0, neutronConst);
        neutron->SetParameter(1, neutronMu);
        neutron->SetParameter(3, neutronSigma);
      }
      pspHist->Fit("gamma", "QR");
      pspHist->Fit("neutron", "QR");
      gammaConst = gamma->GetParameter(0);
      gammaMu = gamma->GetParameter(1);
      gammaSigma = gamma->GetParameter(2);
      neutronConst = neutron->GetParameter(0);
      neutronMu = neutron->GetParameter(1);
      neutronSigma = neutron->GetParameter(2);
      if (gammaConst < neutronConst) {
        std::cerr << "Bad fit. Stopping..." << std::endl;
      }
      currentFOM =
          TMath::Abs(gammaMu - neutronMu) / (gammaSigma + neutronSigma);

      if (iterations == 0) {
        bestFOM = currentFOM;
        bestGammaConst = gammaConst;
        bestGammaMu = gammaMu;
        bestGammaSigma = gammaSigma;
        bestNeutronConst = neutronConst;
        bestNeutronMu = neutronMu;
        bestNeutronSigma = bestNeutronMu;
      } else if (currentFOM > bestFOM) {
        bestShort = s;
        bestLong = l;
        bestFOM = currentFOM;
        bestGammaConst = gammaConst;
        bestGammaMu = gammaMu;
        bestGammaSigma = gammaSigma;
        bestNeutronConst = neutronConst;
        bestNeutronMu = neutronMu;
        bestNeutronSigma = bestNeutronMu;
      }
      iterations++;
      c1->Clear();
    }
  }

  std::cout << "Configuring for best values found... " << std::endl;
  Configure(64, 20, 10, bestLong, bestShort, -1, false);
  std::cout << "Drawing new plot..." << std::endl;
  pspHist->Reset();
  for (int i = 0; i < numEntries; i++) {
    tm.getBranchSamples()->GetEntry(i);
    Estimate();
    pspHist->Fill(energyEstimate.psp);
  }
  pspHist->Draw();
  gamma->SetParameter(0, bestGammaConst);
  gamma->SetParameter(1, bestGammaMu);
  gamma->SetParameter(2, bestGammaSigma);
  neutron->SetParameter(0, bestNeutronConst);
  neutron->SetParameter(1, bestNeutronMu);
  neutron->SetParameter(2, bestNeutronSigma);
  std::cout << "Best gamma mu: " << bestGammaMu << std::endl;
  std::cout << "Best neutron mu: " << bestNeutronMu << std::endl;
  std::cout << "Best FOM: " << bestFOM << std::endl;
  pspHist->Fit("gamma", "R");
  pspHist->Fit("neutron", "R");
  gamma->SetLineColor(kRed);
  gamma->SetLineWidth(3);
  neutron->SetLineColor(kGreen);
  neutron->SetLineWidth(3);
  TLegend *legend = new TLegend(0.5, 0.5, 0.7, 0.7);
  legend->AddEntry(gamma, "Gammas", "l");
  legend->AddEntry(neutron, "Neutrons", "l");
  gamma->Draw("same");
  neutron->Draw("same");
  legend->Draw();
  c1->Print("new.png");
  delete gamma;
  delete neutron;
  delete pspHist;
  delete c1;
}

// function to process raw data from EJ309 and create a ROOT file
// that can be manipulated with TreeModule functions
void WaveModule::ProcessEJ309(TreeModule &tm) {

  std::cout << "Number of samples: " << waveLength << std::endl;

  if (!isConfigured) {
    std::cerr << "WaveModule is not configured. Configure the WaveModule "
                 "before using this function."
              << std::endl;
  }
  TFile *processed =
      new TFile(tm.getFormattedFilename("", "_PROCESSED.root"), "RECREATE");
  TTree *data = new TTree(tm.getFormattedFilename("", ""),
                          tm.getFormattedFilename("", ""));
  int numEntries = tm.getBranchSamples()->GetEntries();
  data->Branch("PSP", &energyEstimate.psp, "PSP/D");
  data->Branch("Energy", &energyEstimate.energy, "Energy/D");
  data->Branch("EnergyShort", &energyEstimate.energyShort, "EnergyShort/D");
  // Long64_t timestamp;
  // tm.getBranchTimestamp()->SetAddress(&timestamp);
  //  include the timestamps and raw data in the processed data file!
  // data->Branch("Timestamp", &timestamp, "Timestamp/L");

  // TO DO: get samples array to work!

  // std::string branchDesc = "Samples[" + std::to_string(waveLength) + "]/S";
  // data->Branch("Samples", &arraySample, branchDesc.c_str());
  for (int i = 0; i < numEntries; i++) {
    // tm.getBranchTimestamp()->GetEntry(i);
    tm.getBranchSamples()->GetEntry(i);
    Estimate();
    data->Fill();
  }
  processed->Write();
  processed->Close();
  std::cout << "Processing complete. Saving new ROOT file..." << std::endl;
}

// DEPRECATED: function to draw histograms from raw data from EJ309
// Instead use ProcessEJ309 and TreeModule functions
void WaveModule::DrawEJ309(TreeModule &tm, bool save = true,
                           bool rand = false) {
  std::cout << "Number of samples: " << waveLength << std::endl;
  Configure(64, 20, 10, 100, 15, -1, false);
  // Configure(32,100,10,200,36,-1,true);
  int numEntries = tm.getBranchSamples()->GetEntries();
  TH1D *pspHist =
      new TH1D("pspHist", "Pulse Shape Parameter; PSP; Entries", 1e4, 0, 1);
  TH1D *eHist = new TH1D("eHist", "Light Output; LO; Entries", 256, 0, 4099);
  TH1D *eShortHist = new TH1D(
      "eShortHist", "Light Output Short; LO Short; Entries", 256, 0, 4099);
  TCanvas *c1 = new TCanvas("c1", "Canvas", 2000, 1600);
  SetCanvasMargins(c1);
  int index1, index2, index3;
  TRandom2 rnd;
  // 0 for random seed every time
  rnd.SetSeed(0);
  // generate 3 random indices to sample raw waveforms
  index1 = rand ? rnd.Uniform(0, numEntries) : 1;
  index2 = rand ? rnd.Uniform(0, numEntries) : 2;
  index3 = rand ? rnd.Uniform(0, numEntries) : 3;
  bool fake = false;
  for (int i = 0; i < numEntries; i++) {
    tm.getBranchSamples()->GetEntry(i);
    Estimate();
    // save raw raveform for the randomly generated indices
    if ((i == index1 | i == index2 | i == index3) && fake) {
      TGraph *g = GetWaveformGraph(tm, i);
      TString graphname =
          TString::Format("Sample: %d; Time (ps); ADC Channel", i);
      g->SetTitle(graphname);
      g->Draw();
      TString filename = TString::Format("%d.png", i);
      // Manually calculate the maximum and minimum y-values
      const double *yValues = g->GetY();
      double ymax = yValues[0];
      double ymin = yValues[0];

      for (int j = 1; j < waveLength; j++) {
        if (yValues[j] > ymax)
          ymax = yValues[j];
        if (yValues[j] < ymin)
          ymin = yValues[j];
      }
      TLine *trigline = new TLine(trigger * 2, ymin, trigger * 2, ymax);
      TLine *shortline = new TLine(trigger * 2 + shortGate * 2, ymin,
                                   trigger * 2 + shortGate * 2, ymax);
      TLine *longline = new TLine(trigger * 2 + longGate * 2, ymin,
                                  trigger * 2 + longGate * 2, ymax);
      TLine *baseline =
          new TLine(0, baselineEstimate, waveLength * 2, baselineEstimate);
      trigline->SetLineColor(kRed);
      trigline->SetLineWidth(2);
      trigline->Draw();
      shortline->SetLineColor(kBlue);
      shortline->SetLineWidth(2);
      shortline->Draw();
      longline->SetLineWidth(2);
      longline->Draw();
      longline->SetLineColor(kGreen);
      baseline->Draw();
      baseline->SetLineWidth(2);
      TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9);
      legend->AddEntry(g, "Raw Data", "l");
      legend->AddEntry(trigline, "Trigger", "l");
      legend->AddEntry(shortline, "Short Gate", "l");
      legend->AddEntry(longline, "Long Gate", "l");
      legend->AddEntry(baseline, "Baseline Estimate", "l");
      legend->Draw();
      if (save) {
        c1->Print(filename);
      }
      c1->Clear();
      delete g;
    }
    pspHist->Fill(energyEstimate.psp);
    eHist->Fill(energyEstimate.energy);
    eShortHist->Fill(energyEstimate.energyShort);
  }

  pspHist->Draw();
  if (save) {
    c1->Print("PSP.png");
  }
  c1->Clear();
  eHist->Draw();
  if (save) {
    c1->Print("LO.png");
  }
  c1->Clear();
  eShortHist->Draw();
  if (save) {
    c1->Print("LOShort.png");
  }

  // memory handling!
  delete pspHist;
  delete eHist;
  delete eShortHist;
  delete c1;
}
