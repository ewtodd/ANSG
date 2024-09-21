#include "../include/Analysis.hh"

Analysis::Analysis() {
  tmGeo0 = nullptr;
  tmGeo1 = nullptr;
  tmGeo2 = nullptr;
  tmGeo3 = nullptr;
  // tmGeo4 = nullptr;
  // tmGeo5 = nullptr;
  // tmGeo6 = nullptr;
}

Analysis::~Analysis() {
  delete tmGeo0;
  delete tmGeo1;
  delete tmGeo2;
  delete tmGeo3;
  // delete tmGeo4;
  // delete tmGeo5;
  // delete tmGeo6;
}

void Analysis::loadFiles() {
  tmGeo0 = new TreeModule("../geo0.root");
  tmGeo1 = new TreeModule("../geo1.root");
  tmGeo2 = new TreeModule("../geo2.root");
  tmGeo3 = new TreeModule("../geo3.root");
  // tmGeo4 = new TreeModule("../geo4.root");
  // tmGeo5 = new TreeModule("../geo5.root");
  // tmGeo6 = new TreeModule("../geo6.root");
}

void Analysis::drawFullHists() {
  TH1D *histGeo0 = tmGeo0->energySpectrumHist(tmGeo0->getFilename());
  TH1D *histGeo1 = tmGeo1->energySpectrumHist(tmGeo1->getFilename());
  TH1D *histGeo2 = tmGeo2->energySpectrumHist(tmGeo2->getFilename());
  TH1D *histGeo3 = tmGeo3->energySpectrumHist(tmGeo3->getFilename());
  // TH1D *histGeo4 = tmGeo4->energySpectrumHist(tmGeo4->getFilename());
  // TH1D *histGeo5 = tmGeo5->energySpectrumHist(tmGeo5->getFilename());
  // TH1D *histGeo6 = tmGeo5->energySpectrumHist(tmGeo6->getFilename());

  // Find the histogram with the largest maximum bin content
  double maxBinContent =
      std::max({histGeo0->GetMaximum(), histGeo1->GetMaximum(),
                histGeo2->GetMaximum(), histGeo3->GetMaximum()});
  //, histGeo4->GetMaximum(), histGeo5->GetMaximum(), histGeo6->GetMaximum()});

  double yAxisMax = maxBinContent * 1.1;

  TCanvas *c1 = new TCanvas("c1", "c1");
  // Set fill colors and styles
  histGeo0->SetLineColor(kRed);
  histGeo1->SetLineColor(kBlue);
  histGeo2->SetLineColor(kGreen);
  histGeo3->SetLineColor(kMagenta);
  // histGeo4->SetLineColor(kOrange);
  // histGeo5->SetLineColor(kViolet);
  // histGeo6->SetLineColor(kCyan);

  histGeo0->GetYaxis()->SetRangeUser(0, yAxisMax);
  // Draw histograms with fill styles
  histGeo0->Draw();
  histGeo1->Draw("SAME");
  histGeo2->Draw("SAME");
  histGeo3->Draw("SAME");
  // histGeo4->Draw("SAME");
  // histGeo5->Draw("SAME");
  // histGeo6->Draw("SAME");

  // Create and draw vertical lines at specified values
  std::vector<double> lineValues = {5850.97, 5867.12, 68.752};
  for (double value : lineValues) {
    TLine *line = new TLine(value, 0, value, yAxisMax);
    line->SetLineColor(kBlack);
    line->SetLineStyle(2); // Dashed line
    line->Draw();
  }

  TLegend *legend =
      new TLegend(0.7, 0.7, 0.9, 0.9); // Adjust the position as needed
  legend->AddEntry(histGeo0, "Geometry 0", "l");
  legend->AddEntry(histGeo1, "Geometry 1", "l");
  legend->AddEntry(histGeo2, "Geometry 2", "l");
  legend->AddEntry(histGeo3, "Geometry 3", "l");
  // legend->AddEntry(histGeo4, "Geometry 4", "l");
  // legend->AddEntry(histGeo5, "Geometry 5", "l");
  // legend->AddEntry(histGeo6, "Geometry 6", "l");
  legend->Draw();

  // Update the canvas to show the drawings
  c1->Update();
  c1->Print("fullrange.png");

  delete c1;
  delete histGeo0;
  delete histGeo1;
  delete histGeo2;
  delete histGeo3;
  // delete histGeo4;
  // delete histGeo5;
  // delete histGeo6;
}

void Analysis::drawPartialHists(double lowerBound, double upperBound) {
  TH1D *histGeo0 = tmGeo0->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo0->getFilename());
  TH1D *histGeo1 = tmGeo1->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo1->getFilename());
  TH1D *histGeo2 = tmGeo2->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo2->getFilename());
  TH1D *histGeo3 = tmGeo3->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo3->getFilename());
  // TH1D *histGeo4 = tmGeo4->partialEnergySpectrumHist(lowerBound, upperBound,
  // tmGeo4->getFilename());
  // TH1D *histGeo5 = tmGeo5->partialEnergySpectrumHist(lowerBound, upperBound,
  // tmGeo5->getFilename());
  //  TH1D *histGeo6 =  tmGeo6->partialEnergySpectrumHist(lowerBound,
  //  upperBound, tmGeo6->getFilename());

  TCanvas *c1 = new TCanvas("c1", "c1");
  // Set fill colors and
  // styles
  histGeo0->SetLineColor(kRed);
  histGeo1->SetLineColor(kBlue);
  histGeo2->SetLineColor(kGreen);
  histGeo3->SetLineColor(kMagenta);
  //  histGeo4->SetLineColor(kOrange);
  // histGeo5->SetLineColor(kViolet);
  //  histGeo6->SetLineColor(kCyan);

  double maxBinContent =
      std::max({histGeo0->GetMaximum(), histGeo1->GetMaximum(),
                histGeo2->GetMaximum(), histGeo3->GetMaximum()});
  // ,
  // histGeo4->GetMaximum(),
  // histGeo5->GetMaximum(),
  // histGeo6->GetMaximum()});

  double yAxisMax = maxBinContent * 1.1;
  histGeo0->GetYaxis()->SetRangeUser(0, yAxisMax);

  // Draw histograms with
  // fill styles
  histGeo0->Draw();
  histGeo1->Draw("SAME");
  histGeo2->Draw("SAME");
  histGeo3->Draw("SAME");
  // histGeo4->Draw("SAME");
  // histGeo5->Draw("SAME");
  // histGeo6->Draw("SAME");

  // Create and draw vertical
  // lines at specified
  // values
  std::vector<double> lineValues = {5850.97, 5867.12, 68.752};
  for (double value : lineValues) {
    if (lowerBound <= value && value <= upperBound) {
      TLine *line = new TLine(value, 0, value, yAxisMax);
      line->SetLineColor(kBlack);
      line->SetLineStyle(2); // Dashed line
      line->Draw();
    }
  }

  TLegend *legend = new TLegend(0.7, 0.7, 0.9,
                                0.9); // Adjust the
                                      // position
                                      // as needed
  legend->AddEntry(histGeo0, "Geometry 0", "l");
  legend->AddEntry(histGeo1, "Geometry 1", "l");
  legend->AddEntry(histGeo2, "Geometry 2", "l");
  legend->AddEntry(histGeo3, "Geometry 3", "l");
  // legend->AddEntry(histGeo4,
  // "Geometry 4", "l");
  // legend->AddEntry(histGeo5,
  // "Geometry 5", "l");
  // legend->AddEntry(histGeo6,
  // "Geometry 6", "l");
  legend->Draw();

  // Update the canvas to
  // show the drawings
  c1->Update();
  TString fileName = Form("partialHist_%."
                          "2f_%.2f.png",
                          lowerBound, upperBound);
  c1->Print(fileName);

  delete c1;
  delete histGeo0;
  delete histGeo1;
  delete histGeo2;
  delete histGeo3;
  // delete histGeo4;
  // delete histGeo5;
  // delete histGeo6;
}
