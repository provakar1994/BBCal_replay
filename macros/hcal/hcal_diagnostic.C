#include <iostream>

#include "TChain.h"
#include "TH2F.h"
#include "TCut.h"

// fundamental constant
const double Mp = 0.938272081;

// kinematic constants
const double E_beam = 4.291;

int hcal_diagnostic(const char* rootfile) {

  TChain *C = new TChain("T");
  C->Add(rootfile);

  int maxNtr = 1000;
  C->SetBranchStatus("*",0);
  // kine
  double k_W2;         C->SetBranchStatus("e.kine.W2",1); C->SetBranchAddress("e.kine.W2", &k_W2);
  // track
  double ntr;          C->SetBranchStatus("bb.tr.n",1); C->SetBranchAddress("bb.tr.n", &ntr);
  double trP[maxNtr];  C->SetBranchStatus("bb.tr.p",1); C->SetBranchAddress("bb.tr.p", &trP);
  double trPz[maxNtr]; C->SetBranchStatus("bb.tr.pz",1); C->SetBranchAddress("bb.tr.pz", &trPz);
  double vz[maxNtr];   C->SetBranchStatus("bb.tr.vz",1); C->SetBranchAddress("bb.tr.vz", &vz);
  // bbcal
  double ePS;          C->SetBranchStatus("bb.ps.e",1); C->SetBranchAddress("bb.ps.e", &ePS);
  double eSH;          C->SetBranchStatus("bb.sh.e",1); C->SetBranchAddress("bb.sh.e", &eSH);
  double nclusSH;      C->SetBranchStatus("bb.sh.nclus",1); C->SetBranchAddress("bb.sh.nclus", &nclusSH);
  double idblkSH;      C->SetBranchStatus("bb.sh.idblk",1); C->SetBranchAddress("bb.sh.idblk", &idblkSH);
  double eblkSH;       C->SetBranchStatus("bb.sh.eblk",1); C->SetBranchAddress("bb.sh.eblk", &eblkSH);
  double rblkSH;       C->SetBranchStatus("bb.sh.rowblk",1); C->SetBranchAddress("bb.sh.rowblk", &rblkSH);
  double cblkSH;       C->SetBranchStatus("bb.sh.colblk",1); C->SetBranchAddress("bb.sh.colblk", &cblkSH);
  double againblkSH;   C->SetBranchStatus("bb.sh.againblk",1); C->SetBranchAddress("bb.sh.againblk", &againblkSH);
  // hcal
  double eHCAL;        C->SetBranchStatus("sbs.hcal.e",1); C->SetBranchAddress("sbs.hcal.e", &eHCAL);
  double idblkHCAL;    C->SetBranchStatus("sbs.hcal.idblk",1); C->SetBranchAddress("sbs.hcal.idblk", &idblkHCAL);
  double eblkHCAL;     C->SetBranchStatus("sbs.hcal.eblk",1); C->SetBranchAddress("sbs.hcal.eblk", &eblkHCAL);
  double rblkHCAL;     C->SetBranchStatus("sbs.hcal.rowblk",1); C->SetBranchAddress("sbs.hcal.rowblk", &rblkHCAL);
  double cblkHCAL;     C->SetBranchStatus("sbs.hcal.colblk",1); C->SetBranchAddress("sbs.hcal.colblk", &cblkHCAL);
  double againblkHCAL; C->SetBranchStatus("sbs.hcal.againblk",1); C->SetBranchAddress("sbs.hcal.againblk", &againblkHCAL);

  TString outFile = "hcal_diagnostic.root";
  TFile *fout = new TFile(outFile, "RECREATE");
  fout->cd();

  TTree *Tout = new TTree("Tout", "");
  double T_W2;      Tout->Branch("W2", &T_W2, "W2/D");
  double T_trP;     Tout->Branch("trP", &T_trP, "trP/D");
  double T_ePS;     Tout->Branch("ePS", &T_ePS, "ePS/D");
  double T_eSH;     Tout->Branch("eSH", &T_eSH, "eSH/D");
  double T_eHCAL;   Tout->Branch("eHCAL", &T_eHCAL, "eHCAL/D");

  //hostograms
  gStyle->SetOptStat(0);
  TH1F *h_k_W2 = new TH1F("h_k_W2", "W2 Distribution using e.kine.W2", 150, -0.2, 5);
  TH1F *h_W2 = new TH1F("h_W2", "W2 Distribution", 150, -0.2, 5);
  TH1F *h_eng_bbcal_elas = new TH1F("h_eng_bbcal_elas", "BBCAL Cluster Eng. [|W2 - 0.88| < 0.4]", 200, 0, 5.);
  TH2F *h2_ADCint_vs_shblk = new TH2F("h2_ADCint_vs_shblk", "ADC int vs SH block", 189, 0, 189, 300, 0, 300);
  TH2F *h2_ADCint_vs_shblk_elas = new TH2F("h2_ADCint_vs_shblk_elas", "ADC int vs SH block [|W2 - 0.88| < 0.4]", 189, 0, 189, 300, 0, 300);
  TH2F *h2_hcal_elas = new TH2F("h2_hcal_elas", "", 12, 0, 12, 24, 0, 24);
  TH2F *h2_ADCint_vs_hcalblk = new TH2F("h2_ADCint_vs_hcalblk", "ADC int vs HCAL block", 288, 1, 289, 200, 0, 200);
  TH2F *h2_ADCint_vs_hcalblk_elas = new TH2F("h2_ADCint_vs_hcalblk_elas", "ADC int vs HCAL block [|W2 - 0.88| < 0.4]", 288, 1, 289, 200, 0, 200);
  TH1F *h_eng_hcal_elas = new TH1F("h_eng_hcal_elas", "HCAL Cluster Eng. [|W2 - 0.88| < 0.4]", 200, 0, 0.5);
  TH2F *h2_eng_vs_hcalblk_elas = new TH2F("h2_eng_vs_hcalblk_elas", "Eng. vs HCAL block [|W2 - 0.88| < 0.4]", 288, 1, 289, 200, 0, 0.5);

  // event loop
  long Nevents = C->GetEntries(), nevent = 0;
  while (C->GetEntry(nevent++)) {

    if(nevent % 100 == 0) cout << nevent << "/" << Nevents << " \r";;
    cout.flush();

    // define some global cuts
    bool globalcut = abs(vz[0]) < 0.27 && ePS > 0.15 && ntr > 0;
    if (!globalcut) continue;

      double P_ang = TMath::ACos(trPz[0]/trP[0]);
      double Q2 = 4. * E_beam * trP[0] * pow(TMath::Sin((P_ang/2.)), 2.);
      P_ang *= TMath::RadToDeg();
      double W2 = Mp*Mp + 2.*Mp*(E_beam - trP[0]) - Q2;
      h_k_W2->Fill(k_W2);
      h_W2->Fill(W2);
    
      T_W2 = W2;
      T_trP = vz[0];
      T_ePS = ePS;
      T_eSH = eSH;
      T_eHCAL = eHCAL;
      Tout->Fill();  

      //bool shedge = rblkSH == 0 || rblkSH == 26 || cblkSH == 0 || cblkSH == 6;
      h2_ADCint_vs_shblk->Fill(idblkSH, eblkSH/againblkSH);

      //bool hcaledge = rblkHCAL == 0 || rblkHCAL == 23 || cblkHCAL == 0 || cblkHCAL == 11;
      h2_ADCint_vs_hcalblk->Fill(idblkHCAL, eblkHCAL/againblkHCAL);

      // choose elastics
      if (abs(W2 - 0.88) < 0.3) {
  	h_eng_bbcal_elas->Fill(eSH + ePS);
  	h2_ADCint_vs_shblk_elas->Fill(idblkSH, eblkSH/againblkSH);

  	h2_hcal_elas->Fill(cblkHCAL, rblkHCAL);    
  	h2_ADCint_vs_hcalblk_elas->Fill(idblkHCAL, eblkHCAL/againblkHCAL);
  	h_eng_hcal_elas->Fill(eHCAL);
  	h2_eng_vs_hcalblk_elas->Fill(idblkHCAL, eblkHCAL);
      }

  } // event loop
  cout << endl;

  fout->Write();
  return 0;
}

