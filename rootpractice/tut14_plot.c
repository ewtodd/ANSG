void tut14_plot() 
{
    // read in the file 
    TFile *file = new TFile("output.root", "READ");

    // get the histogram from the output.root file and cast it to a histogram
    TH1F *hist = (TH1F*)file->Get("hist");

    hist->Draw();

    //file->Close();
}
