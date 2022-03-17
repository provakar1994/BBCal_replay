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

void plot_ped(TString basename) {
  gROOT->Reset();
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(11);
  gStyle->SetTitleOffset(1.,"Y");
  gStyle->SetTitleOffset(.7,"X");
  gStyle->SetLabelSize(0.04,"XY");
  gStyle->SetTitleSize(0.06,"XY");
  gStyle->SetPadLeftMargin(0.14);
  TString outputpdf;
  outputpdf = "plots/"+basename+"_ped.pdf";
  TString inputroot;
  TFile *fhistroot;
  inputroot="hist/"+basename+"_shower_hist.root";
  fhistroot =  new TFile(inputroot);
  //
  TH2F* ped_xy = new TH2F("ped_xy"," Pedestal Mean ; Ncol ; NRow",7,1,8,27,1,28);
  TH2F* sigma_xy = new TH2F("sigma_xy"," Pedestal Sigma ; Ncol ; NRow",7,1,8,27,1,28);
  TH1F* sigma = new TH1F("sigma"," Pedestal Sigma for all blocks ",80,0,20);
  static const Int_t shNCol=7;
  static const Int_t shNRow=27;
  TH1F* h_shADC[shNRow][shNCol];
  for (Int_t nr=0;nr<shNRow;nr++) {
    for (Int_t nc=0;nc<shNCol;nc++) {
      h_shADC[nr][nc] =(TH1F*)fhistroot->Get(Form("h_shADC_row%d_col%d",nr+1,nc+1)) ;
    }}
  //
  TCanvas* can[shNRow];
  TF1* pedfit[shNRow][shNCol];
  Double_t pedmean[shNRow][shNCol];
  Double_t pedsig[shNRow][shNCol];
  for (Int_t nr=0;nr<shNRow;nr++) {
    can[nr]= new TCanvas(Form("can_%d",nr),Form("Row %d ",nr+1),700,700);
    can[nr]->Divide(4,2);
    for (Int_t nc=0;nc<shNCol;nc++) {
      can[nr]->cd(nc+1);
      gPad->SetLogy();
      if ( h_shADC[nr][nc])  {
	Double_t chcent = h_shADC[nr][nc]->GetBinCenter(h_shADC[nr][nc]->GetMaximumBin());
	pedfit[nr][nc] = new TF1(Form("fit_shADC_row%d_col%d",nr+1,nc+1),"gaus",chcent-15,chcent+15);
	h_shADC[nr][nc]->Draw();
	h_shADC[nr][nc]->Fit(Form("fit_shADC_row%d_col%d",nr+1,nc+1),"QR");
	pedmean[nr][nc]=pedfit[nr][nc]->GetParameter(1);
	pedsig[nr][nc]=pedfit[nr][nc]->GetParameter(2);
	ped_xy->Fill(float(nc+1),float(nr+1),pedmean[nr][nc]);
	sigma_xy->Fill(float(nc+1),float(nr+1),pedsig[nr][nc]);
	sigma->Fill(pedsig[nr][nc]);
      }
    }
    if (nr==0) can[nr]->Print(outputpdf+"(");
    if (nr>0&&nr<shNRow-1) can[nr]->Print(outputpdf);
    if (nr==shNRow-1) can[nr]->Print(outputpdf+")");
    }
  //
  //
   outputpdf = "plots/"+basename+"_ped_2d.pdf";
  TCanvas* can2d;
  can2d= new TCanvas("can_2d","2d ",700,1000);
  can2d->Divide(2,2);
  can2d->cd(1);
  ped_xy->Draw("colz");
  can2d->cd(2);
  sigma_xy->Draw("colz");
  can2d->cd(3);
  sigma->Draw("");
  can2d->Print(outputpdf);

  //

  //
  cout << " BB Shower pedestal to go in /home/daq/Analysis/BBcal/DB/db_bb.sh.dat" << endl;
  cout << "bb.sh.pedestal = " << endl;
  for (Int_t nr=0;nr<shNRow;nr++) {
    for (Int_t nc=0;nc<shNCol;nc++) {
      cout << pedmean[nr][nc] << "   " ;
    }
    cout << endl;
  }

  cout << "bb.sh.pednoise = " << endl;
  for (Int_t nr=0;nr<shNRow;nr++) {
    for (Int_t nc=0;nc<shNCol;nc++) {
      cout << pedsig[nr][nc] << "   " ;
    }
    cout << endl;
  }

  //
}
