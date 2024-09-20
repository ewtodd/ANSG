void tut4()
{       
    // create an empty graph
    TGraph *gr = new TGraph();

    // declare fstream object file, open data file
    fstream file;
    file.open("data2.txt", ios::in);
    
    // declare doubles for use in while loop
    double x, y;
    
    while(file >> x >> y) {
    
    // add points to graph while changing numbers of points
    gr->SetPoint(gr->GetN(),x,y);

    }
    
    file.close();
    
    // create canvas so we can draw the graph
    TCanvas *c1 = new TCanvas();

    // change marker style and size before drawing 
    gr->SetMarkerStyle(4);
    gr->SetMarkerSize(1);
    // self-explanatory
    gr->GetXaxis()->SetTitle("X Axis");
    gr->GetYaxis()->SetTitle("Y Axis");    
    gr->SetLineWidth(2);
    gr->SetLineColor(kRed);
    gr->SetTitle("Graph");
    // draw the graph, A means all entries should be drawn and L means line
    // P means points, adding * will cause entries to be marked
    gr->Draw("ALP");
}
