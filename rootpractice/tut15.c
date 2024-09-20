void tut15()
{
    // create a random number generator
    TRandom2 *rand = new TRandom2();
    
    // create a graph with errors
    TGraphErrors *gr = new TGraphErrors();

    fstream file;
    // note the ios::out since we are storing these values in the file
    file.open("data5.txt", ios::out);
    
    for(int i =0; i < 100; i++) {
        // sample from Gaussian distribution with mean 5, stdev 1
        // and store in a file
        double x = i;// static_cast<double>(i);
        double y = rand->Gaus(1,0.01)*sin(x);
        
        file << x << "  " << y << endl;
    }
    
    file.close();
    
    // open file
    file.open("data5.txt",ios::in);
     
    double x,y; 

    // create TFile
    TFile *output = new TFile("data5.root", "RECREATE");
    
    // create a Tree
    TTree *tree = new TTree("tree", "Tree");
    
    // make a branch with variable called x, reference the variable x (&x)
    // tell ROOT it is a double ("x/D" - capital D for double, F for float, I for int.) 
    tree->Branch("x", &x, "x/D");
    tree->Branch("y", &y, "y/D");
    
    while(file >> x >> y) {
        // since we referenced the variables x and y, all we have to do is
        // tell the tree to fill
        tree->Fill();
    }
     
    output->Write();
    output->Close();
    file.close();
}

