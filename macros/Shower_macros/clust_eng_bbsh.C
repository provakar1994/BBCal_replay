// - P. Datta

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
#include "gmn_tree.C"

// Detector parameters
const int kNrows = 27;
const int kNcols = 7;

gmn_tree *T;
int gCurrentEntry = -1;
TCanvas *subCanv[4];
bool E_or_EovP = 1; // 0=E, 1=EovP

// Declare necessary histograms
TH1F *hClusteng[kNrows][kNcols];
TH1F *hResolution[kNrows][kNcols];
// int hClusteng_bin = 175;
// double hClusteng_min = 0.5;
// double hClusteng_max = 3.0;


// Declare necessary functions
string getDate();
TH1F* MakeHisto( Int_t, Int_t, Int_t, const char*, Double_t, Double_t );
void processEvent( int );

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
  // TGLabel *ledLabel;

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
      //ledLabel = new TGLabel(frame1,"LED Bit:    , Count:      ");
      //displayEntryButton = new TGTextButton(frame1,"&Display Entry","clicked_displayEntryButton()");
      //entryInput = new TGNumberEntry(frame1,0,5,-1,TGNumberFormat::kNESInteger);
      //displayNextButton = new TGTextButton(frame1,"&Next Entry","clicked_displayNextButton()");
      exitButton = new TGTextButton(frame1, "&Exit", 
          "gApplication->Terminate(0)");
      TGLayoutHints *frame1LH = new TGLayoutHints(kLHintsTop|kLHintsLeft|
          kLHintsExpandX,2,2,2,2);
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

// Main
void clust_eng_bbsh(const char *configfilename){

  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  shgui::SetupGUI();
  gStyle->SetLabelSize(0.08,"XY");
  gStyle->SetTitleFontSize(0.08);
  string date = getDate();

  TChain *C = new TChain("T");
  T = new gmn_tree(C);

  Int_t Multiple_runs=0;
  Int_t fst_run=11994, lst_run=11994;
  Int_t nBins=175;
  Double_t h_min=0., h_max=5.;

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
      if( skey == "E_or_EovP" ){
  	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
  	E_or_EovP = sval.Atoi();
      }
    } 
    delete tokens;
  }

  TString OutPeaks;
  if(!Multiple_runs){
    if(E_or_EovP){
      OutPeaks = Form("plots/bbcal_EovP_p_shblk_%d.pdf",fst_run);
    }else{
      OutPeaks = Form("plots/bbcal_clusEng_p_shblk_%d.pdf",fst_run);
    }
  }else{
    if(E_or_EovP){
      OutPeaks = Form("plots/bbcal_EovP_p_shblk_%d_%d.pdf",fst_run,lst_run);
    }else{
      OutPeaks = Form("plots/bbcal_clusEng_p_shblk_%d_%d.pdf",fst_run,lst_run);
    }
  }

  //TFile *fout = new TFile(Form("clust_eng_p_blk_%d.root",run),"RECREATE");

  for(int r = 0; r < kNrows; r++) {
    for(int c = 0; c < kNcols; c++) {
      hClusteng[r][c] = MakeHisto(r, c, nBins, "_i", h_min, h_max);
      hResolution[r][c] = MakeHisto(r, c, nBins, "_j", h_min, h_max);
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

  int sub = 0;
  for(int r=0; r<kNrows; r++){
    for(int c=0; c<kNcols; c++){
      sub = r/7;
      subCanv[sub]->cd((r%7)*kNcols + c + 1);
 
      if(E_or_EovP){
	hResolution[r][c]->SetTitle(Form("Clust_eng/tr_p | %d-%d",r+1,c+1));
	hResolution[r][c]->Draw();
      }else{
	hClusteng[r][c]->SetTitle(Form("Clust Eng. | %d-%d",r+1,c+1));
	hClusteng[r][c]->Draw();
      }
   }
  }

  subCanv[0]->SaveAs(Form("%s[",OutPeaks.Data()));
  for( int canC=0; canC<4; canC++ ) subCanv[canC]->SaveAs(Form("%s",OutPeaks.Data()));
  subCanv[3]->SaveAs(Form("%s]",OutPeaks.Data()));
  
  // Post analysis reporting
  cout << "Finishing analysis .." << endl;
  cout << " --------- " << endl;
  cout << " Plots saved to : " << OutPeaks << endl;
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

  Double_t ClusEng = T->bb_sh_e+T->bb_ps_e;
  Double_t Resolution = (T->bb_sh_e+T->bb_ps_e)/T->bb_tr_p[0];

  if(E_or_EovP){
    if( T->bb_sh_nclus>0&&T->bb_ps_nclus>0&&T->bb_ps_idblk!=-1
	&& T->bb_ps_e>0.1 // 100 MeV
	&& T->bb_tr_tg_th[0]>-0.15 && T->bb_tr_tg_th[0]<0.15 
	&& T->bb_tr_tg_ph[0]>-0.3 && T->bb_tr_tg_ph[0]<0.3 )
      {
	hResolution[(int)T->bb_sh_rowblk][(int)T->bb_sh_colblk]
	  ->Fill( Resolution );
      }
  }else{

    if( T->bb_sh_nclus>0&&T->bb_ps_nclus>0&&T->bb_ps_idblk!=-1
	&& T->bb_ps_e>0.1 ){  // 100 MeV

      hClusteng[ (int)T->bb_sh_rowblk ][ (int)T->bb_sh_colblk ]
	->Fill( ClusEng );
    }
  }

} //processEvent

