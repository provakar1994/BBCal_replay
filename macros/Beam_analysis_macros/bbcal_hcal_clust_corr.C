/*
  This script plots the difference between BBCAL and HCAL trigger times 
  and fits the peak to get fraction of good events.
  ------
  P. Datta Created 10/25/2022 
*/

#include <iostream>

#include "TH1F.h"
#include "TChain.h"

//gaussian signal peak
double peak_fit (double *x, double *par) {
  return par[0]*exp(-0.5*pow((x[0]-par[1])/par[2],2.));
}

//poly for background
double bg_fit (double *x, double *par) {
  return par[0] + par[1]*x[0];
}

//global fit function
double total_fit (double *x, double *par) {
  return peak_fit(x,&par[0]) + bg_fit(x,&par[3]);
}

void bbcal_hcal_clust_corr (const char* rootfile) {
  TChain *C = new TChain("T");
  C->Add(rootfile);

  Int_t Ndata_bb_tdctrig_tdcelemID = 0;
  Double_t bb_sh_rowblk = 0., sbs_hcal_rowblk = 0.; 
  Double_t bb_sh_nclus = 0., sbs_hcal_nclus = 0.;
  Double_t bb_tdctrig_tdc[6] = {0.}, bb_tdctrig_tdcelemID[6] = {0.};

  TH1F *h_bbcal_hcal_trigtime_diff = new TH1F("h_bbcal_hcal_trigtime_diff","",240,400,640);
  TH2F *h2_bbcal_hcal_corr = new TH2F("h2_bbh_corr","BBCal-HCal Cluster Correlation; BB Shower Rows; HCal Rows",27,1,28,24,1,25);

  // Declare trees
  C->SetBranchStatus("*",0);
  C->SetBranchStatus("bb.tdctrig.tdc",1);
  C->SetBranchStatus("bb.tdctrig.tdcelemID",1);
  C->SetBranchStatus("Ndata.bb.tdctrig.tdcelemID",1);
  C->SetBranchStatus("bb.sh.nclus",1);
  C->SetBranchStatus("bb.sh.rowblk",1);
  C->SetBranchStatus("sbs.hcal.nclus",1);
  C->SetBranchStatus("sbs.hcal.rowblk",1);
  C->SetBranchAddress("bb.tdctrig.tdc", &bb_tdctrig_tdc);
  C->SetBranchAddress("bb.tdctrig.tdcelemID", &bb_tdctrig_tdcelemID);
  C->SetBranchAddress("Ndata.bb.tdctrig.tdcelemID", &Ndata_bb_tdctrig_tdcelemID);
  C->SetBranchAddress("bb.sh.nclus", &bb_sh_nclus);
  C->SetBranchAddress("sbs.hcal.nclus", &sbs_hcal_nclus);
  C->SetBranchAddress("bb.sh.rowblk", &bb_sh_rowblk);
  C->SetBranchAddress("sbs.hcal.rowblk", &sbs_hcal_rowblk);

  // Loop through events
  Long64_t nevents = C->GetEntries();
  for(Long64_t nevent=0; nevent<nevents; nevent++){

    C->GetEntry(nevent);

    if(bb_sh_nclus==0 || sbs_hcal_nclus==0) continue;

    Double_t bbcal_time=0., hcal_time=0.;
    for(Int_t ihit=0; ihit<Ndata_bb_tdctrig_tdcelemID; ihit++){
      if(bb_tdctrig_tdcelemID[ihit]==5) bbcal_time=bb_tdctrig_tdc[ihit];
      if(bb_tdctrig_tdcelemID[ihit]==0) hcal_time=bb_tdctrig_tdc[ihit];
    }

    Double_t diff = hcal_time - bbcal_time; 
    h_bbcal_hcal_trigtime_diff->Fill(diff);
    if(fabs(diff-506.)<20.){
      h2_bbcal_hcal_corr->Fill(bb_sh_rowblk+1, sbs_hcal_rowblk+1);
    }
  }

  TCanvas *c1 = new TCanvas("c1", "c1", 1200, 600);
  c1->Divide(2,1);
  
  c1->cd(1);
  h_bbcal_hcal_trigtime_diff->Draw();

  double par[5], parf[5];
  TF1* total = new TF1("total", total_fit, 430, 615, 5);
  total->SetNpx(500);

  // first try
  total->SetParameters(1,1,1,1,1);
  h_bbcal_hcal_trigtime_diff->Fit("total", "R0");

  // second try
  total->SetLineColor(kBlack);
  total->SetParameters(377,510,15,155,0.06);
  h_bbcal_hcal_trigtime_diff->Fit("total", "RV+", "ep");
  total->GetParameters(parf);
  double totalev = total->Integral(430,615);
  
  // plot the signal peak
  TF1* signal = new TF1("signal", peak_fit, 430, 615, 3);
  signal->SetNpx(500);
  signal->SetParameters(parf);
  signal->SetLineColor(kRed);
  signal->Draw("same");
  double coin = signal->Integral(430,615);

  cout << endl << "------" << endl;
  cout << "No. of Coincidence Events: " << coin << endl;
  cout << "Total events: " << totalev << endl;
  cout << "Ratio [Coin/Total]: " << coin/totalev << endl;
  cout << "------" << endl << endl;

  c1->cd(2);
  h2_bbcal_hcal_corr->SetMinimum(-0.1);
  h2_bbcal_hcal_corr->SetStats(0);
  h2_bbcal_hcal_corr->Draw("colz");
}
