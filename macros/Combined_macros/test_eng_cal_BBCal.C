/*
  This script has been prepared for the energy calibration of BigBite Calorimeter (BigBite Shower
  + BigBite Preshower) detector. It does so by minimizing the chi2 of the difference between calorimeter
  cluster energy and the reconstructed electron energy(we get that from tracking information). It reads
  in the old adc gain coefficients (GeV/pC) and ratios and writes the new ones in file. One needs a configfile 
  to execute this script. Example content of such a file is attached in the end. To execute, follow:
  ----
  [a-onl@aonl2 macros]$ pwd
  /adaqfs/home/a-onl/sbs/BBCal_replay/macros
  [a-onl@aonl2 macros]$ root -l 
  root [0] .L Combined_macros/test_eng_cal_BBCal.C+
  root [1] eng_cal_BBCal("Combined_macros/setup_eng_cal_BBCal.txt")
  ----
  P. Datta  <pdbforce@jlab.org>  Created  15 Oct 2021 (Based on AJR Puckett & E. Fuchey's version)
*/
#include <iostream>
#include <sstream>
#include <fstream>
#include "TChain.h"
#include "TFile.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TCut.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"
#include "gmn_tree.C"

const Double_t Mp = 0.938; // GeV

const Int_t ncell = 241;   // 189(SH) + 52(PS), Convention: 0-188: SH; 189-240: PS.
const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows
const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows

const Double_t zposSH = 1.901952; //m
const Double_t zposPS = 1.695704; //m

void ReadGain(TString,bool,bool);
bool SHorPS = 1; // SH = 1, PS = 0
bool GainOrRatio = 1; // Gain = 1, Ratio = 0
Double_t oldADCgainSH[kNcolsSH*kNrowsSH] = {0.};
Double_t oldADCratioSH[kNcolsSH*kNrowsSH] = {0.};
Double_t oldADCgainPS[kNcolsPS*kNrowsPS] = {0.};  
Double_t oldADCratioPS[kNcolsPS*kNrowsPS] = {0.};  

void eng_cal_BBCal(const char *configfilename, int iter=1)
{

  TChain *C = new TChain("T");
  gmn_tree *T = new gmn_tree(C);

  Int_t Set = 1;
  Int_t Nmin = 10;
  Double_t minMBratio = 0.1;
  Double_t E_beam = 0.;
  Double_t p_rec_Offset = 1.;
  Double_t W_mean = 0.;
  Double_t W_sigma = 0.;
  Double_t Corr_Factor_Enrg_Calib_w_Cosmic = 1.;

  TMatrixD M(ncell,ncell), M_inv(ncell,ncell);
  TVectorD B(ncell), CoeffR(ncell);
  
  Double_t E_e = 0;
  Double_t p_rec = 0.;
  Double_t A[ncell] = {0.};
  bool badCells[ncell]; // Cells that have events less than Nmin
  TString adcGain_SH, gainRatio_SH;
  TString adcGain_PS, gainRatio_PS;
  Int_t nevents_per_cell[ncell];

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
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("#") ){
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
      if( skey == "Min_Event_Per_Channel" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Nmin = sval.Atof();
      }
      if( skey == "Min_MB_Ratio" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	minMBratio = sval.Atoi();
      }
      if( skey == "p_rec_Offset" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	p_rec_Offset = sval.Atof();
      }
      if( skey == "W_mean" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	W_mean = sval.Atof();
      }
      if( skey == "W_sigma" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	W_sigma = sval.Atof();
      }
      if( skey == "Corr_Factor_Enrg_Calib_w_Cosmic" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Corr_Factor_Enrg_Calib_w_Cosmic = sval.Atof();
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
  SHorPS = 1; // SH
  GainOrRatio = 1; // Gain
  adcGain_SH = Form("Gain/eng_cal_gainCoeff_sh_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_SH,SHorPS,GainOrRatio);
  GainOrRatio = 0; // Ratio
  adcGain_SH = Form("Gain/eng_cal_gainRatio_sh_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_SH,SHorPS,GainOrRatio);
  SHorPS = 0; // PS
  GainOrRatio = 1;
  adcGain_PS = Form("Gain/eng_cal_gainCoeff_ps_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_PS,SHorPS,GainOrRatio);
  GainOrRatio = 0; 
  adcGain_PS = Form("Gain/eng_cal_gainRatio_ps_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_PS,SHorPS,GainOrRatio);

  gStyle->SetOptStat(0);
  gStyle->SetPalette(60);
  TH2D *h2_SHeng_P_SHblk_raw = new TH2D("h2_SHeng_P_SHblk_raw","Raw E_clus(SH) per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_res_P_SHblk_raw = new TH2D("h2_res_P_SHblk_raw","Raw E_clus/p_rec per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_count = new TH2D("h2_count","Count for E_clus/p_rec per per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_res_P_SHblk_trPOS_raw = new TH2D("h2_res_P_SHblk_trPOS_raw","Raw E_clus/p_rec per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_count_trP = new TH2D("h2_count_trP","Count for E_clus/p_rec per per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_P_PSblk_raw = new TH2D("h2_PSeng_P_PSblk_raw","Raw E_clus(PS) per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_res_P_PSblk_raw = new TH2D("h2_res_P_PSblk_raw","Raw E_clus/p_rec per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_count_PS = new TH2D("h2_count_PS","Count for E_clus/p_rec per per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_res_P_PSblk_trPOS_raw = new TH2D("h2_res_P_PSblk_trPOS_raw","Raw E_clus/p_rec per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);
  TH2D *h2_count_trP_PS = new TH2D("h2_count_trP_PS","Count for E_clus/p_rec per per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  TString outFile = Form("hist/eng_cal_BBCal_%d_%d.root",Set,iter);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  // Physics histograms
  Double_t cF = Corr_Factor_Enrg_Calib_w_Cosmic;
  TH1D *h_W = new TH1D("h_W","W distribution",200,0.7,1.6);
  TH1D *h_Q2 = new TH1D("h_Q2","Q2 distribution",100,0.,4.);
  TH1D *h_res_BBCal = new TH1D("h_res_BBCal","E_clus/p_rec",200,0.2,1.6);
  TH1D *h_res_BBCal_custom = new TH1D("h_res_BBCal_custom","E_clus/p_rec",200,0.2,1.6);
  TH1D *h_clusE = new TH1D("h_clusE","Best Cluster Energy (SH+PS)",350,0.,3.5);
  TH1D *h_clusE_custom = new TH1D("h_clusE_custom",Form("Best Cluster Energy (SH+PS) u (sh/ps.e)*%2.2f",cF),350,0.,3.5);
  TH1D *h_SHclusE = new TH1D("h_SHclusE","Best SH Cluster Energy",300,0.,3.0);
  TH1D *h_SHclusE_custom = new TH1D("h_SHclusE_custom",Form("Best SH Cluster Energy u (sh.e)*%2.2f",cF),300,0.,3.0);
  TH1D *h_PSclusE = new TH1D("h_PSclusE","Best PS Cluster Energy",150,0.,1.5);
  TH1D *h_PSclusE_custom = new TH1D("h_PSclusE_custom",Form("Best PS Cluster Energy u (ps.e)*%2.2f",cF),150,0.,1.5);
  TH2D *h2_P_rec_vs_P_ang = new TH2D("h2_P_rec_vs_P_ang","Track p vs Track ang",100,30,45,200,1.0,3.0);

  TH2D *h2_SHeng_P_SHblk = new TH2D("h2_SHeng_P_SHblk",Form("Average E_clus(SH)*%2.2f per SH block",cF),kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_res_P_SHblk = new TH2D("h2_res_P_SHblk",Form("Average E_clus*%2.2f/p_rec per SH block",cF),kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_res_P_SHblk_trPOS = new TH2D("h2_res_P_SHblk_trPOS",Form("Average E_clus*%2.2f/p_rec per SH block(TrPos)",cF),
					kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_P_PSblk = new TH2D("h2_PSeng_P_PSblk",Form("Average E_clus(PS)*%2.2f per PS block",cF),kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_res_P_PSblk = new TH2D("h2_res_P_PSblk",Form("Average E_clus*%2.2f/p_rec per PS block",cF),kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_res_P_PSblk_trPOS = new TH2D("h2_res_P_PSblk_trPOS",Form("Average E_clus*%2.2f/p_rec per PS block(TrPos)",cF),
					kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  Long64_t Nevents = C->GetEntries();  
  for(Long64_t nevent = 0; nevent<Nevents; nevent++){
    if( nevent%1000 == 0){
      cout << nevent << "/" << Nevents << endl;
    }
    T->GetEntry(nevent);
    
    E_e = 0;
    memset(A, 0, ncell*sizeof(double));

    // Choosing track with least chi2 
    Double_t chi2min = 1000.;
    Int_t tr_min = -1;
    for(Int_t tr = 0; tr<T->bb_tr_n; tr++){
      if(T->bb_tr_chi2[tr]<chi2min){
	chi2min = T->bb_tr_chi2[tr];
	tr_min = tr;
      }
    }
    p_rec = (T->bb_tr_p[tr_min])*p_rec_Offset; 
    E_e = p_rec; // Neglecting e- mass. 

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

    // Choosing only events which had clusters in both PS and SH.
    if( T->bb_sh_nclus==0 || T->bb_ps_nclus==0 || T->bb_ps_idblk==-1 ) continue;
   
    if( T->bb_tr_tg_th[tr_min]>-0.15 && T->bb_tr_tg_th[tr_min]<0.15 && T->bb_tr_tg_ph[tr_min]>-0.3 &&
	T->bb_tr_tg_ph[tr_min]<0.3 && fabs(W-W_mean)<W_sigma ){ //cut on good tracks and W

      Int_t cl_max = -1;
      Double_t nblk = -1.;
      Double_t Emax = -10.;

      // ****** Shower ******
      // Loop over all the clusters first: select highest energy
      for(Int_t cl = 0; cl<T->bb_sh_nclus; cl++){
	if(T->bb_sh_clus_e[cl]>Emax){
	  Emax = T->bb_sh_clus_e[cl];
	  cl_max = cl;
	}
      }

      // Reject events with max on the edge
      if(T->bb_sh_clus_row[cl_max]==0 || T->bb_sh_clus_row[cl_max]==26 ||
	 T->bb_sh_clus_col[cl_max]==0 || T->bb_sh_clus_col[cl_max]==6) continue;
    
      // Loop over all the blocks in main cluster and fill in A's
      Double_t ClusEngSH_mod=0.;
      Int_t shrow = 0;
      Int_t shcol = 0;
      nblk = T->bb_sh_clus_nblk[cl_max];
      for(Int_t blk = 0; blk<nblk; blk++){
	Int_t blkID = int(T->bb_sh_clus_blk_id[blk]);
	shrow = int(T->bb_sh_clus_blk_row[blk]);
	shcol = int(T->bb_sh_clus_blk_col[blk]);
	A[blkID] += (T->bb_sh_clus_blk_e[blk])*oldADCratioSH[blkID];
	ClusEngSH_mod += (T->bb_sh_clus_blk_e[blk])*oldADCratioSH[blkID];
	nevents_per_cell[ blkID ]++; 
      }
    
      // ****** PreShower ******
      Double_t ClusEngPS_mod=0.;
      Int_t psrow = 0;
      Int_t pscol = 0;
      nblk = T->bb_ps_clus_nblk[0];
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
      h_res_BBCal->Fill( (clusEngBBCal_mod/p_rec) );
      h_clusE->Fill( clusEngBBCal_mod );
      h_SHclusE->Fill( ClusEngSH_mod );
      h_PSclusE->Fill( ClusEngPS_mod );
      h2_P_rec_vs_P_ang->Fill( P_ang, p_rec );

      h_res_BBCal_custom->Fill( (clusEngBBCal/p_rec) );
      h_clusE_custom->Fill( clusEngBBCal );
      h_SHclusE_custom->Fill( (T->bb_sh_e)*Corr_Factor_Enrg_Calib_w_Cosmic );
      h_PSclusE_custom->Fill( (T->bb_ps_e)*Corr_Factor_Enrg_Calib_w_Cosmic );

      // Checking to see if there is any bias in track recostruction ----
      //SH
      h2_SHeng_P_SHblk_raw->Fill( shcol, shrow, (T->bb_sh_e)*Corr_Factor_Enrg_Calib_w_Cosmic );
      h2_res_P_SHblk_raw->Fill( shcol, shrow, (clusEngBBCal/p_rec) );
      h2_count->Fill( shcol, shrow, 1.);
      h2_SHeng_P_SHblk->Divide( h2_SHeng_P_SHblk_raw, h2_count );
      h2_res_P_SHblk->Divide( h2_res_P_SHblk_raw, h2_count );

      double xtrATsh = T->bb_tr_x[cl_max] + zposSH*T->bb_tr_th[cl_max];
      double ytrATsh = T->bb_tr_y[cl_max] + zposSH*T->bb_tr_ph[cl_max];
      h2_res_P_SHblk_trPOS_raw->Fill( ytrATsh, xtrATsh, (clusEngBBCal/p_rec) );
      h2_count_trP->Fill( ytrATsh, xtrATsh, 1. );
      h2_res_P_SHblk_trPOS->Divide( h2_res_P_SHblk_trPOS_raw, h2_count_trP );

      //PS
      h2_PSeng_P_PSblk_raw->Fill( pscol, psrow, (T->bb_ps_e)*Corr_Factor_Enrg_Calib_w_Cosmic );
      h2_res_P_PSblk_raw->Fill( pscol, psrow, (clusEngBBCal/p_rec) );
      h2_count_PS->Fill( pscol, psrow, 1.);
      h2_PSeng_P_PSblk->Divide( h2_PSeng_P_PSblk_raw, h2_count_PS );
      h2_res_P_PSblk->Divide( h2_res_P_PSblk_raw, h2_count_PS );

      double xtrATps = T->bb_tr_x[cl_max] + zposPS*T->bb_tr_th[cl_max];
      double ytrATps = T->bb_tr_y[cl_max] + zposPS*T->bb_tr_ph[cl_max];
      h2_res_P_PSblk_trPOS_raw->Fill( ytrATps, xtrATps, (clusEngBBCal/p_rec) );
      h2_count_trP_PS->Fill( ytrATps, xtrATps, 1. );
      h2_res_P_PSblk_trPOS->Divide( h2_res_P_PSblk_trPOS_raw, h2_count_trP_PS );
      // -----

      // Let's customize the histograms
      h2_SHeng_P_SHblk->GetZaxis()->SetRangeUser(1.0,2.0);
      h2_res_P_SHblk->GetZaxis()->SetRangeUser(0.8,1.2);
      h2_res_P_SHblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);
      h2_PSeng_P_PSblk->GetZaxis()->SetRangeUser(0.3,1.0);
      h2_res_P_PSblk->GetZaxis()->SetRangeUser(0.8,1.2);
      h2_res_P_PSblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);

      // Let's costruct the matrix
      for(Int_t icol = 0; icol<ncell; icol++){
	B(icol)+= A[icol];
	for(Int_t irow = 0; irow<ncell; irow++){
	  M(icol,irow)+= A[icol]*A[irow]/E_e;
	} 
      }   
    }
  }
  
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
  int cell = 0;
  adcGain_SH = Form("Gain/eng_cal_gainCoeff_sh_%d_%d.txt",Set,iter);
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
  adcGain_PS = Form("Gain/eng_cal_gainCoeff_ps_%d_%d.txt",Set,iter);
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

  cout << " Finishing iteration " << iter << "..." << endl;
  cout << " Resulting histograms have been written to : " << outFile << endl;
  cout << " Gain ratios (new/old) for SH have been written to : " << gainRatio_SH << endl;
  cout << " Gain ratios (new/old) for PS have been written to : " << gainRatio_PS << endl;
  cout << " New adc gain coefficients (GeV/pC) for SH have been written to : " << adcGain_SH << endl;
  cout << " New adc gain coefficients (GeV/pC) for PS have been written to : " << adcGain_PS << endl;
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
  Corr_Factor_Enrg_Calib_w_Cosmic 1.0 
  ----
*/

/*
  List of input and output files:
  *Input files: 
  1. Gain/eng_cal_gainCoeff_sh(ps)_+(iter-1)+.txt # Contains old gain coeff. for SH(PS)  
  *Output files:
  1. hist/eng_cal_BBCal_+iter+.root # Contains all the interesting histograms
  2. Gain/eng_cal_gainRatio_sh(ps)_+iter+.txt # Contains gain ratios (new/old) for SH(PS)
  3. Gain/eng_cal_gainCoeff_sh(ps)_+iter+.txt # Contains new gain coeff. for SH(PS)
*/
