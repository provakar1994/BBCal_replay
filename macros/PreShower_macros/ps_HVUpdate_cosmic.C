/*
  This macro generates calibrated HV for BB PreShower.
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

static const Int_t psNCol=2;
static const Int_t psNRow=26;

Double_t HV_Value[2][16][12];
Int_t HV_Block[2][16][12];
Int_t frun=0, lrun=0;
bool multiRun=0;

void SetHVMap();
void ReadHV(Int_t nrun);
void UpdateHV();
void ReadAlpha();
void ReadPeak(Int_t nrun);
string getDate();

vector<double> gAlpha;
vector<double> sigPeakTrig; 
vector<double> sigPeakErr;

Double_t HVUpdate[psNCol*psNRow];
Double_t HV_Crate[psNCol*psNRow];
Double_t HV_Slot[psNCol*psNRow];
Double_t HV_Chan[psNCol*psNRow];

TH2F* h_hv_xy_new = new TH2F("hv_xy_new","; PS col; PS row",
			     2,1,3,26,1,27);
TH2F* h_hv_xy_old = new TH2F("hv_xy_old","HV Old (V); PS col; PS row",
			     2,1,3,26,1,27);
TH2F* h_hv_xy_shift = new TH2F("hv_xy_shift","Absolute Shift(V)" 
			       " [fabs(HVOld-HVNew)]; PS col; PS row",
			       2,1,3,26,1,27);

void ps_HVUpdate_cosmic(Int_t nrun=0, Double_t des_TrigAmp=15., 
			bool userInput=0) {
  if(userInput){
    multiRun = 1;
    cout << " Enter 1st run number that was analyzed " << endl;
    cin >> frun;
    cout << " Now, enter Last run number that was analyzed " << endl;
    cin >> lrun;
  }

  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings 

  SetHVMap();  
  TCanvas *c1 = new TCanvas("c1","c1",1200,800);
  c1->Divide(2,1);
  TCanvas *c2 = new TCanvas("c2","c2",1200,800);
  
  TString date = getDate();
  TString OutFile, OutPlots;
  if(!multiRun){
    OutFile = Form("hv_set/ps_hv_calib_run_%d_%dmV.set",
		   nrun,(int)des_TrigAmp);
    OutPlots = Form("plots/ps_hv_calib_run_%d_%dmV.pdf",
		    nrun,(int)des_TrigAmp);
  }else{
    OutFile = Form("hv_set/ps_hv_calib_run_%d_%d_%dmV.set",
		   frun,lrun,(int)des_TrigAmp);
    OutPlots = Form("plots/ps_hv_calib_run_%d_%d_%dmV.pdf",
		    frun,lrun,(int)des_TrigAmp);
  }

  cout << "---" << endl;
  ReadHV(nrun);
  ReadAlpha();
  ReadPeak(nrun);
  cout << "---" << endl;
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
	  int blk = HV_Block[nc][ns][nch];
	  int row = blk/psNCol;
	  int col = blk - psNCol*row;

	  double alphaINV = 1./gAlpha.at(blk);
	  double ampRatio = des_TrigAmp/sigPeakTrig.at(blk);
	  double HV_old = HV_Value[nc][ns][nch];
	  double HV_new = HV_Value[nc][ns][nch]*pow(ampRatio,
						    alphaINV);
	  if(fabs(HV_new)>1800 || (HV_new != HV_new)){
	    cout << " *!* New HV for PS Ch. " << row+1 << "-" 
		 << col+1 << " seems too high! " << endl;
	    outfile_hv << " " << HV_old;
	  }else outfile_hv << " " << HV_new;

	  /* -- Filling Diagnostic Histograms -- */
	  h_hv_xy_new->Fill(col+1,row+1,HV_new);
	  h_hv_xy_old->Fill(col+1,row+1,HV_old);
	  h_hv_xy_shift->Fill(col+1,row+1,fabs(HV_old-HV_new));
	}
      }
      outfile_hv << endl;
    }
  }
  outfile_hv.close();     

  /* -- Plotting the histograms -- */
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("4.2f");
  c1->cd(1);
  h_hv_xy_new->SetTitle(Form("HV New (V) [%0.1f mV Trig. Amp.]",des_TrigAmp));
  h_hv_xy_new->Draw("text col");
  h_hv_xy_new->SetMarkerSize(1.6);
  h_hv_xy_new->GetZaxis()->SetRangeUser(-1800.,-900.);
  c1->cd(2);
  h_hv_xy_old->Draw("text col");
  h_hv_xy_old->SetMarkerSize(1.6);
  h_hv_xy_old->GetZaxis()->SetRangeUser(-1800.,-900.);
  c2->cd();
  h_hv_xy_shift->Draw("text col");
  
  c1->SaveAs(Form("%s[",OutPlots.Data()));
  c1->SaveAs(Form("%s",OutPlots.Data()));
  c2->SaveAs(Form("%s",OutPlots.Data()));
  c2->SaveAs(Form("%s]",OutPlots.Data()));

  cout << "-------" << endl;
  cout << " Calibrated HV written to : " << OutFile << endl;
  cout << " Plots saved to : " << OutPlots << endl;  
  cout << "-------" << endl;  
}//

void SetHVMap() {
  //cout << " Setting HV map " << endl;
  for (Int_t nc=0;nc<2;nc++) {
    for (Int_t ns=0;ns<16;ns++) {
      for (Int_t nch=0;nch<12;nch++) {
	HV_Block[nc][ns][nch]=-1;
      } 
    }
  }
  
  Int_t nslot=0;// HV slots = 0 to 15
  Int_t nchan=0;// HV channels = 0 to 11
  for (Int_t nc=0;nc<psNCol;nc++) {
    if (nc==1) nslot=13;
    if (nc==1) nchan=9;
    for (Int_t nr=0;nr<psNRow;nr++) {
      if (nc ==1) {
	HV_Block[1][nslot][nchan++] = nr*psNCol+nc; // nr+psNRow; // Left Row in RPI18
  	HV_Slot[nr*psNCol+nc] =nslot ;
 	HV_Crate[nr*psNCol+nc] =1 ;
 	HV_Chan[nr*psNCol+nc] =nchan-1;
      } else {
	HV_Block[0][nslot][nchan++] = nr*psNCol+nc; // Right row in RPI17
 	HV_Slot[nr*psNCol+nc] =nslot ;
 	HV_Crate[nr*psNCol+nc] =0 ;
 	HV_Chan[nr*psNCol+nc] =nchan-1;
      }
      if (nchan==12) {
	nchan=0;
	nslot++;
      }
    }//nrow
  } // ncol
}
//

void ReadHV(Int_t nrun) {
  TString HVfileName = Form("hv_set/run_%d_hv.set",nrun);
  ifstream file_hv(HVfileName.Data());
  string hvline;
  string crate;
  string slot;
  Int_t CrateNum;
  Int_t SlotNum;
  if (file_hv.is_open()) {
    cout << " Reading HV : " << HVfileName << endl;
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
    cerr << endl << " No file : " << HVfileName 
	 << " <--- ** Attention **" << endl << endl;
    throw;
  }
}//

void ReadAlpha(){
  TString InFile = "../golden/preshower/golden_ps_alpha"
    "_11845_11888_10_30mV.txt";
  ifstream infile_data;
  infile_data.open(InFile);
  TString currentline;
  if (infile_data.is_open() ) {
    cout << " Reading alphas for PS : " << InFile << endl;
    TString temp;
    while( currentline.ReadLine( infile_data ) ){
      TObjArray *tokens = currentline.Tokenize(" ");
      int ntokens = tokens->GetEntries();
      if( ntokens > 1 ){
	// temp = ( (TObjString*) (*tokens)[0] )->GetString();
	// elemID.push_back( temp.Atof() );
	temp = ( (TObjString*) (*tokens)[1] )->GetString();
	Double_t alpha = temp.Atof();
	gAlpha.push_back( alpha );
      }
      delete tokens;
    }
    infile_data.close();
  } else {
    cerr << endl << " No file : " << InFile 
	 << " <--- ** Attention **" << endl << endl;
    throw;
  }
}//

void ReadPeak(Int_t nrun) {
  string InFile;
  if(!multiRun){
    InFile = Form("Output/run_%d_ps_peak_Trigger.txt",nrun);
  }else InFile = Form("Output/run_%d_%d_ps_peak_Trigger.txt",frun,lrun);
  ifstream infile_data;
  infile_data.open(InFile);
  string readline;
  int counter=0;
  if (infile_data.is_open() ) {
    cout << " Reading Trigger Peaks : "<< InFile << endl;
    while (getline(infile_data,readline)) {
      istringstream tokenStream(readline);
      string token;
      char delimiter = ' ';
      Int_t test=1;
      while (getline(tokenStream,token,delimiter))  {
	TString temptoken=token;
	if ( test ==1) {
	  sigPeakTrig.push_back(temptoken.Atof());
	  test=0;
	} else {
	  sigPeakErr.push_back(temptoken.Atof());
	  test=1;
	}
	counter++;
      }
    }
    if(counter!=2*psNCol*psNRow){
      cerr << "--!--" << endl << InFile 
	   << " is BROKEN!!! Please check and try again. " << endl 
	   << "--!--" << endl;
      throw;
    }
    infile_data.close();
  } else {
    cerr << endl << " No file : " << InFile 
	 << " <--- ** Attention **" << endl << endl;
    throw;
  }
}//

string getDate(){
  time_t now = time(0);
  tm ltm = *localtime(&now);

  string yyyy = to_string(1900 + ltm.tm_year);
  string mm = to_string(1 + ltm.tm_mon);
  string dd = to_string(ltm.tm_mday);
  string date = mm + '_' + dd + '_' + yyyy;

  return date;
} 
