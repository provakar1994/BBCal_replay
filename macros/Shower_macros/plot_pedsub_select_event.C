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

void plot_pedsub_select_event(TString basename,Int_t nrun,Bool_t check_all = kTRUE) {
  gROOT->Reset();
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(11);
  gStyle->SetTitleOffset(1.,"Y");
  gStyle->SetTitleOffset(.7,"X");
  gStyle->SetLabelSize(0.04,"XY");
  gStyle->SetTitleSize(0.06,"XY");
  gStyle->SetPadLeftMargin(0.14);
  TString outputpdf;
  outputpdf = "plots/"+basename+"_select_event.pdf";
  TString inputroot;
  TFile *fhistroot;
  inputroot="hist/"+basename+"_shower_hist.root";
  fhistroot =  new TFile(inputroot);
  //
  static const Int_t shNCol=7;
  static const Int_t shNRow=27;
  TH2F* nhits_xy = new TH2F("nhits_xy"," Integral; Ncol ; NRow",7,1,8,27,1,28);
  TH1F* peak_block = new TH1F("peak_block","  ; Block ; Peak position  ",shNCol*shNRow,0,shNCol*shNRow);
  TH2F* mean_xy = new TH2F("mean_xy"," Mean ; Ncol ; NRow",7,1,8,27,1,28);
  TH2F* peakcent_xy = new TH2F("peakcent_xy"," Peak Center (channel) ; Ncol ; NRow",7,1,8,27,1,28);
  TH2F* peaksigma_xy = new TH2F("peaksigma_xy"," Peak Sigma (channel) ; Ncol ; NRow",7,1,8,27,1,28);
  TH1F* peakblocksigmaratio_block = new TH1F("peakblocksigmaratio_block","  ; Block ; Peak position sigma ratio  ",shNCol*shNRow,1,shNCol*shNRow+1);
  TH1F* h_shADC_pedsub_cut[shNRow][shNCol];
  for (Int_t nc=0;nc<shNCol;nc++) {
    for (Int_t nr=0;nr<shNRow;nr++) {
      h_shADC_pedsub_cut[nr][nc] =(TH1F*)fhistroot->Get(Form("h_shADC_pedsub_row%d_col%d",nr+1,nc+1)) ;
    }}
  //
  //
  Double_t nhits[shNRow][shNCol];
  Double_t hist_mean[shNRow][shNCol];
  TCanvas* can[14];
  TF1* peakfit[shNRow][shNCol];
  Double_t peakmean[shNRow][shNCol];
  Double_t peakmean_err[shNRow][shNCol];
  Double_t peaksig[shNRow][shNCol];
  TCanvas* cancheck;
  Int_t blknum=0;
  cancheck= new TCanvas("can_check","check ",500,500);
  cancheck->Divide(1,1);
      //gPad->SetLogy();
   for (Int_t nr=0;nr<shNRow;nr++) {
    for (Int_t nc=0;nc<shNCol;nc++) {
      blknum = nr*shNCol + nc;
      if ( h_shADC_pedsub_cut[nr][nc])  {
	Double_t chcent = h_shADC_pedsub_cut[nr][nc]->GetBinCenter(h_shADC_pedsub_cut[nr][nc]->GetMaximumBin());
	if (chcent < 5) {
	  h_shADC_pedsub_cut[nr][nc]->GetXaxis()->SetRangeUser(0,100);
	chcent = h_shADC_pedsub_cut[nr][nc]->GetBinCenter(h_shADC_pedsub_cut[nr][nc]->GetMaximumBin());
	}
	Double_t lowval=chcent*0.7; 
	Double_t hival=chcent*1.3; 
		peakfit[nr][nc] = new TF1(Form("fit_shADC_pedsub_cut_row%d_col%d",nr+1,nc+1),"gaus",lowval,hival);
		peakfit[nr][nc]->SetParameters(h_shADC_pedsub_cut[nr][nc]->GetMaximum(),chcent,chcent*.3);
	h_shADC_pedsub_cut[nr][nc]->Draw();
	h_shADC_pedsub_cut[nr][nc]->Fit(Form("fit_shADC_pedsub_cut_row%d_col%d",nr+1,nc+1),"QR");
	//
      cancheck->Update();
      Int_t icheck=0;
      if (peakfit[nr][nc]->GetParError(1) > 10. || peakfit[nr][nc]->GetParameter(1) < 100. || check_all ) {
      cout << " enter icheck value (icheck=1 set new fit parameters, =0 fit ok) "<< endl;
      cin >> icheck ;
           while (icheck == 1) {
	     Double_t c,m,s;
	     cout << "enter starting values for Peak counts  mean sigma " << endl;
	     cin>> c >> m >> s ;
	peakfit[nr][nc]->SetParameters(c,m,s);
	h_shADC_pedsub_cut[nr][nc]->Fit(Form("fit_shADC_pedsub_cut_row%d_col%d",nr+1,nc+1),"QR","",.6*m,1.4*m);
      cancheck->Update();
      cout << " enter icheck (1 set new fit) "<< endl;
      cin >> icheck ;
      //  cancheck->WaitPrimitive();	     
	   }
      }
	//
	peakmean[nr][nc]=peakfit[nr][nc]->GetParameter(1);
	peakmean_err[nr][nc]=peakfit[nr][nc]->GetParError(1);
	peaksig[nr][nc]=peakfit[nr][nc]->GetParameter(2);
	nhits[nr][nc]=h_shADC_pedsub_cut[nr][nc]->Integral();
	hist_mean[nr][nc]=h_shADC_pedsub_cut[nr][nc]->GetMean();
	cout << " row col = " << nr+1 << " " << nc+1 << " " << " " << chcent << " peak = " << peakmean[nr][nc] << " " << peaksig[nr][nc] << " " <<lowval<< " " <<hival  << endl;
	//cout << " low " << lowval << " " << hival<< endl;
	nhits_xy->Fill(float(nc+1),float(nr+1),nhits[nr][nc]);
	mean_xy->Fill(float(nc+1),float(nr+1),hist_mean[nr][nc]);
	peakcent_xy->Fill(float(nc+1),float(nr+1),peakmean[nr][nc]);
	peaksigma_xy->Fill(float(nc+1),float(nr+1),peaksig[nr][nc]);
	peak_block->SetBinContent( blknum,peakmean[nr][nc]);
	peak_block->SetBinError( blknum,peakmean_err[nr][nc]);
	peakblocksigmaratio_block->SetBinContent(blknum,peakmean[nr][nc]/peaksig[nr][nc]);//included by Arun
	//	peakblocksigmaratio_block->SetBinError(blknum++,peakmean_err[nr][nc]/peaksig[nr][nc]);//included by Arun
      }
    }
   }
  //
  Int_t offset=0;
  Int_t ncan=0;
  for (Int_t nr=0;nr<shNRow;nr++) {
    if (nr%2 ==0) can[ncan]= new TCanvas(Form("can_%d",ncan),Form("Row %d - %d ",nr+1,nr+2),700,700);
    if (nr%2 ==0) can[ncan]->Divide(7,2);
    offset = 7;
    if (nr%2 ==0) offset = 0;
    for (Int_t nc=0;nc<shNCol;nc++) {
      can[ncan]->cd(nc+1+offset);
	h_shADC_pedsub_cut[nr][nc]->Draw();
    }
    if (ncan==0 && nr%2 ==1) can[ncan]->Print(outputpdf+"(");
    if (ncan>0&&ncan<13&& nr%2 ==1) can[ncan]->Print(outputpdf);
    if (ncan==13&& nr%2 ==0) can[ncan]->Print(outputpdf+")");
    if (nr%2 ==1)ncan++;
  }
  //
  //
  string OutFile = Form("Output/run_%d_peak.txt",nrun);
   ofstream outfile_data;
  outfile_data.open(OutFile);
    for (Int_t nr=0;nr<shNRow;nr++) {
  for (Int_t nc=0;nc<shNCol;nc++) {
    outfile_data << peakmean[nr][nc] << " "   << peakmean_err[nr][nc]<< " " ; 
    }    
  outfile_data << endl;
    }
  outfile_data.close();
 
  //
   outputpdf = "plots/"+basename+"_select_event_2d.pdf";
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
   TCanvas* can1d;
  can1d= new TCanvas("can_1d","1d ",700,700);
  can1d->Divide(1,2); //Divide(1,1)
  can1d->cd(1);
  peak_block->SetMinimum(0);
  peak_block->SetMaximum(50.);
  peak_block->Draw();
  can1d->cd(2);
  peakblocksigmaratio_block->SetMinimum(0);
  peakblocksigmaratio_block->SetMaximum(10.);
  peakblocksigmaratio_block->Draw();
  can1d->Print(outputpdf+")");
 //

  //
}
