#include "../include/BroadAnalysis.hh"

BroadAnalysis::BroadAnalysis() { tmSim = nullptr; }

BroadAnalysis::~BroadAnalysis() { delete tmSim; }

void BroadAnalysis::loadFiles() { tmSim = new TreeModule("../1E9.root"); }

void BroadAnalysis::drawFullHists(const TString detectorName) {
  TH1D *histSim = tmSim->energySpectrumHist(detectorName, 0, 1500, false);
  TH1D *histBroad = tmSim->energySpectrumHist(detectorName, 0, 1500, true);
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
  TH1D *histSim =
      tmSim->energySpectrumHist(detectorName, lowerBound, upperBound, false);
  TH1D *histBroad =
      tmSim->energySpectrumHist(detectorName, lowerBound, upperBound, true);

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
  TString fileName = Form("partialHist_%."
                          "2f_%.2f",
                          lowerBound, upperBound);
  // Update the canvas to show the drawings
  c1->Update();
  c1->Print(fileName + detectorName + ".png");

  delete c1;
  delete histSim;

  TCanvas *c2 = new TCanvas("c2", "c2", 2000, 1500);
  c2->SetLeftMargin(0.15);  // Set the left margin to 15% of the canvas width
  c2->SetRightMargin(0.05); // Set the right margin to 5% of the canvas width
  c2->SetTopMargin(0.1);    // Set the top margin to 10% of the canvas height
  c2->SetBottomMargin(
      0.15); // Set the bottom margin to 15% of the canvas height
  TLegend *legend = fitGaussianToPeak(histBroad, 68.75, 5);
  histBroad->Draw();
  legend->Draw();

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
  c2->Print(fileName + detectorName + "_broad.png");

  delete histBroad;
  delete c2;
}

TLegend *BroadAnalysis::fitGaussianToPeak(TH1D *hist, double peak = 68.75,
                                          double range = 5) {
  // Define the fit range around the peak position
  double fitRangeMin = peak - range;
  double fitRangeMax = peak + range;
  // Define the Gaussian function
  TF1 *gaussFit = new TF1("gaussFit", "gaus + [3]", fitRangeMin, fitRangeMax);

  // Initial parameters: [0] = height, [1] = mean, [2] = sigma
  gaussFit->SetParameters(hist->GetMaximum(), peak, 1.0);

  // Perform the fit
  hist->Fit(gaussFit, "RQ");

  // Retrieve fit parameters and their uncertainties
  double peakMean = gaussFit->GetParameter(1);
  double peakMeanError = gaussFit->GetParError(1);
  // Create the legend
  TLegend *legend = new TLegend(0.65, 0.75, 0.95, 0.9);
  legend->SetTextSize(0.025);
  legend->SetFillColor(0);
  legend->AddEntry(
      gaussFit,
      TString::Format("Mean: %.3f #pm %.3f keV", peakMean, peakMeanError), "l");

  // Output fit results to console
  std::cout << "Peak position (mean): " << peakMean << " ± " << peakMeanError 
            << " keV" << std::endl;

  return legend;
}
