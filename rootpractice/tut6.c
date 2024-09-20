void tut6()
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

    // create a canvas so we can draw the histogram
    TCanvas *c1= new TCanvas();
    hist->Draw();
}
