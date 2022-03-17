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

void plot_pedsub(TString basename) {
 gROOT->Reset();
 gStyle->SetOptStat(0);
 gStyle->SetOptFit(11);
 gStyle->SetTitleOffset(1.,"Y");
 gStyle->SetTitleOffset(.7,"X");
 gStyle->SetLabelSize(0.04,"XY");
 gStyle->SetTitleSize(0.06,"XY");
 gStyle->SetPadLeftMargin(0.14);
      TString outputpdf;
      outputpdf = "plots/"+basename+"_ps_pedsub.pdf";
  TString inputroot;
   TFile *fhistroot;
     inputroot="hist/"+basename+"_preshower_hist.root";
    fhistroot =  new TFile(inputroot);
    //
 static const Int_t psNCol=2;
 static const Int_t psNRow=26;
 static const string side[2] = {"L", "R"};

 TH2F* nhits_xy = new TH2F("nhits_xy"," Integral; Ncol ; NRow",2,1,3,27,1,28);
 TH2F* mean_xy = new TH2F("mean_xy"," Mean ; Ncol ; NRow",2,1,3,27,1,28);
 TH1F* h_psADC_pedsub[psNRow][psNCol];
 for (Int_t nr=0;nr<psNRow;nr++) {
 for (Int_t nc=0;nc<psNCol;nc++) {
   h_psADC_pedsub[nr][nc] =(TH1F*)fhistroot->Get(Form("h_psADC_pedsub_%s%d",side[nc].c_str(),nr+1)) ;
 }}
//
  TCanvas* can[psNRow];
for (Int_t nr=0;nr<psNRow;nr++) {
  can[nr]= new TCanvas(Form("can_%d",nr),Form("Row %d ",nr+1),700,700);
  can[nr]->Divide(2,1);
 for (Int_t nc=0;nc<psNCol;nc++) {
   can[nr]->cd(nc+1);
   gPad->SetLogy();
   if ( h_psADC_pedsub[nr][nc])  {
     h_psADC_pedsub[nr][nc]->Draw();
   }
 }
    if (nr==0) can[nr]->Print(outputpdf+"(");
    if (nr>0&&nr<psNRow-1) can[nr]->Print(outputpdf);
    if (nr==psNRow-1) can[nr]->Print(outputpdf+")");
}
  //

//
}
