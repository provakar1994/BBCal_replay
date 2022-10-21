#include <iostream>

#include "TH1F.h"
#include "TChain.h"

void bbcal_hcal_clust_corr(const char* rootfile){

  //cout << "Processing macro.." << endl;

  //TStopwatch *st = new TStopwatch();
  //st->Start(kTRUE);

  TChain *C = new TChain("T");
  C->Add(rootfile);

  Int_t Ndata_bb_tdctrig_tdcelemID = 0;
  Double_t bb_sh_rowblk = 0., sbs_hcal_rowblk = 0.; 
  Double_t bb_sh_nclus = 0., sbs_hcal_nclus = 0.;
  Double_t bb_tdctrig_tdc[6] = {0.}, bb_tdctrig_tdcelemID[6] = {0.};

  TH1F *h_bbcal_hcal_trigtime_diff = new TH1F("h_bbcal_hcal_trigtime_diff","",200,0,1000);
  TH2F *h2_bbcal_hcal_corr = new TH2F("h2_bbh_corr","BBCal-HCal Cluster Correlation; BB Shower Rows; HCal Rows",27,1,28,24,1,25);

  // Declare trees
  TTree *T = (TTree*) gDirectory->Get("T");
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

  c1->cd(2);
  h2_bbcal_hcal_corr->SetMinimum(-0.1);
  h2_bbcal_hcal_corr->SetStats(0);
  h2_bbcal_hcal_corr->Draw("colz");

  //cout << "Processed macro with " << nevents << " entries." << endl;

  //st->Stop();
  //cout << "CPU time = " << st->CpuTime() << " s " << " Real time = " << st->RealTime() << " s " << endl;
}
