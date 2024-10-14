#include "../include/Analysis.hh"

Analysis::Analysis() {
  tmSim0 = nullptr;
  tmSim1 = nullptr;
  tmSim2 = nullptr;
  tmSim3 = nullptr;
  tmSim4 = nullptr;
  tmSim5 = nullptr;
  tmBroad0 = nullptr;
  tmBroad1 = nullptr;
  tmBroad2 = nullptr;
  tmBroad3 = nullptr;
  tmBroad4 = nullptr;
  tmBroad5 = nullptr;
  CZTStack = new THStack();
  HPGeStack = new THStack();
  SiLiStack = new THStack();
  CZTStackPartial = new THStack();
  HPGeStackPartial = new THStack();
  SiLiStackPartial = new THStack();
}

Analysis::~Analysis() {
  delete tmSim0;
  delete tmSim1;
  delete tmSim2;
  delete tmSim3;
  delete tmSim4;
  delete tmSim5;
  delete tmBroad0;
  delete tmBroad1;
  delete tmBroad2;
  delete tmBroad3;
  delete tmBroad4;
  delete tmBroad5;
  delete CZTStack;
  delete CZTStackPartial;
  delete HPGeStack;
  delete HPGeStackPartial;
  delete SiLiStack;
  delete SiLiStackPartial;
}

void Analysis::loadFiles(bool firstTime = false) {
  if (firstTime) {
    tmSim0 = new TreeModule("../1E6.root");
    tmSim1 = new TreeModule("../5E6.root");
    tmSim2 = new TreeModule("../1E7.root");
    tmSim3 = new TreeModule("../5E7.root");
    tmSim4 = new TreeModule("../1E8.root");
    tmSim5 = new TreeModule("../1E9.root");

    tmSim0->broadenAndStoreEnergy();
    tmSim1->broadenAndStoreEnergy();
    tmSim2->broadenAndStoreEnergy();
    tmSim3->broadenAndStoreEnergy();
    tmSim4->broadenAndStoreEnergy();
    tmSim5->broadenAndStoreEnergy();
  } else {
    tmBroad0 = new BroadTree("../1E6_broadened.root");
    tmBroad1 = new BroadTree("../5E6_broadened.root");
    tmBroad2 = new BroadTree("../1E7_broadened.root");
    tmBroad3 = new BroadTree("../5E7_broadened.root");
    tmBroad4 = new BroadTree("../1E8_broadened.root");
    tmBroad5 = new BroadTree("../1E9_broadened.root");
  }
}

void Analysis::drawHists(const TString detectorName, bool isNormed) {
  THStack *stack = nullptr;
  TString fileName = detectorName;
  TString yTitle = "Counts";

  if (detectorName == "CZT") {
    stack = CZTStack;
  } else if (detectorName == "HPGe") {
    stack = HPGeStack;
  } else if (detectorName == "SiLi") {
    stack = SiLiStack;
  }
  ClearTHStack(stack);
  TH1D *histBroad0 = tmBroad0->energySpectrumHist(detectorName);
  TH1D *histBroad1 = tmBroad1->energySpectrumHist(detectorName);
  TH1D *histBroad2 = tmBroad2->energySpectrumHist(detectorName);
  TH1D *histBroad3 = tmBroad3->energySpectrumHist(detectorName);
  TH1D *histBroad4 = tmBroad4->energySpectrumHist(detectorName);
  TH1D *histBroad5 = tmBroad5->energySpectrumHist(detectorName);

  // Set axis properties for thicker lines and tick marks
  gStyle->SetLineWidth(2);
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(2);
  // Set fill colors and styles
  Short_t lineWidth = 2;
  histBroad0->SetLineWidth(lineWidth);
  histBroad1->SetLineWidth(lineWidth);
  histBroad2->SetLineWidth(lineWidth);
  histBroad3->SetLineWidth(lineWidth);
  histBroad4->SetLineWidth(lineWidth);
  histBroad5->SetLineWidth(lineWidth);
  histBroad0->SetLineColor(kRed);
  histBroad1->SetLineColor(kBlue);
  histBroad2->SetLineColor(kGreen);
  histBroad3->SetLineColor(kCyan);
  histBroad4->SetLineColor(kYellow);
  histBroad5->SetLineColor(kMagenta);

  histBroad0->SetFillColor(TColor::GetColorTransparent(kRed, 0.5));
  histBroad1->SetFillColor(TColor::GetColorTransparent(kBlue, 0.5));
  histBroad2->SetFillColor(TColor::GetColorTransparent(kGreen, 0.5));
  histBroad3->SetFillColor(TColor::GetColorTransparent(kCyan, 0.5));
  histBroad4->SetFillColor(TColor::GetColorTransparent(kYellow, 0.5));
  histBroad5->SetFillColor(TColor::GetColorTransparent(kMagenta, 0.5));

  if (isNormed) {
    histBroad0->Scale(1.0 / histBroad0->Integral());
    histBroad1->Scale(1.0 / histBroad1->Integral());
    histBroad2->Scale(1.0 / histBroad2->Integral());
    histBroad3->Scale(1.0 / histBroad3->Integral());
    histBroad4->Scale(1.0 / histBroad4->Integral());
    histBroad5->Scale(1.0 / histBroad5->Integral());
    fileName += "Normed";
    yTitle = "Counts / # of Incident Neutrons";
  }
  TCanvas *c1 = new TCanvas(detectorName, "c1", 2000, 1500);
  c1->SetLeftMargin(0.15);  // Set the left margin to 15% of the canvas width
  c1->SetRightMargin(0.05); // Set the right margin to 5% of the canvas width
  c1->SetTopMargin(0.1);    // Set the top margin to 10% of the canvas height
  c1->SetBottomMargin(
      0.15); // Set the bottom margin to 15% of the canvas height

  stack->Add(histBroad5);
  stack->Add(histBroad4);
  stack->Add(histBroad3);
  stack->Add(histBroad2);
  stack->Add(histBroad1);
  stack->Add(histBroad0);
  stack->Draw("nostack");
  stack->GetXaxis()->SetTitle("Energy (keV)");
  stack->GetYaxis()->SetTitle(yTitle);
  // Add the title using TLatex
  TLatex *latex = new TLatex();
  latex->SetNDC();
  latex->SetTextSize(0.04);
  // latex->DrawLatex(0.53, 0.95, detectorName);
  // Update the canvas to show the drawings
  c1->Update();
  c1->Print(fileName + ".png");
  delete c1;
}

void Analysis::drawPartialHists(const TString detectorName, double lowerBound,
                                double upperBound, bool isNormed, int nbins) {

  THStack *stackPartial = nullptr;
  TString yTitle = "Log Counts";

  if (detectorName == "CZT") {
    stackPartial = CZTStackPartial;
  } else if (detectorName == "HPGe") {
    stackPartial = HPGeStackPartial;
  } else if (detectorName == "SiLi") {
    stackPartial = SiLiStackPartial;
  }
  ClearTHStack(stackPartial);
  TH1D *histBroad0 =
      tmBroad0->energySpectrumHist(detectorName, lowerBound, upperBound, nbins);
  TH1D *histBroad1 =
      tmBroad1->energySpectrumHist(detectorName, lowerBound, upperBound, nbins);
  TH1D *histBroad2 =
      tmBroad2->energySpectrumHist(detectorName, lowerBound, upperBound, nbins);
  TH1D *histBroad3 =
      tmBroad3->energySpectrumHist(detectorName, lowerBound, upperBound, nbins);
  TH1D *histBroad4 =
      tmBroad4->energySpectrumHist(detectorName, lowerBound, upperBound, nbins);
  TH1D *histBroad5 =
      tmBroad5->energySpectrumHist(detectorName, lowerBound, upperBound, nbins);

  // Set axis properties for thicker lines and tick marks
  gStyle->SetLineWidth(2);
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(2);
  // Set fill colors and styles
  Short_t lineWidth = 2;
  histBroad0->SetLineWidth(lineWidth);
  histBroad1->SetLineWidth(lineWidth);
  histBroad2->SetLineWidth(lineWidth);
  histBroad3->SetLineWidth(lineWidth);
  histBroad4->SetLineWidth(lineWidth);
  histBroad5->SetLineWidth(lineWidth);
  histBroad0->SetLineColor(kRed);
  histBroad1->SetLineColor(kBlue);
  histBroad2->SetLineColor(kGreen);
  histBroad3->SetLineColor(kCyan);
  histBroad4->SetLineColor(kYellow);
  histBroad5->SetLineColor(kMagenta);
  histBroad0->SetFillColor(TColor::GetColorTransparent(kRed, 0.5));
  histBroad1->SetFillColor(TColor::GetColorTransparent(kBlue, 0.5));
  histBroad2->SetFillColor(TColor::GetColorTransparent(kGreen, 0.5));
  histBroad3->SetFillColor(TColor::GetColorTransparent(kCyan, 0.5));
  histBroad4->SetFillColor(TColor::GetColorTransparent(kYellow, 0.5));
  histBroad5->SetFillColor(TColor::GetColorTransparent(kMagenta, 0.5));

  TString fileName = Form("partialHist_%."
                          "2f_%.2f",
                          lowerBound, upperBound);

  if (isNormed) {
    histBroad0->Scale(1.0 / histBroad0->Integral());
    histBroad1->Scale(1.0 / histBroad1->Integral());
    histBroad2->Scale(1.0 / histBroad2->Integral());
    histBroad3->Scale(1.0 / histBroad3->Integral());
    histBroad4->Scale(1.0 / histBroad4->Integral());
    histBroad5->Scale(1.0 / histBroad5->Integral());
    fileName += "Normed";
    yTitle = "Log Normalized Counts";
  }

  TCanvas *c1 = new TCanvas(detectorName, "c1", 2000, 1500);
  c1->SetLeftMargin(0.15);  // Set the left margin to 15% of the canvas width
  c1->SetRightMargin(0.05); // Set the right margin to 5% of the canvas width
  c1->SetTopMargin(0.1);    // Set the top margin to 10% of the canvas height
  c1->SetBottomMargin(
      0.15); // Set the bottom margin to 15% of the canvas height

  TLegend *legend = new TLegend(0.7, 0.65, 0.95, 0.9);
  legend->SetTextSize(0.025);
  legend->SetFillColor(0);
  legend->SetHeader("68.75 keV Peak Position Errors", "C");
  fitGaussianToPeak(legend, histBroad0, 68.75, 5, "0.267 min");
  fitGaussianToPeak(legend, histBroad1, 68.75, 5, "1.3 min");
  fitGaussianToPeak(legend, histBroad2, 68.75, 5, "2.67 min");
  fitGaussianToPeak(legend, histBroad3, 68.75, 5, "13.3 min");
  fitGaussianToPeak(legend, histBroad4, 68.75, 5, "26.7 min");
  fitGaussianToPeak(legend, histBroad5, 68.75, 5, "267 min");
  stackPartial->Add(histBroad5);
  stackPartial->Add(histBroad4);
  stackPartial->Add(histBroad3);
  stackPartial->Add(histBroad2);
  stackPartial->Add(histBroad1);
  stackPartial->Add(histBroad0);
  stackPartial->Draw("nostack, HIST");
  stackPartial->GetXaxis()->SetTitle("Energy (keV)");
  stackPartial->GetYaxis()->SetTitle(yTitle);

  // Add the title using TLatex
  TLatex *latex = new TLatex();
  latex->SetNDC();
  latex->SetTextSize(0.04);
  // latex->DrawLatex(0.53, 0.95, detectorName);

  legend->Draw();
  c1->SetLogy();
  // Update the canvas to show the drawings
  c1->Update();
  c1->Print(fileName + detectorName + ".png");
  delete c1;
}

void Analysis::ClearTHStack(THStack *stack) {
  // Get the list of histograms
  TList *histList = stack->GetHists();

  // Remove and delete each histogram
  TIter next(histList);
  TObject *obj;
  while ((obj = next())) {
    histList->Remove(obj);
    delete obj;
  }
}

void Analysis::fitGaussianToPeak(TLegend *legend, TH1D *hist,
                                 double peak = 68.75, double range = 5,
                                 TString measurementTime = "") {
  // Define the fit range around the peak position
  double fitRangeMin = peak - range;
  double fitRangeMax = peak + range;
  // Define the Gaussian function
  TF1 *gaussFit = new TF1("gaussFit", "gaus + [3]*x*x + [4]*x + [5]",
                          fitRangeMin, fitRangeMax);

  // Initial parameters: [0] = height, [1] = mean, [2] = sigma
  gaussFit->SetParameters(hist->GetMaximum(), peak, 1.0);
  // Set the color for the fit from the histogram's color
  gaussFit->SetLineColor(hist->GetLineColor());
  gaussFit->Draw();
  // Perform the fit
  hist->Fit(gaussFit, "RQ");
  // Retrieve fit parameters and their uncertainties
  double peakMean = gaussFit->GetParameter(1);
  double peakMeanError = gaussFit->GetParError(1);
  // Create the legend
  legend->AddEntry(gaussFit,
                   measurementTime + ": " +
                       TString::Format("#pm %.3f keV", peakMeanError),
                   "l");

  // Output fit results to console
  std::cout << "Peak position (mean): " << peakMean << " ± " << peakMeanError 
            << " keV" << std::endl;
}
