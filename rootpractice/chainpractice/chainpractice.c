void write(TString filename)
{
    // create output file
    TFile *output = new TFile(filename, "RECREATE");
    // create tree to store our data
    TTree *tree = new TTree("tree", "tree");

    double x,y;

    // assign branches to the tree
    // as before, &x means reference, "x/D" says it is a double
    tree->Branch("x",&x,"x/D");
    tree->Branch("y",&y,"y/D");

    TRandom2 *rand = new TRandom2();

    // fill the tree with random numbers
    for(int i=0; i<1e6; i++) {
        x = 1+rand->Rndm()*9;
        y = x*2;
        tree->Fill();
    }

    // write the tree to the file
    output->Write();
    // close the file
    output->Close();
}

void chain(int numFiles) {

    // create a new chain of trees (?)
    TChain *ch1 = new TChain("tree");
    // add all files to the chain, assuming standard naming convention
    for (int i=0; i<numFiles; i++) {
        string name = "chain" + std::to_string(i) + ".root";
        TString l(name);
        ch1->Add(l);
    }

    // tell chain that we care about "x" channel
    double x;
    ch1->SetBranchAddress("x",&x);
    // get number of entries
    int entries = ch1->GetEntries();

    // fill a histogram with the combined x channels and plot it
    TH1F *hist = new TH1F("hist", "Histogram", 100, 1, 10);
    TCanvas *c1 = new TCanvas();

    for (int i=0; i<entries; i++) {
        ch1->GetEntry(i);
        hist->Fill(x);
    }
    hist->Draw();
}


void chainpractice()
{
    // create some number of files with the same tree
    int numFiles = 20;
    for (int i=0; i<numFiles;i++)
    {
        string name = "chain" + std::to_string(i) + ".root";
        TString l(name);
        write(l);
    }
    chain(numFiles);
}
