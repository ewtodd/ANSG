void write()
{
    // create output file
    TFile *output = new TFile("tut20.root", "RECREATE");
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

void cut()
{
    // define cut conditions
    TCut cut1 = "x < 5";
    TCut cut2 = "x > 6";

    // read in the file
    TFile *input = new TFile("tut20.root", "READ");
    TTree *tree = (TTree*)input->Get("tree");


    // or symbol is ||
    // and symbol is &&
    // draw the "y" branch of the tree with the desired conditions
    tree->Draw("y", cut1||cut2);

}

void tut20()
{

    write();
    cut();

}
