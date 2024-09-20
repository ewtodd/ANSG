void tut8()
{       
    // create a 1 dimensional histogram of floats 
    // with name hist, title Histogram
    
    TH1F *hist = new TH1F("hist","Histogram",100,0, 10);
   
    // create a random number generator
    TRandom2 *rand = new TRandom2(3);
    
    fstream file;
    // note the ios::out since we are storing these values in the file
    file.open("data3.txt", ios::out);
    
    for(int i =0; i < 1000; i++) {
        // sample from Gaussian distribution with mean 5, stdev 1
        // and store in a file
        double r = rand->Gaus(5,1);
        // note the direction of << (c.f. >>)
        file << r << endl;
    }
    
    file.close();

    // open file
    file.open("data3.txt",ios::in);

    // declare double object called value;
    double value;    

    // loop through all the entries and fill the histogram with them
    while(file >> value)
    {
        hist->Fill(value);
    }
    
    // close file
    file.close();

    // this is honestly self-explanatory
    hist->GetXaxis()->SetTitle("Distribution");
    hist->GetYaxis()->SetTitle("Entries");    
    
    // define our own function
    TF1 *fit = new TF1("fit", "gaus", 0, 10); 
    
    // set initial parameters
    fit->SetParameter(0,40); // constant
    fit->SetParameter(1,5); // mean
    fit->SetParameter(2,1); // stdev

    
    // create a canvas so we can draw the histogram
    TCanvas *c1= new TCanvas();
    
    // remove the statistics box
    hist->SetStats(0);

    
    hist->Draw();
    // fit the data to our distribution function (which in this case is just Gaussian)
    // the flag R means region; it tells it to use the limits given in
    // the TF1 definition
    hist->Fit("fit", "R");
    
    // create a legend 
    TLegend *leg = new TLegend(0.7,0.7,0.9,0.9);
    leg->AddEntry(hist, "Simulated Data","l");
    leg->AddEntry(fit, "Fit Function", "l");
    leg->Draw();   
}
