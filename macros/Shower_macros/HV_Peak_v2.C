/* 
   This script analyzes multiple cosmic runs simultaneously from 
   a HV scan & fits the signal peak position (at FADC) vs HV plot 
   using a polynomial to extract alpha values for individual Shower 
   PMTs. NOTE: This script reads peak positions and HV settings for
   each cosmic run during execution. One can get the peak positions 
   by executing bbsh_cos_cal.C script. Be careful while while choosing
   signal peaks at trigger or FADC, the choice needs to be consistent 
   between these two scripts. Using trigger amplitude everywhere is 
   recommended. Example execution:
   ----
   [a-onl@aonl2 macros]$ pwd
   /adaqfs/home/a-onl/sbs/BBCal_replay/macros
   [a-onl@aonl2 macros]$ root -l 
   root [0] .L Shower_macros/HV_Peak_v2.C+
   root [1] AddRun(<runnumber1>)
   root [2] AddRun(<runnumber2>)
   root [3] [.. add as many runs as you want]
   root [4] FitRuns(<peak_pos>)
            #peak_pos = -1 => desired signal peak postion at trigger. 
	    #peak_pos != -1 => desired signal peak postion at FADC (mV). 
   root [1] WriteHV()  #writes calibrated HV setting for peak_pos
   ----
   M. K. Jones  Created
   P. Datta     Edited
*/

#include <TString.h>
#include <TLegend.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TLine.h>
#include <TLegend.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
#include <TCutG.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TMath.h>
#include <TProfile.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TLatex.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include<math.h>
using namespace std;


static const Int_t shNCol=7;
static const Int_t shNRow=27;

Double_t Peak_Desired = 10.; // 
Double_t desTrigamp = 0.;
bool trigtoFADC_ratio = false;

Double_t HV_Value[2][16][12];
Int_t HV_Block[2][16][12];

void SetHVMap();
void ReadHV(Int_t nrun);
void ReadPeak(Int_t nrun);
void ReadHist(Int_t nrun);
void PrintRuns();
void UpdateHV();
void WriteHV();
void AddRun(Int_t nrun);
void FitRuns(Double_t Set_Peak);
Double_t fitfunct(Double_t *x,Double_t *par);
void GenDesiredFADCamps(Double_t desTrigamp);

vector <Int_t> RunList;
vector<double> DesiredFADCamps;
vector<vector<double> > HVList;
vector<vector<double> > PeakList;
vector<vector<double> > PeakErrList;
vector<vector<TH1F*> > HistList;

Double_t fit_alpha[shNCol*shNRow];
Double_t fit_alpha_err[shNCol*shNRow];
Double_t fit_chi2[shNCol*shNRow];
Double_t fit_const[shNCol*shNRow];
Double_t HVUpdate[shNCol*shNRow];
Double_t HV_Crate[shNCol*shNRow];
Double_t HV_Slot[shNCol*shNRow];
Double_t HV_Chan[shNCol*shNRow];
//

Double_t fitfunct(Double_t *x,Double_t *par) {
  Float_t xx=x[0];
  Double_t f = par[0] + par[1]*TMath::Log(xx);
  return f;
}
//
void WriteHV() {
  TString OutFile;
  if(!trigtoFADC_ratio){
    OutFile = Form("hv_set/hv_updated_sh_%d_%d_%0.1fmV.set",RunList[0],RunList[RunList.size()-1],desTrigamp);
  }else{
    OutFile = Form("hv_set/hv_updated_sh_%d_%d_%0.1fmV_trig.set",RunList[0],RunList[RunList.size()-1],desTrigamp);
  }
  ofstream outfile_hv;
  cout << " Write update HV to : " << OutFile << endl;
  outfile_hv.open(OutFile);
  TString CrateName[2] = {"rpi17:2001","rpi18:2001"};
  for (Int_t nc=0;nc<2;nc++) {
    for (Int_t ns=0;ns<16;ns++) {
      outfile_hv << CrateName[nc] << " S" << ns << " " << "DV" ;
      for (Int_t nch=0;nch<12;nch++) {
	if (HV_Block[nc][ns][nch] == -1 ) {
	  outfile_hv << " " <<  HV_Value[nc][ns][nch] ;
	} else {
	  Int_t blk=HV_Block[nc][ns][nch];
	  outfile_hv << " " <<  HVUpdate[blk] ;
	}
      }
      outfile_hv << endl;
    }}
  outfile_hv.close();     
}
//

void FitRuns(Double_t Set_Peak=200.) {
  // bool trigtoFADC_ratio = false;
  if ( Set_Peak==-1 ){
    // Double_t desTrigamp = 0.;
    cout << " Enter desired trigger amplitude (mV) " << endl;
    cin >> desTrigamp;
    GenDesiredFADCamps(desTrigamp);
    trigtoFADC_ratio = true;
  }else if (Set_Peak >0 && Set_Peak < 500) Peak_Desired=Set_Peak;
  
  Int_t MC_index[shNCol] = {1,2,3,4,6,7,8};
  Int_t MS_index[shNCol] = {21,22,23,24,25,26,27};
  TH2F* alpha_xy = new TH2F("alpha_xy"," alpha ; Ncol ; NRow",7,1,8,27,1,28);
  TString outpdffile=Form("plots/hv_fit_%d_%d.pdf",RunList[0],RunList[RunList.size()-1]);
  TF1 *f1[shNCol*shNRow];
  for (Int_t nr=0;nr<shNRow;nr++) {
    TCanvas *canHV ;
    TMultiGraph *mg1 = new TMultiGraph();
    TMultiGraph *mgfit = new TMultiGraph();
    TLegend *leg;
    TGraph *grHVPeakFit[shNCol];
    TGraphErrors *grHVPeak[shNCol];
    TGraphErrors *grHVPeakLog[shNCol];
    TLine *fitline[shNCol];
    cout << " Row : " << nr+1 << endl;
    canHV = new TCanvas(Form("row_%d",nr+1),Form("row_%d",nr+1),700,700);
    leg = new TLegend(.1,.75,.6,.95);
    Double_t MaxHV=0;
    Double_t MaxPeak=0;
    Double_t MinHV=10000;
    Double_t MinPeak=10000;
    for (Int_t nc=0;nc<shNCol;nc++) {
      Double_t MaxHVBlk=0;
      Double_t MinHVBlk=10000;
      f1[nr*shNCol+nc] = new TF1(Form("f1_%d_%d",nr,nc),fitfunct,0,2000,2);
      vector<double> vecHV;
      vector<double> vecHVErr;
      vector<double> vecPeak;
      vector<double> vecPeakLog;
      vector<double> vecPeakErr;
      vector<double> vecPeakErrLog;
      vector<double> vecPeakCalc;
      cout << " Col : " << nc ;
      for  (UInt_t nru=0;nru<RunList.size();nru++) {
	if (abs(HVList[nru][nr*shNCol+nc]) > MaxHVBlk) MaxHVBlk= abs(HVList[nru][nr*shNCol+nc]);
	if (abs(HVList[nru][nr*shNCol+nc]) < MinHVBlk) MinHVBlk= abs(HVList[nru][nr*shNCol+nc]);
	if (PeakList[nru][nr*shNCol+nc] > MaxPeak) MaxPeak= PeakList[nru][nr*shNCol+nc];
	if (PeakList[nru][nr*shNCol+nc] < MinPeak) MinPeak= PeakList[nru][nr*shNCol+nc];
	vecHV.push_back(abs(HVList[nru][nr*shNCol+nc]));
	vecHVErr.push_back(0.001);
	vecPeak.push_back(PeakList[nru][nr*shNCol+nc]);
	vecPeakLog.push_back(TMath::Log(PeakList[nru][nr*shNCol+nc]));
	vecPeakErr.push_back(PeakErrList[nru][nr*shNCol+nc]);
	vecPeakErrLog.push_back(PeakErrList[nru][nr*shNCol+nc]/PeakList[nru][nr*shNCol+nc]);
	cout << " HV peak : " << HVList[nru][nr*shNCol+nc] << " " << PeakList[nru][nr*shNCol+nc] ;	   
      }
      grHVPeak[nc] = new TGraphErrors(vecHV.size(),&vecHV[0],&vecPeak[0],&vecHVErr[0],&vecPeakErr[0]);
      grHVPeakLog[nc] = new TGraphErrors(vecHV.size(),&vecHV[0],&vecPeakLog[0],&vecHVErr[0],&vecPeakErrLog[0]);
      grHVPeakLog[nc]->Fit(Form("f1_%d_%d",nr,nc),"Q");
      fit_chi2[nr*shNCol+nc]=f1[nr*shNCol+nc]->GetChisquare();
      fit_alpha[nr*shNCol+nc]=f1[nr*shNCol+nc]->GetParameter(1);
      alpha_xy->Fill(float(nc+1),float(nr+1),fit_alpha[nr*shNCol+nc]);
      fit_alpha_err[nr*shNCol+nc]=f1[nr*shNCol+nc]->GetParError(1);
      fit_const[nr*shNCol+nc]=f1[nr*shNCol+nc]->GetParameter(0);
      grHVPeak[nc]->SetMarkerColor(MC_index[nc]);
      grHVPeak[nc]->SetMarkerStyle(MS_index[nc]);
      mg1->Add(grHVPeak[nc],"P");
      
      if(!trigtoFADC_ratio){
	HVUpdate[nr*shNCol+nc]=-TMath::Exp((TMath::Log(Peak_Desired) - fit_const[nr*shNCol+nc])/fit_alpha[nr*shNCol+nc]);
      }else{
	Double_t temp = DesiredFADCamps.at(nr*shNCol+nc);
	HVUpdate[nr*shNCol+nc]=-TMath::Exp((TMath::Log(temp) - fit_const[nr*shNCol+nc])/fit_alpha[nr*shNCol+nc]);
      }
      if (fit_alpha[nr*shNCol+nc]<1) {
	if(!trigtoFADC_ratio){
	  Double_t HV_temp=vecHV[0]*TMath::Power(Peak_Desired/vecPeak[0],1./6.);
	  HVUpdate[nr*shNCol+nc]=-HV_temp;
	}else{
	  Double_t temp = DesiredFADCamps.at(nr*shNCol+nc);
	  Double_t HV_temp=vecHV[0]*TMath::Power(temp/vecPeak[0],1./6.);
	  HVUpdate[nr*shNCol+nc]=-HV_temp;
	}
      }
      if (abs(HVUpdate[nr*shNCol+nc]) > 2000) HVUpdate[nr*shNCol+nc]=-2000.;
      leg->AddEntry(grHVPeak[nc],Form(" Col %d alpha = %5.2f HV= %6.1f #chi^{2}= %5.2f",nc+1,fit_alpha[nr*shNCol+nc],HVUpdate[nr*shNCol+nc],fit_chi2[nr*shNCol+nc] ));
      cout << " " <<fit_alpha[nr*shNCol+nc] << " " <<fit_const[nr*shNCol+nc] << " " <<HVUpdate[nr*shNCol+nc]  << endl;
      Double_t fit_lo = TMath::Exp(fit_const[nr*shNCol+nc] + fit_alpha[nr*shNCol+nc]*TMath::Log(MinHVBlk));
      Double_t fit_hi = TMath::Exp(fit_const[nr*shNCol+nc] + fit_alpha[nr*shNCol+nc]*TMath::Log(MaxHVBlk));
      fitline[nc] = new TLine(MinHVBlk,fit_lo,MaxHVBlk,fit_hi);
      fitline[nc]->SetLineColor(MC_index[nc]);
      if (MaxHVBlk > MaxHV) MaxHV=MaxHVBlk;
      if (MinHVBlk < MinHV) MinHV=MinHVBlk;
      sort(vecHV.begin(),vecHV.end());
      vector<double > vecFitPeak;
      for  (UInt_t nru=0;nru<vecHV.size();nru++) {
	Double_t fit_temp = TMath::Exp(fit_const[nr*shNCol+nc] + fit_alpha[nr*shNCol+nc]*TMath::Log(vecHV[nru]));
	vecFitPeak.push_back(fit_temp);
      }	  
      grHVPeakFit[nc] = new TGraph(vecHV.size(),&vecHV[0],&vecFitPeak[0]);
      mg1->Add(grHVPeakFit[nc],"L");
    }
    cout << " make plot: " << endl;
    canHV->Divide(1,1);
    canHV->cd(0);
    mg1->SetTitle(Form("Row %d",nr+1));
    mg1->GetXaxis()->SetRangeUser(MinHV-50,MaxHV+50);
    mg1->GetYaxis()->SetRangeUser(0,MaxPeak+20);
    mg1->GetXaxis()->SetTitle("HV ");
    mg1->GetYaxis()->SetTitle("Peak ");
    mg1->Draw("APL");
    for (Int_t nc=0;nc<shNCol;nc++) {	 
      //	  fitline[nc]->Draw("same");
    }
    leg->Draw();
    //gPad->SetLogy();
    //gPad->SetLogx();
    if (nr==0) canHV->Print(outpdffile+"(");
    if (nr!=0&&nr!=26) canHV->Print(outpdffile);
    if (nr==26) canHV->Print(outpdffile+")");
    canHV->WaitPrimitive();
    canHV->Close();
  }
  outpdffile=Form("plots/alpha_2d_%d_%d.pdf",RunList[0],RunList[RunList.size()-1]);
  gStyle->SetOptStat(0);
  gStyle->SetPalette(1,0);
  const Int_t Number=3;
  Double_t Red[Number] = { 1.0,0.0,0.0};
  Double_t Blue[Number] = { 1.0,0.0,1.0};
  Double_t Green[Number] = { 0.0,1.0,0.0};
  Double_t Len[Number] = { 0.0,.5,1.0};
  Int_t nb=50;
  TColor::CreateGradientColorTable(Number,Len,Red,Green,Blue,nb);
  TCanvas* can2d;
  TCanvas* canalpha;
  can2d= new TCanvas("can_2d","2d ",700,1000);
  can2d->Divide(1,1);
  can2d->cd(1);
  alpha_xy->SetMaximum(22);
  alpha_xy->SetMinimum(0);
  alpha_xy->Draw("text colz");
  gStyle->SetPaintTextFormat("4.2f");
  can2d->Print(outpdffile);
  can2d->WaitPrimitive();
  can2d->Close();
  TH1F *alpha_blocknumber = new TH1F("alpha_blocknumber","Alpha; Block #; alpha",shNCol*shNRow,1,shNCol*shNRow + 1);
  canalpha= new TCanvas("can_1d","1d ",700,700);//added by Arun
  canalpha->Divide(1,2);
  canalpha->cd(1);
  //alpha_blocknumber->SetBinContent(blknum,alpha_xy[nr][nc]);
  alpha_blocknumber->SetMinimum(0);
  alpha_blocknumber->SetMaximum(0);
  alpha_blocknumber->Draw();

  string OutFile = Form("Output/run_%d_%d_alpha.txt",RunList[0],RunList[RunList.size()-1]);
  cout << " Write alphas to : " << OutFile << endl;
  ofstream outfile_data;
  outfile_data.open(OutFile);
  for (Int_t nr=0;nr<shNRow;nr++) {
    for (Int_t nc=0;nc<shNCol;nc++) {
      //outfile_data << " Row = " << nr+1 << " Col = " << nc+1<< " " << fit_alpha[nr*shNCol+nc] << " +/- " << fit_alpha_err[nr*shNCol+nc]  << " New HV = " <<  HVUpdate[nr*shNCol+nc]<< endl;
      outfile_data << nr*shNCol+nc << " " << fit_alpha[nr*shNCol+nc] << " " << fit_alpha_err[nr*shNCol+nc] << endl;
    }
    ;
  }
  outfile_data.close();
}
//
void AddRun(Int_t nrun) {
  if (RunList.size()==0) SetHVMap();
  RunList.push_back(nrun);
  ReadHV(nrun);	
  double tempHV[shNCol*shNRow];
  for (Int_t nc=0;nc<2;nc++) {
    for (Int_t ns=0;ns<16;ns++) {
      for (Int_t nch=0;nch<12;nch++) {
	if (HV_Block[nc][ns][nch]!= -1) tempHV[HV_Block[nc][ns][nch]]=HV_Value[nc][ns][nch];
      }}}
  vector<double > vecHV;
  for (Int_t nr=0;nr<shNRow;nr++) {
    for (Int_t nc=0;nc<shNCol;nc++) {

      vecHV.push_back(tempHV[nr*shNCol+nc]);
    }}
  HVList.push_back(vecHV);
  //
  ReadPeak(nrun);	 
  //ReadHist(nrun);	 
}

void CompHistRuns() {
  gStyle->SetOptStat(0);
  for (Int_t nr=0;nr<shNRow;nr++) {
    TCanvas *canHV ;
    canHV= new TCanvas(Form("can_%d",nr),Form("Row %d",nr),900,700);    
    canHV->Divide(2,4);	  
    for (Int_t nc=0;nc<shNCol;nc++) {
      TLegend *leg = new TLegend(.5,.45,.9,.9);
      canHV->cd(nc+1);
      Double_t ymax=0;
      for  (UInt_t nru=0;nru<RunList.size();nru++) {
	Double_t rat=HistList[nru][nr*shNCol+nc]->GetMaximum();
	if (rat > ymax) ymax = rat;
      }
      for  (UInt_t nru=0;nru<RunList.size();nru++) {
	Int_t colind = nru+1;
	if (colind>=5) colind=colind+1;
	HistList[nru][nr*shNCol+nc]->SetLineColor(colind);
	HistList[nru][nr*shNCol+nc]->SetMaximum(ymax);
	if (nru==0) HistList[nru][nr*shNCol+nc]->DrawNormalized();
	if (nru!=0) HistList[nru][nr*shNCol+nc]->DrawNormalized("same");
	leg->AddEntry(HistList[nru][nr*shNCol+nc],Form("Run %d HV = %6.1f",RunList[nru],HVList[nru][nr*shNCol+nc]));
      }
      leg->Draw();
    } 
    canHV->WaitPrimitive();
    TString outpdffile=Form("plots/hv_peaks_%d_%d.pdf",RunList[0],RunList[RunList.size()-1]);
    if (nr==0) canHV->Print(outpdffile+"(");
    if (nr!=0&&nr!=26) canHV->Print(outpdffile);
    if (nr==26) canHV->Print(outpdffile+")");
    //canHV->WaitPrimitive();
    canHV->Close();
  }
}

void ReadHist(Int_t nrun) {
  TString inputroot;
  TFile *fhistroot;
  inputroot=Form("hist/bbshower_%d_shower_hist.root",nrun);
  fhistroot =  new TFile(inputroot);
  vector<TH1F*> h_shADC_pedsub_cut;
  for (Int_t nr=0;nr<shNRow;nr++) {
    for (Int_t nc=0;nc<shNCol;nc++) {
      TH1F* temphist ;
      temphist=(TH1F*)fhistroot->Get(Form("h_shADC_pedsub_row%d_col%d",nr+1,nc+1)) ;
      h_shADC_pedsub_cut.push_back(temphist);
    }}
  HistList.push_back(h_shADC_pedsub_cut);
    
}

void ReadPeak(Int_t nrun) {
  string InFile = Form("Output/run_%d_sh_peak_FADC.txt",nrun);
  ifstream infile_data;
  infile_data.open(InFile);
  string readline;
  if (infile_data.is_open() ) {
    vector<double > vecPeak;
    vector<double > vecPeakErr;
    cout << " add peak file : "<< InFile << endl;
    while (getline(infile_data,readline)) {
      istringstream tokenStream(readline);
      string token;
      char delimiter = ' ';
      Int_t test=1;
      while (getline(tokenStream,token,delimiter))  {
	TString temptoken=token;
	if ( test ==1) {
	  vecPeak.push_back(temptoken.Atof());
	  test=0;
	} else {
	  vecPeakErr.push_back(temptoken.Atof());
	  test=1;
	}
      }
    }
    PeakList.push_back(vecPeak);
    PeakErrList.push_back(vecPeakErr);
    infile_data.close();
  } else {
    cout << " No file : " << InFile << endl;
  }
}



void PrintRuns() {
  for (Int_t nr=0;nr<shNRow;nr++) {
    for (UInt_t nru=0;nru<RunList.size();nru++) {
      cout << RunList[nru] << " " << " Row : " << nr ;
      for (Int_t nc=0;nc<shNCol;nc++) {
	cout << " Col : " << nc<< " " << HVList[nru][nr*shNCol+nc] << " " << PeakList[nru][nr*shNCol+nc];
      }
      cout << endl;
    }
  }
}

void SetHVMap() {
  cout << " Setting HV map " << endl;
  for (Int_t nc=0;nc<2;nc++) {
    for (Int_t ns=0;ns<16;ns++) {
      for (Int_t nch=0;nch<12;nch++) {
	HV_Block[nc][ns][nch]=-1;
      }}}
  
  Int_t nslot=2;// HV slots = 0 to 15
  Int_t nchan=3;// HV channels = 0 to 11
  for (Int_t nr=0;nr<shNRow;nr++) {
    if (nr==12) nslot=5;
    if (nr==12) nchan=0;
    for (Int_t nc=0;nc<shNCol;nc++) {
      if (nr > 11) {
	HV_Block[1][nslot][nchan++] = nr*shNCol+nc;
 	HV_Slot[nr*shNCol+nc] =nslot ;
 	HV_Crate[nr*shNCol+nc] =1 ;
 	HV_Chan[nr*shNCol+nc] =nchan-1 ;
      } else {
	HV_Block[0][nslot][nchan++] = nr*shNCol+nc;
  	HV_Slot[nr*shNCol+nc] =nslot ;
 	HV_Crate[nr*shNCol+nc] =0 ;
 	HV_Chan[nr*shNCol+nc] =nchan-1 ;
      }
      if (nchan==12) {
	nchan=0;
	nslot++;
      }
    }}
}
//
void ReadHV(Int_t nrun) {
  TString HVfileName = Form("hv_set/run_%d_hv.set",nrun);
  cout << " read file = " << HVfileName << endl;
  ifstream file_hv(HVfileName.Data());
  string hvline;
  string crate;
  string slot;
  Int_t CrateNum;
  Int_t SlotNum;
  if (file_hv.is_open()) {
    while (getline(file_hv,hvline)) {
      if (hvline.compare(0,1,"#")!=0) {	     
	istringstream tokenStream(hvline);
	string token;
	char delimiter = ' ';
	vector<TString> tokens;
	while (getline(tokenStream,token,delimiter))  {
	  tokens.push_back(token);
	}
	CrateNum=-1;
	if (tokens[0] == "rpi17:2001") CrateNum=0;
	if (tokens[0] == "rpi18:2001") CrateNum=1;
	TString SlotNumStr(tokens[1](1,tokens[1].Sizeof()+1));
	SlotNum=SlotNumStr.Atoi();
	for (UInt_t  it=3;it<tokens.size();it++) {
	  HV_Value[CrateNum][SlotNum][it-3] = tokens[it].Atof();
	}
      }
    }
    file_hv.close();
  } else {
    cout << " could not open : " << HVfileName << endl;
  }
 
}

//
void UpdateHV(Int_t nrun, Double_t HVShift) {
  SetHVMap();
  string OutFile = Form("hv_set/run_%d_hv_newset.set",nrun);
  ReadHV(nrun);
  ofstream outfile_hv;
  outfile_hv.open(OutFile);
  TString CrateName[2] = {"rpi17:2001","rpi18:2001"};
  for (Int_t nc=0;nc<2;nc++) {
    for (Int_t ns=0;ns<16;ns++) {
      outfile_hv << CrateName[nc] << " S" << ns << " " << "DV" ;
      for (Int_t nch=0;nch<12;nch++) {
	if (HV_Block[nc][ns][nch] == -1 ) {
	  outfile_hv << " " <<  HV_Value[nc][ns][nch] ;
	} else {
	  outfile_hv << " " <<  HV_Value[nc][ns][nch]-HVShift ;
	}
      }
      outfile_hv << endl;
    }}
  outfile_hv.close();     
}

void GenDesiredFADCamps(Double_t desTrigamp){
  string InFile = "Coefficients/trigtoFADCcoef_SH.txt";
  ifstream infile_data;
  infile_data.open(InFile);
  TString currentline;
  if (infile_data.is_open() ) {
    cout << " Reading trigger to FADC ratios from " << InFile << endl;
    cout << " Generating desired FADC amps to align trigger amps to " << desTrigamp << " mV. " << endl;
    TString temp;
    while( currentline.ReadLine( infile_data ) ){
      TObjArray *tokens = currentline.Tokenize("\t");
      int ntokens = tokens->GetEntries();
      if( ntokens > 1 ){
	// temp = ( (TObjString*) (*tokens)[0] )->GetString();
	// elemID.push_back( temp.Atof() );
	temp = ( (TObjString*) (*tokens)[1] )->GetString();
	Double_t target_FADC_amp_pCh = desTrigamp/temp.Atof();
	DesiredFADCamps.push_back( target_FADC_amp_pCh );
      }
      delete tokens;
    }
    infile_data.close();
  } else {
    cerr << " No file : " << InFile << endl;
    throw;
  }
}

