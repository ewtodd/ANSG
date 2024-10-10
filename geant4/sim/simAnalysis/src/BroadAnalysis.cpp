#include "../include/BroadAnalysis.hh"

BroadAnalysis::BroadAnalysis() {
  tmSim = nullptr;
  tmBroad = nullptr;
}

BroadAnalysis::~BroadAnalysis() {
  delete tmSim;
  delete tmBroad;
}

void BroadAnalysis::loadFiles() {
  tmSim = new TreeModule("../est.root");
  tmBroad = new TreeModule("../est.root");
}

void BroadAnalysis::drawFullHists(const TString detectorName) {
  TH1D *histSim =
      tmSim->energySpectrumHist(detectorName, tmSim->getFilename(), false);
  TH1D *histBroad =
      tmBroad->energySpectrumHist(detectorName, tmSim->getFilename(), true);
  // Set axis properties for thicker lines and tick marks
  gStyle->SetLineWidth(2);
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(2);
  TCanvas *c1 = new TCanvas("c1", "c1", 2000, 1500);
  // Set the margins
  c1->SetLeftMargin(0.15);  // Set the left margin to 15% of the canvas width
  c1->SetRightMargin(0.05); // Set the right margin to 5% of the canvas width
  c1->SetTopMargin(0.1);    // Set the top margin to 10% of the canvas height
  c1->SetBottomMargin(
      0.15); // Set the bottom margin to 15% of the canvas height
  // Set fill colors and styles
  Short_t lineWidth = 2;
  histSim->SetLineColor(kRed);
  histSim->SetLineWidth(lineWidth);
  histBroad->SetLineWidth(lineWidth);

  // Draw histograms with fill styles
  histSim->Draw();

  // Create and draw vertical lines at specified values
  std::vector<double> lineValues = {68.752};
  for (double value : lineValues) {
    TLine *line = new TLine(value, 0, value, histSim->GetMaximum());
    line->SetLineColor(kBlack);
    line->SetLineStyle(2); // Dashed line
    line->Draw();
  }
  // Add the title using TLatex
  TLatex *latex = new TLatex();
  latex->SetNDC();
  latex->SetTextSize(0.04);
  latex->DrawLatex(0.5, 0.95, detectorName);
  // Update the canvas to show the drawings
  c1->Update();
  c1->Print("fullrange" + detectorName + ".png");

  delete c1;
  delete histSim;

  TCanvas *c2 = new TCanvas("c2", "c2", 2000, 1500);
  c2->SetLeftMargin(0.15);  // Set the left margin to 15% of the canvas width
  c2->SetRightMargin(0.05); // Set the right margin to 5% of the canvas width
  c2->SetTopMargin(0.1);    // Set the top margin to 10% of the canvas height
  c2->SetBottomMargin(
      0.15); // Set the bottom margin to 15% of the canvas height

  histBroad->Draw();
  for (double value : lineValues) {
    TLine *line = new TLine(value, 0, value, histBroad->GetMaximum());
    line->SetLineColor(kBlack);
    line->SetLineStyle(2); // Dashed line
    line->Draw();
  }
  // Add the title using TLatex
  latex->SetNDC();
  latex->SetTextSize(0.04);
  latex->DrawLatex(0.5, 0.95, detectorName);
  // Update the canvas to show the drawings
  c2->Update();
  c2->Print("fullrange" + detectorName + "_broad.png");

  delete histBroad;
  delete c2;
}

void BroadAnalysis::drawPartialHists(const TString detectorName,
                                     double lowerBound, double upperBound) {
  tmSim->partialEnergySpectrumHist(detectorName, lowerBound, upperBound,
                                   tmSim->getFilename(), false);
  tmBroad->partialEnergySpectrumHist(detectorName, lowerBound, upperBound,
                                     tmBroad->getFilename(), true);
}
