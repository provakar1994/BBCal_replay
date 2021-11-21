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
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include<math.h>
using namespace std;

Double_t HV_Value[2][16][12];

void ReadHV(Int_t nrun);
void Shift_HV();

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
void Shift_HV(Int_t nrun, Double_t HVShift) {
  string OutFile;
  if(HVShift>0){
    OutFile = Form("hv_set/run_%d_hv_plus%0.1fmV.set",nrun,HVShift);
  }else{
    OutFile = Form("hv_set/run_%d_hv_minus%0.1fmV.set",nrun,abs(HVShift));
  }
  ReadHV(nrun);
  ofstream outfile_hv;
  outfile_hv.open(OutFile);
  TString CrateName[2] = {"rpi17:2001","rpi18:2001"};
  for (Int_t nc=0;nc<2;nc++) {
    for (Int_t ns=0;ns<16;ns++) {
      if( (nc==0&&ns<10) || (nc==1&&ns>4) ){
	outfile_hv << CrateName[nc] << " S" << ns << " " << "DV" ;
	for (Int_t nch=0;nch<12;nch++) {
	  if( (nc==0&&ns==9&&nch>2) ){
	    outfile_hv << " " <<  HV_Value[nc][ns][nch];
	  }else{
	    outfile_hv << " " <<  HV_Value[nc][ns][nch]-HVShift;
	  }
	}
	outfile_hv << endl;
      }
    }}
  outfile_hv.close();  
  cout << " Updated HV written to : " << OutFile << endl;   
}



