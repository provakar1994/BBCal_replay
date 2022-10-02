/*
  This script has been prepared for the energy calibration of BigBite Calorimeter (BigBite Shower
  + BigBite Preshower) detector. It does so by minimizing the chi2 of the difference between calorimeter
  cluster energy and the reconstructed electron energy(we get that from tracking information). It reads
  in the old adc gain coefficients (GeV/pC) and ratios and writes the new ones in file. One needs a configfile 
  to execute this script. Example content of such a file is attached at the end. To execute, follow:
  ----
  [a-onl@aonl2 macros]$ pwd
  /adaqfs/home/a-onl/sbs/BBCal_replay/macros
  [a-onl@aonl2 macros]$ root -l 
  root [0] .L Combined_macros/test_eng_cal_BBCal.C+
  root [1] test_eng_cal_BBCal("Combined_macros/setup_eng_cal_BBCal.txt")
  ----
  P. Datta  <pdbforce@jlab.org>  Created  15 Oct 2021 (Based on AJR Puckett & E Fuchey's version)
*/
#include <iostream>
#include <sstream>
#include <fstream>

#include "TCut.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TMath.h"
#include "TChain.h"
#include "TString.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TStopwatch.h"
#include "TTreeFormula.h"

const Double_t Mp = 0.938272081; // +/- 6E-9 GeV

const Int_t ncell = 241;   // 189(SH) + 52(PS), Convention: 0-188: SH; 189-240: PS.
const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows
const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows

const Double_t zposSH = 1.901952; //m
const Double_t zposPS = 1.695704; //m

void ReadGain(TString, Double_t*);

void bbcal_eng_calib_w_h2(const char *configfilename)
{
  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  TChain *C = new TChain("T");

  Int_t Set=0;
  Int_t Nmin=10;
  Double_t minMBratio=0.1;
  Double_t E_beam=0.;
  Double_t p_rec_Offset=1., p_min_cut=0., p_max_cut=0.;
  Double_t W_mean=0., W_sigma=0., EovP_cut_limit=0.3;
  bool cut_on_W=0, cut_on_EovP=0, cut_on_pmin=0, cut_on_pmax=0, farm_submit=0;
  Double_t Corr_Factor_Enrg_Calib_w_Cosmic=1., cF=1.;
  Double_t h_W_bin=200, h_W_min=0., h_W_max=5.;
  Double_t h_Q2_bin=200, h_Q2_min=0., h_Q2_max=5.;
  Double_t h_EovP_bin=200, h_EovP_min=0., h_EovP_max=5.;
  Double_t h_clusE_bin=200, h_clusE_min=0., h_clusE_max=5.;
  Double_t h_shE_bin=200, h_shE_min=0., h_shE_max=5.;
  Double_t h_psE_bin=200, h_psE_min=0., h_psE_max=5.;
  Double_t h2_p_bin=200, h2_p_min=0., h2_p_max=5.;
  Double_t h2_pang_bin=200, h2_pang_min=0., h2_pang_max=5.;
  Double_t h2_p_coarse_bin=25, h2_p_coarse_min=0., h2_p_coarse_max=5.;
  Double_t h2_EovP_bin=200, h2_EovP_min=0., h2_EovP_max=5.;
  //parameters to calculate calibrated momentum
  bool mom_calib=0;
  Double_t A_fit=0., B_fit=0., C_fit=0.;
  Double_t bb_magdist=1., GEMpitch=10.;

  TMatrixD M(ncell,ncell), M_inv(ncell,ncell);
  TVectorD B(ncell), CoeffR(ncell);
  
  Double_t E_e = 0;
  Double_t p_rec = 0.;
  Double_t A[ncell] = {0.};
  bool badCells[ncell]; // Cells that have events less than Nmin
  TString adcGain_SH, gainRatio_SH, outFile;
  TString adcGain_PS, gainRatio_PS;
  Int_t nevents_per_cell[ncell];

  // Define a clock to check macro processing time
  TStopwatch *sw = new TStopwatch();
  TStopwatch *sw2 = new TStopwatch();
  sw->Start();
  sw2->Start();

  // Reading config file
  ifstream configfile(configfilename);
  TString currentline;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endRunlist") ){
    if( !currentline.BeginsWith("#") ){
      C->Add(currentline);
    }   
  } 
  TCut globalcut = "";
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endcut") ){
    if( !currentline.BeginsWith("#") ){
      globalcut += currentline;
    }    
  }
  TTreeFormula *GlobalCut = new TTreeFormula("GlobalCut", globalcut, C);
  while( currentline.ReadLine( configfile ) ){
    if( currentline.BeginsWith("#") ) continue;
    TObjArray *tokens = currentline.Tokenize(" ");
    Int_t ntokens = tokens->GetEntries();
    if( ntokens>1 ){
      TString skey = ( (TObjString*)(*tokens)[0] )->GetString();
      if( skey == "Set" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Set = sval.Atoi();
      }
      if( skey == "E_beam" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	E_beam = sval.Atof();
      }
      if( skey == "farm_submit" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	farm_submit = sval.Atof();
      }
      if( skey == "Min_Event_Per_Channel" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Nmin = sval.Atof();
      }
      if( skey == "Min_MB_Ratio" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	minMBratio = sval.Atoi();
      }
      if( skey == "W_cut" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	cut_on_W = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	W_mean = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	W_sigma = sval2.Atof();
      }
      if( skey == "pmin_cut" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	cut_on_pmin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	p_min_cut = sval1.Atof();
      }
      if( skey == "pmax_cut" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	cut_on_pmax = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	p_max_cut = sval1.Atof();
      }
      if( skey == "EovP_cut" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	cut_on_EovP = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	EovP_cut_limit = sval1.Atof();
      }
      if( skey == "h_W" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_W_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_W_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_W_max = sval2.Atof();
      }
      if( skey == "h_Q2" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_Q2_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_Q2_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_Q2_max = sval2.Atof();
      }
      if( skey == "h_EovP" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_EovP_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_EovP_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_EovP_max = sval2.Atof();
      }
      if( skey == "h_clusE" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_clusE_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_clusE_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_clusE_max = sval2.Atof();
      }
      if( skey == "h_shE" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_shE_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_shE_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_shE_max = sval2.Atof();
      }
      if( skey == "h_psE" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_psE_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_psE_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_psE_max = sval2.Atof();
      }
      if( skey == "h2_p" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h2_p_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h2_p_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h2_p_max = sval2.Atof();
      }
      if( skey == "h2_pang" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h2_pang_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h2_pang_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h2_pang_max = sval2.Atof();
      }
      if( skey == "h2_p_coarse" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h2_p_coarse_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h2_p_coarse_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h2_p_coarse_max = sval2.Atof();
      }
      if( skey == "h2_EovP" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h2_EovP_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h2_EovP_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h2_EovP_max = sval2.Atof();
      }
      if( skey == "p_rec_Offset" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	p_rec_Offset = sval.Atof();
      }
      if( skey == "Corr_Factor_Enrg_Calib_w_Cosmic" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	cF = sval.Atof();
      }
      if( skey == "mom_calib" ){
      	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
      	mom_calib = sval.Atoi();
      	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
      	A_fit = sval1.Atof();
      	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
      	B_fit = sval2.Atof();
      	TString sval3 = ( (TObjString*)(*tokens)[4] )->GetString();
      	C_fit = sval3.Atof();
      	TString sval4 = ( (TObjString*)(*tokens)[5] )->GetString();
      	GEMpitch = sval4.Atof();
      	TString sval5 = ( (TObjString*)(*tokens)[6] )->GetString();
        bb_magdist = sval5.Atof();
      }
      if( skey == "*****" ){
	break;
      }
    } 
    delete tokens;
  }
  
  // Check for empty rootfiles and set tree branches
  if(C->GetEntries()==0){
    cerr << endl << " --- No ROOT file found!! --- " << endl << endl;
    throw;
  }else cout << endl << "Found " << C->GetEntries() << " events. Starting analysis.. " << endl;

  int maxNtr = 1000;
  C->SetBranchStatus("*", 0);
  // bb.ps branches
  C->SetBranchStatus("bb.ps.*", 1);
  Double_t psIdblk;            C->SetBranchAddress("bb.ps.idblk", &psIdblk);
  Double_t psRowblk;           C->SetBranchAddress("bb.ps.rowblk", &psRowblk);
  Double_t psColblk;           C->SetBranchAddress("bb.ps.colblk", &psColblk);
  Double_t psNblk;             C->SetBranchAddress("bb.ps.nblk", &psNblk);
  //Double_t psAgainblk;         C->SetBranchAddress("bb.ps.againblk", &psAgainblk);
  Double_t psNclus;            C->SetBranchAddress("bb.ps.nclus", &psNclus);
  Double_t psE;                C->SetBranchAddress("bb.ps.e", &psE);
  Double_t psClBlkId[maxNtr];  C->SetBranchAddress("bb.ps.clus_blk.id", &psClBlkId);
  Double_t psClBlkE[maxNtr];   C->SetBranchAddress("bb.ps.clus_blk.e", &psClBlkE);
  // bb.sh branches
  C->SetBranchStatus("bb.sh.*", 1);
  Double_t shIdblk;            C->SetBranchAddress("bb.sh.idblk", &shIdblk);
  Double_t shRowblk;           C->SetBranchAddress("bb.sh.rowblk", &shRowblk);
  Double_t shColblk;           C->SetBranchAddress("bb.sh.colblk", &shColblk);
  Double_t shNblk;             C->SetBranchAddress("bb.sh.nblk", &shNblk);
  //Double_t shAgainblk;         C->SetBranchAddress("bb.sh.againblk", &shAgainblk);
  Double_t shNclus;            C->SetBranchAddress("bb.sh.nclus", &shNclus);
  Double_t shE;                C->SetBranchAddress("bb.sh.e", &shE);
  Double_t shClBlkId[maxNtr];  C->SetBranchAddress("bb.sh.clus_blk.id", &shClBlkId);
  Double_t shClBlkE[maxNtr];   C->SetBranchAddress("bb.sh.clus_blk.e", &shClBlkE);
  // bb.tr branches
  C->SetBranchStatus("bb.tr.*", 1);
  Double_t trP[maxNtr];        C->SetBranchAddress("bb.tr.p", &trP);
  Double_t trPz[maxNtr];       C->SetBranchAddress("bb.tr.pz", &trPz);
  Double_t trX[maxNtr];        C->SetBranchAddress("bb.tr.x", &trX);
  Double_t trY[maxNtr];        C->SetBranchAddress("bb.tr.y", &trY);
  Double_t trN[maxNtr];        C->SetBranchAddress("bb.tr.n", &trN);
  Double_t trTh[maxNtr];       C->SetBranchAddress("bb.tr.th", &trTh);
  Double_t trPh[maxNtr];       C->SetBranchAddress("bb.tr.ph", &trPh);
  Double_t trVz[maxNtr];       C->SetBranchAddress("bb.tr.vz", &trVz);
  Double_t trTgth[maxNtr];     C->SetBranchAddress("bb.tr.tg_th", &trTgth);
  Double_t trTgph[maxNtr];     C->SetBranchAddress("bb.tr.tg_ph", &trTgph);
  Double_t trRth[maxNtr];      C->SetBranchAddress("bb.tr.r_th", &trRth);
  Double_t trRph[maxNtr];      C->SetBranchAddress("bb.tr.r_ph", &trRph);
  // turning on additional branches for the global cut
  C->SetBranchStatus("sbs.hcal.e", 1);
  C->SetBranchStatus("bb.gem.track.nhits", 1);

  // Clear arrays
  memset(nevents_per_cell, 0, ncell*sizeof(int));
  memset(badCells, 0, ncell*sizeof(bool));
  
  // Let's read in old gain coefficients for both SH and PS
  cout << endl;
  Double_t oldADCgainSH[kNcolsSH*kNrowsSH];
  Double_t oldADCgainPS[kNcolsPS*kNrowsPS];
  for (int i=0; i<189; i++) {oldADCgainSH[i] = -1000.;}  
  for (int i=0; i<52; i++) {oldADCgainPS[i] = -1000.;}  
  adcGain_SH = Form("Gain/eng_cal_gainCoeff_sh_%d.txt", Set);
  ReadGain(adcGain_SH, oldADCgainSH);
  adcGain_PS = Form("Gain/eng_cal_gainCoeff_ps_%d.txt", Set);
  ReadGain(adcGain_PS, oldADCgainPS);

  gStyle->SetOptStat(0);
  gStyle->SetPalette(60);
  TH2D *h2_SHeng_vs_SHblk_raw = new TH2D("h2_SHeng_vs_SHblk_raw","Raw E_clus(SH) per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_raw = new TH2D("h2_EovP_vs_SHblk_raw","Raw E_clus/p_rec per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_count = new TH2D("h2_count","Count for E_clus/p_rec per per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS_raw = new TH2D("h2_EovP_vs_SHblk_trPOS_raw","Raw E_clus/p_rec per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_count_trP = new TH2D("h2_count_trP","Count for E_clus/p_rec per per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_vs_PSblk_raw = new TH2D("h2_PSeng_vs_PSblk_raw","Raw E_clus(PS) per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_raw = new TH2D("h2_EovP_vs_PSblk_raw","Raw E_clus/p_rec per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_count_PS = new TH2D("h2_count_PS","Count for E_clus/p_rec per per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_trPOS_raw = new TH2D("h2_EovP_vs_PSblk_trPOS_raw","Raw E_clus/p_rec per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);
  TH2D *h2_count_trP_PS = new TH2D("h2_count_trP_PS","Count for E_clus/p_rec per per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  // Creating output ROOT file to contain histograms
  outFile = Form("hist/eng_cal_BBCal_%d.root", Set);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  // Physics histograms
  TH1D *h_W = new TH1D("h_W","W distribution",h_W_bin,h_W_min,h_W_max);
  TH1D *h_Q2 = new TH1D("h_Q2","Q2 distribution",h_Q2_bin,h_Q2_min,h_Q2_max);
  TH1D *h_EovP = new TH1D("h_EovP","E_clus/p_rec",h_EovP_bin,h_EovP_min,h_EovP_max);
  TH1D *h_EovP_calib = new TH1D("h_EovP_calib","E_clus/p_rec",h_EovP_bin,h_EovP_min,h_EovP_max);
  TH1D *h_clusE = new TH1D("h_clusE","Best Cluster Energy (SH+PS)",h_clusE_bin,h_clusE_min,h_clusE_max);
  TH1D *h_clusE_calib = new TH1D("h_clusE_calib",Form("Best Cluster Energy (SH+PS) u (sh/ps.e)*%2.2f",cF),h_clusE_bin,h_clusE_min,h_clusE_max);
  TH1D *h_SHclusE = new TH1D("h_SHclusE","Best SH Cluster Energy",h_shE_bin,h_shE_min,h_shE_max);
  TH1D *h_SHclusE_calib = new TH1D("h_SHclusE_calib",Form("Best SH Cluster Energy u (sh.e)*%2.2f",cF),h_shE_bin,h_shE_min,h_shE_max);
  TH1D *h_PSclusE = new TH1D("h_PSclusE","Best PS Cluster Energy",h_psE_bin,h_psE_min,h_psE_max);
  TH1D *h_PSclusE_calib = new TH1D("h_PSclusE_calib",Form("Best PS Cluster Energy u (ps.e)*%2.2f",cF),h_psE_bin,h_psE_min,h_psE_max);
  TH2D *h2_P_rec_vs_P_ang = new TH2D("h2_P_rec_vs_P_ang","Track p vs Track ang",h2_pang_bin,h2_pang_min,h2_pang_max,h2_p_bin,h2_p_min,h2_p_max);

  TH2D *h2_EovP_vs_P = new TH2D("h2_EovP_vs_P","E/p vs p; p (GeV); E/p",h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h2_EovP_bin,h2_EovP_min,h2_EovP_max);
  TH2D *h2_EovP_vs_P_calib = new TH2D("h2_EovP_vs_P_calib","E/p vs p; p (GeV); E/p",h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h2_EovP_bin,h2_EovP_min,h2_EovP_max);

  TH2D *h2_SHeng_vs_SHblk = new TH2D("h2_SHeng_vs_SHblk","Average E_clus(SH)*%2.2f per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk = new TH2D("h2_EovP_vs_SHblk","Average E_clus*%2.2f/p_rec per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS = new TH2D("h2_EovP_vs_SHblk_trPOS","Average E_clus*%2.2f/p_rec per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_vs_PSblk = new TH2D("h2_PSeng_vs_PSblk","Average E_clus(PS)*%2.2f per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk = new TH2D("h2_EovP_vs_PSblk","Average E_clus*%2.2f/p_rec per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_trPOS = new TH2D("h2_EovP_vs_PSblk_trPOS","Average E_clus*%2.2f/p_rec per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  TH1D *h_thetabend = new TH1D("h_thetabend","",100,0.,0.25);

  // Looping over good events ====================================================================//
  Long64_t Nevents = C->GetEntries(), nevent=0;  
  cout << endl << "Processing " << Nevents << " events.." << endl;
  Double_t timekeeper = 0., timeremains = 0.;
  int treenum = 0, currenttreenum = 0;
  TEventList *gevlist = new TEventList("gevlist"); // list of good events passed all the cuts (excluding edge cut)
  vector<Long64_t> goodevents; // list of good events passed all the cuts (excluding edge cut)

  while(C->GetEntry( nevent++ )) {

    // Calculating remaining time 
    sw2->Stop();
    timekeeper += sw2->RealTime();
    if( nevent%25000 == 0 && nevent!=0 ) 
      timeremains = timekeeper*( double(Nevents)/double(nevent) - 1. ); 
    sw2->Reset();
    sw2->Continue();

    if( nevent % 100 == 0 ) cout << nevent << "/" << Nevents  << ", " << int(timeremains/60.) << "m \r";;
    cout.flush();
    // ------

    // get old gain coefficients
    // oldADCgainSH[int(shIdblk)] = shAgainblk;
    // oldADCgainPS[int(psIdblk)] = psAgainblk;

    // apply global cuts efficiently (AJRP method)
    currenttreenum = C->GetTreeNumber();
    if (nevent == 1 || currenttreenum != treenum) {
      treenum = currenttreenum;
      GlobalCut->UpdateFormulaLeaves();
    } 
    bool passedgCut = GlobalCut->EvalInstance(0) != 0;   
    if (passedgCut) {
    
      E_e = 0;
      memset(A, 0, ncell*sizeof(double));

      p_rec = (trP[0])*p_rec_Offset; 
      E_e = p_rec; // Neglecting e- mass. 

      // *---- calculating calibrated momentum (Helps avoiding replay)
      if(mom_calib){
	TVector3 enhat_tgt( trTgth[0], trTgph[0], 1.0 );
	enhat_tgt = enhat_tgt.Unit();	
	TVector3 enhat_fp( trRth[0], trRph[0], 1.0 );
	enhat_fp = enhat_fp.Unit();
	TVector3 GEMzaxis(-sin(GEMpitch*TMath::DegToRad()),0,cos(GEMpitch*TMath::DegToRad()));
	TVector3 GEMyaxis(0,1,0);
	TVector3 GEMxaxis = (GEMyaxis.Cross(GEMzaxis)).Unit();	
	TVector3 enhat_fp_rot = enhat_fp.X() * GEMxaxis + enhat_fp.Y() * GEMyaxis + enhat_fp.Z() * GEMzaxis;
	double thetabend = acos( enhat_fp_rot.Dot( enhat_tgt ) );
	h_thetabend->Fill(thetabend);

	p_rec = (A_fit*( 1. + (B_fit + C_fit*bb_magdist)*trTgth[0] )/thetabend)*p_rec_Offset;
	E_e = p_rec;
      }
      // *----

      // cut on E/p
      if( cut_on_EovP ) if( fabs( (shE+psE)/p_rec - 1. )>EovP_cut_limit ) continue;

      // cut on p
      if( cut_on_pmin ) if( p_rec<p_min_cut ) continue;
      if( cut_on_pmax ) if( p_rec>p_max_cut ) continue;

      if(trP[0]==0 || E_e==0 ) continue;

      Double_t P_ang = TMath::ACos(trPz[0]/trP[0]);
      Double_t Q2 = 4. * E_beam * p_rec * pow(TMath::Sin((P_ang/2.)), 2.);
      P_ang *= TMath::RadToDeg();
      Double_t W2 = Mp*Mp + 2.*Mp*(E_beam - p_rec) - Q2;
      Double_t W = sqrt(max(0., W2));
      h_Q2->Fill(Q2);
      h_W->Fill(W);

      // cut on W
      if( cut_on_W ) if( fabs(W-W_mean)>W_sigma ) continue;

      // Reject events with max edep on the edge
      if(shRowblk==0 || shRowblk==26 ||
	 shColblk==0 || shColblk==6) continue; 

      // storing good event numbers for 2nd loop
      goodevents.push_back(nevent);
      
      // Loop over all the blocks in main cluster and fill in A's
      for(Int_t blk=0; blk<shNblk; blk++){
	Int_t blkID = int(shClBlkId[blk]);
	A[blkID] += shClBlkE[blk];
	nevents_per_cell[ blkID ]++; 
      }
    
      // ****** PreShower ******
      for(Int_t blk=0; blk<psNblk; blk++){
	Int_t blkID = int(psClBlkId[blk]);
	A[189+blkID] += psClBlkE[blk];
	nevents_per_cell[189+blkID]++;
      }
    

      // Let's fill some interesting histograms
      Double_t clusEngBBCal = (shE + psE)*Corr_Factor_Enrg_Calib_w_Cosmic;
      Double_t ClusEngSH = shE*Corr_Factor_Enrg_Calib_w_Cosmic;
      Double_t ClusEngPS = psE*Corr_Factor_Enrg_Calib_w_Cosmic;
      h_EovP->Fill( (clusEngBBCal/p_rec) );
      h_clusE->Fill( clusEngBBCal );
      h_SHclusE->Fill( ClusEngSH );
      h_PSclusE->Fill( ClusEngPS );
      h2_P_rec_vs_P_ang->Fill( P_ang, p_rec );

      // Checking to see if there is any bias in track recostruction ----
      //SH
      h2_SHeng_vs_SHblk_raw->Fill( shColblk, shRowblk, ClusEngSH );
      h2_EovP_vs_SHblk_raw->Fill( shColblk, shRowblk, (clusEngBBCal/p_rec) );
      h2_count->Fill( shColblk, shRowblk, 1.);
      h2_SHeng_vs_SHblk->Divide( h2_SHeng_vs_SHblk_raw, h2_count );
      h2_EovP_vs_SHblk->Divide( h2_EovP_vs_SHblk_raw, h2_count );

      Double_t xtrATsh = trX[0] + zposSH*trTh[0];
      Double_t ytrATsh = trY[0] + zposSH*trPh[0];
      h2_EovP_vs_SHblk_trPOS_raw->Fill( ytrATsh, xtrATsh, (clusEngBBCal/p_rec) );
      h2_count_trP->Fill( ytrATsh, xtrATsh, 1. );
      h2_EovP_vs_SHblk_trPOS->Divide( h2_EovP_vs_SHblk_trPOS_raw, h2_count_trP );

      //PS
      h2_PSeng_vs_PSblk_raw->Fill( psColblk, psRowblk, ClusEngPS );
      h2_EovP_vs_PSblk_raw->Fill( psColblk, psRowblk, (clusEngBBCal/p_rec) );
      h2_count_PS->Fill( psColblk, psRowblk, 1.);
      h2_PSeng_vs_PSblk->Divide( h2_PSeng_vs_PSblk_raw, h2_count_PS );
      h2_EovP_vs_PSblk->Divide( h2_EovP_vs_PSblk_raw, h2_count_PS );

      Double_t xtrATps = trX[0] + zposPS*trTh[0];
      Double_t ytrATps = trY[0] + zposPS*trPh[0];
      h2_EovP_vs_PSblk_trPOS_raw->Fill( ytrATps, xtrATps, (clusEngBBCal/p_rec) );
      h2_count_trP_PS->Fill( ytrATps, xtrATps, 1. );
      h2_EovP_vs_PSblk_trPOS->Divide( h2_EovP_vs_PSblk_trPOS_raw, h2_count_trP_PS );
      // -----

      // E/p vs. p
      h2_EovP_vs_P->Fill( p_rec, clusEngBBCal/p_rec );

      // Let's customize the histograms
      h2_SHeng_vs_SHblk->GetZaxis()->SetRangeUser(0.9,2.0);
      h2_EovP_vs_SHblk->GetZaxis()->SetRangeUser(0.8,1.2);
      h2_EovP_vs_SHblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);
      h2_PSeng_vs_PSblk->GetZaxis()->SetRangeUser(0.36,1.28);
      h2_EovP_vs_PSblk->GetZaxis()->SetRangeUser(0.8,1.2);
      h2_EovP_vs_PSblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);

      // Let's costruct the matrix
      for(Int_t icol = 0; icol<ncell; icol++){
	B(icol)+= A[icol];
	for(Int_t irow = 0; irow<ncell; irow++){
	  M(icol,irow)+= A[icol]*A[irow]/E_e;
	} 
      }   
      
    } //global cut
  } //event loop

  cout << endl << endl;
  
  // B.Print();  
  // M.Print();

  //Diagnostic histograms
  TH1D *h_nevent_blk_SH = new TH1D("h_nevent_blk_SH","No. of Good Events; SH Blocks",189,0,189);
  TH1D *h_coeff_Ratio_SH = new TH1D("h_coeff_Ratio_SH","Ratio of Gain Coefficients(new/old); SH Blocks",189,0,189);
  TH1D *h_coeff_blk_SH = new TH1D("h_coeff_blk_SH","ADC Gain Coefficients(GeV/pC); SH Blocks",189,0,189);
  TH1D *h_Old_Coeff_blk_SH = new TH1D("h_Old_Coeff_blk_SH","Old ADC Gain Coefficients(GeV/pC); SH Blocks",189,0,189);
  TH2D *h_coeff_detView_SH = new TH2D("h_coeff_detView_SH","ADC Gain Coefficients(Detector View)",kNcolsSH,1,kNcolsSH+1,kNrowsSH,1,kNrowsSH+1);

  TH1D *h_nevent_blk_PS = new TH1D("h_nevent_blk_PS","No. of Good Events; PS Blocks",52,0,52);
  TH1D *h_coeff_Ratio_PS = new TH1D("h_coeff_Ratio_PS","Ratio of Gain Coefficients(new/old); PS Blocks",52,0,52);
  TH1D *h_coeff_blk_PS = new TH1D("h_coeff_blk_PS","ADC Gain Coefficients(GeV/pC); PS Blocks",52,0,52);
  TH1D *h_Old_Coeff_blk_PS = new TH1D("h_Old_Coeff_blk_PS","Old ADC Gain Coefficients(GeV/pC); PS Blocks",52,0,52);
  TH2D *h_coeff_detView_PS = new TH2D("h_coeff_detView_PS","ADC Gain Coefficients(Detector View)",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);

  // Leave the bad channels out of the calculation
  for(Int_t j = 0; j<ncell; j++){
    badCells[j]=false;
    if( nevents_per_cell[j]<Nmin || M(j,j)< minMBratio*B(j) ){
      B(j) = 1.;
      M(j, j) = 1.;
      for(Int_t k = 0; k<ncell; k++){
	if(k!=j){
	  M(j, k) = 0.;
	  M(k, j) = 0.;
	}
      }
      badCells[j]=true;
    }
  }  
  
  // Getting coefficients (rather ratios)
  M_inv = M.Invert();
  CoeffR = M_inv*B;

  // SH : Filling diagnostic histograms
  Int_t cell = 0;
  adcGain_SH = Form("Gain/eng_cal_gainCoeff_sh_%d_1.txt", Set);
  gainRatio_SH = Form("Gain/eng_cal_gainRatio_sh_%d_1.txt", Set);
  Double_t newADCgratioSH[kNcolsSH*kNrowsSH];
  for (int i=0; i<189; i++) {newADCgratioSH[i] = -1000.;}  
  ofstream adcGainSH_outData, gainRatioSH_outData;
  adcGainSH_outData.open(adcGain_SH);
  gainRatioSH_outData.open(gainRatio_SH);
  for(Int_t shrow = 0; shrow<kNrowsSH; shrow++){
    for(Int_t shcol = 0; shcol<kNcolsSH; shcol++){
      Double_t oldCoeff = oldADCgainSH[shrow*kNcolsSH+shcol];
      if(!badCells[cell]){
	h_coeff_Ratio_SH->Fill( cell, CoeffR(cell) );
	h_coeff_blk_SH->Fill( cell, CoeffR(cell)*oldCoeff );
	h_nevent_blk_SH->Fill( cell, nevents_per_cell[cell] );
	h_Old_Coeff_blk_SH->Fill( cell, oldCoeff );
	h_coeff_detView_SH->Fill( shcol+1, shrow+1, CoeffR(cell)*oldCoeff );

	cout << CoeffR(cell) << "  ";
	adcGainSH_outData << CoeffR(cell)*oldCoeff << " ";
	gainRatioSH_outData << CoeffR(cell) << " ";
	newADCgratioSH[cell] = CoeffR(cell);
      }else{
	h_nevent_blk_SH->Fill( cell, nevents_per_cell[cell] );
	h_Old_Coeff_blk_SH->Fill( cell, oldCoeff );
	h_coeff_Ratio_SH->Fill( cell, 1.*Corr_Factor_Enrg_Calib_w_Cosmic );
	h_coeff_blk_SH->Fill( cell, oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic );
	h_coeff_detView_SH->Fill( shcol+1, shrow+1, oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic );

	cout << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << "  ";
	adcGainSH_outData << oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	gainRatioSH_outData << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	newADCgratioSH[cell] = 1.*Corr_Factor_Enrg_Calib_w_Cosmic;
      }
      cell++;
    }
    cout << endl;
    adcGainSH_outData << endl;
    gainRatioSH_outData << endl;
  }
  cout << endl;

  // PS : Filling diagnostic histograms
  adcGain_PS = Form("Gain/eng_cal_gainCoeff_ps_%d_1.txt", Set);
  gainRatio_PS = Form("Gain/eng_cal_gainRatio_ps_%d_1.txt", Set);
  Double_t newADCgratioPS[kNcolsPS*kNrowsPS];
  for (int i=0; i<52; i++) {newADCgratioPS[i] = -1000.;}  
  ofstream adcGainPS_outData, gainRatioPS_outData;
  adcGainPS_outData.open(adcGain_PS);
  gainRatioPS_outData.open(gainRatio_PS);
  for(Int_t psrow = 0; psrow<kNrowsPS; psrow++){
    for(Int_t pscol = 0; pscol<kNcolsPS; pscol++){
      Int_t psBlock = psrow*kNcolsPS+pscol;
      Double_t oldCoeff = oldADCgainPS[psrow*kNcolsPS+pscol];
      if(!badCells[cell]){
	h_coeff_Ratio_PS->Fill( psBlock, CoeffR(cell) );
	h_coeff_blk_PS->Fill( psBlock, CoeffR(cell)*oldCoeff );
	h_nevent_blk_PS->Fill( psBlock, nevents_per_cell[cell] );
	h_Old_Coeff_blk_PS->Fill( psBlock, oldCoeff );
	h_coeff_detView_PS->Fill( pscol+1, psrow+1, CoeffR(cell)*oldCoeff );

	cout << CoeffR(cell) << "  ";
	adcGainPS_outData << CoeffR(cell)*oldCoeff << " ";
	gainRatioPS_outData << CoeffR(cell) << " ";
	newADCgratioPS[psrow*kNcolsPS+pscol] = CoeffR(cell);
      }else{
	h_nevent_blk_PS->Fill( psBlock, nevents_per_cell[cell] );
	h_Old_Coeff_blk_PS->Fill( psBlock, oldCoeff );
	h_coeff_Ratio_PS->Fill( psBlock, 1.*Corr_Factor_Enrg_Calib_w_Cosmic );
	h_coeff_blk_PS->Fill( psBlock, oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic );
	h_coeff_detView_PS->Fill( pscol+1, psrow+1, oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic );

	cout << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << "  ";
	adcGainPS_outData << oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	gainRatioPS_outData << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	newADCgratioPS[psrow*kNcolsPS+pscol] = 1.*Corr_Factor_Enrg_Calib_w_Cosmic;
      }
      cell++;
    }
    cout << endl;
    adcGainPS_outData << endl;
    gainRatioPS_outData << endl;
  }
  cout << endl;

  Long64_t itr = 0; nevent = 0;
  cout << "Looping over events again to check calibration.." << endl; 
  while(C->GetEntry(nevent++)) {

    if (nevent % 100 == 0) cout << nevent << "/" << Nevents  << "\r";;
    cout.flush();    

    if (nevent == goodevents[itr]) {
    
      p_rec = trP[0] * p_rec_Offset; 

      // ****** Shower ******
      Double_t shClusE = 0.;
      for(Int_t blk=0; blk<shNblk; blk++){
	Int_t blkID = int(shClBlkId[blk]);
	shClusE += shClBlkE[blk] * newADCgratioSH[blkID];
      }
    
      // ****** PreShower ******
      Double_t psClusE = 0.;
      for(Int_t blk=0; blk<psNblk; blk++){
	Int_t blkID = int(psClBlkId[blk]);
	psClusE += psClBlkE[blk] * newADCgratioPS[blkID];
      }

      // Let's fill diagnostic histograms
      Double_t clusEngBBCal = shClusE + psClusE;
      h_EovP_calib->Fill( (clusEngBBCal / p_rec) );
      h_clusE_calib->Fill( clusEngBBCal );
      h_SHclusE_calib->Fill( shClusE );
      h_PSclusE_calib->Fill( psClusE );
      h2_EovP_vs_P_calib->Fill( p_rec, clusEngBBCal/p_rec );
      itr++;
    }
  }
  cout << endl << endl;

  fout->Write();
  fout->Close();

  M.Clear();
  B.Clear();
  C->Delete();
  CoeffR.Clear();
  fout->Delete();
  adcGainSH_outData.close();
  adcGainPS_outData.close();
  gainRatioSH_outData.close();
  gainRatioPS_outData.close();

  cout << "Finishing analysis.." << endl;
  cout << " --------- " << endl;
  cout << " Resulting histograms  written to : " << outFile << endl;
  cout << " Gain ratios (new/old) for SH written to : " << gainRatio_SH << endl;
  cout << " Gain ratios (new/old) for PS written to : " << gainRatio_PS << endl;
  cout << " New adc gain coefficients (GeV/pC) for SH written to : " << adcGain_SH << endl;
  cout << " New adc gain coefficients (GeV/pC) for PS written to : " << adcGain_PS << endl;
  cout << " --------- " << endl;

  sw->Stop();
  sw2->Stop();
  cout << "CPU time elapsed = " << sw->CpuTime() << " s. Real time = " 
       << sw->RealTime() << " s. " << endl << endl;

  sw->Delete();
  sw2->Delete();
}


void ReadGain( TString adcGain_rfile, Double_t* adcGain ){
  ifstream adcGain_data;
  adcGain_data.open(adcGain_rfile);
  string readline;
  Int_t elemID=0;
  if( adcGain_data.is_open() ){
    cout << " Reading ADC gain from : "<< adcGain_rfile << endl;
    while(getline(adcGain_data,readline)){
      istringstream tokenStream(readline);
      string token;
      char delimiter = ' ';
      while(getline(tokenStream,token,delimiter)){
  	TString temptoken=token;
  	adcGain[elemID] = temptoken.Atof();
  	elemID++;
      }
    }
  }else{
    cerr << " **!** No file : " << adcGain_rfile << endl << endl;
    throw;
  }
  adcGain_data.close();
}


/*
  Example configfile required to run this script:
  ----
  [a-onl@aonl2 Combined_macros]$ cat setup_example.txt 
  example_run_list.txt
  endRunlist
  root.example1.tree>0&&root.example2.tree
  endcut
  E_beam 1.92
  Min_Event_Per_Channel 1000
  Min_MB_Ratio 0.1
  p_rec_Offset 1.0
  W_mean 0.9379
  W_sigma 0.02
  cut_on_W 1
  Corr_Factor_Enrg_Calib_w_Cosmic 1.0 
  ----
*/

/*
  List of input and output files:
  *Input files: 
  1. Gain/eng_cal_gainCoeff_sh(ps)_+(iter-1)+.txt # Contains old gain coeff. for SH(PS) 
  2. Gain/eng_cal_gainRatio_sh(ps)_+(iter-1)+.txt # Contains gain ratio (new/old) for SH(PS)   
  *Output files:
  1. hist/eng_cal_BBCal_+iter+.root # Contains all the interesting histograms
  2. Gain/eng_cal_gainRatio_sh(ps)_+iter+.txt # Contains gain ratios (new/old) for SH(PS)
  3. Gain/eng_cal_gainCoeff_sh(ps)_+iter+.txt # Contains new gain coeff. for SH(PS)
*/
