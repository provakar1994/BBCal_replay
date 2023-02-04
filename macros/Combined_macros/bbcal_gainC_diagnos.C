/* This macro generates diagnosis histograms for ADC gain coefficients. 
   ----
   P. Datta <pdbforce@jlab.org> Created 9 Mar 2022
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "TChain.h"
#include "TFile.h"
#include "TCut.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"

const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows
const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows

void ReadGain(TString,vector<double>&);
bool SHorPS=1; // SH=1, PS=0
vector<double> oldADCgainSH, newADCgainSH;
vector<double> oldADCgainPS, newADCgainPS;

void bbcal_gainC_diagnos( int set, int i_old, int i_new ){

  TString adcGain, outFile;
  cout << " Reading ADC gain coefficients for Shower " << endl;
  SHorPS=1;
  adcGain = Form("Gain/eng_cal_gainCoeff_sh_%d_%d.txt",set,i_old);
  ReadGain(adcGain, oldADCgainSH);
  adcGain = Form("Gain/eng_cal_gainCoeff_sh_%d_%d.txt",set,i_new);
  ReadGain(adcGain, newADCgainSH);
  // ------
  cout << " Reading ADC gain coefficients for PreShower " << endl;
  SHorPS=0;
  adcGain = Form("Gain/eng_cal_gainCoeff_ps_%d_%d.txt",set,i_old);
  ReadGain(adcGain, oldADCgainPS);
  adcGain = Form("Gain/eng_cal_gainCoeff_ps_%d_%d.txt",set,i_new);
  ReadGain(adcGain, newADCgainPS);

  outFile = Form("hist/bbcal_gC_diagnos_%d_%d_%d.root",set,i_old,i_new);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  //defining SH histos
  TH1F *h_oldCoeff_sh = new TH1F("h_oldCoeff_sh","h_oC_sh",189,0,189);
  TH1F *h_newCoeff_sh = new TH1F("h_newCoeff_sh","h_nC_sh",189,0,189);
  TH1F *h_diff_sh = new TH1F("h_diff_sh","h_diff_sh",189,0,189);
  //defining PS histos
  TH1F *h_oldCoeff_ps = new TH1F("h_oldCoeff_ps","h_oC_ps",52,0,52);
  TH1F *h_newCoeff_ps = new TH1F("h_newCoeff_ps","h_nC_ps",52,0,52);
  TH1F *h_diff_ps = new TH1F("h_diff_ps","h_diff_ps",52,0,52);

  //filling Shower histograms
  Int_t ish=0;
  while( ish<(kNcolsSH*kNrowsSH) ){
    h_oldCoeff_sh->Fill( ish, oldADCgainSH.at(ish) );
    h_newCoeff_sh->Fill( ish, newADCgainSH.at(ish) );

    Double_t diff = ((oldADCgainSH.at(ish)-newADCgainSH.at(ish))/oldADCgainSH.at(ish))*100.;
    h_diff_sh->Fill( ish, diff );
    ish++;
  }
  //filling PreShower histograms
  Int_t ips=0;
  while( ips<(kNcolsPS*kNrowsPS) ){
    h_oldCoeff_ps->Fill( ips, oldADCgainPS.at(ips) );
    h_newCoeff_ps->Fill( ips, newADCgainPS.at(ips) );

    Double_t diff = ((oldADCgainPS.at(ips)-newADCgainPS.at(ips))/oldADCgainPS.at(ips))*100.;
    h_diff_ps->Fill( ips, diff );
    ips++;
  }

  cout << "Finishing computaion..." << endl;
  cout << " --------- " << endl;
  cout << " Resulting histogram written to : " << outFile << endl;
  cout << " --------- " << endl;

  fout->Write();
  fout->Close(); fout->Delete();
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
  if( SHorPS==1 && elemID!=(kNcolsSH*kNrowsSH) ){
    cerr << " Broken file : " << adcGain << endl;
    throw;
  }else if( SHorPS==0 && elemID!=(kNcolsPS*kNrowsPS) ){
    cerr << " Broken file : " << adcGain << endl;
    throw;
  }
  adcGain_data.close();
}
