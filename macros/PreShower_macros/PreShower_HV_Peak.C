/* 
   This script analyzes multiple cosmic runs simultaneously from 
   a HV scan & fits the signal peak position (at FADC) vs HV plot 
   using a polynomial to extract alpha values for individual PreShower 
   PMTs. NOTE: This script reads peak positions and HV settings for
   each cosmic run during execution. Example execution:
   ----
   [a-onl@aonl2 macros]$ pwd
   /adaqfs/home/a-onl/sbs/BBCal_replay/macros
   [a-onl@aonl2 macros]$ root -l 
   root [0] .L PreShower_macros/PreShower_HV_Peak.C+
   root [1] AddRun(<runnumber1>)
   root [2] AddRun(<runnumber2>)
   root [3] [.. add as many runs as you want]
   root [4] FitRuns(<peak_pos>)  #peak_pos=>desired FADC peak for all signal
   root [1] WriteHV()  #writes calibrated HV setting for peak_pos
   ----
   M. K. Jones  Created
   P. Datta     Edited

   *** This script uses a outdated PS channel map and also doesn't
   take into consideration the trigger to FADC ratios. The most up
   to date version is PreShower_HV_Peak_v2.C .
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
#include <TLatex.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include<math.h>
using namespace std;


  static const Int_t shNCol=2;
  static const Int_t shNRow=26;

Double_t Peak_Desired = 300.; // 

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

vector <Int_t> RunList;
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
  string OutFile = Form("hv_set/hv_updated_ps_%d_%d.set",RunList[0],RunList[RunList.size()-1]);
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
  if (Set_Peak >0 && Set_Peak < 500) Peak_Desired=Set_Peak;
  Int_t MC_index[shNCol] = {1,2};
  Int_t MS_index[shNCol] = {21,22};
  TH2F* alpha_xy = new TH2F("alpha_xy"," alpha ; Ncol ; NRow",2,1,3,27,1,28);
  TH2F* hv_xy = new TH2F("hv_xy",Form(" HV (peak at %5.1f)  ; Ncol ; NRow",Peak_Desired),2,1,3,27,1,28);
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
	 leg = new TLegend(.1,.75,.4,.95);
	 Double_t MaxHV=0;
	 Double_t MaxPeak=0;
	 Double_t MinHV=10000;
	 Double_t MinPeak=10000;
         for (Int_t nc=0;nc<shNCol;nc++) {
	 Double_t MaxHVBlk=0;
	 Double_t MinHVBlk=10000;
	 Int_t nelem=nr+nc*shNRow;
	   f1[nelem] = new TF1(Form("f1_%d_%d",nr,nc),fitfunct,0,2000,2);
	   vector<double> vecHV;
	   vector<double> vecHVErr;
	   vector<double> vecPeak;
	   vector<double> vecPeakLog;
	   vector<double> vecPeakErr;
	   vector<double> vecPeakErrLog;
	   vector<double> vecPeakCalc;
	   cout << " Col : " << nc ;
	   for  (UInt_t nru=0;nru<RunList.size();nru++) {
	     if (abs(HVList[nru][nelem]) > MaxHVBlk) MaxHVBlk= abs(HVList[nru][nelem]);
	     if (abs(HVList[nru][nelem]) < MinHVBlk) MinHVBlk= abs(HVList[nru][nelem]);
	     if (PeakList[nru][nelem] > MaxPeak) MaxPeak= PeakList[nru][nelem];
	     if (PeakList[nru][nelem] < MinPeak) MinPeak= PeakList[nru][nelem];
	     vecHV.push_back(abs(HVList[nru][nelem]));
	     vecHVErr.push_back(0.001);
	     vecPeak.push_back(PeakList[nru][nelem]);
	     vecPeakLog.push_back(TMath::Log(PeakList[nru][nelem]));
	     vecPeakErr.push_back(PeakErrList[nru][nelem]);
	     vecPeakErrLog.push_back(PeakErrList[nru][nelem]/PeakList[nru][nelem]);
	     cout << " HV peak : " << HVList[nru][nelem] << " " << PeakList[nru][nelem] ;	   
           }
	   grHVPeak[nc] = new TGraphErrors(vecHV.size(),&vecHV[0],&vecPeak[0],&vecHVErr[0],&vecPeakErr[0]);
	   grHVPeakLog[nc] = new TGraphErrors(vecHV.size(),&vecHV[0],&vecPeakLog[0],&vecHVErr[0],&vecPeakErrLog[0]);
	   grHVPeakLog[nc]->Fit(Form("f1_%d_%d",nr,nc),"Q");
	   fit_alpha[nelem]=f1[nelem]->GetParameter(1);
	   fit_alpha_err[nelem]=f1[nelem]->GetParError(1);
	   alpha_xy->Fill(float(nc+1),float(nr+1),fit_alpha[nelem]);
	   fit_const[nelem]=f1[nelem]->GetParameter(0);
	   grHVPeak[nc]->SetMarkerColor(MC_index[nc]);
	   grHVPeak[nc]->SetMarkerStyle(MS_index[nc]);
	   mg1->Add(grHVPeak[nc],"P");
	   HVUpdate[nelem]=-TMath::Exp((TMath::Log(Peak_Desired) - fit_const[nelem])/fit_alpha[nelem]);
	   if (fit_alpha[nelem]<1) {
	     Double_t HV_temp=vecHV[0]*TMath::Power(Peak_Desired/vecPeak[0],1./6.);
                HVUpdate[nelem]=-HV_temp;
	   }
	   hv_xy->Fill(float(nc+1),float(nr+1),HVUpdate[nelem]);
	   leg->AddEntry(grHVPeak[nc],Form(" Col %d alpha = %5.2f HV= %6.1f",nc,fit_alpha[nelem],HVUpdate[nelem] ));
	   cout << " " <<fit_alpha[nelem] << " " <<fit_const[nelem] << " " <<HVUpdate[nelem]  << endl;
	   Double_t fit_lo = TMath::Exp(fit_const[nelem] + fit_alpha[nelem]*TMath::Log(MinHVBlk));
	   Double_t fit_hi = TMath::Exp(fit_const[nelem] + fit_alpha[nelem]*TMath::Log(MaxHVBlk));
	  fitline[nc] = new TLine(MinHVBlk,fit_lo,MaxHVBlk,fit_hi);
          fitline[nc]->SetLineColor(MC_index[nc]);
	  if (MaxHVBlk > MaxHV) MaxHV=MaxHVBlk;
	  if (MinHVBlk < MinHV) MinHV=MinHVBlk;
	  sort(vecHV.begin(),vecHV.end());
	  vector<double > vecFitPeak;
	   for  (UInt_t nru=0;nru<vecHV.size();nru++) {
	   Double_t fit_temp = TMath::Exp(fit_const[nelem] + fit_alpha[nelem]*TMath::Log(vecHV[nru]));
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
	 mg1->GetYaxis()->SetRangeUser(MinPeak-50,MaxPeak+50);
	 mg1->GetXaxis()->SetTitle("HV ");
	 mg1->GetYaxis()->SetTitle("Peak ");
	 mg1->Draw("APL");
         for (Int_t nc=0;nc<shNCol;nc++) {	 
	   //	  fitline[nc]->Draw("same");
	 }
	 leg->Draw();
      	 //gPad->SetLogy();
	 //gPad->SetLogx();
	 TString outpdffile=Form("plots/hv_fit_ps_%d_%d.pdf",RunList[0],RunList[RunList.size()-1]);
	 if (nr==0) canHV->Print(outpdffile+"(");
	 if (nr!=0&&nr!=25) canHV->Print(outpdffile);
	 if (nr==25) canHV->Print(outpdffile+")");
	   				 canHV->WaitPrimitive();
	 canHV->Close();
    }
	 TString outpdffile=Form("plots/hv_alpha_ps_%d_%d.pdf",RunList[0],RunList[RunList.size()-1]);
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
  can2d= new TCanvas("can_2d","2d ",700,700);
  can2d->Divide(1,1);
  can2d->cd(1);
  gStyle->SetPaintTextFormat("4.2f");
  alpha_xy->SetMaximum(15);
  alpha_xy->SetMinimum(0);
  alpha_xy->Draw("text colz");
  can2d->WaitPrimitive();
  can2d->Print(outpdffile+"(");
  can2d->Close();
      TCanvas* canhv;
  canhv= new TCanvas("canhv","hv ",700,700);
  canhv->Divide(1,1);
  canhv->cd(1);
  gStyle->SetPaintTextFormat("6.1f");
  hv_xy->Draw("text colz");
  canhv->WaitPrimitive();
  canhv->Print(outpdffile+")");
  canhv->Close();
  //
  string OutFile = Form("Output/run_%d_%d_alpha_ps.txt",RunList[0],RunList[RunList.size()-1]);
  cout << " Write alphas to : " << OutFile;
   ofstream outfile_data;
  outfile_data.open(OutFile);
    for (Int_t nr=0;nr<shNRow;nr++) {
       outfile_data << " Row " << nr+1 << " : ";
        for (Int_t nc=0;nc<shNCol;nc++) {
	  outfile_data << " " << fit_alpha[nr*shNCol+nc] << " +/- " << fit_alpha_err[nr*shNCol+nc] ;
	}
	outfile_data << endl;
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
    for (Int_t nc=0;nc<shNCol;nc++) {
    for (Int_t nr=0;nr<shNRow;nr++) {
	 Int_t nelem=nr+nc*shNRow;
	 //	 cout << " nelem nr nc HV = " << nelem << " " << nr << " " << nc << " " << tempHV[nelem] << endl;
       vecHV.push_back(tempHV[nelem]);
     }}
    HVList.push_back(vecHV);
    //
   ReadPeak(nrun);	 
   ReadHist(nrun);	 
}

void CompHistRuns() {
  gStyle->SetOptStat(0);
    for (Int_t nr=0;nr<shNRow;nr++) {
         TCanvas *canHV ;
	   canHV= new TCanvas(Form("can_%d",nr),Form("Row %d",nr),1200,900);    
	   canHV->Divide(1,2);	  
         for (Int_t nc=0;nc<shNCol;nc++) {
	   TLegend *leg = new TLegend(.5,.45,.9,.9);
           	   canHV->cd(nc+1);
	 Int_t nelem=nr+nc*shNRow;
		   Double_t ymax=0;
	   for  (UInt_t nru=0;nru<RunList.size();nru++) {
	     Double_t rat=HistList[nru][nelem]->GetMaximum();
	     if (rat > ymax) ymax = rat;
	   }
	   for  (UInt_t nru=0;nru<RunList.size();nru++) {
	     Int_t colind = nru+1;
	     if (colind>=5) colind=colind+1;
	     if (HistList[nru][nelem]) {
	     HistList[nru][nelem]->SetLineColor(colind);
	     HistList[nru][nelem]->SetMaximum(ymax*1.2);
	     if (nru==0) HistList[nru][nelem]->DrawNormalized();
	     if (nru!=0) HistList[nru][nelem]->DrawNormalized("same");
	     leg->AddEntry(HistList[nru][nelem],Form("Run %d HV = %6.1f",RunList[nru],HVList[nru][nelem]));
	     }
	   }
	     leg->Draw();
	 }
	 canHV->WaitPrimitive();
	 TString outpdffile=Form("plots/hv_peaks_ps_%d_%d.pdf",RunList[0],RunList[RunList.size()-1]);
	 if (nr==0) canHV->Print(outpdffile+"(");
	 if (nr!=0&&nr!=25) canHV->Print(outpdffile);
	 if (nr==25) canHV->Print(outpdffile+")");
					 //canHV->WaitPrimitive();
	 canHV->Close();
    }
}

void ReadHist(Int_t nrun) {
  TString inputroot;
  TFile *fhistroot;
  inputroot=Form("hist/bbshower_%d_preshower_hist.root",nrun);
  cout << " Read hist file " << inputroot << endl;
  fhistroot =  new TFile(inputroot);
  vector<TH1F*> h_shADC_pedsub_cut;
    for (Int_t nc=0;nc<shNCol;nc++) {
    for (Int_t nr=0;nr<shNRow;nr++) {
      TH1F* temphist ;
      if (nc==0) temphist=(TH1F*)fhistroot->Get(Form("h_psADC_pedsub_cut_L%d",nr+1)) ;
      if (nc==1) temphist=(TH1F*)fhistroot->Get(Form("h_psADC_pedsub_cut_R%d",nr+1)) ;
      h_shADC_pedsub_cut.push_back(temphist);
     }}
    HistList.push_back(h_shADC_pedsub_cut);
    
}

void ReadPeak(Int_t nrun) {
   string InFile = Form("Output/run_%d_ps_peak.txt",nrun);
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
	 Int_t nelem=nr+nc*shNRow;
	    cout  << " Col : " << nc<< " " << nelem<< " " << HVList[nru][nelem] << " " << PeakList[nru][nelem];
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
  
  Int_t nslot=13;// HV slots = 0 to 15
  Int_t nchan=9;// HV channels = 0 to 11
  for (Int_t nc=0;nc<shNCol;nc++) {
      if (nc==1) nslot=0;
      if (nc==1) nchan=0;
    for (Int_t nr=0;nr<shNRow;nr++) {
      if (nc ==1) {
	HV_Block[0][nslot][nchan++] = nr+shNRow; // Right row in RPI17
 	HV_Slot[nr+shNRow] =nslot ;
 	HV_Crate[nr+shNRow] =0 ;
 	HV_Chan[nr+shNRow] =nchan-1;
     } else {
	HV_Block[1][nslot][nchan++] = nr; // Left Row in RPI18
  	HV_Slot[nr] =nslot ;
 	HV_Crate[nr] =1 ;
 	HV_Chan[nr] =nchan-1;
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
	     //	     cout << " HV Cratenum, slot, chan = " << CrateNum << " " << SlotNum ;
	     for (UInt_t  it=3;it<tokens.size();it++) {
	       HV_Value[CrateNum][SlotNum][it-3] = tokens[it].Atof();
	       //	       cout  << " " << it-3 << " " << HV_Value[CrateNum][SlotNum][it-3]; 
	     }
	     //	     cout << endl;
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
  string OutFile = Form("hv_set/run_%d_hv_newset.txt",nrun);
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



