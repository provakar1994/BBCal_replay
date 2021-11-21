/*
  This macro combines HV files generated for Shower and PreShower.
  ----
  P. Datta  <pdbforce@jlab.org>  Created  15 Sep 2021
*/
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <TH2F.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TString.h>
#include <TStyle.h>
#include <TGraph.h>
#include <TError.h>
#include <TObjArray.h>
#include <TObjString.h>
using namespace std;

Double_t SHHV_Value[2][16][12];
Double_t PSHV_Value[2][16][12];

void ReadSHHV();
void ReadPSHV();
void Combine_HV();
string getDate();

TString SH_HV, PS_HV, OutFile, OutPlots;

TH2F *h2_rpi17_cv = new TH2F("h2_rpi17_cv","rpi17; Slot; Channel",
			     16,0,16,12,-12,0);
TH2F *h2_rpi18_cv = new TH2F("h2_rpi18_cv","rpi18; Slot; Channel",
			     16,0,16,12,-12,0);

void ReadSHHV() {
  TString SHHVfileName = SH_HV;
  // cout << "Shower HV file name?" << endl;
  // cin >> SHHVfileName;

  TString HVfileName = Form("hv_set/%s",SHHVfileName.Data());
  ifstream file_hv(HVfileName.Data());
  string hvline;
  string crate;
  string slot;
  Int_t CrateNum;
  Int_t SlotNum;
  if (file_hv.is_open()) {
    cout << " Read Shower HV file = " << HVfileName << endl;
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
	  SHHV_Value[CrateNum][SlotNum][it-3] = tokens[it].Atof();
	}
      }
    }
    file_hv.close();
  } else {
    cerr << " Could not open : " << HVfileName << endl;
    throw;
  } 
}//

void ReadPSHV() {
  TString PSHVfileName = PS_HV;
  // cout << "PreShower HV file name?" << endl;
  // cin >> PSHVfileName;

  TString HVfileName = Form("hv_set/%s",PSHVfileName.Data());
  ifstream file_hv(HVfileName.Data());
  string hvline;
  string crate;
  string slot;
  Int_t CrateNum;
  Int_t SlotNum;
  if (file_hv.is_open()) {
    cout << " Read PreShower HV file = " << HVfileName << endl;
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
	  PSHV_Value[CrateNum][SlotNum][it-3] = tokens[it].Atof();
	}
      }
    }
    file_hv.close();
  } else {
    cerr << " Could not open : " << HVfileName << endl;
    throw;
  } 
}//

void Combine_HV( int userInput=1, 
		 TString sh_hv="sh_hv.set",
		 TString ps_hv="ps_hv.set",
		 TString output_hv="output_hv") {

  if(userInput){
    cout << " Shower HV file name? " << endl;
    cin >> sh_hv;
    SH_HV = sh_hv;

    cout << " PreShower HV file name? " << endl;
    cin >> ps_hv;
    PS_HV = ps_hv;

    cout << " Preferred output file name? [No extension] " << endl;
    cin >> output_hv;
    OutFile = output_hv;
  }else{
    SH_HV = sh_hv;
    PS_HV = ps_hv;
  }

  ReadSHHV();
  ReadPSHV();

  // TString OutFile = "hv_combined.set", outfn, OutPlots;
  // cout << "Preferred output file name?" << endl;
  // cin >> outfn; 
  // OutFile = Form("hv_set/%s.set",outfn.Data());
  // OutPlots = Form("plots/%s.pdf",outfn.Data());

  string date = getDate();
  OutFile = Form("hv_set/%s_%s.set",output_hv.Data(),date.c_str());
  OutPlots = Form("plots/%s.pdf",output_hv.Data());

  ofstream outfile_hv;
  outfile_hv.open(OutFile);
  TString CrateName[2] = {"rpi17:2001","rpi18:2001"};
  for (Int_t nc=0;nc<2;nc++) {
    for (Int_t ns=0;ns<16;ns++) {
      if( (nc==0&&ns<10) || (nc==1&&ns>4) ){

	outfile_hv << CrateName[nc] << " S" << ns << " " << "DV" ;

	for (Int_t nch=0;nch<12;nch++) {

	  if( (nc==0&&ns<2) || (nc==0&&ns==2&&nch<3) 
	      || (nc==1&&ns>13) || (nc==1&&ns==13&&nch>8) ){ //All PS channels

	    double psHV = PSHV_Value[nc][ns][nch];

	    outfile_hv << " " <<  psHV;
	    if(nc==0)h2_rpi17_cv->Fill(ns,-nch-1,psHV);
	    if(nc==1)h2_rpi18_cv->Fill(ns,-nch-1,psHV);

	  }else{
	    double shHV = SHHV_Value[nc][ns][nch];

	    outfile_hv << " " <<  shHV;
	    if(nc==0)h2_rpi17_cv->Fill(ns,-nch-1,shHV);
	    if(nc==1)h2_rpi18_cv->Fill(ns,-nch-1,shHV);
	  }
	}
	outfile_hv << endl;
      }
    }
  }
  outfile_hv.close();  

  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings 

  TCanvas *c1 = new TCanvas("c1","c1",1200,800);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("4.2f");
  c1->Divide(1,2);
  c1->cd(1);
  h2_rpi17_cv->SetMarkerSize(1.8);
  h2_rpi17_cv->Draw("text col");
  c1->cd(2);
  h2_rpi18_cv->SetMarkerSize(1.8);
  h2_rpi18_cv->Draw("text col");

  c1->SaveAs(Form("%s[",OutPlots.Data()));
  c1->SaveAs(Form("%s",OutPlots.Data()));
  c1->SaveAs(Form("%s]",OutPlots.Data()));

  cout << "-------" << endl;
  cout << " Combined HV written to : " << OutFile << endl; 
  cout << " Plots saved to : " << OutPlots << endl; 
  cout << "-------" << endl;
}

string getDate(){
  time_t now = time(0);
  tm ltm = *localtime(&now);

  string yyyy = to_string(1900 + ltm.tm_year);
  string mm = to_string(1 + ltm.tm_mon);
  string dd = to_string(ltm.tm_mday);
  string date = mm + '_' + dd + '_' + yyyy;

  return date;
} 
