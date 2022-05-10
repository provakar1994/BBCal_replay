/*

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

const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows
const Int_t ncell = 52;   // total PS blocks

void ReadGain(TString,vector<double>&);
vector<double> oldADCgainPS, oldADCratioPS;

void calib_psEng_u_pionPeak(const char *configfilename)
{
  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  TChain *C = new TChain("T");

  Double_t minMBratio=0.1;
  Int_t Set=0, Iter=0, Nmin=10;
  Double_t h_psE_bin=200, h_psE_min=0., h_psE_max=5.;

  TMatrixD M(ncell,ncell), M_inv(ncell,ncell);
  TVectorD B(ncell), CoeffR(ncell);

  Double_t pi_e = 0.05; // minimum energy deposit by pi- in lead glass (50MeV default)
  Double_t A[ncell] = {0.};
  bool badCells[ncell]; // Cells that have events less than Nmin
  Int_t nevents_per_cell[ncell];

  // Define a clock to check macro processing time
  TStopwatch *sw = new TStopwatch();
  TStopwatch *sw2 = new TStopwatch();
  sw->Start();
  sw2->Start();

  // reading the config file
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
      if( skey == "pi_eng" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	pi_e = sval.Atof();
      }
      if( skey == "Set" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Set = sval.Atof();
      }
      if( skey == "Iter" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Iter = sval.Atof();
      }
      if( skey == "Min_Event_Per_Channel" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Nmin = sval.Atof();
      }
      if( skey == "Min_MB_Ratio" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	minMBratio = sval.Atoi();
      }
      if( skey == "h_psE" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_psE_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_psE_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_psE_max = sval2.Atof();
      }
      if( skey == "*****" ){
	break;
      }
    } 
    delete tokens;
  }
  cout << endl;

  // rading old ADC gain and gain ratios for PS
  TString adcGain, gainRatio;
  // adcGain = Form("Gain/eng_cal_gainCoeff_ps_%d_%d.txt",Set,Iter);
  adcGain = Form("Gain/eng_calib_gainCoeff_ps_piPeak_%d_%d.txt",Set,Iter-1);
  ReadGain(adcGain, oldADCgainPS);
  // gainRatio = Form("Gain/eng_cal_gainRatio_ps_%d_%d.txt",Set,Iter);
  gainRatio = Form("Gain/eng_calib_gainRatio_ps_piPeak_%d_%d.txt",Set,Iter-1);
  ReadGain(gainRatio, oldADCratioPS);

  // Clear arrays
  memset(nevents_per_cell, 0, ncell*sizeof(int));
  memset(badCells, 0, ncell*sizeof(bool));

  // Implementing global cuts
  if(C->GetEntries()==0){
    cerr << endl << " --- No ROOT file found!! --- " << endl << endl;
    throw;
  }else cout << endl << "Found " << C->GetEntries() << " events. Implementing global cuts.. " << endl;
  TEventList *elist = new TEventList("elist","Event list for BBCAL energy calibration");
  C->Draw(">>elist",globalcut);  
  gmn_tree *T = new gmn_tree(C);

  TString outFile = Form("hist/eng_calib_ps_piPeak_%d_%d.root",Set,Iter);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();
  TH1D *h_PSclusE = new TH1D("h_PSclusE","Best PS Cluster Energy"
			     ,h_psE_bin,h_psE_min,h_psE_max);

  // Looping over good events ====================================================================//
  Long64_t Nevents = elist->GetN(), nevent=0;  
  cout << endl << "Processing " << Nevents << " events.." << endl;
  Double_t timekeeper = 0., timeremains = 0.;

  while( T->GetEntry( elist->GetEntry(nevent++) ) ){

    // keeping track of progress
    sw2->Stop();
    timekeeper += sw2->RealTime();
    if( nevent%25000 == 0 && nevent!=0 ) 
      timeremains = timekeeper*( double(Nevents)/double(nevent) - 1. ); 
    sw2->Reset();
    sw2->Continue();
    if( nevent % 100 == 0 ) cout << nevent << "/" << Nevents  << ", " << int(timeremains/60.) << "m \r";;
    cout.flush();
    // ------

    memset(A, 0, ncell*sizeof(double));

    Double_t ClusEngPS_mod=0.;
    Int_t psrow= T->bb_ps_rowblk; //0;
    Int_t pscol= T->bb_ps_colblk; //0;
    Int_t nblk = T->bb_ps_nblk;
    for(Int_t blk=0; blk<nblk; blk++){
      Int_t blkID = int(T->bb_ps_clus_blk_id[blk]);
      A[blkID]+=(T->bb_ps_clus_blk_e[blk])*oldADCratioPS[blkID];
      ClusEngPS_mod += (T->bb_ps_clus_blk_e[blk])*oldADCratioPS[blkID];
      nevents_per_cell[blkID]++;
    }

    h_PSclusE->Fill( ClusEngPS_mod );

    // Let's costruct the matrix
    for(Int_t icol = 0; icol<ncell; icol++){
      B(icol) += A[icol];
      for(Int_t irow = 0; irow<ncell; irow++){
	M(icol,irow) += A[icol]*A[irow]/pi_e;
      } 
    }   

  } //event loop

  cout << endl << endl;

  // B.Print();  
  // M.Print();

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
  // CoeffR.Print();

  TH1D *h_nevent_blk_PS = new TH1D("h_nevent_blk_PS","No. of Good Events; PS Blocks",52,0,52);
  TH1D *h_coeff_Ratio_PS = new TH1D("h_coeff_Ratio_PS","Ratio of Gain Coefficients(new/old); PS Blocks"
				    ,52,0,52);
  TH1D *h_coeff_blk_PS = new TH1D("h_coeff_blk_PS","ADC Gain Coefficients(GeV/pC); PS Blocks",52,0,52);
  TH1D *h_Old_Coeff_blk_PS = new TH1D("h_Old_Coeff_blk_PS","Old ADC Gain Coefficients(GeV/pC); PS Blocks"
				      ,52,0,52);
  TH2D *h_coeff_detView_PS = new TH2D("h_coeff_detView_PS","ADC Gain Coefficients(Detector View)",
				      kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);
  
  Int_t cell = 0;
  TString adcGain_PS = Form("Gain/eng_calib_gainCoeff_ps_piPeak_%d_%d.txt",Set,Iter);
  TString gainRatio_PS = Form("Gain/eng_calib_gainRatio_ps_piPeak_%d_%d.txt",Set,Iter);
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
	if(Iter==7){
	  h_coeff_Ratio_PS->Fill( psBlock, 1. );
	  h_coeff_blk_PS->Fill( psBlock, oldCoeff );
	  h_coeff_detView_PS->Fill( pscol+1, psrow+1, oldCoeff );

	  cout << 1. << "  ";
	  adcGainPS_outData << oldCoeff << " ";
	  gainRatioPS_outData << 1. << " ";
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
  adcGainPS_outData.close();
  gainRatioPS_outData.close();

  cout << "Finishing iteration " << Iter << "..." << endl;
  cout << " --------- " << endl;
  cout << " Resulting histograms  written to : " << outFile << endl;
  cout << " Gain ratios (new/old) for PS written to : " << gainRatio_PS << endl;
  cout << " New adc gain coefficients (GeV/pC) for PS written to : " << adcGain_PS << endl;
  cout << " --------- " << endl;

  sw->Stop();
  sw2->Stop();
  cout << "CPU time elapsed = " << sw->CpuTime() << " s. Real time = " 
       << sw->RealTime() << " s. " << endl << endl;

  sw->Delete();
  sw2->Delete();
}


void ReadGain( TString adcGain, vector<double>& gain ){
  gain.clear();
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
	gain.push_back(temptoken.Atof());
	elemID++;
      }
    }
  }else{
    cerr << " No file : " << adcGain << endl;
    throw;
  }
  //check whether ADC gain is there for all blocks
  if( elemID!=(kNcolsPS*kNrowsPS) ){
    cerr << " Broken file : " << adcGain << endl;
    throw;
  }
  adcGain_data.close();
}
