/* 
   M. K. Jones  Created
*/

#include <TSystem.h>
#include <TChain.h>
#include <TString.h>
#include "TFile.h"
#include "TTree.h"
#include <TNtuple.h>
#include "TCanvas.h"
#include <iostream>
#include <fstream>
#include "TMath.h"
#include "TH1F.h"
#include <TH2.h>
#include <TStyle.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TMath.h>
#include <TLegend.h>
#include <TPaveLabel.h>
#include <TProfile.h>
#include <TPolyLine.h>
#include <TObjArray.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include<math.h>
using namespace std;

void make_hist_preshower(TString basename="",Int_t nrun=2043,Int_t nev=-1, Bool_t show_track = kFALSE){
  if (basename=="") {
    cout << " Input the basename of the root file (assumed to be in worksim)" << endl;
    cin >> basename;
  }
  TString fullname = Form("%s_%d",basename.Data(),nrun);
  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(1000011);
  gStyle->SetOptFit(11);
  gStyle->SetTitleOffset(1.,"Y");
  gStyle->SetTitleOffset(.7,"X");
  gStyle->SetLabelSize(0.04,"XY");
  gStyle->SetTitleSize(0.06,"XY");
  gStyle->SetPadLeftMargin(0.12);
  TString inputroot;
  inputroot="rootfiles/"+fullname+".root";
  TString outputhist;
  outputhist= "hist/"+fullname+"_preshower_hist.root";
  TObjArray HList(0);
  //
  TString Chainroot;
  Int_t nseg = 0;
  Chainroot = Form("rootfiles/%s_%d_%d.root",basename.Data(),nrun,nev);
  TChain *fchain = new TChain("T");
  fchain->Add(Chainroot);
  cout << "check file " << Chainroot << endl;
  /*
  while (gSystem->FindFile(".",Chainroot)) { 
  Int_t npart = 1;
  cout << " add file " << Chainroot << endl;
  fchain->Add(Chainroot);
  Chainroot = Form("../../rootfiles/%s_%d_%d_%d.root",basename.Data(),nrun,nseg,npart);
  while (gSystem->FindFile(".",Chainroot)) {
	cout <<" add file " << Chainroot<< endl; 
          fchain->Add(Chainroot);
	 npart++;
  Chainroot = Form("../../rootfiles/%s_%d_%d_%d.root",basename.Data(),nrun,nseg,npart);
       } 
  nseg++;
  Chainroot = Form("../../rootfiles/%s_%d_%d.root",basename.Data(),nrun,nseg);
  }
  */
  /*
  TFile *fdata = new TFile(inputroot); 
  TTree *tdata = (TTree*) fdata->Get("T");
  */
  static const Int_t psNum=52;//used to be 54
  static const Int_t psNRow=26;//used to be 27
  static const Int_t psNCol=2;
  static const string side[2] = {"L", "R"};
  Double_t psADC[psNum]; // raw adc
  fchain->SetBranchAddress("bb.ps.a_amp",&psADC) ;
  Double_t psADC_pedsub[psNum]; // raw adc
  fchain->SetBranchAddress("bb.ps.a_amp_p",&psADC_pedsub) ;
  Double_t psADC_time[psNum]; // raw adc
  fchain->SetBranchAddress("bb.ps.a_time",&psADC_time) ;
  //
  TH1F* h_NhitsEvent = new TH1F("h_NhitsEvent"," ; Number of blocks hit per event",100,0,100);
      HList.Add(h_NhitsEvent);
  TH1F* h_psADC[psNRow][psNCol];
  TH1F* h_psADC_pedsub[psNRow][psNCol];
  TH1F* h_psADC_pedsub_cut[psNRow][psNCol];
  for (Int_t nc=0;nc<psNCol;nc++) {
    for (Int_t nr=0;nr<psNRow;nr++) {
      h_psADC[nr][nc] = new TH1F(Form("h_psADC_%s%d",side[nc].c_str(),nr+1),Form("Run %d; PreShower  %s%d ADC amp (mV) ",nrun,side[nc].c_str(),nr+1),1000,0,1000);
      HList.Add(h_psADC[nr][nc]);
      h_psADC_pedsub[nr][nc] = new TH1F(Form("h_psADC_pedsub_%s%d",side[nc].c_str(),nr+1),Form("Pedsub Run %d; PreShower %s%dADC amp (mV) ",nrun,side[nc].c_str(),nr+1),50,0,100);
      HList.Add(h_psADC_pedsub[nr][nc]);
      h_psADC_pedsub_cut[nr][nc] = new TH1F(Form("h_psADC_pedsub_cut_%s%d",side[nc].c_str(),nr+1),Form("Select event Pedsub Run %d; PreShower %s%d ADC amp (mV)",nrun,side[nc].c_str(),nr+1),50,0,100);
      HList.Add(h_psADC_pedsub_cut[nr][nc]);
    }
  }
  //
  TH1F* h_nhitsL = new TH1F("h_nhitsL","; Nhits Left side (Hit row L2 and L26);Counts",26,1,27);//used to be 27 rows
  //  TH1F* h_nhitsL = new TH1F("h_nhitsL","; Nhits Left side (Hit row L2 and L26);Counts",27,1,28);//used to be 27 rows
  TH1F* h_nhitsR = new TH1F("h_nhitsR","; Nhits Right side (Hit row R2 and R26);Counts",26,1,27);//used to be 27 rows
  //  TH1F* h_nhitsR = new TH1F("h_nhitsR","; Nhits Right side (Hit row R2 and R26);Counts",27,1,28);//used to be 27 rows
  TH2F* h_track = new TH2F("h_track","track;L        side        R;row",2,1,3,26,1,27);
  //  TH2F* h_track = new TH2F("h_track","track;L        side        R;row",2,1,3,27,1,28);
  TCanvas* can2d;
  if (show_track) {
    can2d= new TCanvas("can_2d","2d ",700,1000);
    can2d->Divide(1,1);
  }
  //
  Long64_t nentries = fchain->GetEntries();
  Int_t nselect=0;
  Int_t nselectL=0;
  Int_t nselectR=0;
   cout << " Total Entry = " << nentries << endl;
  for (int i = 0; i < nentries; i++) {
    fchain->GetEntry(i);
    if (i%50000==0) cout << " Entry = " << i << endl;
    
    //Bool_t select_event1=kFALSE; // select events that hit row 20
    Bool_t select_event=kFALSE; // provided the geometry of the setup, we need to do an "Or" of all the blocks...
    if (show_track) h_track->Reset();
    Double_t nhitsL=0;
    Double_t nhitsR=0;
    Double_t nhitsL_all=0;
    Double_t nhitsR_all=0;
    Bool_t hit_first_row1_L=kFALSE;
    Bool_t hit_first_row2_L=kFALSE;
    Bool_t hit_first_row1_R=kFALSE;
    Bool_t hit_first_row2_R=kFALSE;
    Int_t NumHitsEv = 0;
    for (Int_t nr=0;nr<psNRow;nr++) {
      for (Int_t nc=0;nc<psNCol;nc++) {
	Int_t nelem=nr*psNCol+nc;
	if (psADC_time[nelem] >0 ) {
        h_psADC[nr][nc]->Fill(psADC[nelem]);
	h_psADC_pedsub[nr][nc]->Fill(psADC_pedsub[nelem]);
	if (psADC_pedsub[nelem]>5.) NumHitsEv++;
	if (psADC_pedsub[nelem]>5. &&nc==0) nhitsL_all++;
	if (psADC_pedsub[nelem]>5. &&nc==1) nhitsR_all++;
	if (nr == 2  && nc == 0  && psADC_pedsub[nelem]>5) hit_first_row1_L=kTRUE;
	if (nr == 24  && nc == 0  && psADC_pedsub[nelem]>5) hit_first_row2_L=kTRUE;
	if (nr == 2  && nc == 1  && psADC_pedsub[nelem]>5) hit_first_row1_R=kTRUE;
	if (nr == 24  && nc == 1  && psADC_pedsub[nelem]>5) hit_first_row2_R=kTRUE;
	}
      }		
    }
    h_NhitsEvent->Fill(float(NumHitsEv));
    select_event = (hit_first_row1_L&&hit_first_row2_L) || (hit_first_row1_R&&hit_first_row2_R);
    select_event = NumHitsEv>10;
    //select_event= kTRUE;
	if (select_event) nselect++;
	if ((hit_first_row1_L&&hit_first_row2_L) ) nselectL++;
	if ((hit_first_row1_R&&hit_first_row2_R) ) nselectR++;
    for (Int_t nr=0;nr<psNRow;nr++) {
      for (Int_t nc=0;nc<psNCol;nc++) {
	Int_t nelem=nr*psNCol+nc;
	if ((hit_first_row1_L&&hit_first_row2_L)&& psADC_pedsub[nelem]>20. &&nc==0&& nhitsR_all==0) nhitsL++;
	if ((hit_first_row1_R&&hit_first_row2_R)&& psADC_pedsub[nelem]>20.&&nc==1 && nhitsL_all==0) nhitsR++;
	if (nc==0 && select_event&& psADC_pedsub[nelem]>5) h_psADC_pedsub_cut[nr][nc]->Fill(psADC_pedsub[nelem]);
	if (nc==1 && select_event&& psADC_pedsub[nelem]>5 ) h_psADC_pedsub_cut[nr][nc]->Fill(psADC_pedsub[nelem]);
	if (show_track&&select_event&&psADC_pedsub[nelem]>50) {
	  h_track->Fill(float(nc+1),float(nr+1),psADC_pedsub[nelem]);
	} 
      }
    }
    if (nhitsL>0) h_nhitsL->Fill(nhitsL);
    if (nhitsR>0) h_nhitsR->Fill(nhitsR);
    if (show_track&&select_event) {
      can2d->cd(1);
      h_track->Draw("colz");
      can2d->Update();
      can2d->WaitPrimitive();
    }
  }
  //
  cout << " number select events = " << nselect << endl;
  cout << " number selectL events = " << nselectL << endl;
  cout << " number selectR events = " << nselectR << endl;
  TFile hsimc(outputhist,"recreate");
  HList.Write();
  //
}
