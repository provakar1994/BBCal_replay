/*
  ADC time alignment script using cosmic data:
  This script plots the ADC time for individual SH blocks, fits them using Gaussian and
  calculates the ADC time offsets w.r.t. to a target ADC time value provided by the user.
  ------
  P. Datta <pdbforce@jlab.org> Created 09-19-2022
*/

#include <TH2F.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TStopwatch.h>
#include <TGraph.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "cosmic_tree.C"

// Detector parameters
const int kNrows = 27;
const int kNcols = 7;
const int kNblks = kNrows*kNcols;

cosmic_tree *T;
int gCurrentEntry = -1;

// Declare necessary histograms
TH1F *h_atime[kNrows][kNcols];
TH1F *hResolution[kNrows][kNcols];
// int h_atime_bin = 175;
// double h_atime_min = 0.5;
// double h_atime_max = 3.0;


// Declare necessary functions
string getDate();
TH1F* MakeHisto( Int_t, Int_t, Int_t, const char*, Double_t, Double_t );
void processEvent( int );

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
      frame1->AddFrame(runLabel,frame1LH);
      //frame1->AddFrame(displayEntryButton,frame1LH);
      //frame1->AddFrame(entryInput,frame1LH);
      //frame1->AddFrame(displayNextButton,frame1LH);
      frame1->AddFrame(exitButton,frame1LH);
      //frame1->Resize(800, displayNextButton->GetDefaultHeight());
      main->AddFrame(frame1, new TGLayoutHints(kLHintsBottom | 
					       kLHintsRight, 2, 2, 5, 1));

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

// Main
void sh_atime_align_cosmic (const char *configfilename){

  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  shgui::SetupGUI();
  gStyle->SetLabelSize(0.08,"XY");
  gStyle->SetTitleFontSize(0.08);
  string date = getDate();

  TChain *C = new TChain("T");
  T = new cosmic_tree(C);

  Int_t Multiple_runs=0;
  Int_t fst_run=11994, lst_run=11994;
  Int_t nBins=120;
  Double_t h_min=0., h_max=60.;
  Double_t target_atime=40.; // ns, Target ADC time for alignment

  // Reading config file
  ifstream configfile(configfilename);
  TString currentline;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endlist") ){
    if( !currentline.BeginsWith("#") ){
      C->Add(currentline);
    }   
  }
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("#") ){
    TObjArray *tokens = currentline.Tokenize(" ");
    Int_t ntokens = tokens->GetEntries();
    if( ntokens>1 ){
      TString skey = ( (TObjString*)(*tokens)[0] )->GetString();
      if( skey == "Multiple_runs" ){
  	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
  	Multiple_runs = sval.Atoi();
      }
      if( skey == "fst_run" ){
  	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
  	fst_run = sval.Atoi();
      }
      if( skey == "lst_run" ){
  	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
  	lst_run = sval.Atoi();
      }
      if( skey == "target_atime" ){
  	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
  	target_atime = sval.Atof();
      }
      if( skey == "nBins" ){
  	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
  	nBins = sval.Atoi();
      }
      if( skey == "h_min" ){
  	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
  	h_min = sval.Atof();
      }
      if( skey == "h_max" ){
  	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
  	h_max = sval.Atof();
      }
    } 
    delete tokens;
  }

  TString OutPeaks, outFile, outAtimeOffset;
  if(!Multiple_runs){
    OutPeaks = Form("plots/sh_atime_align_cosmic_%d.pdf",fst_run);
    outFile = Form("hist/sh_atime_align_cosmic_%d.root",fst_run);
    outAtimeOffset = Form("Output/sh_atime_cosmic_offset_target_%0.1fns_%d.txt",target_atime,fst_run);
  }else{
    OutPeaks = Form("plots/sh_atime_align_cosmic_%d_%d.pdf",fst_run,lst_run);
    outFile = Form("hist/sh_atime_align_cosmic_%d_%d.root",fst_run,lst_run);
    outAtimeOffset = Form("Output/sh_atime_cosmic_offset_target_%0.1fns_%d_%d.txt",target_atime,fst_run,lst_run);
  }

  for(int r = 0; r < kNrows; r++) {
    for(int c = 0; c < kNcols; c++) {
      h_atime[r][c] = MakeHisto(r, c, nBins, "_i", h_min, h_max);
    }
  }

  Long64_t nevents = C->GetEntries();
  cout << endl << "Processing " << nevents << " events ...." << endl;

  // Looping through events
  double progress = 0.;
  while(progress<1.0){
    int barwidth = 70;
    for (int nev = 0; nev < nevents; nev++){ 
      processEvent( nev );
      
      cout << "[";
      int pos = barwidth * progress;
      for(int i=0; i<barwidth; ++i){
    	if(i<pos) cout << "=";
    	else if(i==pos) cout << ">";
    	else cout << " ";
      }
      progress = (double)((nev+1.)/nevents);
      cout << "] " << int(progress*100.) << "%\r";
      cout.flush();
    }
  }
  cout << endl << endl;

  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();
  TH1F *h_atimePeakpos = new TH1F("h_atimePeakpos","ADC time peak pos (ns) vs SH blocks; SH blocks",kNblks,0,kNblks);
  h_atimePeakpos->GetYaxis()->SetRangeUser(30.,50.);
  h_atimePeakpos->GetYaxis()->SetLabelSize(0.045);
  h_atimePeakpos->GetXaxis()->SetLabelSize(0.045);
  h_atimePeakpos->SetLineWidth(0);
  h_atimePeakpos->SetMarkerStyle(kFullCircle);

  // Let's fit the histograms with Gauss
  ofstream atimeOffset;
  atimeOffset.open(outAtimeOffset);

  TF1 *fgaus = new TF1("fgaus","gaus");

  int sub = 0;
  for(int r=0; r<kNrows; r++){
    for(int c=0; c<kNcols; c++){
      sub = r/7;
      subCanv[sub]->cd((r%7)*kNcols + c + 1);

      int lowerBinC = h_min;
      int upperBinC = h_max;
      int maxBin = h_atime[r][c]->GetMaximumBin();
      double maxBinCenter = h_atime[r][c]->GetXaxis()->GetBinCenter( maxBin );
      double maxCount = h_atime[r][c]->GetMaximum();
      double binWidth = h_atime[r][c]->GetBinWidth(maxBin);
      double stdDev = h_atime[r][c]->GetStdDev();

      if( h_atime[r][c]->GetEntries()>100 && stdDev>2.*binWidth){

      	// Create fit functions for each module
      	fgaus->SetLineColor(1);
      	fgaus->SetNpx(1000);

      	if( h_atime[r][c]->GetBinContent(maxBin-1) == 0. ){
      	  while ( h_atime[r][c]->GetBinContent(maxBin+1) < h_atime[r][c]->GetBinContent(maxBin) || 
      		  h_atime[r][c]->GetBinContent(maxBin+1) == h_atime[r][c]->GetBinContent(maxBin) ) 
      	    {
      	      maxBin++;
      	    };
      	  h_atime[r][c]->GetXaxis()->SetRange( maxBin+1 , h_atime[r][c]->GetNbinsX() );
      	  maxBin = h_atime[r][c]->GetMaximumBin();
      	  maxBinCenter = h_atime[r][c]->GetXaxis()->GetBinCenter( maxBin );
      	  maxCount = h_atime[r][c]->GetMaximum();
      	  binWidth = h_atime[r][c]->GetBinWidth(maxBin);
      	  stdDev = h_atime[r][c]->GetStdDev();
      	}

      	// Frist fit
      	fgaus->SetParameters( maxCount,maxBinCenter,stdDev );
      	fgaus->SetRange( h_min, h_max );
      	h_atime[r][c]->Fit(fgaus,"+RQ");

      	h_atimePeakpos->Fill( r*kNcols+c, fgaus->GetParameter(1) );
	// generating offsets
	atimeOffset << (target_atime - fgaus->GetParameter(1)) << " ";

      }

      h_atime[r][c]->SetTitle(Form("ADC time(ns) | %d-%d",r+1,c));
      h_atime[r][c]->Draw();
    }
    atimeOffset << endl;
  }

  subCanv[0]->SaveAs(Form("%s[",OutPeaks.Data()));
  for( int canC=0; canC<4; canC++ ) subCanv[canC]->SaveAs(Form("%s",OutPeaks.Data()));
  subCanv[3]->SaveAs(Form("%s]",OutPeaks.Data()));

  fout->Write();
  fout->Close();
  fout->Delete();
  
  // Post analysis reporting
  cout << "Finishing analysis .." << endl;
  cout << " --------- " << endl;
  cout << Form(" ADC time offsets w.r.t. target_atime, %0.1f saved to : %s ", target_atime, outAtimeOffset.Data()) << endl;
  cout << " Plots saved to : " << OutPeaks << endl;
  cout << " Histogram saved to : " << outFile << endl;
  cout << " --------- " << endl;
  
} //main



// Get today's date
string getDate(){
  time_t now = time(0);
  tm ltm = *localtime(&now);

  string yyyy = to_string(1900 + ltm.tm_year);
  string mm = to_string(1 + ltm.tm_mon);
  string dd = to_string(ltm.tm_mday);
  string date = mm + '_' + dd + '_' + yyyy;

  return date;
} // getDate

// Create generic histogram function
TH1F* MakeHisto(Int_t row, Int_t col, Int_t bins, const char* suf="", Double_t min=0., Double_t max=50.)
{
  TH1F *h = new TH1F(TString::Format("h_R%d_C%d_Blk_%d%s",row,col,row*kNcols+col,suf),
		     "",bins,min,max);
  //h->SetStats(0);
  h->SetLineWidth(1);
  h->SetLineColor(2);
  h->GetYaxis()->SetLabelSize(0.06);
  //h->GetYaxis()->SetLabelOffset(-0.17);
  h->GetYaxis()->SetNdivisions(5);
  return h;
}

// Process events
void processEvent( int entry = -1 ){

  if(entry == -1) {
    gCurrentEntry++;
  } else {
    gCurrentEntry = entry;
  }
  
  if(gCurrentEntry<0) {
    gCurrentEntry = 0;
  }

  // Get the event from the TTree
  T->GetEntry(gCurrentEntry);

  int nhit = T->Ndata_bb_sh_a_time;
  for (int ihit=0; ihit<nhit; ihit++) {
    int shrow = T->bb_sh_adcrow[ihit];
    int shcol = T->bb_sh_adccol[ihit];
    double atime = T->bb_sh_a_time[ihit];
    h_atime[shrow][shcol]->Fill(atime);
  }
  

} //processEvent

