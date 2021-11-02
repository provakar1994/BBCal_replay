/*
This script has been prepared for the energy calibration of BigBite Calorimeter (BigBite Shower
+ BigBite Preshower) detector. It does so by minimizing the chi2 of the difference between calorimeter
cluster energy and the reconstructed electron energy(we get that from tracking information). It reads
in the old adc gain coefficients (GeV/pC) and writes the new ones in file. One needs a configfile to
execute this script. Example content of such a file is attached in the end. To execute, do:
----
[a-onl@aonl2 macros]$ pwd
/adaqfs/home/a-onl/sbs/BBCal_replay/macros
[a-onl@aonl2 macros]$ root -l 
root [0] .L Combined_macros/eng_cal_BBCal.C+
root [1] eng_cal_BBCal("Combined_macros/setup_eng_cal_BBCal.txt")
----
P. Datta  <pdbforce@jlab.org>  Created  15 Oct 2021 (Based on AJR Puckett & E. Fuchey s' version)
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

const Int_t ncell = 241;   // 189(SH) + 52(PS), Convention: 0-188: SH; 189-240: PS.
const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows
const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows

const Double_t Mp = 0.938; // GeV

void ReadGain(TString,bool);
bool SHorPS = 1; // SH = 1, PS = 0
Double_t oldADCgainSH[kNcolsSH*kNrowsSH] = {0.};
Double_t oldADCgainPS[kNcolsPS*kNrowsPS] = {0.};  

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
    } 
    delete tokens;
  }


  TString outFile = Form("hist/eng_cal_BBCal_%d_%d.root",Set,iter);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  // Physics histograms
  TH1D* h_deltaE = new TH1D("h_deltaE","1.-E_clus/p_rec",100,-1.5,1.5);
  TH1D* h_clusE = new TH1D("h_clusE","Best Cluster Energy (SH+PS)",100,0.,2.);
  TH2D* h_corPandAng = new TH2D("h_corPandAng","Track p vs Track ang",100,30,60,100,0.4,1.2);
  TH1D* h_W = new TH1D("h_W","W distribution",200,0.7,1.6);
  TH1D* h_Q2 = new TH1D("h_Q2","Q2 distribution",40,0.,4.);
  
  TMatrixD M(ncell,ncell);
  TVectorD B(ncell);
  
  Int_t events_per_cell[ncell];
  memset(events_per_cell, 0, ncell*sizeof(int));
  
  Double_t E_e = 0;
  Double_t p_rec = 0.;
  Double_t A[ncell] = {0.};
  bool badcells[ncell]; // Cells that have events less than Nmin
  TString adcGain_SH, gainRatio_SH;
  TString adcGain_PS, gainRatio_PS;

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
    if(W2>0.){
      W = TMath::Sqrt(W2);  
      h_W->Fill(W);
    }

    // Choosing only events which had clusters in both PS and SH.
    if(T->bb_sh_nclus==0 || T->bb_ps_nclus==0 ) continue;
   
    if( T->bb_tr_tg_th[tr_min]>-0.15&&T->bb_tr_tg_th[tr_min]<0.15&&T->bb_tr_tg_ph[tr_min]>-0.3&&T->bb_tr_tg_ph[tr_min]<0.3 && fabs(W-W_mean)<W_sigma ){ //cut on tracks and W
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
      nblk = T->bb_sh_clus_nblk[cl_max];
      for(Int_t blk = 0; blk<nblk; blk++){
	Int_t blkID = int(T->bb_sh_clus_blk_id[blk]);
	A[blkID] += T->bb_sh_clus_blk_e[blk];
	events_per_cell[ blkID ]++; 
      }
    
      // ****** PreShower ******
      nblk = T->bb_ps_clus_nblk[0];
      for(Int_t blk=0; blk<nblk; blk++){
	Int_t blkID = int(T->bb_ps_clus_blk_id[blk]);
	A[189+blkID]+=T->bb_ps_clus_blk_e[blk];
	events_per_cell[189+blkID]++;
      }

      // Let's fill some interesting histograms
      Double_t clusEngBBCal = T->bb_sh_e + T->bb_ps_e;
      h_deltaE->Fill( 1.-(clusEngBBCal/p_rec) );
      h_clusE->Fill( clusEngBBCal );
      h_corPandAng->Fill( P_ang, p_rec );

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
  TH1D *h_neventChan_SH = new TH1D("h_neventChan_SH","No. of Good Events; SH Blocks",189,0,189);
  TH1D *h_coeffRatio_SH = new TH1D("h_coeffRatio_SH","Ratio of Gain Coefficients(new/old); SH Blocks",189,0,189);
  TH1D *h_coeffChan_SH = new TH1D("h_coeffChan_SH","ADC Gain Coefficients(GeV/pC); SH Blocks",189,0,189);
  TH1D *h_oldCoeffChan_SH = new TH1D("h_onlCoeffChan_SH","Old ADC Gain Coefficients(GeV/pC); SH Blocks",189,0,189);
  TH2D *h_coeffDV_SH = new TH2D("h_coeffDV_SH","ADC Gain Coefficients(Detector View)",kNcolsSH,1,kNcolsSH+1,kNrowsSH,1,kNrowsSH+1);

  TH1D *h_neventChan_PS = new TH1D("h_neventChan_PS","No. of Good Events; PS Blocks",52,0,52);
  TH1D *h_coeffRatio_PS = new TH1D("h_coeffRatio_PS","Ratio of Gain Coefficients(new/old); PS Blocks",52,0,52);
  TH1D *h_coeffChan_PS = new TH1D("h_coeffChan_PS","ADC Gain Coefficients(GeV/pC); PS Blocks",52,0,52);
  TH1D *h_oldCoeffChan_PS = new TH1D("h_onlCoeffChan_PS","Old ADC Gain Coefficients(GeV/pC); PS Blocks",52,0,52);
  TH2D *h_coeffDV_PS = new TH2D("h_coeffDV_PS","ADC Gain Coefficients(Detector View)",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);

  // Leave the bad channel out of the calculation
  for(Int_t j = 0; j<ncell; j++){
    badcells[j]=false;
    if( events_per_cell[j]<Nmin || M(j,j)< minMBratio*B(j) ){
      B(j) = 1.;
      M(j, j) = 1.;
      for(Int_t k = 0; k<ncell; k++){
	if(k!=j){
	  M(j, k) = 0.;
	  M(k, j) = 0.;
	}
      }
      badcells[j]=true;
    }
  }  
  
  // Getting coefficients (rather ratios)
  TMatrixD M_inv = M.Invert();
  TVectorD CoeffR = M_inv*B;
  
  // Let's read in old coefficients for both SH and PS
  adcGain_SH = Form("Gain/eng_cal_gainCoeff_sh_%d_%d.txt",Set,iter-1);
  adcGain_PS = Form("Gain/eng_cal_gainCoeff_ps_%d_%d.txt",Set,iter-1);
  ReadGain(adcGain_SH,SHorPS);
  SHorPS = 0; // Setting the flag for PS
  ReadGain(adcGain_PS,SHorPS);

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
      if(!badcells[cell]){
	h_coeffRatio_SH->Fill( cell, CoeffR(cell) );
	h_coeffChan_SH->Fill( cell, CoeffR(cell)*oldCoeff );
	h_neventChan_SH->Fill( cell, events_per_cell[cell] );
	h_coeffDV_SH->Fill( shcol+1, shrow+1, CoeffR(cell)*oldCoeff );
	h_oldCoeffChan_SH->Fill( cell, oldCoeff );

	cout << CoeffR(cell) << "  ";
	adcGainSH_outData << CoeffR(cell)*oldCoeff << " ";
	gainRatioSH_outData << CoeffR(cell) << " ";
      }else{
	h_coeffRatio_SH->Fill( cell, 1. );
	h_coeffChan_SH->Fill( cell, oldCoeff );
	h_neventChan_SH->Fill( cell, events_per_cell[cell] );
	h_coeffDV_SH->Fill( shcol+1, shrow+1, oldCoeff );
	h_oldCoeffChan_SH->Fill( cell, oldCoeff );

	cout << 1. << "  ";
	adcGainSH_outData << oldCoeff << " ";
	gainRatioSH_outData << 1. << " ";
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
      Double_t oldCoeff = oldADCgainPS[psBlock];
      if(!badcells[cell]){
	h_coeffRatio_PS->Fill( psBlock, CoeffR(cell) );
	h_coeffChan_PS->Fill( psBlock, CoeffR(cell)*oldCoeff );
	h_neventChan_PS->Fill( psBlock, events_per_cell[cell] );
	h_coeffDV_PS->Fill( pscol+1, psrow+1, CoeffR(cell)*oldCoeff );
	h_oldCoeffChan_PS->Fill( psBlock, oldCoeff );

	cout << CoeffR(cell) << "  ";
	adcGainPS_outData << CoeffR(cell)*oldCoeff << " ";
	gainRatioPS_outData << CoeffR(cell) << " ";
      }else{
	h_coeffRatio_PS->Fill( psBlock, 1. );
	h_coeffChan_PS->Fill( psBlock, oldCoeff );
	h_neventChan_PS->Fill( psBlock, events_per_cell[cell] );
	h_coeffDV_PS->Fill( pscol+1, psrow+1, oldCoeff );
	h_oldCoeffChan_PS->Fill( psBlock, oldCoeff );

	cout << 1. << "  ";
	adcGainPS_outData << oldCoeff << " ";
	gainRatioPS_outData << 1. << " ";
     }
      cell++;
    }
    cout << endl;
    adcGainPS_outData << endl;
    gainRatioPS_outData << endl;
  }

  fout->Write();
  adcGainSH_outData.close();
  adcGainPS_outData.close();
  gainRatioSH_outData.close();
  gainRatioPS_outData.close();

  cout << " Resulting histograms have been written to : " << outFile << endl;
  cout << " Gain ratios (new/old) for SH have been written to : " << gainRatio_SH << endl;
  cout << " Gain ratios (new/old) for PS have been written to : " << gainRatio_PS << endl;
  cout << " New adc gain coefficients (GeV/pC) for SH have been written to : " << adcGain_SH << endl;
  cout << " New adc gain coefficients (GeV/pC) for PS have been written to : " << adcGain_PS << endl;
}


void ReadGain( TString adcGain, bool SHorPS ){
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
	  oldADCgainSH[elemID] = temptoken.Atof();
	}else{
	  oldADCgainPS[elemID] = temptoken.Atof();
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
