/*
This macro calculates the adc gain coefficient for BBSH or PS (per user input) for estimated energy
deposition in the detector due to cosmic muons.
------
P.Datta <pdbforce@jlab.org> Created 10 Oct 2021 
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

using namespace std;

static const Int_t kNrows=27;
static const Int_t kNcols=7;
static const Int_t kNrowsPS=26;
static const Int_t kNcolsPS=2;

void ReadAmpToInt(TString,bool);
bool SHorPS=1; // SH=1,PS=0
double cF = 1.21;
double trigAmp = 25.;
vector<double> ampToint;
vector<double> elemID;

void calculate_adcGain_cos(int nrun=100){
  
  cout << " Run number? " << endl;
  cin >> nrun;
  cout << " Shower(SH) or PreShower(PS)? [SH=1, PS=0] " << endl;
  cin >> SHorPS;
  cout << " Trigger amplitude? (mV) [Default: 25mV] " << endl;
  cin >> trigAmp;
  cout << " Correction factor for gain calibration? [Default: 1.21]" << endl;
  cin >> cF;

  ampToint.clear();
  TString InFile;
  if(SHorPS){
    InFile = Form("Output/fit_results/run_%d_sh_ampToint.txt",nrun);
    ReadAmpToInt(InFile,SHorPS);
  }else{
    InFile = Form("Output/fit_results/run_%d_ps_ampToint.txt",nrun);
    ReadAmpToInt(InFile,SHorPS);
  }
 
  TH2F* h_adcGain_SH = new TH2F("h_adcGain_SH"," ADC Gain Factor : SH (MeV/pC) ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
  TH2F* h_adcGain_PS = new TH2F("h_adcGain_PS"," ADC Gain Factor : PS (MeV/pC) ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);
  TCanvas* can2d;
  TString OutFile;

  gStyle->SetOptStat(0);
  gStyle->SetPalette(1,0);
  gStyle->SetPaintTextFormat("4.2f");  
  const Int_t Number=3;
  Double_t Red[Number] = { 1.0,0.0,0.0};
  Double_t Blue[Number] = { 1.0,0.0,1.0};
  Double_t Green[Number] = { 0.0,1.0,0.0};
  Double_t Len[Number] = { 0.0,.5,1.0};
  Int_t nb=50;
  TColor::CreateGradientColorTable(Number,Len,Red,Green,Blue,nb);
  can2d = new TCanvas("can_2d","2d ",700,1000);
  can2d->cd();

  if(SHorPS){
    OutFile = Form("Output/adcGain_%d_SH_%0.1fmV_cF%0.2f.txt",nrun,trigAmp,cF);
    cout << " ADC gain factors (GeV/pC) written to : " << OutFile << endl;
    ofstream outfile_data;
    outfile_data.open(OutFile);
    for(int r=0; r<kNrows; r++){
      for(int c=0; c<kNcols; c++){
	Double_t convF = (ampToint.at(r*kNcols+c)*0.06*cF)/trigAmp; //GeV/pC : Assuming 60MeV cosmic eng. dep. 
	h_adcGain_SH->Fill(float(c+1),float(r+1),1000.*convF);  //and trigger amp is aligned at 10mV.
	outfile_data << convF << ' ';
      }
      outfile_data << endl;
    }
    outfile_data.close();

    //h_adcGain_SH->SetMaximum(14); 
    //h_adcGain_SH->SetMinimum(10); 
    h_adcGain_SH->Draw("text colz");
    can2d->SaveAs(Form("plots/adcGain_%d_SH_%0.1fmV_cF%0.2f.pdf",nrun,trigAmp,cF));
  }else{
    OutFile = Form("Output/adcGain_%d_PS_%0.1fmV_cF%0.2f.txt",nrun,trigAmp,cF);
    cout << " ADC gain factors (GeV/pC) written to : " << OutFile << endl;
    ofstream outfile_data;
    outfile_data.open(OutFile);
    for(int r=0; r<kNrowsPS; r++){
      for(int c=0; c<kNcolsPS; c++){
	Double_t convF = (ampToint.at(r*kNcolsPS+c)*0.06*cF)/trigAmp; //GeV/pC : Assuming 60MeV cosmic eng. dep.       
	h_adcGain_PS->Fill(float(c+1),float(r+1),1000.*convF);   //and trigger amp is aligned at 10mV.
	outfile_data << convF << ' ';
      }
      outfile_data << endl;
    }
    outfile_data.close();

    //h_adcGain_PS->SetMaximum(19); //19 
    //h_adcGain_PS->SetMinimum(15); //15
    h_adcGain_PS->Draw("text colz");
    can2d->SaveAs(Form("plots/adcGain_%d_PS_%0.1fmV_cF%0.2f.pdf",nrun,trigAmp,cF));
  }
  
  cout << " --------- " << endl;
  cout << " ADC Gains have been  written to : " << OutFile << endl;
  cout << " --------- " << endl;

}

void ReadAmpToInt( TString adcGain, bool SHorPS ){
  ifstream adcGain_data;
  adcGain_data.open(adcGain);
  string readline;
  Int_t elemID=0;
  if( adcGain_data.is_open() ){
    cout << " Reading Amp to Int ratios from : "<< adcGain << endl;
    while(getline(adcGain_data,readline)){
      istringstream tokenStream(readline);
      string token;
      char delimiter = ' ';
      while(getline(tokenStream,token,delimiter)){
	TString temptoken=token;
	ampToint.push_back( temptoken.Atof() );
	elemID++;
      }
    }
  }else{
    cerr << " No file : " << adcGain << endl;
    throw;
  }
  adcGain_data.close();
}
