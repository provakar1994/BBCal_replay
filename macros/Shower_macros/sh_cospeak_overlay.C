/*
  This macro overlays signal amplitude distribution peaks from two cosmic runs (generated
  be bbsh_cos_cal.C macro) for visual comparison of any shift in the peak. To execute, do:
  -----
  [a-onl@aonl2 macros]$ root -l
  root [0] .x Shower_macros/sh_cospeak_overlay.C(<nrun1>,<nrun2>)
  -----
  P. Datta <pdbforce@jlab.org> Created 11-13-21  
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <TH2F.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TStopwatch.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TError.h>
#include "fadc_data.h"
using namespace std;

// Detector geometry
const int kNrows = 27;
const int kNcols = 7;

TH1F *h1[kNrows*kNcols];
TH1F *h2[kNrows*kNcols];
TLegend *ll[kNrows*kNcols];
TText *t1[kNrows*kNcols];
TText *t2[kNrows*kNcols];

TCanvas *subCanv[4];
const Int_t kCanvSize = 100;
namespace shgui {
  TGMainFrame *main = 0;
  TGHorizontalFrame *frame1 = 0;
  TGTab *fTab;
  TGLayoutHints *fL3;
  TGCompositeFrame *tf;
  TGTextButton *exitButton;
  // TGTextButton *displayEntryButton;
  // TGTextButton *displayNextButton;
  TGNumberEntry *entryInput;
  TGLabel *runLabel;

  TRootEmbeddedCanvas *canv[4];

  TGCompositeFrame* AddTabSub(Int_t sub) {
    tf = fTab->AddTab(Form("SH Sub%d",sub+1));

    TGCompositeFrame *fF5 = new TGCompositeFrame(tf, (12+1)*kCanvSize,(6+1)*kCanvSize , kHorizontalFrame);
    TGLayoutHints *fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
					   kLHintsExpandY, 5, 5, 5, 5);
    TRootEmbeddedCanvas *fEc1 = new TRootEmbeddedCanvas(Form("shSubCanv%d",sub), fF5, 6*kCanvSize,8*kCanvSize);
    //TRootEmbeddedCanvas *fEc1 = new TRootEmbeddedCanvas(0, fF5, 600, 600);
    //Int_t wid = fEc1->GetCanvasWindowId();
    //subCanv[sub] = new TCanvas(Form("subCanv%d",sub),10,10,wid);
    //subCanv[sub]->Divide(12,6);
    //fEc1->AdoptCanvas(subCanv[sub]);
    canv[sub] = fEc1;
    fF5->AddFrame(fEc1,fL4);
    tf->AddFrame(fF5,fL4);
    return tf;
  }

  void SetupGUI() {
    if(!main) {
      main = new TGMainFrame(gClient->GetRoot(), 1200, 1100);
      frame1 = new TGHorizontalFrame(main, 150, 20, kFixedWidth);
      runLabel = new TGLabel(frame1,"Run ");
      //displayEntryButton = new TGTextButton(frame1,"&Display Entry","clicked_displayEntryButton()");
      //entryInput = new TGNumberEntry(frame1,0,5,-1,TGNumberFormat::kNESInteger);
      //displayNextButton = new TGTextButton(frame1,"&Next Entry","clicked_displayNextButton()");
      exitButton = new TGTextButton(frame1, "&Exit", 
				    "gApplication->Terminate(0)");
      TGLayoutHints *frame1LH = new TGLayoutHints(kLHintsTop|kLHintsLeft|
						  kLHintsExpandX,2,2,2,2);
      //frame1->AddFrame(runLabel,frame1LH);
      //frame1->AddFrame(displayEntryButton,frame1LH); 
      //frame1->AddFrame(entryInput,frame1LH);
      //frame1->AddFrame(displayNextButton,frame1LH);
      frame1->AddFrame(exitButton,frame1LH);
      //frame1->Resize(800, displayNextButton->GetDefaultHeight());
      main->AddFrame(frame1, new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1));

      // Create the tab widget
      fTab = new TGTab(main, 300, 300);
      fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

      // Create Tab1 (SH Sub1)
      for(Int_t i = 0; i < 4; i++) {
        tf = AddTabSub(i);
      }
      main->AddFrame(fTab, new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
					     kLHintsExpandY, 2, 2, 5, 1));
      main->MapSubwindows();
      main->Resize();   // resize to default size
      main->MapWindow();

      for(Int_t i = 0; i < 4; i++) {
        subCanv[i] = canv[i]->GetCanvas();
	subCanv[i]->Divide(kNcols,7,0.001,0.001);
      }
    }
  }
};

void sh_cospeak_overlay(int nrun1 = 100, int nrun2 = 200){

  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings 

  shgui::SetupGUI();

  TFile *f1 = new TFile(Form("hist/run_%d_sh_peak_Trigger.root",nrun1),"READ");
  TFile *f2 = new TFile(Form("hist/run_%d_sh_peak_Trigger.root",nrun2),"READ");

  TString OutF_peaks;
  OutF_peaks = Form("plots/SH_cos_run_overlay_%d_and_%d.pdf",nrun1,nrun2);
 
  int sub=0;
  TString hist_name;
  for(int r=0; r<kNrows; r++){
    for(int c=0; c<kNcols; c++){
      int block = r*kNcols+c;
      hist_name = Form("h_R%d_C%d_Blk%02d_i",r+1,c+1,block);

      f1->GetObject(hist_name,h1[block]);
      f2->GetObject(hist_name,h2[block]);

      sub = r/7;
      subCanv[sub]->cd((r%7)*kNcols + c + 1);

      h1[block]->SetLineColor(1); h1[block]->SetLineWidth(1);
      h2[block]->SetLineColor(4); h2[block]->SetLineWidth(1);
      h1[block]->Draw(); h2[block]->Draw("same");

      ll[block] = new TLegend(0.1,0.7,0.48,0.9);
      ll[block]->AddEntry(h1[block],Form("%d",nrun1));
      ll[block]->AddEntry(h2[block],Form("%d",nrun2));
      ll[block]->SetBorderSize(0); ll[block]->SetFillStyle(0);
      ll[block]->Draw("same");
    }
  }

  subCanv[0]->SaveAs(Form("%s[",OutF_peaks.Data()));
  for( int canC=0; canC<4; canC++ ) subCanv[canC]->SaveAs(Form("%s",OutF_peaks.Data()));
  subCanv[3]->SaveAs(Form("%s]",OutF_peaks.Data()));

  cout << " --------- " << endl;
  cout << " Overlaid peaks saved to : " << OutF_peaks << endl;
  cout << " --------- " << endl;
}
