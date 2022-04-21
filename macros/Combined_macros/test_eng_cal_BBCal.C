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
#include "TChain.h"
#include "TFile.h"
#include "TCut.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TStopwatch.h"
#include "gmn_tree.C"

const Double_t Mp = 0.938272; // GeV

const Int_t ncell = 241;   // 189(SH) + 52(PS), Convention: 0-188: SH; 189-240: PS.
const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows
const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows

const Double_t zposSH = 1.901952; //m
const Double_t zposPS = 1.695704; //m

void ReadGain(TString,bool,bool);
bool SHorPS=1; // SH = 1, PS = 0
bool GainOrRatio=1; // Gain = 1, Ratio = 0
Double_t oldADCgainSH[kNcolsSH*kNrowsSH]={0.};
Double_t oldADCratioSH[kNcolsSH*kNrowsSH]={0.};
Double_t oldADCgainPS[kNcolsPS*kNrowsPS]={0.};  
Double_t oldADCratioPS[kNcolsPS*kNrowsPS]={0.};  

void test_eng_cal_BBCal(const char *configfilename, Int_t iter=1)
{
  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  TChain *C = new TChain("T");
  // gmn_tree *T = new gmn_tree(C);

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
  TCut globalcut = "";
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endcut") ){
    if( !currentline.BeginsWith("#") ){
      globalcut += currentline;
    }    
  }
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
  
  // Clear arrays
  memset(nevents_per_cell, 0, ncell*sizeof(int));
  memset(badCells, 0, ncell*sizeof(bool));
  
  // Let's read in old coefficients and ratios(new/old) for both SH and PS
  cout << endl;
  SHorPS = 1; // SH
  GainOrRatio = 1; // Gain
  if(farm_submit)
    adcGain_SH = Form("%s/eng_cal_gainCoeff_sh_%d_%d.txt",getenv("GAIN_DIR"),Set,iter-1);
  else
    adcGain_SH = Form("Gain/eng_cal_gainCoeff_sh_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_SH,SHorPS,GainOrRatio);
  GainOrRatio = 0; // Ratio
  if(farm_submit)
    adcGain_SH = Form("%s/eng_cal_gainRatio_sh_%d_%d.txt",getenv("GAIN_DIR"),Set,iter-1);
  else
    adcGain_SH = Form("Gain/eng_cal_gainRatio_sh_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_SH,SHorPS,GainOrRatio);
  SHorPS = 0; // PS
  GainOrRatio = 1;
  if(farm_submit)
    adcGain_PS = Form("%s/eng_cal_gainCoeff_ps_%d_%d.txt",getenv("GAIN_DIR"),Set,iter-1);
  else
    adcGain_PS = Form("Gain/eng_cal_gainCoeff_ps_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_PS,SHorPS,GainOrRatio);
  GainOrRatio = 0; 
  if(farm_submit)
    adcGain_PS = Form("%s/eng_cal_gainRatio_ps_%d_%d.txt",getenv("GAIN_DIR"),Set,iter-1);
  else
    adcGain_PS = Form("Gain/eng_cal_gainRatio_ps_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_PS,SHorPS,GainOrRatio);

  gStyle->SetOptStat(0);
  gStyle->SetPalette(60);
  TH2D *h2_SHeng_vs_SHblk_raw = new TH2D("h2_SHeng_vs_SHblk_raw","Raw E_clus(SH) per SH block",
					kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_raw = new TH2D("h2_EovP_vs_SHblk_raw","Raw E_clus/p_rec per SH block",
				      kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_count = new TH2D("h2_count","Count for E_clus/p_rec per per SH block",
			    kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS_raw = new TH2D("h2_EovP_vs_SHblk_trPOS_raw",
					      "Raw E_clus/p_rec per SH block(TrPos)",
					      kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_count_trP = new TH2D("h2_count_trP","Count for E_clus/p_rec per per SH block(TrPos)",
				kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_vs_PSblk_raw = new TH2D("h2_PSeng_vs_PSblk_raw","Raw E_clus(PS) per PS block",
					kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_raw = new TH2D("h2_EovP_vs_PSblk_raw","Raw E_clus/p_rec per PS block",
				      kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_count_PS = new TH2D("h2_count_PS","Count for E_clus/p_rec per per PS block",
			       kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_trPOS_raw = new TH2D("h2_EovP_vs_PSblk_trPOS_raw",
					      "Raw E_clus/p_rec per PS block(TrPos)",
					    kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);
  TH2D *h2_count_trP_PS = new TH2D("h2_count_trP_PS","Count for E_clus/p_rec"
				   " per per PS block(TrPos)",
				   kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  // Implementing global cuts
  if(C->GetEntries()==0){
    cerr << endl << " --- No ROOT file found!! --- " << endl << endl;
    throw;
  }else cout << endl << "Found " << C->GetEntries() << " events. Implementing global cuts.. " << endl;
  TEventList *elist = new TEventList("elist","Event list for BBCAL energy calibration");
  C->Draw(">>elist",globalcut);  
  gmn_tree *T = new gmn_tree(C);

  // Creating output ROOT file to contain histograms
  if(farm_submit)
    outFile = Form("eng_cal_BBCal_%d_%d.root",Set,iter);
  else
    outFile = Form("hist/eng_cal_BBCal_%d_%d.root",Set,iter);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  // Physics histograms
  TH1D *h_W = new TH1D("h_W","W distribution",h_W_bin,h_W_min,h_W_max);
  TH1D *h_Q2 = new TH1D("h_Q2","Q2 distribution",h_Q2_bin,h_Q2_min,h_Q2_max);
  TH1D *h_EovP = new TH1D("h_EovP","E_clus/p_rec",h_EovP_bin,h_EovP_min,h_EovP_max);
  TH1D *h_EovP_custom = new TH1D("h_EovP_custom","E_clus/p_rec"
				      ,h_EovP_bin,h_EovP_min,h_EovP_max);
  TH1D *h_clusE = new TH1D("h_clusE","Best Cluster Energy (SH+PS)"
			   ,h_clusE_bin,h_clusE_min,h_clusE_max);
  TH1D *h_clusE_custom = new TH1D("h_clusE_custom",Form("Best Cluster Energy (SH+PS) u (sh/ps.e)*%2.2f",cF)
				  ,h_clusE_bin,h_clusE_min,h_clusE_max);
  TH1D *h_SHclusE = new TH1D("h_SHclusE","Best SH Cluster Energy"
			     ,h_shE_bin,h_shE_min,h_shE_max);
  TH1D *h_SHclusE_custom = new TH1D("h_SHclusE_custom",Form("Best SH Cluster Energy u (sh.e)*%2.2f",cF)
				    ,h_shE_bin,h_shE_min,h_shE_max);
  TH1D *h_PSclusE = new TH1D("h_PSclusE","Best PS Cluster Energy"
			     ,h_psE_bin,h_psE_min,h_psE_max);
  TH1D *h_PSclusE_custom = new TH1D("h_PSclusE_custom",Form("Best PS Cluster Energy u (ps.e)*%2.2f",cF)
				    ,h_psE_bin,h_psE_min,h_psE_max);
  TH2D *h2_P_rec_vs_P_ang = new TH2D("h2_P_rec_vs_P_ang","Track p vs Track ang",
				     h2_pang_bin,h2_pang_min,h2_pang_max,
				     h2_p_bin,h2_p_min,h2_p_max);

  TH2D *h2_EovP_vs_P = new TH2D("h2_EovP_vs_P","E/p vs p; p (GeV); E/p",
				h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,
				h2_EovP_bin,h2_EovP_min,h2_EovP_max);
  TH2D *h2_EovP_vs_P_mod = new TH2D("h2_EovP_vs_P_mod","E/p vs p; p (GeV); E/p",
				h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,
				h2_EovP_bin,h2_EovP_min,h2_EovP_max);

  TH2D *h2_SHeng_vs_SHblk = new TH2D("h2_SHeng_vs_SHblk","Average E_clus(SH)*%2.2f per SH block",
				    kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk = new TH2D("h2_EovP_vs_SHblk","Average E_clus*%2.2f/p_rec per SH block",
				  kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS = new TH2D("h2_EovP_vs_SHblk_trPOS","Average E_clus*%2.2f/p_rec per SH "
					"block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_vs_PSblk = new TH2D("h2_PSeng_vs_PSblk","Average E_clus(PS)*%2.2f per PS block",
				    kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk = new TH2D("h2_EovP_vs_PSblk","Average E_clus*%2.2f/p_rec per PS block",
				  kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_trPOS = new TH2D("h2_EovP_vs_PSblk_trPOS","Average E_clus*%2.2f/p_rec per PS "
					"block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  TH1D *h_thetabend = new TH1D("h_thetabend","",100,0.,0.25);

  // Looping over good events ====================================================================//
  Long64_t Nevents = elist->GetN(), nevent=0;  
  cout << endl << "Processing " << Nevents << " events.." << endl;
  Double_t timekeeper = 0., timeremains = 0.;

  // Double_t progress = 0.;
  // while(progress<1.0){
  //   Int_t barwidth = 70;

  while( T->GetEntry( elist->GetEntry(nevent++) ) ){

    // // Creating a progress bar
    // cout << "[";
    // Int_t pos = barwidth * progress;
    // for(Int_t i=0; i<barwidth; ++i){
    //   if(i<pos) cout << "=";
    //   else if(i==pos) cout << ">";
    //   else cout << " ";
    // }

    // Calculating remaining time 
    sw2->Stop();
    timekeeper += sw2->RealTime();
    if( nevent%25000 == 0 && nevent!=0 ) 
      timeremains = timekeeper*( double(Nevents)/double(nevent) - 1. ); 
    sw2->Reset();
    sw2->Continue();

    // progress = (double)((nevent+1.)/Nevents);
    // cout << "] " << int(progress*100.) << "%, " << int(timeremains/60.) << "m \r";
    // cout.flush();

    if( nevent % 100 == 0 ) cout << nevent << "/" << Nevents  << ", " << int(timeremains/60.) << "m \r";;
    cout.flush();
    // ------

    //T->GetEntry(nevent);
    
    E_e = 0;
    memset(A, 0, ncell*sizeof(double));

    // Choosing track with least chi2 
    Int_t tr_min = -1;
    // Double_t chi2min = 1000.;
    // for(Int_t tr = 0; tr<T->bb_tr_n; tr++){
    //   if(T->bb_tr_chi2[tr]<chi2min){
    // 	chi2min = T->bb_tr_chi2[tr];
    // 	tr_min = tr;
    //   }
    // }
    tr_min = 0; //tracks are already sorted

    p_rec = (T->bb_tr_p[tr_min])*p_rec_Offset; 
    E_e = p_rec; // Neglecting e- mass. 

    // *---- calculating calibrated momentum
    if(mom_calib){
      TVector3 enhat_tgt( T->bb_tr_tg_th[0], T->bb_tr_tg_ph[0], 1.0 );
      enhat_tgt = enhat_tgt.Unit();	
      TVector3 enhat_fp( T->bb_tr_r_th[0], T->bb_tr_r_ph[0], 1.0 );
      enhat_fp = enhat_fp.Unit();
      TVector3 GEMzaxis(-sin(GEMpitch*TMath::DegToRad()),0,cos(GEMpitch*TMath::DegToRad()));
      TVector3 GEMyaxis(0,1,0);
      TVector3 GEMxaxis = (GEMyaxis.Cross(GEMzaxis)).Unit();	
      TVector3 enhat_fp_rot = enhat_fp.X() * GEMxaxis + enhat_fp.Y() * GEMyaxis + enhat_fp.Z() * GEMzaxis;
      double thetabend = acos( enhat_fp_rot.Dot( enhat_tgt ) );
      h_thetabend->Fill(thetabend);

      p_rec = (A_fit*( 1. + (B_fit + C_fit*bb_magdist)*T->bb_tr_tg_th[0] )/thetabend)*p_rec_Offset;
      E_e = p_rec;
    }
    // *----

    // cut on E/p
    if( cut_on_EovP ) if( fabs( (T->bb_sh_e+T->bb_ps_e)/p_rec - 1. )>EovP_cut_limit ) continue;

    // cut on p
    if( cut_on_pmin ) if( p_rec<p_min_cut ) continue;
    if( cut_on_pmax ) if( p_rec>p_max_cut ) continue;

    if(T->bb_tr_p[tr_min]==0 || E_e==0 || tr_min<0) continue;

    Double_t P_ang = 57.3*TMath::ACos(T->bb_tr_pz[tr_min]/T->bb_tr_p[tr_min]);
    Double_t Q2 = 4.*E_beam*p_rec*pow( TMath::Sin(P_ang/57.3/2.),2. );
    Double_t W2 = Mp*Mp + 2.*Mp*(E_beam-p_rec) - Q2;
    Double_t W = 0.;

    h_Q2->Fill(Q2);
    if( W2>0. ){
      W = TMath::Sqrt(W2);  
      h_W->Fill(W);
    }

    // cut on W
    if( cut_on_W ) if( fabs(W-W_mean)>W_sigma ) continue;
   
    Int_t cl_max=-1;
    Double_t nblk=-1.;
    Double_t Emax=-10.;

    // ****** Shower ******
    // Loop over all the clusters first: select highest energy
    // for(Int_t cl = 0; cl<T->bb_sh_nclus; cl++){
    //   if(T->bb_sh_clus_e[cl]>Emax){
    //     Emax = T->bb_sh_clus_e[cl];
    //     cl_max = cl;
    //   }
    // }
	
    cl_max=0; //the clusters are already sorted 

    // Reject events with max on the edge
    // if(T->bb_sh_clus_row[cl_max]==0 || T->bb_sh_clus_row[cl_max]==26 ||
    //    T->bb_sh_clus_col[cl_max]==0 || T->bb_sh_clus_col[cl_max]==6) continue; 

    // Don't include the blocks at the edge in the fit
    if(T->bb_sh_rowblk==0 || T->bb_sh_rowblk==26 ||
       T->bb_sh_colblk==0 || T->bb_sh_colblk==6) continue; 


    // Loop over all the blocks in main cluster and fill in A's
    Double_t ClusEngSH_mod=0.;
    Int_t shrow=0;
    Int_t shcol=0;
    nblk = T->bb_sh_nblk;
    for(Int_t blk=0; blk<nblk; blk++){
      Int_t blkID = int(T->bb_sh_clus_blk_id[blk]);
      shrow = int(T->bb_sh_clus_blk_row[blk]);
      shcol = int(T->bb_sh_clus_blk_col[blk]);
      A[blkID] += (T->bb_sh_clus_blk_e[blk])*oldADCratioSH[blkID];
      ClusEngSH_mod += (T->bb_sh_clus_blk_e[blk])*oldADCratioSH[blkID];
      nevents_per_cell[ blkID ]++; 
    }
    
    // ****** PreShower ******
    Double_t ClusEngPS_mod=0.;
    Int_t psrow=0;
    Int_t pscol=0;
    nblk = T->bb_ps_nblk;
    for(Int_t blk=0; blk<nblk; blk++){
      Int_t blkID = int(T->bb_ps_clus_blk_id[blk]);
      psrow = int(T->bb_ps_clus_blk_row[blk]);
      pscol = int(T->bb_ps_clus_blk_col[blk]);
      A[189+blkID]+=(T->bb_ps_clus_blk_e[blk])*oldADCratioPS[blkID];
      ClusEngPS_mod += (T->bb_ps_clus_blk_e[blk])*oldADCratioPS[blkID];
      nevents_per_cell[189+blkID]++;
    }

    // Let's fill some interesting histograms
    Double_t clusEngBBCal = (T->bb_sh_e + T->bb_ps_e)*Corr_Factor_Enrg_Calib_w_Cosmic;
    Double_t clusEngBBCal_mod = ClusEngSH_mod + ClusEngPS_mod;
    h_EovP->Fill( (clusEngBBCal_mod/p_rec) );
    h_clusE->Fill( clusEngBBCal_mod );
    h_SHclusE->Fill( ClusEngSH_mod );
    h_PSclusE->Fill( ClusEngPS_mod );
    h2_P_rec_vs_P_ang->Fill( P_ang, p_rec );

    h_EovP_custom->Fill( (clusEngBBCal/p_rec) );
    h_clusE_custom->Fill( clusEngBBCal );
    h_SHclusE_custom->Fill( (T->bb_sh_e)*Corr_Factor_Enrg_Calib_w_Cosmic );
    h_PSclusE_custom->Fill( (T->bb_ps_e)*Corr_Factor_Enrg_Calib_w_Cosmic );

    // Checking to see if there is any bias in track recostruction ----
    //SH
    h2_SHeng_vs_SHblk_raw->Fill( shcol, shrow, ClusEngSH_mod );
    h2_EovP_vs_SHblk_raw->Fill( shcol, shrow, (clusEngBBCal_mod/p_rec) );
    h2_count->Fill( shcol, shrow, 1.);
    h2_SHeng_vs_SHblk->Divide( h2_SHeng_vs_SHblk_raw, h2_count );
    h2_EovP_vs_SHblk->Divide( h2_EovP_vs_SHblk_raw, h2_count );

    Double_t xtrATsh = T->bb_tr_x[cl_max] + zposSH*T->bb_tr_th[cl_max];
    Double_t ytrATsh = T->bb_tr_y[cl_max] + zposSH*T->bb_tr_ph[cl_max];
    h2_EovP_vs_SHblk_trPOS_raw->Fill( ytrATsh, xtrATsh, (clusEngBBCal_mod/p_rec) );
    h2_count_trP->Fill( ytrATsh, xtrATsh, 1. );
    h2_EovP_vs_SHblk_trPOS->Divide( h2_EovP_vs_SHblk_trPOS_raw, h2_count_trP );

    //PS
    h2_PSeng_vs_PSblk_raw->Fill( pscol, psrow, ClusEngPS_mod );
    h2_EovP_vs_PSblk_raw->Fill( pscol, psrow, (clusEngBBCal_mod/p_rec) );
    h2_count_PS->Fill( pscol, psrow, 1.);
    h2_PSeng_vs_PSblk->Divide( h2_PSeng_vs_PSblk_raw, h2_count_PS );
    h2_EovP_vs_PSblk->Divide( h2_EovP_vs_PSblk_raw, h2_count_PS );

    Double_t xtrATps = T->bb_tr_x[cl_max] + zposPS*T->bb_tr_th[cl_max];
    Double_t ytrATps = T->bb_tr_y[cl_max] + zposPS*T->bb_tr_ph[cl_max];
    h2_EovP_vs_PSblk_trPOS_raw->Fill( ytrATps, xtrATps, (clusEngBBCal_mod/p_rec) );
    h2_count_trP_PS->Fill( ytrATps, xtrATps, 1. );
    h2_EovP_vs_PSblk_trPOS->Divide( h2_EovP_vs_PSblk_trPOS_raw, h2_count_trP_PS );
    // -----

    // E/p vs. p
    h2_EovP_vs_P->Fill( p_rec, clusEngBBCal/p_rec );
    h2_EovP_vs_P_mod->Fill( p_rec, clusEngBBCal_mod/p_rec );

    // Let's customize the histograms
    //h2_SHeng_vs_SHblk->GetZaxis()->SetRangeUser(0.6,1.0); //(1.0,2.0);
    h2_EovP_vs_SHblk->GetZaxis()->SetRangeUser(0.8,1.2);
    h2_EovP_vs_SHblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);
    //h2_PSeng_vs_PSblk->GetZaxis()->SetRangeUser(0.,0.5); //(0.3,1.0);
    h2_EovP_vs_PSblk->GetZaxis()->SetRangeUser(0.8,1.2);
    h2_EovP_vs_PSblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);

    // Let's costruct the matrix
    for(Int_t icol = 0; icol<ncell; icol++){
      B(icol)+= A[icol];
      for(Int_t irow = 0; irow<ncell; irow++){
	M(icol,irow)+= A[icol]*A[irow]/E_e;
      } 
    }   
      
  } //event loop

  // } //progress bar loop
  cout << endl << endl;
  
  // B.Print();  
  // M.Print();

  //Diagnostic histograms
  TH1D *h_nevent_blk_SH = new TH1D("h_nevent_blk_SH","No. of Good Events; SH Blocks",189,0,189);
  TH1D *h_coeff_Ratio_SH = new TH1D("h_coeff_Ratio_SH","Ratio of Gain Coefficients(new/old); SH Blocks"
				    ,189,0,189);
  TH1D *h_coeff_blk_SH = new TH1D("h_coeff_blk_SH","ADC Gain Coefficients(GeV/pC); SH Blocks"
				  ,189,0,189);
  TH1D *h_Old_Coeff_blk_SH = new TH1D("h_Old_Coeff_blk_SH","Old ADC Gain Coefficients(GeV/pC); SH Blocks"
				      ,189,0,189);
  TH2D *h_coeff_detView_SH = new TH2D("h_coeff_detView_SH","ADC Gain Coefficients(Detector View)",
				      kNcolsSH,1,kNcolsSH+1,kNrowsSH,1,kNrowsSH+1);

  TH1D *h_nevent_blk_PS = new TH1D("h_nevent_blk_PS","No. of Good Events; PS Blocks",52,0,52);
  TH1D *h_coeff_Ratio_PS = new TH1D("h_coeff_Ratio_PS","Ratio of Gain Coefficients(new/old); PS Blocks"
				    ,52,0,52);
  TH1D *h_coeff_blk_PS = new TH1D("h_coeff_blk_PS","ADC Gain Coefficients(GeV/pC); PS Blocks",52,0,52);
  TH1D *h_Old_Coeff_blk_PS = new TH1D("h_Old_Coeff_blk_PS","Old ADC Gain Coefficients(GeV/pC); PS Blocks"
				      ,52,0,52);
  TH2D *h_coeff_detView_PS = new TH2D("h_coeff_detView_PS","ADC Gain Coefficients(Detector View)",
				      kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);

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
  if(farm_submit)
    adcGain_SH = Form("eng_cal_gainCoeff_sh_%d_%d.txt",Set,iter);
  else
    adcGain_SH = Form("Gain/eng_cal_gainCoeff_sh_%d_%d.txt",Set,iter);
  if(farm_submit)
    gainRatio_SH = Form("eng_cal_gainRatio_sh_%d_%d.txt",Set,iter);
  else
    gainRatio_SH = Form("Gain/eng_cal_gainRatio_sh_%d_%d.txt",Set,iter);
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
      }else{
	h_nevent_blk_SH->Fill( cell, nevents_per_cell[cell] );
	h_Old_Coeff_blk_SH->Fill( cell, oldCoeff );
	if(iter==1){
	  h_coeff_Ratio_SH->Fill( cell, 1.*Corr_Factor_Enrg_Calib_w_Cosmic );
	  h_coeff_blk_SH->Fill( cell, oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic );
	  h_coeff_detView_SH->Fill( shcol+1, shrow+1, oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic );

	  cout << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << "  ";
	  adcGainSH_outData << oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	  gainRatioSH_outData << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	}else{
	  h_coeff_Ratio_SH->Fill( cell, 1. );
	  h_coeff_blk_SH->Fill( cell, oldCoeff );
	  h_coeff_detView_SH->Fill( shcol+1, shrow+1, oldCoeff );

	  cout << 1. << "  ";
	  adcGainSH_outData << oldCoeff << " ";
	  gainRatioSH_outData << 1. << " ";
	}
      }
      cell++;
    }
    cout << endl;
    adcGainSH_outData << endl;
    gainRatioSH_outData << endl;
  }
  cout << endl;

  // PS : Filling diagnostic histograms
  if(farm_submit)
    adcGain_PS = Form("eng_cal_gainCoeff_ps_%d_%d.txt",Set,iter);
  else
    adcGain_PS = Form("Gain/eng_cal_gainCoeff_ps_%d_%d.txt",Set,iter);
  if(farm_submit)
    gainRatio_PS = Form("eng_cal_gainRatio_ps_%d_%d.txt",Set,iter);
  else
    gainRatio_PS = Form("Gain/eng_cal_gainRatio_ps_%d_%d.txt",Set,iter);
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
      }else{
	h_nevent_blk_PS->Fill( psBlock, nevents_per_cell[cell] );
	h_Old_Coeff_blk_PS->Fill( psBlock, oldCoeff );
	if(iter==1){
	  h_coeff_Ratio_PS->Fill( psBlock, 1.*Corr_Factor_Enrg_Calib_w_Cosmic );
	  h_coeff_blk_PS->Fill( psBlock, oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic );
	  h_coeff_detView_PS->Fill( pscol+1, psrow+1, oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic );

	  cout << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << "  ";
	  adcGainPS_outData << oldCoeff*Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	  gainRatioPS_outData << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	}else{
	  h_coeff_Ratio_PS->Fill( psBlock, 1. );
	  h_coeff_blk_PS->Fill( psBlock, oldCoeff );
	  h_coeff_detView_PS->Fill( pscol+1, psrow+1, oldCoeff );

	  cout << 1. << "  ";
	  adcGainPS_outData << oldCoeff << " ";
	  gainRatioPS_outData << 1. << " ";
	}
      }
      cell++;
    }
    cout << endl;
    adcGainPS_outData << endl;
    gainRatioPS_outData << endl;
  }
  cout << endl;

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

  cout << "Finishing iteration " << iter << "..." << endl;
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


void ReadGain( TString adcGain, bool SHorPS, bool GainOrRatio ){
  ifstream adcGain_data;
  adcGain_data.open(adcGain);
  string readline;
  Int_t elemID=0;
  if( adcGain_data.is_open() ){
    cout << " Reading ADC gain from : "<< adcGain << endl;
    while(getline(adcGain_data,readline)){
      istringstream tokenStream(readline);
      string token;
      char delimiter = ' ';
      while(getline(tokenStream,token,delimiter)){
	TString temptoken=token;
	if(SHorPS){
	  if(GainOrRatio){
	    oldADCgainSH[elemID] = temptoken.Atof();
	  }else{
	    oldADCratioSH[elemID] = temptoken.Atof();
	  }
	}else{
	  if(GainOrRatio){
	    oldADCgainPS[elemID] = temptoken.Atof();
	  }else{
	    oldADCratioPS[elemID] = temptoken.Atof();
	  }
	}
	elemID++;
      }
    }
  }else{
    cerr << " No file : " << adcGain << endl;
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
