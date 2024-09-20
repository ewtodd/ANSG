void tut2()
{       
    // create two arrays of 5 doubles for the x,y values
    double x[5] = {1,2,3,4,5};
    double y[5] = {1,4,9,16,25};

    // create the graph with 5 entries
    TGraph *gr = new TGraph(5, x, y);

    // create canvas so we can draw the graph
    TCanvas *c1 = new TCanvas();

    // draw the graph, A means all entries should be drawn and L means line
    // P means points, adding * will cause entries to be marked
    // also change marker style and size before drawing 
    gr->SetMarkerStyle(4);
    gr->SetMarkerSize(1);
    gr->GetXaxis()->SetTitle("X Axis");
    gr->GetYaxis()->SetTitle("Y Axis");    

   
    gr->Draw("ALP");
}
