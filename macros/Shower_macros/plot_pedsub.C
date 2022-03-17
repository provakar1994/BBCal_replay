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
      outputpdf = "plots/"+basename+"_pedsub.pdf";
  TString inputroot;
   TFile *fhistroot;
     inputroot="hist/"+basename+"_shower_hist.root";
    fhistroot =  new TFile(inputroot);
    //
 static const Int_t shNCol=7;
 static const Int_t shNRow=27;
 TH2F* nhits_xy = new TH2F("nhits_xy"," Integral; Ncol ; NRow",7,1,8,27,1,28);
 TH2F* mean_xy = new TH2F("mean_xy"," Mean ; Ncol ; NRow",7,1,8,27,1,28);
  TH1F* h_shADC_pedsub[shNRow][shNCol];
 for (Int_t nr=0;nr<shNRow;nr++) {
 for (Int_t nc=0;nc<shNCol;nc++) {
   h_shADC_pedsub[nr][nc] =(TH1F*)fhistroot->Get(Form("h_shADC_pedsub_row%d_col%d",nr+1,nc+1)) ;
 }}
//
  TCanvas* can[shNRow];
TF1* peakfit[shNRow][shNCol];
for (Int_t nr=0;nr<shNRow;nr++) {
   can[nr]= new TCanvas(Form("can_%d",nr),Form("Row %d ",nr+1),700,700);
       can[nr]->Divide(4,2);
 for (Int_t nc=0;nc<shNCol;nc++) {
   can[nr]->cd(nc+1);
   gPad->SetLogy();
   if ( h_shADC_pedsub[nr][nc])  {
      h_shADC_pedsub[nr][nc]->Draw();
  }
 } 
    if (nr==0) can[nr]->Print(outputpdf+"(");
    if (nr>0&&nr<shNRow-1) can[nr]->Print(outputpdf);
    if (nr==shNRow-1) can[nr]->Print(outputpdf+")");
}
  //

//
}
