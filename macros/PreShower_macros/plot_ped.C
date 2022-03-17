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
  outputpdf = "plots/"+basename+"_ps_ped.pdf";
  TString inputroot;
  TFile *fhistroot;
  inputroot="hist/"+basename+"_preshower_hist.root";
  fhistroot =  new TFile(inputroot);
  //
  TH2F* ped_xy = new TH2F("ped_xy"," Pedestal Mean ; Ncol ; NRow",2,1,3,26,1,27);
  //  TH2F* ped_xy = new TH2F("ped_xy"," Pedestal Mean ; Ncol ; NRow",2,1,3,27,1,28);
  TH2F* sigma_xy = new TH2F("sigma_xy"," Pedestal Sigma ; Ncol ; NRow",2,1,3,26,1,27);
  //  TH2F* sigma_xy = new TH2F("sigma_xy"," Pedestal Sigma ; Ncol ; NRow",2,1,3,27,1,28);
  TH1F* sigma = new TH1F("sigma"," Pedestal Sigma for all blocks ",80,0,20);
  static const Int_t psNCol=2;
  static const Int_t psNRow=26;
  //  static const Int_t psNRow=27;
  static const string side[2] = {"L", "R"};
  
  TH1F* h_psADC[psNRow][psNCol];
  for (Int_t nr=0;nr<psNRow;nr++) {
    for (Int_t nc=0;nc<psNCol;nc++) {
      h_psADC[nr][nc] =(TH1F*)fhistroot->Get(Form("h_psADC_%s%d",side[nc].c_str(),nr+1)) ;
    }}
  //
  TCanvas* can[psNRow];
  TF1* pedfit[psNRow][psNCol];
  Double_t pedmean[psNRow][psNCol];
  Double_t pedsig[psNRow][psNCol];
  for (Int_t nr=0;nr<psNRow;nr++) {
    can[nr]= new TCanvas(Form("can_%d",nr),Form("Row %d ",nr+1),700,700);
    can[nr]->Divide(2,1);
    for (Int_t nc=0;nc<psNCol;nc++) {
      can[nr]->cd(nc+1);
      gPad->SetLogy();
      if ( h_psADC[nr][nc])  {
	Double_t chcent = h_psADC[nr][nc]->GetBinCenter(h_psADC[nr][nc]->GetMaximumBin());
	pedfit[nr][nc] = new TF1(Form("fit_psADC_row%d_col%d",nr+1,nc+1),"gaus",chcent-15,chcent+15);
	h_psADC[nr][nc]->Draw();
	h_psADC[nr][nc]->Fit(Form("fit_psADC_row%d_col%d",nr+1,nc+1),"QR");
	pedmean[nr][nc]=pedfit[nr][nc]->GetParameter(1);
	pedsig[nr][nc]=pedfit[nr][nc]->GetParameter(2);
	ped_xy->Fill(float(nc+1),float(nr+1),pedmean[nr][nc]);
	sigma_xy->Fill(float(nc+1),float(nr+1),pedsig[nr][nc]);
	sigma->Fill(pedsig[nr][nc]);
      }
    }
    if (nr==0) can[nr]->Print(outputpdf+"(");
    if (nr>0&&nr<psNRow-1) can[nr]->Print(outputpdf);
    if (nr==psNRow-1) can[nr]->Print(outputpdf+")");
}
  //
   outputpdf = "plots/"+basename+"_ps_ped_2d.pdf";
  //
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
  cout << " BB Preshower pedestal to go in /home/daq/Analysis/BBcal/DB/db_bb.ps.dat" << endl;
  cout << "bb.ps.pedestal = " << endl;

    for (Int_t nc=0;nc<psNCol;nc++) {
      for (Int_t nr=0;nr<psNRow;nr++) {
      cout << pedmean[nr][nc] << "   " ;
      if (nr==16) cout <<endl;
    }
    cout << endl;
  }

  cout << "bb.ps.pednoise = " << endl;
    for (Int_t nc=0;nc<psNCol;nc++) {
  for (Int_t nr=0;nr<psNRow;nr++) {
      cout << pedsig[nr][nc] << "   " ;
    }
    cout << endl;
  }

  //
}
