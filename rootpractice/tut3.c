void tut3()
{       
    // create a 1 dimensional histogram of floats 
    // with name hist, title Histogram
    // with 6 bins and x-axis limits [1,7)
    TH1F *hist = new TH1F("hist","Histogram",6,1, 7);
   
    // declare fstream object called file
    fstream file;
    // open file
    file.open("data1.txt",ios::in);

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
    hist->GetXaxis()->SetTitle("X Axis");
    hist->GetYaxis()->SetTitle("Y Axis");    

    // create a canvas so we can draw the histogram
    TCanvas *c1= new TCanvas();
    hist->Draw();
}
