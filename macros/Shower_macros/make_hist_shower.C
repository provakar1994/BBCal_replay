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

void make_hist_shower(TString basename="",Int_t nrun=2043,Int_t nev=-1, Bool_t show_track = kFALSE){
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
  outputhist= "hist/"+fullname+"_shower_hist.root";
  TObjArray HList(0);
   //
  TString Chainroot;
  Int_t nseg = 0;
  Chainroot = Form("rootfiles/%s_%d_%d.root",basename.Data(),nrun,nev);
  cout << inputroot << endl;
  cout << Chainroot << endl;
  TChain *fchain = new TChain("T");
  //  cout << "check file " << Chainroot << endl;
  if (!(gSystem->FindFile(".",Chainroot))) {
    cout << " no file " << Chainroot << endl;
    return;
  }
  fchain->Add(Chainroot);
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
  static const Int_t shNum=189;
  static const Int_t shNRow=27;
  static const Int_t shNCol=7;
  Double_t shADC[shNum]; // raw adc
  fchain->SetBranchAddress("bb.sh.a_amp",&shADC) ;
  Double_t shADC_pedsub[shNum]; // raw adc
  fchain->SetBranchAddress("bb.sh.a_amp_p",&shADC_pedsub) ;
  Double_t shADC_time[shNum]; // raw adc
  fchain->SetBranchAddress("bb.sh.a_time",&shADC_time) ;
  //
  
  double ADCSum_thr[189];
  double EvtSum_thr[189];
  
  for(int i = 0; i<189; i++){
    ADCSum_thr[i] = 0;
    EvtSum_thr[i] = 0;
  }
  
  Int_t nc1=4;
  Int_t nr1=20;
  Int_t nr2=21;
  Int_t nc2=4;
  TH2F* h_row21_col4_row20_col4 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
   nc1=4;nr1=20;nc2=4;nr2=19;
  TH2F* h_row19_col4_row20_col4 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
   nc1=3;nr1=21;nc2=3;nr2=20;
  TH2F* h_row21_col3_row20_col3 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
   nc1=5;nr1=21;nc2=5;nr2=20;
  TH2F* h_row21_col5_row20_col5 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
   nc1=6;nr1=21;nc2=6;nr2=20;
  TH2F* h_row21_col6_row20_col6 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
   nc1=7;nr1=21;nc2=7;nr2=20;
  TH2F* h_row21_col7_row20_col7 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
   nc1=2;nr1=21;nc2=2;nr2=20;
  TH2F* h_row21_col2_row20_col2 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
   nc1=1;nr1=21;nc2=1;nr2=20;
  TH2F* h_row21_col1_row20_col1 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
   nc1=4;nr1=26;nc2=4;nr2=27;
  TH2F* h_row27_col4_row26_col4 = new TH2F(Form("h_shADC_pedsub_row%d_col%d_row%d_col%d",nr1,nc1,nr2,nc2),Form("Pedsub Run %d; Shower ADC Row %d Col %d; Shower ADC Row %d Col %d",nrun,nr1,nc1,nr2,nc2),75,20,1500,75,20,1500);
  //
  TH1F* h_shADC[shNRow][shNCol];
  TH1F* h_shADC_pedsub[shNRow][shNCol];
  TH1F* h_shADC_pedsub_should[shNRow][shNCol];
  TH1F* h_shADC_pedsub_did[shNRow][shNCol];
  TH1F* h_shADC_pedsub_cut[shNRow][shNCol];
  TH2F* h_shADC_pedsub_cut_event[shNRow][shNCol];
  for (Int_t nc=0;nc<shNCol;nc++) {
    for (Int_t nr=0;nr<shNRow;nr++) {
      h_shADC[nr][nc] = new TH1F(Form("h_shADC_row%d_col%d",nr+1,nc+1),Form("Run %d; Shower ADC Row %d Col %d",nrun,nr+1,nc+1),50,0,100);
      HList.Add(h_shADC[nr][nc]);
      h_shADC_pedsub[nr][nc] = new TH1F(Form("h_shADC_pedsub_row%d_col%d",nr+1,nc+1),Form("Pedsub Run %d; Shower ADC Row %d Col %d",nrun,nr+1,nc+1),50,0,100);
      HList.Add(h_shADC_pedsub[nr][nc]);
      h_shADC_pedsub_should[nr][nc] = new TH1F(Form("h_shADC_pedsub_should_row%d_col%d",nr+1,nc+1),Form("Pedsub Run %d; Shower ADC Row %d Col %d",nrun,nr+1,nc+1),50,0,100);
      HList.Add(h_shADC_pedsub_should[nr][nc]);
      h_shADC_pedsub_did[nr][nc] = new TH1F(Form("h_shADC_pedsub_did_row%d_col%d",nr+1,nc+1),Form("Pedsub Run %d; Shower ADC Row %d Col %d",nrun,nr+1,nc+1),50,0,100);
      HList.Add(h_shADC_pedsub_did[nr][nc]);
      h_shADC_pedsub_cut[nr][nc] = new TH1F(Form("h_shADC_pedsub_cut_row%d_col%d",nr+1,nc+1),Form("Select event Pedsub Run %d; Shower ADC Row %d Col %d",nrun,nr+1,nc+1),50,0,100);
      HList.Add(h_shADC_pedsub_cut[nr][nc]);
      h_shADC_pedsub_cut_event[nr][nc] = new TH2F(Form("h_shADC_pedsub_cut_event_row%d_col%d",nr+1,nc+1),Form("Select event Pedsub Run %d; event number ; Shower ADC Row %d Col %d",nrun,nr+1,nc+1),700,0,700000,10,20,520);
      HList.Add(h_shADC_pedsub_cut_event[nr][nc]);
    }
  }
  TH2F* h_track = new TH2F("h_track","track; col ;row ",7,1,8,27,1,28);
  TCanvas* can2d;
  if (show_track) {
    can2d= new TCanvas("can_2d","2d ",700,1000);
    can2d->Divide(1,1);
  }
  //
  Long64_t nentries = fchain->GetEntries();
  for (int i = 0; i < nentries; i++) {
    fchain->GetEntry(i);
    if (i%50000==0) cout << " Entry = " << i << endl;
    Bool_t select_event1=kFALSE; // select events that hit
    Bool_t select_event2=kFALSE; // select events that hit
    if (show_track) h_track->Reset();
      Int_t nr1 = 26;// actual row is nr+1 for number 1-27 
      Int_t nr2 = 0;
      Bool_t sel_hirow[shNCol];
      Bool_t sel_lorow[shNCol];
      Int_t sel_col_nr1 = -1;
      Int_t sel_col_nr2 = -1;
    for (Int_t nc=0;nc<shNCol;nc++) {
      sel_hirow[nc]=kFALSE;
      sel_lorow[nc]=kFALSE;
    }
    for (Int_t nc=0;nc<shNCol;nc++) {
       if (shADC_pedsub[nr1*shNCol+nc]>20) {
            sel_hirow[nc]=kTRUE;
            select_event1=kTRUE; // select events that hit
	  sel_col_nr1 =nc;
      }
     if (shADC_pedsub[nr2*shNCol+nc]>20) {
           sel_lorow[nc]=kTRUE;
          select_event2=kTRUE; // select events that hit
	  sel_col_nr2 =nc;
       }
    }	
    //
    for (Int_t nr=0;nr<shNRow;nr++) {
     for (Int_t nc=0;nc<shNCol;nc++) {
	Int_t nelem=nr*shNCol+nc;
	if (select_event1 &&select_event2 &&shADC_pedsub[nelem]>25) {
	  h_shADC_pedsub_cut[nr][nc]->Fill(shADC_pedsub[nelem]);        
	  h_shADC_pedsub_cut_event[nr][nc]->Fill(float(i),shADC_pedsub[nelem]);
	    }
      }}
   /*
   nr1=9;
   nr2=0;
      for (Int_t nc=0;nc<shNCol;nc++) {
      sel_hirow[nc]=kFALSE;
      sel_lorow[nc]=kFALSE;
      if (shADC_pedsub[nr1*shNCol+nc]>20 && shADC_pedsub[nr2*shNCol+nc]>20 && shADC_pedsub[nr1*shNCol+nc]<600 && shADC_pedsub[nr2*shNCol+nc]<600) {
            sel_hirow[nc]=kTRUE;
            sel_lorow[nc]=kTRUE;
      }}
   for (Int_t nr=nr2;nr<nr1+1;nr++) {
      for (Int_t nc=0;nc<shNCol;nc++) {
	Int_t nelem=nr*shNCol+nc;
	if (sel_hirow[nc] && sel_lorow[nc]&&shADC_pedsub[nelem]>15) h_shADC_pedsub_cut[nr][nc]->Fill(shADC_pedsub[nelem]);
      }}
   //
   //
   nr1=17;
   nr2=9;
      for (Int_t nc=0;nc<shNCol;nc++) {
      sel_hirow[nc]=kFALSE;
      sel_lorow[nc]=kFALSE;
      if (shADC_pedsub[nr1*shNCol+nc]>20 && shADC_pedsub[nr2*shNCol+nc]>20 && shADC_pedsub[nr1*shNCol+nc]<600 && shADC_pedsub[nr2*shNCol+nc]<600) {
            sel_hirow[nc]=kTRUE;
            sel_lorow[nc]=kTRUE;
      }}
   for (Int_t nr=nr2;nr<nr1+1;nr++) {
      for (Int_t nc=0;nc<shNCol;nc++) {
	Int_t nelem=nr*shNCol+nc;
	if (sel_hirow[nc] && sel_lorow[nc]&&shADC_pedsub[nelem]>15) h_shADC_pedsub_cut[nr][nc]->Fill(shADC_pedsub[nelem]);
      }}
   */
   //cout << " hi row = "<< select_event1 << " lo row " << select_event2 << " sel_col = " << sel_col+1 << endl;
   nc1=4;nr1=20;nc2=4;nr2=21;
   h_row21_col4_row20_col4->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
   nc1=4;nr1=20;nc2=4;nr2=19;
   if (shADC_pedsub[(21-1)*shNCol+4-1]>300) h_row19_col4_row20_col4->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
   nc1=3;nr1=21;nc2=3;nr2=20;
   h_row21_col3_row20_col3->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
   nc1=5;nr1=21;nc2=5;nr2=20;
   h_row21_col5_row20_col5->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
   nc1=6;nr1=21;nc2=6;nr2=20;
   h_row21_col6_row20_col6->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
   nc1=7;nr1=21;nc2=7;nr2=20;
   h_row21_col7_row20_col7->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
   nc1=2;nr1=21;nc2=2;nr2=20;
    h_row21_col2_row20_col2->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
   nc1=1;nr1=21;nc2=1;nr2=20;
    h_row21_col1_row20_col1->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
   nc1=4;nr1=26;nc2=4;nr2=27;
   h_row27_col4_row26_col4->Fill(shADC_pedsub[(nr1-1)*shNCol+nc1-1],shADC_pedsub[(nr2-1)*shNCol+nc2-1]);   
    //
   Double_t eff_thres=5.;
   for (Int_t nr=0;nr<shNRow;nr++) {
      for (Int_t nc=0;nc<shNCol;nc++) {
	Int_t nelem=nr*shNCol+nc;
	if (shADC_time[nelem] >0 ) {
	h_shADC[nr][nc]->Fill(shADC[nelem]);
	h_shADC_pedsub[nr][nc]->Fill(shADC_pedsub[nelem]);
	}
	Int_t nc_check=nc;
	if (nc==shNCol-1) nc_check=nc-1;
	if (nr==0) {
	  if ( (shADC_pedsub[(nr+2)*shNCol+nc_check]>eff_thres && shADC_pedsub[(nr+1)*shNCol+nc_check]>eff_thres && shADC_pedsub[(nr+2)*shNCol+nc_check+1]<eff_thres && shADC_pedsub[(nr+1)*shNCol+nc_check+1]<eff_thres) || ( shADC_pedsub[(nr+1)*shNCol+nc_check]>eff_thres && shADC_pedsub[(nr+3)*shNCol+nc_check]>eff_thres&& shADC_pedsub[(nr+1)*shNCol+nc_check+1]<eff_thres && shADC_pedsub[(nr+3)*shNCol+nc_check+1]<eff_thres) || ( shADC_pedsub[(nr+2)*shNCol+nc_check]>eff_thres && shADC_pedsub[(nr+3)*shNCol+nc_check]>eff_thres&& shADC_pedsub[(nr+2)*shNCol+nc_check+1]<eff_thres && shADC_pedsub[(nr+3)*shNCol+nc_check+1]<eff_thres) ) {
             h_shADC_pedsub_should[nr][nc]->Fill(shADC_pedsub[nelem]);
	     if (shADC_pedsub[nelem]>eff_thres) h_shADC_pedsub_did[nr][nc]->Fill(shADC_pedsub[nelem]);
	  }}
	if (nr==1) {
	  if ( (shADC_pedsub[(nr-1)*shNCol+nc]>eff_thres && shADC_pedsub[(nr+1)*shNCol+nc]>eff_thres && shADC_pedsub[(nr-1)*shNCol+nc+1]>eff_thres && shADC_pedsub[(nr+1)*shNCol+nc]>eff_thres) || (shADC_pedsub[(nr-1)*shNCol+nc]>eff_thres  && shADC_pedsub[(nr+2)*shNCol+nc]>eff_thres) || (shADC_pedsub[(nr+1)*shNCol+nc]>eff_thres  && shADC_pedsub[(nr+2)*shNCol+nc]>eff_thres)) {
             h_shADC_pedsub_should[nr][nc]->Fill(shADC_pedsub[nelem]);
	     if (shADC_pedsub[nelem]>eff_thres) h_shADC_pedsub_did[nr][nc]->Fill(shADC_pedsub[nelem]);
	  }}
	if (nr>1    && nr <shNRow-2) {
	  if ( (shADC_pedsub[(nr-1)*shNCol+nc]>eff_thres && shADC_pedsub[(nr+1)*shNCol+nc]>eff_thres) || (shADC_pedsub[(nr-2)*shNCol+nc]>eff_thres && shADC_pedsub[(nr+2)*shNCol+nc]>eff_thres)) {
             h_shADC_pedsub_should[nr][nc]->Fill(shADC_pedsub[nelem]);
	     if (shADC_pedsub[nelem]>eff_thres) h_shADC_pedsub_did[nr][nc]->Fill(shADC_pedsub[nelem]);
	  }}
	if (nr ==shNRow-2) {
	  if (shADC_pedsub[(nr-1)*shNCol+nc]>eff_thres && shADC_pedsub[(nr-2)*shNCol+nc]>eff_thres && shADC_pedsub[(nr+1)*shNCol+nc]>eff_thres) {
             h_shADC_pedsub_should[nr][nc]->Fill(shADC_pedsub[nelem]);
	     if (shADC_pedsub[nelem]>eff_thres) h_shADC_pedsub_did[nr][nc]->Fill(shADC_pedsub[nelem]);
	  }}	
	if (nr ==shNRow-1) {
	  if (shADC_pedsub[(nr-1)*shNCol+nc]>eff_thres && shADC_pedsub[(nr-2)*shNCol+nc]>eff_thres && shADC_pedsub[(nr-3)*shNCol+nc]>eff_thres) {
             h_shADC_pedsub_should[nr][nc]->Fill(shADC_pedsub[nelem]);
	     if (shADC_pedsub[nelem]>eff_thres) h_shADC_pedsub_did[nr][nc]->Fill(shADC_pedsub[nelem]);
	  }}	
	  if (show_track&& select_event1 && select_event2&&shADC_pedsub[nelem]>20) h_track->Fill(float(nc+1),float(nr+1),shADC_pedsub[nelem]);
      }
    }
    if (show_track) {
      if (select_event1&&select_event2) {
      cout << "select track " << i << endl;
      can2d->cd(1);
      h_track->SetTitle(Form("Evt %d", i));
      h_track->Draw("colz, text");
      can2d->Update();
      can2d->WaitPrimitive();
      }  else {
	//cout << "do not select track " << i << endl;
      /*
      can2d->cd(1);
      h_track->SetTitle(Form("Evt %d", i));
      h_track->Draw("colz, text");
      can2d->Update();
      can2d->WaitPrimitive();	
      */
      }
    }
  }   
	//
  TH2F* should_xy = new TH2F("Should_xy","Number of SHould hits; Ncol ; NRow",7,1,8,27,1,28);
  HList.Add(should_xy);
  TH2F* did_xy = new TH2F("Did_xy","Number of Did hits; Ncol ; NRow",7,1,8,27,1,28);
  HList.Add(did_xy);
  
  TH2F* eff_xy = new TH2F("eff_xy","Efficiency  ; Ncol ; NRow",7,1,8,27,1,28);
  HList.Add(eff_xy);

   for (Int_t nr=0;nr<shNRow;nr++) {
      for (Int_t nc=0;nc<shNCol;nc++) {
	should_xy->Fill(float(nc+1),float(nr+1),float(h_shADC_pedsub_should[nr][nc]->Integral()));
			did_xy->Fill(float(nc+1),float(nr+1),float(h_shADC_pedsub_did[nr][nc]->Integral()));
			Double_t eff=0.;
			if (h_shADC_pedsub_should[nr][nc]->Integral() > 10) eff=h_shADC_pedsub_did[nr][nc]->Integral()/h_shADC_pedsub_should[nr][nc]->Integral();
   eff_xy->Fill(float(nc+1),float(nr+1),eff);
    if (eff < .90) cout << " inefficient block nr,nc = " << nr+1 << " " << nc+1 << " " << h_shADC_pedsub_should[nr][nc]->Integral() << " " << h_shADC_pedsub_did[nr][nc]->Integral() << " " << eff << endl;
      }}
   /*
   TCanvas* caneff;
   caneff= new TCanvas(Form("can_%d",0),Form("Efficiency plot "),700,700);
   caneff->Divide(3,1);
   caneff->cd(1);
   should_xy->Draw("colz");
   caneff->cd(2);
   did_xy->Draw("colz");
   caneff->cd(3);
   eff_xy->SetMaximum(1.05);
   eff_xy->SetMinimum(0.85);
   eff_xy->Draw("colz");
   */
  TFile hsimc(outputhist,"recreate");
  HList.Write();
  //
}
