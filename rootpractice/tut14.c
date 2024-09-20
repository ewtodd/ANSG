void tut14() 
{
    // create TFile called output.root
    // RECREATE flag means if file already exists, erase and create it again
    TFile *file = new TFile("output.root", "RECREATE");
    

    // make histogram to store in the file
    TH1F *hist = new TH1F("hist", "Histogram", 100, 0, 100);
    
    hist->Fill(10);
    hist->Fill(90);
    

    // write to and then close file
    file->Write();
    file->Close();

}
