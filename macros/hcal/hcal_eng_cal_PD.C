/*
  This script has been prepared for the energy calibration of Hadron Calorimeter detector. 
  It does so by minimizing the chi2 of the difference between calorimeter cluster energy 
  and the KE of scattered proton. It reads
  in the old adc gain coefficients (GeV/pC) and writes the new ones in file. One needs a configfile to
  execute this script. Example content of such a file is attached in the end. To execute, do:
  ----
  [a-onl@aonl2 macros]$ root -l 
  root [0] .L Combined_macros/eng_cal_BBCal.C+
  root [1] eng_cal_BBCal("Combined_macros/setup_eng_cal_BBCal.txt")
  ----
  P. Datta  <pdbforce@jlab.org>  Created  15 Oct 2021 (Built on top of E. Fuchey's version)
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

const Int_t ncell = 288;  
const Int_t kNcols = 12; // HCal columns
const Int_t kNrows = 24; // HCal rows

const Double_t Mp = 0.938272; // GeV

void ReadGain(TString,bool);
bool GainOrRatio = 1; // Gain = 1, Ratio = 0
Double_t oldADCgain[kNcols*kNrows] = {0.};
Double_t oldADCratio[kNcols*kNrows] = {0.};

void hcal_eng_cal(const char *configfilename, int iter=1)
{

  TChain *C = new TChain("T");
  gmn_tree *T = new gmn_tree(C);

  Int_t Nmin = 10;
  Double_t minMBratio = 0.1;
  Double_t E_beam = 0.;
  Double_t p_rec_Offset = 1.;
  Double_t W_mean = 0.;
  Double_t W_sigma = 0.;
  Double_t Scale_Factor_for_BadChannels = 1.;

  TMatrixD M(ncell,ncell), M_inv(ncell,ncell);
  TVectorD B(ncell), CoeffR(ncell);
  
  Double_t E_e = 0;
  Double_t KE_p = 0;
  Double_t p_rec = 0.;
  Double_t A[ncell] = {0.};
  bool badCells[ncell]; // Cells that have events less than Nmin
  TString adcGain, gainRatio;
  Int_t events_per_cell[ncell];

  // Reading config file
  ifstream configfile(configfilename);
  TString currentline;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endlist") ){
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
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("#") ){
    TObjArray *tokens = currentline.Tokenize(" ");
    Int_t ntokens = tokens->GetEntries();
    if( ntokens>1 ){
      TString skey = ( (TObjString*)(*tokens)[0] )->GetString();
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
      if( skey == "Scale_Factor_for_BadChannels" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Scale_Factor_for_BadChannels = sval.Atof();
      }
    } 
    delete tokens;
  }
  
  // Clear arrays
  memset(events_per_cell, 0, ncell*sizeof(int));
  memset(badCells, 0, ncell*sizeof(bool));
  
  // Let's read in old coefficients and ratios(new/old) for both SH and PS
  GainOrRatio = 1; // Gain
  adcGain = Form("Gain_h/hcal_eng_cal_gainCoeff_%d.txt",iter-1);
  ReadGain(adcGain,GainOrRatio);
  GainOrRatio = 0; // Ratio
  adcGain = Form("Gain_h/hcal_eng_cal_gainRatio_%d.txt",iter-1);
  ReadGain(adcGain,GainOrRatio);

  TString outFile = Form("hist_h/hcal_eng_cal_%d.root",iter);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  // Physics histograms
  TH1D *h_W = new TH1D("h_W","W distribution",200,0.7,1.6);
  TH1D *h_Q2 = new TH1D("h_Q2","Q2 distribution",40,0.,4.);
  TH1D *h_KE_p = new TH1D("h_KE_p","E_beam - E_e",100,-1.5,1.5);
  TH1D *h_deltaE = new TH1D("h_deltaE","1.-E_clus/p_rec",100,-1.5,1.5);
  TH1D *h_clusE = new TH1D("h_clusE","Best Cluster Energy",100,0.,2.);
  TH2D *h_corPandAng = new TH2D("h_corPandAng","Track p vs Track ang",100,30,60,100,0.4,1.2);

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

    KE_p = E_beam - E_e;

    if(T->bb_tr_p[tr_min]==0 || E_e==0 || tr_min<0) continue;

    Double_t P_ang = 57.3*TMath::ACos(T->bb_tr_pz[tr_min]/T->bb_tr_p[tr_min]);
    Double_t Q2 = 4.*E_beam*p_rec*pow( TMath::Sin(P_ang/57.3/2.),2. );
    Double_t W2 = Mp*Mp + 2.*Mp*(E_beam-p_rec) - Q2;
    Double_t W = 0.;

    if(W2>0.){
      W = TMath::Sqrt(W2);  
      h_W->Fill(W);
    }
    h_Q2->Fill(Q2);
    h_KE_p->Fill(KE_p);

    // Choosing only events which had clusters in both PS and SH.
    if(T->sbs_hcal_nclus==0 ) continue;
   
    if( T->bb_tr_tg_th[tr_min]>-0.15&&T->bb_tr_tg_th[tr_min]<0.15&&T->bb_tr_tg_ph[tr_min]>-0.3&&T->bb_tr_tg_ph[tr_min]<0.3 && fabs(W-W_mean)<W_sigma ){ //cut on tracks and W
      Int_t cl_max = -1;
      Double_t nblk = -1.;
      Double_t Emax = -10.;

      // ****** Shower ******
      // Loop over all the clusters first: select highest energy
      for(Int_t cl = 0; cl<T->sbs_hcal_nclus; cl++){
	if(T->sbs_hcal_clus_e[cl]>Emax){
	  Emax = T->sbs_hcal_clus_e[cl];
	  cl_max = cl;
	}
      }

      // Reject events with max on the edge
      if(T->sbs_hcal_clus_row[cl_max]==0 || T->sbs_hcal_clus_row[cl_max]==23 ||
	 T->sbs_hcal_clus_col[cl_max]==0 || T->sbs_hcal_clus_col[cl_max]==11) continue;
    
      // Loop over all the blocks in main cluster and fill in A's
      Double_t ClusEng = 0.;
      nblk = T->sbs_hcal_clus_nblk[cl_max];
      for(Int_t blk = 0; blk<nblk; blk++){
	Int_t blkID = int(T->sbs_hcal_clus_blk_id[blk])-1;
	A[blkID] += (T->sbs_hcal_clus_blk_e[blk])*oldADCratio[blkID];
	ClusEng += (T->sbs_hcal_clus_blk_e[blk])*oldADCratio[blkID];
	events_per_cell[ blkID ]++; 
      }

      // Let's fill some interesting histograms
      h_deltaE->Fill( 1.-(ClusEng/KE_p) );
      h_clusE->Fill( ClusEng );
      h_corPandAng->Fill( P_ang, p_rec );

      // Let's construct the matrix
      for(Int_t icol = 0; icol<ncell; icol++){
	B(icol)+= A[icol];
	for(Int_t irow = 0; irow<ncell; irow++){
	  M(icol,irow)+= A[icol]*A[irow]/(0.0795*KE_p); //Including the sampling fraction of the detector (0.0659MeV/0.8286MeV sampled/KE_p)
	} 
      }   
    }
  }
  
  // B.Print();  
  // M.Print();

  //Diagnostic histograms
  TH1D *h_neventChan = new TH1D("h_neventChan","No. of Good Events; HCal Blocks",189,0,189);
  TH1D *h_coeffRatio = new TH1D("h_coeffRatio","Ratio of Gain Coefficients(new/old); HCal Blocks",189,0,189);
  TH1D *h_coeffChan = new TH1D("h_coeffChan","ADC Gain Coefficients(GeV/pC); HCal Blocks",189,0,189);
  TH1D *h_oldCoeffChan = new TH1D("h_onlCoeffChan","Old ADC Gain Coefficients(GeV/pC); HCal Blocks",189,0,189);
  TH2D *h_coeffDV = new TH2D("h_coeffDV","ADC Gain Coefficients(Detector View)",kNcols,1,kNcols+1,kNrows,1,kNrows+1);

  // Leave the bad channel out of the calculation
  for(Int_t j = 0; j<ncell; j++){
    badCells[j]=false;
    if( events_per_cell[j]<Nmin || M(j,j)< minMBratio*B(j) ){
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
  adcGain = Form("Gain_h/hcal_eng_cal_gainCoeff_%d.txt",iter);
  gainRatio = Form("Gain_h/hcal_eng_cal_gainRatio_%d.txt",iter);
  ofstream adcGain_outData, gainRatio_outData;
  adcGain_outData.open(adcGain);
  gainRatio_outData.open(gainRatio);
  for(Int_t row = 0; row<kNrows; row++){
    for(Int_t col = 0; col<kNcols; col++){
      Double_t oldCoeff = oldADCgain[row*kNcols+col];
      if(!badCells[cell]){
	h_coeffRatio->Fill( cell, CoeffR(cell) );
	h_coeffChan->Fill( cell, CoeffR(cell)*oldCoeff );
	h_neventChan->Fill( cell, events_per_cell[cell] );
	h_oldCoeffChan->Fill( cell, oldCoeff );
	h_coeffDV->Fill( col+1, row+1, CoeffR(cell)*oldCoeff );

	cout << CoeffR(cell) << "  ";
	adcGain_outData << CoeffR(cell)*oldCoeff << " ";
	gainRatio_outData << CoeffR(cell) << " ";
      }else{
	h_coeffRatio->Fill( cell, 1.*Scale_Factor_for_BadChannels );
	h_coeffChan->Fill( cell, oldCoeff*Scale_Factor_for_BadChannels );
	h_neventChan->Fill( cell, events_per_cell[cell] );
	h_oldCoeffChan->Fill( cell, oldCoeff );
	h_coeffDV->Fill( col+1, row+1, oldCoeff*Scale_Factor_for_BadChannels );

	cout << 1.*Scale_Factor_for_BadChannels << "  ";
	adcGain_outData << oldCoeff*Scale_Factor_for_BadChannels << " ";
	gainRatio_outData << 1.*Scale_Factor_for_BadChannels << " ";
      }
      cell++;
    }
    cout << endl;
    adcGain_outData << endl;
    gainRatio_outData << endl;
  }

  fout->Write();

  M.Clear();
  B.Clear();
  C->Delete();
  CoeffR.Clear();
  adcGain_outData.close();
  gainRatio_outData.close();

  cout << " Finishing iteration " << iter << "..." << endl;
  cout << " Resulting histograms have been written to : " << outFile << endl;
  cout << " Gain ratios (new/old) have been written to : " << gainRatio << endl;
  cout << " New adc gain coefficients (GeV/pC) have been written to : " << adcGain << endl;
}


void ReadGain( TString adcGain, bool GainOrRatio ){
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
	if(GainOrRatio){
	  oldADCgain[elemID] = temptoken.Atof();
	}else{
	  oldADCratio[elemID] = temptoken.Atof();
	}
	elemID++;
      }
    }
  }else{
    cout << " No file : " << adcGain << endl;
  }
  adcGain_data.close();
}


/*
  Example configfile required to run this script:
  ----
  [a-onl@aonl2 Combined_macros]$ cat setup_example.txt 
  example1.root
  example2.root
  example3.root
  example4.root
  endlist
  root.example1.tree>0&&root.example2.tree
  endcut
  E_beam 1.92
  Min_Event_Per_Channel 300
  Min_MB_Ratio 0.1
  p_rec_Offset 1.05
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
