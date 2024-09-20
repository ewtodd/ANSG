#include "../include/Analysis.hh"

Analysis::Analysis() {
  tmGeo0 = nullptr;
  tmGeo1 = nullptr;
  tmGeo2 = nullptr;
  tmGeo3 = nullptr;
  tmGeo4 = nullptr;
  tmGeo5 = nullptr;
}

Analysis::~Analysis() {
  delete tmGeo0;
  delete tmGeo1;
  delete tmGeo2;
  delete tmGeo3;
  delete tmGeo4;
  delete tmGeo5;
}

void Analysis::loadFiles() {
  tmGeo0 = new TreeModule("../geo0.root");
  tmGeo1 = new TreeModule("../geo1.root");
  tmGeo2 = new TreeModule("../geo2.root");
  tmGeo3 = new TreeModule("../geo3.root");
  tmGeo4 = new TreeModule("../geo4.root");
  tmGeo5 = new TreeModule("../geo5.root");
}

void Analysis::drawFullHists() {
  TH1D *histGeo0 = tmGeo0->energySpectrumHist(tmGeo0->getFilename());
  TH1D *histGeo1 = tmGeo1->energySpectrumHist(tmGeo1->getFilename());
  TH1D *histGeo2 = tmGeo2->energySpectrumHist(tmGeo2->getFilename());
  TH1D *histGeo3 = tmGeo3->energySpectrumHist(tmGeo3->getFilename());
  TH1D *histGeo4 = tmGeo4->energySpectrumHist(tmGeo4->getFilename());
  TH1D *histGeo5 = tmGeo5->energySpectrumHist(tmGeo5->getFilename());

  TCanvas *c1 = new TCanvas("c1", "c1");
  histGeo0->Draw();
  histGeo1->Draw("SAME");
  histGeo2->Draw("SAME");
  histGeo3->Draw("SAME");
  histGeo4->Draw("SAME");
  histGeo5->Draw("SAME");

  histGeo0->SetLineColor(kRed);
  histGeo1->SetLineColor(kBlue);
  histGeo2->SetLineColor(kGreen);
  histGeo3->SetLineColor(kMagenta);
  histGeo4->SetLineColor(kOrange);
  histGeo5->SetLineColor(kViolet);
  TLegend *legend =
      new TLegend(0.7, 0.7, 0.9, 0.9); // Adjust the position as needed
  legend->AddEntry(histGeo0, "Geometry 0", "l");
  legend->AddEntry(histGeo1, "Geometry 1", "l");
  legend->AddEntry(histGeo2, "Geometry 2", "l");
  legend->AddEntry(histGeo3, "Geometry 3", "l");
  legend->AddEntry(histGeo4, "Geometry 4", "l");
  legend->AddEntry(histGeo4, "Geometry 5", "l");
  legend->Draw();

  // Update the canvas to show the drawings
  c1->Update();
  c1->Print("fullrange.png");

  delete c1;
  delete histGeo0;
  delete histGeo1;
  delete histGeo2;
  delete histGeo3;
  delete histGeo4;
  delete histGeo5;
}

void Analysis::drawPartialHists(int lowerBound, int upperBound) {
  TH1D *histGeo0 = tmGeo0->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo0->getFilename());
  TH1D *histGeo1 = tmGeo1->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo1->getFilename());
  TH1D *histGeo2 = tmGeo2->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo2->getFilename());
  TH1D *histGeo3 = tmGeo3->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo3->getFilename());
  TH1D *histGeo4 = tmGeo4->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo4->getFilename());
  TH1D *histGeo5 = tmGeo5->partialEnergySpectrumHist(lowerBound, upperBound,
                                                     tmGeo5->getFilename());

  TCanvas *c1 = new TCanvas("c1", "c1");
  histGeo0->Draw();
  histGeo1->Draw("SAME");
  histGeo2->Draw("SAME");
  histGeo3->Draw("SAME");
  histGeo4->Draw("SAME");
  histGeo5->Draw("SAME");

  histGeo0->SetLineColor(kRed);
  histGeo1->SetLineColor(kBlue);
  histGeo2->SetLineColor(kGreen);
  histGeo3->SetLineColor(kMagenta);
  histGeo4->SetLineColor(kOrange);
  histGeo5->SetLineColor(kViolet);
  TLegend *legend =
      new TLegend(0.7, 0.7, 0.9, 0.9); // Adjust the position as needed
  legend->AddEntry(histGeo0, "Geometry 0", "l");
  legend->AddEntry(histGeo1, "Geometry 1", "l");
  legend->AddEntry(histGeo2, "Geometry 2", "l");
  legend->AddEntry(histGeo3, "Geometry 3", "l");
  legend->AddEntry(histGeo4, "Geometry 4", "l");
  legend->AddEntry(histGeo4, "Geometry 5", "l");
  legend->Draw();

  // Update the canvas to show the drawings
  c1->Update();
  TString fileName = Form("partialHist_%d_%d.png", lowerBound, upperBound);
  c1->Print(fileName);

  delete c1;
  delete histGeo0;
  delete histGeo1;
  delete histGeo2;
  delete histGeo3;
  delete histGeo4;
  delete histGeo5;
}
