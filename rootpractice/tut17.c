void tut17() 
{
    // # of x bins, x limits; # of y bins, y limits
    TH2F *hist = new TH2F("hist", "2D Histogram", 100, -1, 1, 100, -1, 1);
    TCanvas *c1  = new TCanvas();
    
    // change style to be rainbow!
    gStyle->SetPalette(kRainBow);
     
    // create a random number generator
    TRandom2 *rand = new TRandom2();
    
    // create a graph with errors
    TGraphErrors *gr = new TGraphErrors();

    for(int i =0; i < 1e7; i++) {
        double x = rand->Gaus();
        double y = rand->Gaus();
        
        hist->Fill(x,y);
    }
    
    hist->SetStats(0);
    
    hist->GetXaxis()->SetTitle("x");
    hist->GetYaxis()->SetTitle("y");
    hist->GetZaxis()->SetTitle("# of entries");
    
    c1->SetLogz();
    hist->SetContour(1000);
    hist->Draw("colz");       
    
}
