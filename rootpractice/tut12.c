void tut12()
{
    TCanvas *c1 = new TCanvas();

    TF1 *func = new TF1("func","exp(-[0]*x)*[1]*sin([2]*x)", 0, 10*TMath::Pi());

    func->SetParameter(0, 0.1);
    func->SetParameter(1, 2);
    func->SetParameter(2,3);
    func->Draw();
}
