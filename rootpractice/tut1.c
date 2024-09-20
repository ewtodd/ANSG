void tut1()
{       
    // create a 1 dimensional histogram of floats 
    // with name hist, title Histogram
    // with 100 bins and x-axis limits [0,100)
    TH1F *hist = new TH1F("hist","Histogram",100,0, 100);
    
    // put one entry at 10 and one at 90
    hist->Fill(10);
    hist->Fill(90);
    
    // this is honestly self-explanatory
    hist->GetXaxis()->SetTitle("X Axis");
    hist->GetYaxis()->SetTitle("Y Axis");    

    // create a canvas so we can draw the histogram
    TCanvas *c1= new TCanvas();
    hist->Draw();
}
