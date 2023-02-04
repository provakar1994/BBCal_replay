/*
  This script has been prepared for the energy calibration of BigBite Calorimeter (BigBite Shower
  + BigBite Preshower) detector. It does so by minimizing the chi2 of the difference between calorimeter
  cluster energy and the reconstructed electron energy. It gets the old adc gain coefficients (GeV/pC) 
  from tree and writes the new adc gain coeffs. and ratios (New/Old) in file. One needs a configfile 
  to execute this script [see cfg/example.cfg]. To execute, do:
  ----
  [a-onl@aonl2 macros]$ pwd
  /adaqfs/home/a-onl/sbs/BBCal_replay/macros
  [a-onl@aonl2 macros]$ root -l 
  root [0] .x Combined_macros/bbcal_eng_calib_w_h2.C("Combined_macros/cfg/example.cfg")
  ----
  P. Datta  <pdbforce@jlab.org>  Created  28 Sep 2022 (Based on test_eng_cal_BBCal.C)
*/
#include <sstream>
#include <fstream>
#include <iostream>

#include "TCut.h"
#include "TH2D.h"
#include "TMath.h"
#include "TChain.h"
#include "TString.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TStopwatch.h"
#include "TTreeFormula.h"

const Double_t Mp = 0.938272081;  // +/- 6E-9 GeV

const Int_t ncell = 241;          // 189(SH) + 52(PS), Convention: 0-188: SH; 189-240: PS.
const Int_t kNcolsSH = 7;         // SH columns
const Int_t kNrowsSH = 27;        // SH rows
const Int_t kNcolsPS = 2;         // PS columns
const Int_t kNrowsPS = 26;        // PS rows
const Double_t zposSH = 1.901952; // m
const Double_t zposPS = 1.695704; // m

string getDate();
void ReadGain(TString, Double_t*);
TString GetOutFileBase(TString);

void bbcal_eng_calib_w_h2(const char *configfilename)
{
  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  TChain *C = new TChain("T");
  //creating base for outfile names
  TString cfgfilebase = GetOutFileBase(configfilename);

  TString macros_dir;
  Int_t Nmin = 10;
  Double_t minMBratio = 0.1;
  Double_t E_beam = 0.;
  Double_t p_rec_Offset = 1., p_min_cut = 0., p_max_cut = 0.;
  Double_t W_mean = 0., W_sigma = 0., EovP_cut_limit = 0.3;
  bool cut_on_W = 0, cut_on_EovP = 0, cut_on_pmin = 0, cut_on_pmax = 0, read_gain = 0;
  Double_t Corr_Factor_Enrg_Calib_w_Cosmic = 1., cF = 1.;
  Double_t h_W_bin = 200, h_W_min = 0., h_W_max = 5.;
  Double_t h_Q2_bin = 200, h_Q2_min = 0., h_Q2_max = 5.;
  Double_t h_EovP_bin = 200, h_EovP_min = 0., h_EovP_max = 5., EovP_fit_width = 1.5;
  Double_t h_clusE_bin = 200, h_clusE_min = 0., h_clusE_max = 5.;
  Double_t h_shE_bin = 200, h_shE_min = 0., h_shE_max = 5.;
  Double_t h_psE_bin = 200, h_psE_min = 0., h_psE_max = 5.;
  Double_t h2_p_bin = 200, h2_p_min = 0., h2_p_max = 5.;
  Double_t h2_pang_bin = 200, h2_pang_min = 0., h2_pang_max = 5.;
  Double_t h2_p_coarse_bin = 25, h2_p_coarse_min = 0., h2_p_coarse_max = 5.;
  Double_t h2_EovP_bin = 200, h2_EovP_min = 0., h2_EovP_max = 5.;
  //parameters to calculate calibrated momentum
  bool mom_calib = 0;
  Double_t A_fit = 0., B_fit = 0., C_fit = 0.;
  Double_t bb_magdist = 1., GEMpitch = 10.;

  TMatrixD M(ncell,ncell), M_inv(ncell,ncell);
  TVectorD B(ncell), CoeffR(ncell);
  
  Double_t E_e = 0;
  Double_t p_rec = 0.;
  Double_t A[ncell];
  bool badCells[ncell]; // Cells that have events less than Nmin
  Int_t nevents_per_cell[ncell];

  // Define a clock to check macro processing time
  TStopwatch *sw = new TStopwatch();
  TStopwatch *sw2 = new TStopwatch();
  sw->Start();
  sw2->Start();

  // Reading config file
  ifstream configfile(configfilename);
  char runlistfile[1000]; 
  TString currentline, readline;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endRunlist") ){
    if( !currentline.BeginsWith("#") ){
      sprintf(runlistfile,"%s",currentline.Data());
      ifstream run_list(runlistfile);
      while( readline.ReadLine( run_list ) && !readline.BeginsWith("endlist") ){
  	if( !readline.BeginsWith("#") ){
  	  cout << readline << endl;
  	  C->Add(readline);
  	}
      }   
    } 
  }
  // TString currentline;
  // while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endRunlist") ){
  //   if( !currentline.BeginsWith("#") ){
  //     C->Add(currentline);
  //   }   
  // } 
  TCut globalcut = ""; TString gcutstr;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endcut") ){
    if( !currentline.BeginsWith("#") ){
      globalcut += currentline;
      gcutstr += currentline;
    }    
  }
  TTreeFormula *GlobalCut = new TTreeFormula("GlobalCut", globalcut, C);
  while( currentline.ReadLine( configfile ) ){
    if( currentline.BeginsWith("#") ) continue;
    TObjArray *tokens = currentline.Tokenize(" ");
    Int_t ntokens = tokens->GetEntries();
    if( ntokens>1 ){
      TString skey = ( (TObjString*)(*tokens)[0] )->GetString();
      if( skey == "macros_dir" ){
	macros_dir = ( (TObjString*)(*tokens)[1] )->GetString();
      }
      if( skey == "E_beam" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	E_beam = sval.Atof();
      }
      if( skey == "read_gain" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	read_gain = sval.Atof();
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
     if( skey == "EovP_fit_width" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	EovP_fit_width = sval.Atof();
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
  // beam energy
  Double_t HALLA_p;            C->SetBranchStatus("HALLA_p", 1); C->SetBranchAddress("HALLA_p", &HALLA_p);
  // bb.ps branches
  C->SetBranchStatus("bb.ps.*", 1);
  Double_t psIdblk;            C->SetBranchAddress("bb.ps.idblk", &psIdblk);
  Double_t psRowblk;           C->SetBranchAddress("bb.ps.rowblk", &psRowblk);
  Double_t psColblk;           C->SetBranchAddress("bb.ps.colblk", &psColblk);
  Double_t psNblk;             C->SetBranchAddress("bb.ps.nblk", &psNblk);
  Double_t psNclus;            C->SetBranchAddress("bb.ps.nclus", &psNclus);
  Double_t psE;                C->SetBranchAddress("bb.ps.e", &psE);
  Double_t psClBlkId[maxNtr];  C->SetBranchAddress("bb.ps.clus_blk.id", &psClBlkId);
  Double_t psClBlkE[maxNtr];   C->SetBranchAddress("bb.ps.clus_blk.e", &psClBlkE);
  Double_t psAgainblk;         if (!read_gain) C->SetBranchAddress("bb.ps.againblk", &psAgainblk);
  // bb.sh branches
  C->SetBranchStatus("bb.sh.*", 1);
  Double_t shIdblk;            C->SetBranchAddress("bb.sh.idblk", &shIdblk);
  Double_t shRowblk;           C->SetBranchAddress("bb.sh.rowblk", &shRowblk);
  Double_t shColblk;           C->SetBranchAddress("bb.sh.colblk", &shColblk);
  Double_t shNblk;             C->SetBranchAddress("bb.sh.nblk", &shNblk);
  Double_t shNclus;            C->SetBranchAddress("bb.sh.nclus", &shNclus);
  Double_t shE;                C->SetBranchAddress("bb.sh.e", &shE);
  Double_t shClBlkId[maxNtr];  C->SetBranchAddress("bb.sh.clus_blk.id", &shClBlkId);
  Double_t shClBlkE[maxNtr];   C->SetBranchAddress("bb.sh.clus_blk.e", &shClBlkE);
  Double_t shAgainblk;         if (!read_gain) C->SetBranchAddress("bb.sh.againblk", &shAgainblk);
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
  for (int i=0; i<189; i++) { oldADCgainSH[i] = -1000; }  
  for (int i=0; i<52; i++) { oldADCgainPS[i] = -1000; }  
  TString adcGain_SH = Form("%s/Gain/%s_gainCoeff_sh.txt", macros_dir.Data(), cfgfilebase.Data());
  TString adcGain_PS = Form("%s/Gain/%s_gainCoeff_ps.txt", macros_dir.Data(), cfgfilebase.Data());
  if (read_gain) {
    ReadGain(adcGain_SH, oldADCgainSH);
    ReadGain(adcGain_PS, oldADCgainPS);
  }
  
  gStyle->SetOptStat(0);
  TH2D *h2_SHeng_vs_SHblk_raw = new TH2D("h2_SHeng_vs_SHblk_raw","Raw E_clus(SH) per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_raw = new TH2D("h2_EovP_vs_SHblk_raw","Raw E_clus/p_rec per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_raw_calib = new TH2D("h2_EovP_vs_SHblk_raw_calib","Raw E_clus/p_rec per SH block | After Calib.",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_count = new TH2D("h2_count","Count for E_clus/p_rec per per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS_raw = new TH2D("h2_EovP_vs_SHblk_trPOS_raw","Raw E_clus/p_rec per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_count_trP = new TH2D("h2_count_trP","Count for E_clus/p_rec per per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_vs_PSblk_raw = new TH2D("h2_PSeng_vs_PSblk_raw","Raw E_clus(PS) per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_raw = new TH2D("h2_EovP_vs_PSblk_raw","Raw E_clus/p_rec per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_raw_calib = new TH2D("h2_EovP_vs_PSblk_raw_calib","Raw E_clus/p_rec per PS block | After Calib.",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_count_PS = new TH2D("h2_count_PS","Count for E_clus/p_rec per per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_trPOS_raw = new TH2D("h2_EovP_vs_PSblk_trPOS_raw","Raw E_clus/p_rec per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);
  TH2D *h2_count_trP_PS = new TH2D("h2_count_trP_PS","Count for E_clus/p_rec per per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  // Creating output ROOT file to contain histograms
  TString outFile = Form("%s/hist/%s_bbcal_eng_calib.root", macros_dir.Data(), cfgfilebase.Data());
  TString outPlot = Form("%s/plots/%s_bbcal_eng_calib.pdf", macros_dir.Data(), cfgfilebase.Data());
  TFile *fout = new TFile(outFile, "RECREATE");
  fout->cd();

  // Physics histograms
  TH1D *h_W = new TH1D("h_W", "W distribution", h_W_bin, h_W_min, h_W_max);
  TH1D *h_Q2 = new TH1D("h_Q2", "Q2 distribution", h_Q2_bin, h_Q2_min, h_Q2_max);
  TH1D *h_EovP = new TH1D("h_EovP", "E/p (Before Calib.)", h_EovP_bin, h_EovP_min, h_EovP_max);
  TH1D *h_EovP_calib = new TH1D("h_EovP_calib", "E/p", h_EovP_bin, h_EovP_min, h_EovP_max);
  TH1D *h_clusE = new TH1D("h_clusE", "Best SH+PS cl. eng.", h_clusE_bin, h_clusE_min, h_clusE_max);
  TH1D *h_clusE_calib = new TH1D("h_clusE_calib", Form("Best SH+PS cl. eng. u (sh/ps.e)*%2.2f", cF), h_clusE_bin, h_clusE_min, h_clusE_max);
  TH1D *h_SHclusE = new TH1D("h_SHclusE", "Best SH Cluster Energy", h_shE_bin, h_shE_min, h_shE_max);
  TH1D *h_SHclusE_calib = new TH1D("h_SHclusE_calib", Form("Best SH cl. eng. u (sh.e)*%2.2f", cF), h_shE_bin, h_shE_min, h_shE_max);
  TH1D *h_PSclusE = new TH1D("h_PSclusE", "Best PS Cluster Energy", h_psE_bin, h_psE_min, h_psE_max);
  TH1D *h_PSclusE_calib = new TH1D("h_PSclusE_calib", Form("Best PS cl. eng. u (ps.e)*%2.2f", cF), h_psE_bin, h_psE_min, h_psE_max);
  TH2D *h2_P_rec_vs_P_ang = new TH2D("h2_P_rec_vs_P_ang", "Track p vs Track ang", h2_pang_bin, h2_pang_min, h2_pang_max, h2_p_bin, h2_p_min, h2_p_max);

  TH2D *h2_EovP_vs_P = new TH2D("h2_EovP_vs_P", "E/p vs p; p (GeV); E/p", h2_p_coarse_bin, h2_p_coarse_min, h2_p_coarse_max, h2_EovP_bin, h2_EovP_min, h2_EovP_max);
  TProfile *h2_EovP_vs_P_prof = new TProfile("h2_EovP_vs_P_prof","E/p vs P (Profile)",h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h_EovP_min,h_EovP_max);
  TH2D *h2_EovP_vs_P_calib = new TH2D("h2_EovP_vs_P_calib", "E/p vs p | After Calib.; p (GeV); E/p", h2_p_coarse_bin, h2_p_coarse_min, h2_p_coarse_max, h2_EovP_bin, h2_EovP_min, h2_EovP_max);
  TProfile *h2_EovP_vs_P_calib_prof = new TProfile("h2_EovP_vs_P_calib_prof","E/p vs P (Profile) a clib.",h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h_EovP_min,h_EovP_max);

  TH2D *h2_SHeng_vs_SHblk = new TH2D("h2_SHeng_vs_SHblk", "SH cl. eng. per SH block", kNcolsSH, 0, kNcolsSH, kNrowsSH, 0, kNrowsSH);
  TH2D *h2_EovP_vs_SHblk = new TH2D("h2_EovP_vs_SHblk", "E/p per SH block", kNcolsSH, 0, kNcolsSH, kNrowsSH, 0, kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_calib = new TH2D("h2_EovP_vs_SHblk_calib", "E/p per SH block | After Calib.", kNcolsSH, 0, kNcolsSH, kNrowsSH, 0, kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS = new TH2D("h2_EovP_vs_SHblk_trPOS", "E/p per SH block (TrPos)", kNcolsSH, -0.2992, 0.2992, kNrowsSH, -1.1542, 1.1542);

  TH2D *h2_PSeng_vs_PSblk = new TH2D("h2_PSeng_vs_PSblk", "PS cl. eng. per PS block", kNcolsPS, 0, kNcolsPS, kNrowsPS, 0, kNrowsPS);
  TH2D *h2_EovP_vs_PSblk = new TH2D("h2_EovP_vs_PSblk", "E/p per PS block", kNcolsPS, 0, kNcolsPS, kNrowsPS, 0, kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_calib = new TH2D("h2_EovP_vs_PSblk_calib", "E/p per PS block | After Calib.", kNcolsPS, 0, kNcolsPS, kNrowsPS, 0, kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_trPOS = new TH2D("h2_EovP_vs_PSblk_trPOS", "E/p per PS block (TrPos)", kNcolsPS, -0.3705, 0.3705, kNrowsPS, -1.201, 1.151);

  TH1D *h_thetabend = new TH1D("h_thetabend", "", 100, 0., 0.25);

  TH2D *h2_EovP_vs_trX = new TH2D("h2_EovP_vs_trX","E/p vs Track x",200,-0.8,0.8,200,0,2);
  TH2D *h2_EovP_vs_trX_calib = new TH2D("h2_EovP_vs_trX_calib","E/p vs Track x | After Calib.",200,-0.8,0.8,200,0,2);
  TH2D *h2_EovP_vs_trY = new TH2D("h2_EovP_vs_trY","E/p vs Track y",200,-0.16,0.16,200,0,2);
  TH2D *h2_EovP_vs_trY_calib = new TH2D("h2_EovP_vs_trY_calib","E/p vs Track y | After Calib.",200,-0.16,0.16,200,0,2);
  TH2D *h2_EovP_vs_trTh = new TH2D("h2_EovP_vs_trTh","E/p vs Track theta",200,-0.2,0.2,200,0,2);
  TH2D *h2_EovP_vs_trTh_calib = new TH2D("h2_EovP_vs_trTh_calib","E/p vs Track theta | After Calib.",200,-0.2,0.2,200,0,2);
  TH2D *h2_EovP_vs_trPh = new TH2D("h2_EovP_vs_trPh","E/p vs Track phi",200,-0.08,0.08,200,0,2);
  TH2D *h2_EovP_vs_trPh_calib = new TH2D("h2_EovP_vs_trPh_calib","E/p vs Track phi | After Calib.",200,-0.08,0.08,200,0,2);
  TH2D *h2_PSeng_vs_trX = new TH2D("h2_PSeng_vs_trX","PS energy vs Track x",200,-0.8,0.8,200,0,4);
  TH2D *h2_PSeng_vs_trX_calib = new TH2D("h2_PSeng_vs_trX_calib","PS energy vs Track x | After Calib.",200,-0.8,0.8,200,0,4);
  TH2D *h2_PSeng_vs_trY = new TH2D("h2_PSeng_vs_trY","PS energy vs Track y",200,-0.16,0.16,200,0,4);
  TH2D *h2_PSeng_vs_trY_calib = new TH2D("h2_PSeng_vs_trY_calib","PS energy vs Track y | After Calib.",200,-0.16,0.16,200,0,4);  

  TTree *Tout = new TTree("Tout", "");
  Double_t T_ebeam;   Tout->Branch("ebeam", &E_beam, "ebeam/D");
  Double_t T_W2;      Tout->Branch("W2", &T_W2, "W2/D");
  Double_t T_trP;     Tout->Branch("trP", &T_trP, "trP/D");
  Double_t T_trX;     Tout->Branch("trX", &T_trX, "trX/D");
  Double_t T_trY;     Tout->Branch("trY", &T_trY, "trY/D");
  Double_t T_trTh;    Tout->Branch("trTh", &T_trTh, "trTh/D");
  Double_t T_trPh;    Tout->Branch("trPh", &T_trPh, "trPh/D");
  Double_t T_psE;     Tout->Branch("psE", &T_psE, "psE/D");
  Double_t T_clusE;   Tout->Branch("clusE", &T_clusE, "clusE/D");

  // Looping over all events ====================================================================//
  cout << endl;
  Long64_t Nevents = C->GetEntries(), nevent=0;  
  Double_t timekeeper = 0., timeremains = 0.;
  int treenum = 0, currenttreenum = 0;
  vector<Long64_t> goodevents; // list of good events passed all the cuts
  while(C->GetEntry(nevent++)) {
    // Calculating remaining time 
    sw2->Stop();
    timekeeper += sw2->RealTime();
    if (nevent % 25000 == 0 && nevent != 0) 
      timeremains = timekeeper * (double(Nevents) / double(nevent) - 1.); 
    sw2->Reset();
    sw2->Continue();

    if(nevent % 100 == 0) cout << nevent << "/" << Nevents  << ", " << int(timeremains/60.) << "m \r";;
    cout.flush();
    // ------

    // get old gain coefficients
    if (!read_gain) {
      oldADCgainSH[int(shIdblk)] = shAgainblk;
      oldADCgainPS[int(psIdblk)] = psAgainblk;
    }

    // get E_beam from the tree. For the events which are missing data use the value
    // from the ones which come right before those.
    if (HALLA_p > 0.) {
      E_beam = HALLA_p/1000.; // GeV 
    }

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

      p_rec = trP[0] * p_rec_Offset; 
      E_e = p_rec; // Neglecting e- mass. 

      // *---- calculating calibrated momentum (Helps avoiding replay)
      if(mom_calib){
	TVector3 enhat_tgt(trTgth[0], trTgph[0], 1.0);
	enhat_tgt = enhat_tgt.Unit();	
	TVector3 enhat_fp(trRth[0], trRph[0], 1.0);
	enhat_fp = enhat_fp.Unit();
	TVector3 GEMzaxis(-sin(GEMpitch*TMath::DegToRad()),0,cos(GEMpitch*TMath::DegToRad()));
	TVector3 GEMyaxis(0,1,0);
	TVector3 GEMxaxis = (GEMyaxis.Cross(GEMzaxis)).Unit();	
	TVector3 enhat_fp_rot = enhat_fp.X() * GEMxaxis + enhat_fp.Y() * GEMyaxis + enhat_fp.Z() * GEMzaxis;
	double thetabend = acos(enhat_fp_rot.Dot(enhat_tgt));
	h_thetabend->Fill(thetabend);

	p_rec = (A_fit * (1. + (B_fit + C_fit*bb_magdist) * trTgth[0]) / thetabend) * p_rec_Offset;
	E_e = p_rec;
      }
      // *----

      // cut on E/p
      if (cut_on_EovP) if(fabs((shE + psE) / p_rec - 1.) > EovP_cut_limit) continue;

      // cut on p
      if (cut_on_pmin) if(p_rec < p_min_cut) continue;
      if (cut_on_pmax) if(p_rec > p_max_cut) continue;

      if (trP[0]==0 || E_e==0 ) continue;

      Double_t P_ang = TMath::ACos(trPz[0]/trP[0]);
      Double_t Q2 = 4. * E_beam * p_rec * pow(TMath::Sin((P_ang/2.)), 2.);
      P_ang *= TMath::RadToDeg();
      Double_t W2 = Mp*Mp + 2.*Mp*(E_beam - p_rec) - Q2;
      Double_t W = sqrt(max(0., W2));
      h_W->Fill(W);
      h_Q2->Fill(Q2);

      // Reject events with max edep on the edge
      if (shRowblk == 0 || shRowblk == 26 ||
	  shColblk == 0 || shColblk == 6) continue; 

      // cut on W
      if (cut_on_W) if (fabs(W - W_mean) > W_sigma) continue;

      // storing good event numbers for 2nd loop
      goodevents.push_back(nevent);
      
      // Loop over all the blocks in main cluster and fill in A's
      for(Int_t blk=0; blk<shNblk; blk++){
	Int_t blkID = int(shClBlkId[blk]);
	A[blkID] += shClBlkE[blk];
	nevents_per_cell[blkID]++; 
      }
    
      // ****** PreShower ******
      for(Int_t blk=0; blk<psNblk; blk++){
	Int_t blkID = int(psClBlkId[blk]);
	A[189+blkID] += psClBlkE[blk];
	nevents_per_cell[189+blkID]++;
      }
    
      // Let's fill some interesting histograms
      Double_t clusEngBBCal = (shE + psE) * Corr_Factor_Enrg_Calib_w_Cosmic;
      Double_t ClusEngSH = shE * Corr_Factor_Enrg_Calib_w_Cosmic;
      Double_t ClusEngPS = psE * Corr_Factor_Enrg_Calib_w_Cosmic;
      h_EovP->Fill(clusEngBBCal / p_rec);
      h_clusE->Fill(clusEngBBCal);
      h_SHclusE->Fill(ClusEngSH);
      h_PSclusE->Fill(ClusEngPS);
      h2_P_rec_vs_P_ang->Fill(P_ang, p_rec);

      // fill out tree branches
      T_W2 = W2;
      T_trP = p_rec;
      T_trX = trX[0];
      T_trY = trY[0];
      T_trTh = trTh[0];
      T_trPh = trPh[0];
      T_psE = ClusEngPS;
      T_clusE = clusEngBBCal;
      Tout->Fill();

      // Checking to see if there is any bias in track recostruction ----
      //SH
      h2_SHeng_vs_SHblk_raw->Fill(shColblk, shRowblk, ClusEngSH);
      h2_EovP_vs_SHblk_raw->Fill(shColblk, shRowblk, clusEngBBCal/p_rec);
      h2_count->Fill(shColblk, shRowblk, 1.);
      h2_SHeng_vs_SHblk->Divide(h2_SHeng_vs_SHblk_raw, h2_count);
      h2_EovP_vs_SHblk->Divide(h2_EovP_vs_SHblk_raw, h2_count);

      Double_t xtrATsh = trX[0] + zposSH*trTh[0];
      Double_t ytrATsh = trY[0] + zposSH*trPh[0];
      h2_EovP_vs_SHblk_trPOS_raw->Fill(ytrATsh, xtrATsh, clusEngBBCal/p_rec);
      h2_count_trP->Fill(ytrATsh, xtrATsh, 1.);
      h2_EovP_vs_SHblk_trPOS->Divide(h2_EovP_vs_SHblk_trPOS_raw, h2_count_trP);

      //PS
      h2_PSeng_vs_PSblk_raw->Fill(psColblk, psRowblk, ClusEngPS);
      h2_EovP_vs_PSblk_raw->Fill(psColblk, psRowblk, clusEngBBCal/p_rec);
      h2_count_PS->Fill(psColblk, psRowblk, 1.);
      h2_PSeng_vs_PSblk->Divide(h2_PSeng_vs_PSblk_raw, h2_count_PS);
      h2_EovP_vs_PSblk->Divide(h2_EovP_vs_PSblk_raw, h2_count_PS);

      Double_t xtrATps = trX[0] + zposPS*trTh[0];
      Double_t ytrATps = trY[0] + zposPS*trPh[0];
      h2_EovP_vs_PSblk_trPOS_raw->Fill(ytrATps, xtrATps, clusEngBBCal/p_rec);
      h2_count_trP_PS->Fill(ytrATps, xtrATps, 1.);
      h2_EovP_vs_PSblk_trPOS->Divide(h2_EovP_vs_PSblk_trPOS_raw, h2_count_trP_PS);
      // -----

      // E/p vs. p
      h2_EovP_vs_P->Fill(p_rec, clusEngBBCal/p_rec);
      h2_EovP_vs_P_prof->Fill( p_rec, clusEngBBCal/p_rec, 1. );

      // histos to check bias in tracking
      h2_EovP_vs_trX->Fill( trX[0], (clusEngBBCal/p_rec) );
      h2_EovP_vs_trY->Fill( trY[0], (clusEngBBCal/p_rec) );
      h2_EovP_vs_trTh->Fill( trTh[0], (clusEngBBCal/p_rec) );
      h2_EovP_vs_trPh->Fill( trPh[0], (clusEngBBCal/p_rec) );
      h2_PSeng_vs_trX->Fill( trX[0], ClusEngPS );
      h2_PSeng_vs_trY->Fill( trY[0], ClusEngPS );

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

  // Let's customize the histogram ranges
  h2_SHeng_vs_SHblk->GetZaxis()->SetRangeUser(0.9,2.0);
  h2_EovP_vs_SHblk->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_EovP_vs_SHblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_PSeng_vs_PSblk->GetZaxis()->SetRangeUser(0.36,1.28);
  h2_EovP_vs_PSblk->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_EovP_vs_PSblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);
  
  // B.Print();  
  // M.Print();

  //Diagnostic histograms
  TH1D *h_nevent_blk_SH = new TH1D("h_nevent_blk_SH", "No. of Good Events; SH Blocks", 189, 0, 189);
  TH1D *h_coeff_Ratio_SH = new TH1D("h_coeff_Ratio_SH", "Ratio of Gain Coefficients(new/old); SH Blocks", 189, 0, 189);
  TH1D *h_coeff_blk_SH = new TH1D("h_coeff_blk_SH", "ADC Gain Coefficients(GeV/pC); SH Blocks", 189, 0, 189);
  TH1D *h_old_coeff_blk_SH = new TH1D("h_old_coeff_blk_SH", "Old ADC Gain Coefficients(GeV/pC); SH Blocks", 189, 0, 189);
  TH2D *h2_old_coeff_detView_SH = new TH2D("h2_old_coeff_detView_SH", "Old ADC Gain Coefficients | SH", kNcolsSH, 1, kNcolsSH+1, kNrowsSH, 1, kNrowsSH+1);
  TH2D *h2_coeff_detView_SH = new TH2D("h2_coeff_detView_SH", "New ADC Gain Coefficients | SH", kNcolsSH, 1, kNcolsSH+1, kNrowsSH, 1, kNrowsSH+1);

  TH1D *h_nevent_blk_PS = new TH1D("h_nevent_blk_PS", "No. of Good Events; PS Blocks", 52, 0, 52);
  TH1D *h_coeff_Ratio_PS = new TH1D("h_coeff_Ratio_PS", "Ratio of Gain Coefficients(new/old); PS Blocks", 52, 0, 52);
  TH1D *h_coeff_blk_PS = new TH1D("h_coeff_blk_PS", "ADC Gain Coefficients(GeV/pC); PS Blocks", 52, 0, 52);
  TH1D *h_old_coeff_blk_PS = new TH1D("h_old_coeff_blk_PS", "Old ADC Gain Coefficients(GeV/pC); PS Blocks", 52, 0, 52);
  TH2D *h2_old_coeff_detView_PS = new TH2D("h2_old_coeff_detView_PS", "Old ADC Gain Coefficients | PS", kNcolsPS, 1, kNcolsPS+1, kNrowsPS, 1, kNrowsPS+1);
  TH2D *h2_coeff_detView_PS = new TH2D("h2_coeff_detView_PS", "New ADC Gain Coefficients | PS", kNcolsPS, 1, kNcolsPS+1, kNrowsPS, 1, kNrowsPS+1);

  // Leave the bad channels out of the calculation
  for(Int_t j = 0; j<ncell; j++){
    badCells[j]=false;
    if (nevents_per_cell[j] < Nmin || M(j,j) < minMBratio*B(j)) {
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
  adcGain_SH = Form("%s/Gain/%s_gainCoeff_sh_calib.txt", macros_dir.Data(), cfgfilebase.Data());
  TString gainRatio_SH = Form("%s/Gain/%s_gainRatio_sh_calib.txt", macros_dir.Data(), cfgfilebase.Data());
  Double_t newADCgratioSH[kNcolsSH*kNrowsSH];
  for (int i=0; i<189; i++) { newADCgratioSH[i] = -1000; }  
  ofstream adcGainSH_outData, gainRatioSH_outData;
  adcGainSH_outData.open(adcGain_SH);
  gainRatioSH_outData.open(gainRatio_SH);
  for(Int_t shrow = 0; shrow<kNrowsSH; shrow++){
    for(Int_t shcol = 0; shcol<kNcolsSH; shcol++){
      Double_t oldCoeff = oldADCgainSH[shrow*kNcolsSH+shcol];
      if(!badCells[cell]){
	h_coeff_Ratio_SH->Fill(cell, CoeffR(cell));
	h_coeff_blk_SH->Fill(cell, CoeffR(cell) * oldCoeff);
	h_nevent_blk_SH->Fill(cell, nevents_per_cell[cell]);
	h_old_coeff_blk_SH->Fill(cell, oldCoeff);
	h2_old_coeff_detView_SH->Fill(shcol+1, shrow+1, oldCoeff);
	h2_coeff_detView_SH->Fill(shcol+1, shrow+1, CoeffR(cell) * oldCoeff);

	cout << CoeffR(cell) << "  ";
	adcGainSH_outData << CoeffR(cell) * oldCoeff << " ";
	gainRatioSH_outData << CoeffR(cell) << " ";
	newADCgratioSH[cell] = CoeffR(cell);
      }else{
	h_nevent_blk_SH->Fill(cell, nevents_per_cell[cell] );
	h_old_coeff_blk_SH->Fill(cell, oldCoeff);
	h_coeff_Ratio_SH->Fill(cell, 1. * Corr_Factor_Enrg_Calib_w_Cosmic);
	h_coeff_blk_SH->Fill(cell, oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic);
	h2_old_coeff_detView_SH->Fill(shcol+1, shrow+1, oldCoeff);
	h2_coeff_detView_SH->Fill(shcol+1, shrow+1, oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic);

	cout << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << "  ";
	adcGainSH_outData << oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	gainRatioSH_outData << 1. * Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	newADCgratioSH[cell] = 1. * Corr_Factor_Enrg_Calib_w_Cosmic;
      }
      cell++;
    }
    cout << endl;
    adcGainSH_outData << endl;
    gainRatioSH_outData << endl;
  }
  cout << endl;

  // customizing histograms
  h_nevent_blk_SH->SetLineWidth(0); h_nevent_blk_SH->SetMarkerStyle(8);
  h_coeff_Ratio_SH->SetLineWidth(0); h_coeff_Ratio_SH->SetMarkerStyle(8);
  h_coeff_blk_SH->SetLineWidth(0); h_coeff_blk_SH->SetMarkerStyle(8);
  h_old_coeff_blk_SH->SetLineWidth(0); h_old_coeff_blk_SH->SetMarkerStyle(8);

  // PS : Filling diagnostic histograms
  adcGain_PS = Form("%s/Gain/%s_gainCoeff_ps_calib.txt", macros_dir.Data(), cfgfilebase.Data());
  TString gainRatio_PS = Form("%s/Gain/%s_gainRatio_ps_calib.txt", macros_dir.Data(), cfgfilebase.Data());
  Double_t newADCgratioPS[kNcolsPS*kNrowsPS];
  for (int i=0; i<52; i++) { newADCgratioPS[i] = -1000; }  
  ofstream adcGainPS_outData, gainRatioPS_outData;
  adcGainPS_outData.open(adcGain_PS);
  gainRatioPS_outData.open(gainRatio_PS);
  for(Int_t psrow = 0; psrow<kNrowsPS; psrow++){
    for(Int_t pscol = 0; pscol<kNcolsPS; pscol++){
      Int_t psBlock = psrow * kNcolsPS + pscol;
      Double_t oldCoeff = oldADCgainPS[psrow*kNcolsPS+pscol];
      if(!badCells[cell]){
	h_coeff_Ratio_PS->Fill(psBlock, CoeffR(cell));
	h_coeff_blk_PS->Fill(psBlock, CoeffR(cell) * oldCoeff);
	h_nevent_blk_PS->Fill(psBlock, nevents_per_cell[cell]);
	h_old_coeff_blk_PS->Fill(psBlock, oldCoeff);
	h2_old_coeff_detView_PS->Fill(pscol+1, psrow+1, oldCoeff);
	h2_coeff_detView_PS->Fill(pscol+1, psrow+1, CoeffR(cell) * oldCoeff);

	cout << CoeffR(cell) << "  ";
	adcGainPS_outData << CoeffR(cell) * oldCoeff << " ";
	gainRatioPS_outData << CoeffR(cell) << " ";
	newADCgratioPS[psBlock] = CoeffR(cell);
      }else{
	h_nevent_blk_PS->Fill(psBlock, nevents_per_cell[cell]);
	h_old_coeff_blk_PS->Fill(psBlock, oldCoeff);
	h_coeff_Ratio_PS->Fill(psBlock, 1. * Corr_Factor_Enrg_Calib_w_Cosmic);
	h_coeff_blk_PS->Fill(psBlock, oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic);
	h2_old_coeff_detView_PS->Fill(pscol+1, psrow+1, oldCoeff);
	h2_coeff_detView_PS->Fill(pscol+1, psrow+1, oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic);

	cout << 1. * Corr_Factor_Enrg_Calib_w_Cosmic << "  ";
	adcGainPS_outData << oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	gainRatioPS_outData << 1. * Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	newADCgratioPS[psBlock] = 1. * Corr_Factor_Enrg_Calib_w_Cosmic;
      }
      cell++;
    }
    cout << endl;
    adcGainPS_outData << endl;
    gainRatioPS_outData << endl;
  }
  cout << endl;

  // customizing histograms
  h_nevent_blk_PS->SetLineWidth(0); h_nevent_blk_PS->SetMarkerStyle(8);
  h_coeff_Ratio_PS->SetLineWidth(0); h_coeff_Ratio_PS->SetMarkerStyle(8);
  h_coeff_blk_PS->SetLineWidth(0); h_coeff_blk_PS->SetMarkerStyle(8);
  h_old_coeff_blk_PS->SetLineWidth(0); h_old_coeff_blk_PS->SetMarkerStyle(8);

  // add branches to Tout
  Double_t T_psE_calib;    TBranch *T_psE_c = Tout->Branch("psE_calib", &T_psE_calib, "psE_calib/D");
  Double_t T_clusE_calib;  TBranch *T_clusE_c = Tout->Branch("clusE_calib", &T_clusE_calib, "clusE_calib/D");

  Long64_t itr = 0; nevent = 0;
  cout << "Looping over events again to check calibration.." << endl; 
  while(C->GetEntry(nevent++)) {
    // progress report
    if (nevent % 100 == 0) cout << nevent << "/" << Nevents  << "\r";;
    cout.flush();    

    if (nevent == goodevents[itr]) { // choosing good events
      itr++;

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
      h_EovP_calib->Fill(clusEngBBCal / p_rec);
      h_clusE_calib->Fill(clusEngBBCal);
      h_SHclusE_calib->Fill(shClusE);
      h_PSclusE_calib->Fill(psClusE);

      h2_EovP_vs_P_calib->Fill(p_rec, clusEngBBCal/p_rec);
      h2_EovP_vs_P_calib_prof->Fill( p_rec, clusEngBBCal/p_rec, 1. );

      h2_EovP_vs_SHblk_raw_calib->Fill(shColblk, shRowblk, clusEngBBCal/p_rec);
      h2_EovP_vs_SHblk_calib->Divide(h2_EovP_vs_SHblk_raw_calib, h2_count);

      h2_EovP_vs_PSblk_raw_calib->Fill(psColblk, psRowblk, clusEngBBCal/p_rec);
      h2_EovP_vs_PSblk_calib->Divide(h2_EovP_vs_PSblk_raw_calib, h2_count_PS);

      T_psE_calib = psClusE;        T_psE_c->Fill();
      T_clusE_calib = clusEngBBCal; T_clusE_c->Fill();

      // histos to check bias in tracking
      h2_EovP_vs_trX_calib->Fill( trX[0], (clusEngBBCal/p_rec) );
      h2_EovP_vs_trY_calib->Fill( trY[0], (clusEngBBCal/p_rec) );
      h2_EovP_vs_trTh_calib->Fill( trTh[0], (clusEngBBCal/p_rec) );
      h2_EovP_vs_trPh_calib->Fill( trPh[0], (clusEngBBCal/p_rec) );
      h2_PSeng_vs_trX_calib->Fill( trX[0], psClusE );
      h2_PSeng_vs_trY_calib->Fill( trY[0], psClusE );
    }
  }
  cout << endl << endl;

  // Let's customize the histogram ranges
  h2_EovP_vs_SHblk_calib->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_EovP_vs_PSblk_calib->GetZaxis()->SetRangeUser(0.8,1.2);

  // draw some diagnostic plots
  TCanvas *c1 = new TCanvas("c1","E/p",1500,1200);
  c1->Divide(3,2);

  c1->cd(1);
  gPad->SetGridx();
  Double_t param[3], param_bc[3], sigerr, sigerr_bc;
  Int_t maxBin_bc = h_EovP->GetMaximumBin();
  Double_t binW_bc = h_EovP->GetBinWidth(maxBin_bc), norm_bc = h_EovP->GetMaximum();
  Double_t mean_bc = h_EovP->GetMean(), stdev_bc = h_EovP->GetStdDev();
  Double_t lower_lim_bc = h_EovP_min + maxBin_bc*binW_bc - EovP_fit_width*stdev_bc;
  Double_t upper_lim_bc = h_EovP_min + maxBin_bc*binW_bc + EovP_fit_width*stdev_bc; 
  TF1* fitg_bc = new TF1("fitg_bc","gaus",h_EovP_min,h_EovP_max);
  fitg_bc->SetRange(lower_lim_bc,upper_lim_bc);
  fitg_bc->SetParameters(norm_bc,mean_bc,stdev_bc);
  fitg_bc->SetLineWidth(2); fitg_bc->SetLineColor(2);
  h_EovP->Fit(fitg_bc,"NO+QR"); fitg_bc->GetParameters(param_bc); fitg_bc->GetParError(2);
  h_EovP->SetLineWidth(2); h_EovP->SetLineColor(kGreen+2);
  Int_t maxBin = h_EovP_calib->GetMaximumBin();
  Double_t binW = h_EovP_calib->GetBinWidth(maxBin), norm = h_EovP_calib->GetMaximum();
  Double_t mean = h_EovP_calib->GetMean(), stdev = h_EovP_calib->GetStdDev();
  Double_t lower_lim = h_EovP_min + maxBin*binW - EovP_fit_width*stdev;
  Double_t upper_lim = h_EovP_min + maxBin*binW + EovP_fit_width*stdev; 
  TF1* fitg = new TF1("fitg","gaus",h_EovP_min,h_EovP_max);
  fitg->SetRange(lower_lim,upper_lim);
  fitg->SetParameters(norm,mean,stdev);
  fitg->SetLineWidth(2); fitg->SetLineColor(2);
  h_EovP_calib->Fit(fitg,"QR"); fitg->GetParameters(param); fitg->GetParError(2);
  h_EovP_calib->SetLineWidth(2); h_EovP_calib->SetLineColor(1);
  // adjusting histogram height for the legend to fit in
  if (norm > norm_bc) h_EovP_calib->GetYaxis()->SetRangeUser(0.,norm*1.2);
  else h_EovP_calib->GetYaxis()->SetRangeUser(0.,norm_bc*1.2);
  h_EovP_calib->Draw(); h_EovP->Draw("same");

  // draw the legend
  TLegend *l = new TLegend(0.10,0.78,0.90,0.90);
  l->SetTextFont(42);
  l->AddEntry(h_EovP,Form("Before calib., #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param_bc[1],param_bc[2]*100,sigerr_bc*100),"l");
  l->AddEntry(h_EovP_calib,Form("After calib., #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param[1],param[2]*100,sigerr*100),"l");
  l->Draw();

  c1->cd(2);
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  h2_EovP_vs_P->SetStats(0);
  h2_EovP_vs_P->Draw("colz");
  h2_EovP_vs_P_prof->SetStats(0);
  h2_EovP_vs_P_prof->SetMarkerStyle(20);
  h2_EovP_vs_P_prof->SetMarkerColor(1);
  h2_EovP_vs_P_prof->Draw("same");

  c1->cd(3);
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  h2_EovP_vs_P_calib->SetStats(0);
  h2_EovP_vs_P_calib->Draw("colz");
  h2_EovP_vs_P_calib_prof->SetStats(0);
  h2_EovP_vs_P_calib_prof->SetMarkerStyle(20);
  h2_EovP_vs_P_calib_prof->SetMarkerColor(1);
  h2_EovP_vs_P_calib_prof->Draw("same");

  c1->cd(4);
  h2_EovP_vs_SHblk->SetStats(0);
  h2_EovP_vs_SHblk->Draw("colz");

  c1->cd(5);
  h2_EovP_vs_SHblk_calib->SetStats(0);
  h2_EovP_vs_SHblk_calib->Draw("colz");

  c1->cd(6);
  h2_EovP_vs_PSblk_calib->SetStats(0);
  h2_EovP_vs_PSblk_calib->Draw("colz");

  TCanvas *c2 = new TCanvas("c2","tr X,Y,Th",1500,1200);
  c2->Divide(3,2);

  c2->cd(1);
  gPad->SetGridy();
  h2_EovP_vs_trX->SetStats(0);
  h2_EovP_vs_trX->Draw("colz");

  c2->cd(2);
  gPad->SetGridy();
  h2_EovP_vs_trY->SetStats(0);
  h2_EovP_vs_trY->Draw("colz");

  c2->cd(3);
  gPad->SetGridy();
  h2_EovP_vs_trTh->SetStats(0);
  h2_EovP_vs_trTh->Draw("colz");

  c2->cd(4);
  gPad->SetGridy();
  h2_EovP_vs_trX_calib->SetStats(0);
  h2_EovP_vs_trX_calib->Draw("colz");

  c2->cd(5);
  gPad->SetGridy();
  h2_EovP_vs_trY_calib->SetStats(0);
  h2_EovP_vs_trY_calib->Draw("colz");

  c2->cd(6);
  gPad->SetGridy();
  h2_EovP_vs_trTh_calib->SetStats(0);
  h2_EovP_vs_trTh_calib->Draw("colz");

  TCanvas *c3 = new TCanvas("c3","tr Ph,PS",1500,1200);
  c3->Divide(3,2);

  c3->cd(1);
  gPad->SetGridy();
  h2_EovP_vs_trPh->SetStats(0);
  h2_EovP_vs_trPh->Draw("colz");

  c3->cd(2);
  gPad->SetGridy();
  h2_PSeng_vs_trX->SetStats(0);
  h2_PSeng_vs_trX->Draw("colz");

  c3->cd(3);
  gPad->SetGridy();
  h2_PSeng_vs_trY->SetStats(0);
  h2_PSeng_vs_trY->Draw("colz");

  c3->cd(4);
  gPad->SetGridy();
  h2_EovP_vs_trPh_calib->SetStats(0);
  h2_EovP_vs_trPh_calib->Draw("colz");

  c3->cd(5);
  gPad->SetGridy();
  h2_PSeng_vs_trX_calib->SetStats(0);
  h2_PSeng_vs_trX_calib->Draw("colz");

  c3->cd(6);
  gPad->SetGridy();
  h2_PSeng_vs_trY_calib->SetStats(0);
  h2_PSeng_vs_trY_calib->Draw("colz");

  TCanvas *c4 = new TCanvas("c4","gain Coeff",1200,1000);
  c4->Divide(2,2);
  
  c4->cd(1); Double_t h_max;
  h_max = h_old_coeff_blk_SH->GetMaximum();
  h2_old_coeff_detView_SH->GetZaxis()->SetRangeUser(0.,h_max); h2_old_coeff_detView_SH->Draw("text col");

  c4->cd(2);
  h_max = h_coeff_blk_SH->GetMaximum();
  h2_coeff_detView_SH->GetZaxis()->SetRangeUser(0.,h_max); h2_coeff_detView_SH->Draw("text col");

  c4->cd(3);
  h_max = h_old_coeff_blk_PS->GetMaximum();
  h2_old_coeff_detView_PS->GetZaxis()->SetRangeUser(0.,h_max); h2_old_coeff_detView_PS->Draw("text col");

  c4->cd(4);
  h_max = h_coeff_blk_PS->GetMaximum();
  h2_coeff_detView_PS->GetZaxis()->SetRangeUser(0.,h_max); h2_coeff_detView_PS->Draw("text col");

  // let's record the summary
  TCanvas *c5 = new TCanvas("c5","Summary");
  c5->cd();

  TPaveText *pt = new TPaveText(.05,.1,.95,.8);
  pt->AddText(Form("Configfile: %s.cfg",cfgfilebase.Data()));
  pt->AddText(Form(" Date of creation: %s", getDate().c_str()));
  pt->AddText(Form(" Total no. of events analyzed: %lld", Nevents));
  pt->AddText(Form(" E/p  (before calib.) | #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param_bc[1],param_bc[2]*100,sigerr_bc*100));
  pt->AddText(Form(" E/p (after calib.) | #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param[1],param[2]*100,sigerr*100));
  pt->AddText(Form(" Global cuts: %s",gcutstr.Data()));
  if (cut_on_W) pt->AddText(Form(" |W - %.3f| < %.3f",W_mean,W_sigma));
  if (cut_on_EovP) pt->AddText(Form(" |E/p - 1| < %.1f",EovP_cut_limit));
  if (cut_on_pmin && cut_on_pmax) pt->AddText(Form(" %.1f < p_recon < %.1f GeV/c",p_min_cut,p_max_cut));
  else if (cut_on_pmin) pt->AddText(Form(" p_recon > %.1f GeV/c",p_min_cut));
  else if (cut_on_pmax) pt->AddText(Form(" p_recon < %.1f GeV/c",p_max_cut));
  TText *t1 = pt->GetLineWith("Configfile");
  t1->SetTextColor(kBlue);
  pt->Draw();

  // let's save the canvases in a pdf file
  c1->SaveAs(Form("%s[",outPlot.Data()));
  c1->SaveAs(Form("%s",outPlot.Data()));
  c2->SaveAs(Form("%s",outPlot.Data()));
  c3->SaveAs(Form("%s",outPlot.Data()));
  c4->SaveAs(Form("%s",outPlot.Data()));
  c5->SaveAs(Form("%s",outPlot.Data()));
  c5->SaveAs(Form("%s]",outPlot.Data()));

  c1->Write();
  c2->Write();
  c3->Write();
  c4->Write();
  c5->Write();
  fout->Write();
  //fout->Close();

  M.Clear();
  B.Clear();
  C->Delete();
  CoeffR.Clear();
  //fout->Delete();
  adcGainSH_outData.close();
  adcGainPS_outData.close();
  gainRatioSH_outData.close();
  gainRatioPS_outData.close();

  cout << "List of output files:" << endl;
  cout << " --------- " << endl;
  cout << " 1. Summary plots : "        << outPlot << endl;
  cout << " 2. Resulting histograms : " << outFile << endl;
  cout << " 3. Gain ratios (new/old) for SH : " << gainRatio_SH << endl;
  cout << " 4. Gain ratios (new/old) for PS : " << gainRatio_PS << endl;
  cout << " 5. New ADC gain coeffs. (GeV/pC) for SH : " << adcGain_SH << endl;
  cout << " 6. New ADC gain coeffs. (GeV/pC) for PS : " << adcGain_PS << endl;
  cout << " --------- " << endl;

  sw->Stop();
  sw2->Stop();
  cout << "CPU time elapsed = " << sw->CpuTime() << " s. Real time = " << sw->RealTime() << " s. " << endl << endl;

  sw->Delete();
  sw2->Delete();
}

// **** ========== Useful functions ========== ****  
void ReadGain(TString adcGain_rfile, Double_t* adcGain){
  ifstream adcGain_data;
  adcGain_data.open(adcGain_rfile);
  string readline;
  Int_t elemID=0;
  if(adcGain_data.is_open()){
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

TString GetOutFileBase(TString configfilename) {
  //Returns output file base from configfilename
  std::stringstream ss(configfilename.Data());
  std::vector<std::string> result;
  while( ss.good() ){
    std::string substr;
    std::getline(ss, substr, '/');
    result.push_back(substr);
  }
  TString temp = result[result.size() - 1];
  return temp.ReplaceAll(".cfg", "");
}

// ---------------- Get today's date ----------------
string getDate(){
  time_t now = time(0);
  tm ltm = *localtime(&now);

  string yyyy = to_string(1900 + ltm.tm_year);
  string mm = to_string(1 + ltm.tm_mon);
  string dd = to_string(ltm.tm_mday);
  string date = mm + '/' + dd + '/' + yyyy;

  return date;
}

/*
  List of input and output files:
  *Input files: 
  1. Gain/<configFileBase>_gainCoeff_sh(ps).txt # Old gain coeff. for SH(PS) [Needed if, "read_gain" = 1]
  *Output files:
  1. plots/<configFileBase>_bbcal_eng_calib.pdf # Contains all the canvases
  2. hist/<configFileBase>_bbcal_eng_calib.root # Contains all the interesting histograms
  3. Gain/<configFileBase>_gainRatio_sh(ps)_calib.txt # Contains gain ratios (new/old) for SH(PS)
  4. Gain/<configFileBase>_gainCoeff_sh(ps)_calib.txt # Contains new gain coeff. for SH(PS)
*/
