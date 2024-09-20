#include <TCanvas.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>

// Set many histogram attributes at once for TH1
void SetHistAttributes(TH1 *th1, const char *xTitle, const char *yTitle,
                       const char *title = "", int linecolor = 1,
                       int linewidth = 2, int linestyle = 1,
                       int rebinConstant = 1, double scale = 1) {
  th1->SetTitle(title);
  th1->GetXaxis()->SetTitle(xTitle);
  th1->GetXaxis()->CenterTitle(true);
  th1->GetXaxis()->SetLabelSize(0.05);
  th1->GetXaxis()->SetTitleSize(0.05);
  th1->GetYaxis()->SetTitle(yTitle);
  th1->GetYaxis()->CenterTitle(true);
  th1->GetYaxis()->SetLabelSize(0.05);
  th1->GetYaxis()->SetTitleSize(0.05);
  th1->SetLineColor(linecolor);
  th1->SetLineWidth(linewidth);
  th1->SetLineStyle(linestyle);
  th1->Rebin(rebinConstant);
  th1->SetStats(kFALSE);
  if (scale != 1)
    th1->Scale(scale);
}

// Set many histogram attributes at once for TH2
void Set2DHistAttributes(TH2 *th2, const char *xTitle, const char *yTitle,
                         const char *title = "", int linecolor = 1,
                         int linewidth = 2, int linestyle = 1,
                         int rebinConstant = 1, double scale = 1) {
  th2->SetTitle(title);
  th2->GetXaxis()->SetTitle(xTitle);
  th2->GetXaxis()->CenterTitle(true);
  th2->GetXaxis()->SetLabelSize(0.05);
  th2->GetXaxis()->SetTitleSize(0.05);
  th2->GetYaxis()->SetTitle(yTitle);
  th2->GetYaxis()->CenterTitle(true);
  th2->GetYaxis()->SetLabelSize(0.05);
  th2->GetYaxis()->SetTitleSize(0.05);
  th2->SetLineColor(linecolor);
  th2->SetLineWidth(linewidth);
  th2->SetLineStyle(linestyle);
  th2->Rebin(rebinConstant);
  th2->SetStats(kFALSE);
  if (scale != 1)
    th2->Scale(scale);
}

// Set many graph attributes at once
void SetGraphAttributes(TGraph *g1, const char *xTitle, const char *yTitle,
                        const char *title = "", int linecolor = 1,
                        int linewidth = 2, int linestyle = 1) {
  g1->SetTitle(title);
  g1->GetXaxis()->SetTitle(xTitle);
  g1->GetXaxis()->CenterTitle(true);
  g1->GetXaxis()->SetLabelSize(0.05);
  g1->GetXaxis()->SetTitleSize(0.05);
  g1->GetYaxis()->SetTitle(yTitle);
  g1->GetYaxis()->CenterTitle(true);
  g1->GetYaxis()->SetLabelSize(0.05);
  g1->GetYaxis()->SetTitleSize(0.05);
  g1->SetLineColor(linecolor);
  g1->SetLineWidth(linewidth);
  g1->SetLineStyle(linestyle);
}

// Make canvas margins large so labels won't get cut off
void SetCanvasMargins(TCanvas *c1) {
  // Set the margins at 15% of canvas height or width
  c1->SetLeftMargin(0.15);
  c1->SetRightMargin(0.15);
  c1->SetTopMargin(0.15);
  c1->SetBottomMargin(0.15);
}
