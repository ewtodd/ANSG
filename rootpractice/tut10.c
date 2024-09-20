void tut10()
{       
    // create a random number generator
    TRandom2 *rand = new TRandom2();
    
    // create a graph with errors
    TGraphErrors *gr = new TGraphErrors();

    fstream file;
    // note the ios::out since we are storing these values in the file
    file.open("data4.txt", ios::out);
    
    for(int i =0; i < 100; i++) {
        // sample from Gaussian distribution with mean 5, stdev 1
        // and store in a file
        double x = i;// static_cast<double>(i);
        double y = sin(x/10);
        double error_x = 0; // rand->Gaus(0.1, 0.01);
        double error_y = rand->Gaus(0.1,0.01);
        // note the direction of << (c.f. >>)
        file << x << "  " << y << " " << error_x << " " << error_y << endl;
    }
    
    file.close();

    // open file
    file.open("data4.txt",ios::in);
    
    double x, y, ex, ey;
    int n = 0;
    
    while (file >> x >> y >> ex >> ey) { 
        n = gr->GetN();
        gr->SetPoint(n,x,y);
        gr->SetPointError(n,ex,ey);
    }
    
    // close file
    file.close();

    // create a canvas so we can draw the histogram
    TCanvas *c1 = new TCanvas();

    // add additional ticks
    //c1->SetTickx();
    //c1->SetTicky();    
    
    TF1 *func = new TF1("func", "[0]*sin([1]*x)", 0, 10);
    func->SetParameter(0, 1);
    func->SetParameter(1,0.1);
    gr->Fit("func");    
    gr->Draw("AL*");
       
}
