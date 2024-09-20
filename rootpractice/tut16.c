void tut16() {
    
    TFile *input = new TFile("data5.root", "READ");
    TCanvas *c1 = new TCanvas();

    // get the tree from the ROOT file and cast it to a TTree
    TTree *tree = (TTree*)input->Get("tree");
    
    int entries = tree->GetEntries();
    
    double x,y;
    
    tree->SetBranchAddress("x", &x);
    tree->SetBranchAddress("y", &y);
    
    TH1F *histx = new TH1F("histx", "x Histogram", 100, 0, 100);
    TH1F *histy = new TH1F("histy", "y Histogram", 100, 0, 100);
 
    for (int i=0; i<entries; i++) {
        tree->GetEntry(i);
        histx->Fill(x);
        histy->Fill(y);
    }
    
           
    // input->Close();
    histy->Draw();

}
