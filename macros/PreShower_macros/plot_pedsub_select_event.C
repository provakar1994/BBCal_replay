/* 
   M. K. Jones  Created
*/

#include <TString.h>
#include <TLegend.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TCutG.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TMath.h>
#include <TProfile.h>
#include <TObjArray.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include<math.h>
using namespace std;

void plot_pedsub_select_event(TString basename,Int_t nrun) {
  gROOT->Reset();
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(11);
  gStyle->SetTitleOffset(1.,"Y");
  gStyle->SetTitleOffset(.7,"X");
  gStyle->SetLabelSize(0.04,"XY");
  gStyle->SetTitleSize(0.06,"XY");
  gStyle->SetPadLeftMargin(0.14);
  TString outputpdf;
  outputpdf = "plots/"+basename+"_ps_select_event.pdf";
  TString inputroot;
  TFile *fhistroot;
  inputroot="hist/"+basename+"_preshower_hist.root";
  fhistroot =  new TFile(inputroot);
  //
  static const Int_t psNCol=2;
  static const Int_t psNRow=26;
  static const string side[2] = {"L", "R"};
 TH1F* peak_block = new TH1F("peak_block","  ; Block ; Peak position  ",psNCol*psNRow,0,psNCol*psNRow);
   TH2F* nhits_xy = new TH2F("nhits_xy"," Integral; Ncol ; NRow",2,1,3,27,1,28);
  TH2F* mean_xy = new TH2F("mean_xy"," Mean ; Ncol ; NRow",2,1,3,27,1,28);
  TH2F* peakcent_xy = new TH2F("peakcent_xy"," Peak Center (channel) ; Ncol ; NRow",2,1,3,27,1,28);
  TH2F* peaksigma_xy = new TH2F("peaksigma_xy"," Peak Sigma (channel) ; Ncol ; NRow",2,1,3,27,1,28);
  TH1F* h_psADC_pedsub_cut[psNRow][psNCol];
  for (Int_t nc=0;nc<psNCol;nc++) {
    for (Int_t nr=0;nr<psNRow;nr++) {
      h_psADC_pedsub_cut[nr][nc] =(TH1F*)fhistroot->Get(Form("h_psADC_pedsub_cut_%s%d",side[nc].c_str(),nr+1));
    }
  }

  //
  Double_t nhits[psNRow][psNCol];
  Double_t hist_mean[psNRow][psNCol];
  TCanvas* can[4];
  TF1* peakfit[psNRow][psNCol];
  Double_t peakmean[psNRow][psNCol];
  Double_t peakmean_err[psNRow][psNCol];
  Double_t peaksig[psNRow][psNCol];
  //Int_t offset=0;
  TCanvas* cancheck;
  Int_t blknum=1;
  cancheck= new TCanvas("can_check","check ",500,500);
  cancheck->Divide(1,1);
  //
  Int_t ncan=0;
  for (Int_t nr=0;nr<psNRow;nr++) {
    //offset = 7;
    //if (nr%2 ==0) offset = 0;
    for (Int_t nc=0;nc<psNCol;nc++) {
      //      cout << "Canvas pad "<< nr+1+nc*7 << endl;
      if ( h_psADC_pedsub_cut[nr][nc])  {
	if (h_psADC_pedsub_cut[nr][nc]->Integral() > 50) {//it was 100
	Double_t chcent = h_psADC_pedsub_cut[nr][nc]->GetBinCenter(h_psADC_pedsub_cut[nr][nc]->GetMaximumBin());
	if (chcent < 5 ) {
	  h_psADC_pedsub_cut[nr][nc]->GetXaxis()->SetRangeUser(10,100);
	  chcent = h_psADC_pedsub_cut[nr][nc]->GetBinCenter(h_psADC_pedsub_cut[nr][nc]->GetMaximumBin()); 
	}
	Double_t lowlimit=5.;
	Double_t hrms= h_psADC_pedsub_cut[nr][nc]->GetRMS();
        Double_t lowval = max(chcent-hrms,lowlimit);
        Double_t hival = chcent+hrms;
	peakfit[nr][nc] = new TF1(Form("fit_psADC_pedsub_cut_%s%d",side[nc].c_str(),nr+1),"gaus",lowval,hival);
	peakfit[nr][nc]->SetParameters(h_psADC_pedsub_cut[nr][nc]->GetMaximum(),chcent,hrms/2.);
	    h_psADC_pedsub_cut[nr][nc]->GetXaxis()->SetRangeUser(0,100);
	h_psADC_pedsub_cut[nr][nc]->Draw();
	h_psADC_pedsub_cut[nr][nc]->Fit(Form("fit_psADC_pedsub_cut_%s%d",side[nc].c_str(),nr+1),"QR");
	//
      cancheck->Update();
      Bool_t check_all = kTRUE;
      Int_t icheck=0;
      if (peakfit[nr][nc]->GetParError(1) > 10. || peakfit[nr][nc]->GetParameter(1) < 100. || check_all ) {
      cout << " enter icheck value (icheck=1 set new fit parameters, =0 fit ok) "<< endl;
      cin >> icheck ;
           while (icheck == 1) {
	     Double_t c,m,s;
	     cout << "enter starting values for Peak counts  mean sigma " << endl;
	     cin>> c >> m >> s ;
	peakfit[nr][nc]->SetParameters(c,m,s);
	h_psADC_pedsub_cut[nr][nc]->Fit(Form("fit_psADC_pedsub_cut_%s%d",side[nc].c_str(),nr+1),"QR","",.7*m,1.3*m);
      cancheck->Update();
      cout << " enter icheck (1 set new fit) "<< endl;
      cin >> icheck ;
	}
      }
	//
	peakmean[nr][nc]=peakfit[nr][nc]->GetParameter(1);
	peakmean_err[nr][nc]=peakfit[nr][nc]->GetParError(1);
	peaksig[nr][nc]=peakfit[nr][nc]->GetParameter(2);
	nhits[nr][nc]=h_psADC_pedsub_cut[nr][nc]->Integral();
	hist_mean[nr][nc]=h_psADC_pedsub_cut[nr][nc]->GetMean();
	} else {
	h_psADC_pedsub_cut[nr][nc]->GetXaxis()->SetRangeUser(45,1000);
	h_psADC_pedsub_cut[nr][nc]->Draw();
	peakmean[nr][nc]=h_psADC_pedsub_cut[nr][nc]->GetMean();
	peakmean_err[nr][nc]=h_psADC_pedsub_cut[nr][nc]->GetMeanError();
	peaksig[nr][nc]=h_psADC_pedsub_cut[nr][nc]->GetRMS();
	nhits[nr][nc]=h_psADC_pedsub_cut[nr][nc]->Integral();
	hist_mean[nr][nc]=h_psADC_pedsub_cut[nr][nc]->GetMean();
	}
      Float_t blknum= nr*psNCol + nc;
	nhits_xy->Fill(float(nc+1),float(nr+1),nhits[nr][nc]);
	mean_xy->Fill(float(nc+1),float(nr+1),hist_mean[nr][nc]);
	peakcent_xy->Fill(float(nc+1),float(nr+1),peakmean[nr][nc]);
	peaksigma_xy->Fill(float(nc+1),float(nr+1),peaksig[nr][nc]);
	peak_block->SetBinContent( blknum,peakmean[nr][nc]);
	peak_block->SetBinError( blknum,peakmean_err[nr][nc]);
      }
    }
    //    cout << ncan << endl;
  }
  //
  string OutFile = Form("Output/run_%d_ps_peak.txt",nrun);
   ofstream outfile_data;
  outfile_data.open(OutFile);
  ncan=0;
  for (Int_t nc=0;nc<psNCol;nc++) {
    for (Int_t nr=0;nr<psNRow;nr++) {
      outfile_data << peakmean[nr][nc] << " "   << peakmean_err[nr][nc]<< " " << endl; 
    }    
    }
  outfile_data.close();
   //
  Int_t nelem=0;
  for (Int_t nr=0;nr<psNRow;nr++) {
    if (nr%7 ==0) can[ncan]= new TCanvas(Form("can_%d",ncan),Form("Row %d - %d ",nr+1,min(nr+7, 27)),1000,700);
    if (nr%7 ==0) can[ncan]->Divide(7,2);
    for (Int_t nc=0;nc<psNCol;nc++) {
      can[ncan]->cd(nr%7+1+nc*7);
      gPad->SetLogy();
 	h_psADC_pedsub_cut[nr][nc]->Draw();
	nelem++;
    }
    if (ncan==0 && nelem==14) can[ncan]->Print(outputpdf+"(");
    if (ncan>0 && nelem<psNRow*psNCol) can[ncan]->Print(outputpdf);
    if (ncan==3&& nelem==psNRow*psNCol) can[ncan]->Print(outputpdf+")");
    if (nr%7 ==6)ncan++;
  }
 //
   outputpdf = "plots/"+basename+"_ps_select_event_2d.pdf";
  TCanvas* can2d;
  can2d= new TCanvas("can_2d","2d ",700,1000);
  can2d->Divide(2,2);
  can2d->cd(1);
  nhits_xy->Draw("colz");
  can2d->cd(2);
  mean_xy->Draw("colz");
  can2d->cd(3);
  peakcent_xy->Draw("colz");
  can2d->cd(4);
  peaksigma_xy->Draw("colz");
  can2d->Print(outputpdf+"(");

  //
   TCanvas* can1d;
  can1d= new TCanvas("can_1d","1d ",700,700);
  can1d->Divide(1,1);
  can1d->cd(1);
  peak_block->Draw();
  can1d->Print(outputpdf+")");

  //
}
