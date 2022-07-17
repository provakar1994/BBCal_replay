#include <TH2.h>
#include <TH2F.h>
#include <TChain.h>
#include <TCanvas.h>
#include <vector>
#include <iostream>
#include <TSystem.h>
#include "fadc_data.h"
#include "gmn_tree.C"

const Int_t kNrows = 27;
const Int_t kNcols = 7;
const Int_t kNrowsPS = 26;
const Int_t kNcolsPS = 2;

const Double_t zposSH = 1.901952; //m
const Double_t zposPS = 1.695704; //m

const Int_t kCanvSize = 100;
std::string user_input;
Int_t gCurrentEntry = -1;

gmn_tree *T;
Int_t foundModules = 0;
TCanvas *canvas = 0;
TCanvas *subCanv[4];


void clicked_displayEntryButton();
void clicked_displayNextButton();
namespace shgui {
  TGMainFrame *main = 0;
  TGHorizontalFrame *frame1 = 0;
  TGTab *fTab;
  TGLayoutHints *fL3;
  TGCompositeFrame *tf;
  TGTextButton *exitButton;
  TGTextButton *displayEntryButton;
  TGTextButton *displayNextButton;
  TGNumberEntry *entryInput;
  TGLabel *ledLabel;

  TRootEmbeddedCanvas *canv[1];

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
      main = new TGMainFrame(gClient->GetRoot(), 1000, 900);
      frame1 = new TGHorizontalFrame(main, 150, 20, kFixedWidth);
      ledLabel = new TGLabel(frame1,"Run #: ");
      displayEntryButton = new TGTextButton(frame1,"&Display Entry","clicked_displayEntryButton()");
      entryInput = new TGNumberEntry(frame1,0,5,-1,TGNumberFormat::kNESInteger);
      displayNextButton = new TGTextButton(frame1,"&Next Entry","clicked_displayNextButton()");
      exitButton = new TGTextButton(frame1, "&Exit", 
				    "gApplication->Terminate(0)");
      TGLayoutHints *frame1LH = new TGLayoutHints(kLHintsTop|kLHintsLeft|
						  kLHintsExpandX,2,2,2,2);
      frame1->AddFrame(ledLabel,frame1LH);
      frame1->AddFrame(displayEntryButton,frame1LH);
      frame1->AddFrame(entryInput,frame1LH);
      frame1->AddFrame(displayNextButton,frame1LH);
      frame1->AddFrame(exitButton,frame1LH);
      frame1->Resize(800, displayNextButton->GetDefaultHeight());
      main->AddFrame(frame1, new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1));

      // Create the tab widget
      fTab = new TGTab(main, 300, 300);
      fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

      // Create Tab1 (SH Sub1)
      for(Int_t i = 0; i < 5; i++) {
        tf = AddTabSub(i);
      }
      main->AddFrame(fTab, new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
					     kLHintsExpandY, 2, 2, 5, 1));
      main->MapSubwindows();
      main->Resize();   // resize to default size
      main->MapWindow();

      for(Int_t i = 0; i < 5; i++) {
        subCanv[i] = canv[i]->GetCanvas();
      	// if( kNrows<12 || kNcols<12) {
      	//   //subCanv[i]->Divide(kNrows,kNcols,0.001,0.001);
      	//   subCanv[i]->Divide(kNcols,kNrows,0.001,0.001);
        // } else {
      	//   subCanv[i]->Divide(12,6,0.001,0.001);
        // }
      	subCanv[i]->Divide(2,1,0.001,0.001);
      }
    }
  }
};

Double_t nhit = 0;
TH1F *histos[kNrows][kNcols];
TH2F* hSH_int = new TH2F("sh_int","Shower(a_p) ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
TH2F* hSH_intEng = new TH2F("sh_intE","Shower(a_c) ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
TH2F* hSH_clus_e = new TH2F("sh_clus_e","Shower Cluster ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
TH2F* hSH_xypos = new TH2F("sh_cl_tr","SH Clust. + Track Pos. ; Shower y (m) ; Shower x (m)",kNcols,-0.2992,0.2992,kNrows,-1.1542,1.1542);
TH2F* hPS_int = new TH2F("ps_int","PreShower(a_p) ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);
TH2F* hPS_intEng = new TH2F("ps_intE","PreShower(a_c) ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);
TH2F* hPS_clus_e = new TH2F("ps_clus_e","PreShower Cluster ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);
TH2F* hPS_xypos = new TH2F("ps_cl_tr","PS Clust. + Track Pos.; PreShower y (m) ; PreShower x (m)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

bool is_number(const std::string& mystring)
{
  std::string::const_iterator it = mystring.begin();
  while (it != mystring.end() && std::isdigit(*it)) ++it;
  return !mystring.empty() && it == mystring.end();
}

void displayEvent(Int_t entry = -1, Int_t run = 7 )
{
  if(entry == -1) {
    gCurrentEntry++;
  } else {
    gCurrentEntry = entry;
  }

  if(gCurrentEntry<0) {
    gCurrentEntry = 0;
  }

  T->GetEntry(gCurrentEntry);
  cout << endl;
  std::cout << "Displaying event " << gCurrentEntry << std::endl;
  shgui::ledLabel->SetText(TString::Format("Run #: %d",run));

  // Clear old histograms, just in case modules are not in the tree
  hSH_int->Reset("ICES M");
  hSH_intEng->Reset("ICES M");
  hPS_int->Reset("ICES M");
  hPS_intEng->Reset("ICES M");
  //Clustering
  hSH_clus_e->Reset("ICES M");
  hSH_xypos->Reset("ICES M");
  hPS_clus_e->Reset("ICES M");
  hPS_xypos->Reset("ICES M");

  // Shower
  for(Int_t m = 0; m < T->Ndata_bb_sh_adcrow; m++) {
    int r = T->bb_sh_adcrow[m];
    int c = T->bb_sh_adccol[m];
    if(r < 0 || c < 0) {
      std::cerr << "Why is row negative? Or col?" << std::endl;
      continue;
    }
    if(r>= kNrows || c >= kNcols)
      continue;
    if( T->bb_sh_a_time[m]>0 ){
     hSH_int->Fill( double(c+1), double(r+1), T->bb_sh_a_p[m] );
     cout << "r " << r << " c= " << c <<  "a_c= " << T->bb_sh_a_c[m] << endl;								     
     hSH_intEng->Fill( double(c+1), double(r+1), T->bb_sh_a_c[m] );     
    }
  }

  // for(int ihit=0;ihit<T->Ndata_bb_sh_a_p;ihit++){
  //   int r = T->bb_sh_adcrow[ihit];
  //   int c = T->bb_sh_adcrow[ihit];
  //   if(r < 0 || c < 0) {
  //     std::cerr << "Why is row negative? Or col?" << std::endl;
  //     continue;
  //   }
  //   if(r>= kNrows || c >= kNcols)
  //     continue;
  //   hSH_int->Fill( double(c+1),double(r+1), T->bb_sh_a_p[ihit] );
  //   hSH_intEng->Fill( double(c+1),double(r+1), T->bb_sh_a_c[ihit] ); 

  //   cout << " ***Ndata= " << T->Ndata_bb_sh_a_p << " a_c= " << T->bb_sh_a_c[ihit] << " a_p= " <<  T->bb_sh_a_p[ihit] << endl;
  // }
  
  // SH Clustering
  cout << "Number of SH clusters in this event= " << T->bb_sh_nclus << endl;
  //cout << "Clus xpos= " << T->bb_sh_x << " ypos= " << T->bb_sh_y << endl;
  cout << "HE blkID (idblk) = " << T->bb_sh_idblk << "  " << "HE blk Row,Col (rowblk+1,colblk+1)= " << T->bb_sh_rowblk+1 << "," << T->bb_sh_colblk+1 << endl;
  //cout << "HE Blk e_c= " << T->bb_sh_eblk_c << "  " << "HE Blk xpos,ypos= " << T->bb_sh_x << "," << T->bb_sh_y << endl;
  for( int cl=0; cl<(int)T->bb_sh_nclus; cl++ ){
    cout << "Clus ID " << T->bb_sh_clus_id[cl] << "\t" << "Clus e= " << T->bb_sh_clus_e[cl];
    cout << "\tNo. of blocks involved= " << T->bb_sh_clus_nblk[cl] << endl;
    int cID = (int)T->bb_sh_clus_id[cl];
    if( cID!=-1 ){
      int rCl = (int)T->bb_sh_clus_row[cl];
      int cCl = (int)T->bb_sh_clus_col[cl];
      hSH_clus_e->Fill( double(cCl+1), double(rCl+1), T->bb_sh_clus_e[cl] );
      if( cl==0 ){
	for( int b = 0; b < (int)T->bb_sh_clus_nblk[cl]; b++ ){
	  int cblkID = (int)T->bb_sh_clus_blk_id[b];
	  int rblkCl = (int)T->bb_sh_clus_blk_row[b];
	  int cblkCl = (int)T->bb_sh_clus_blk_col[b];
	  double xblkCl = T->bb_sh_clus_blk_x[b];
	  double yblkCl = T->bb_sh_clus_blk_y[b];
	  if(cblkID!=cID){
	    hSH_clus_e->Fill( double(cblkCl+1), double(rblkCl+1), T->bb_sh_clus_blk_e[b] );
	  }
	  hSH_xypos->Fill( yblkCl, xblkCl, T->bb_sh_clus_blk_e[b] );
	}
      }
    }
  }//

  cout << endl;
  
  // Pre-Shower
  for(Int_t m = 0; m < T->Ndata_bb_ps_adcrow; m++) {
    int r = T->bb_ps_adcrow[m];
    int c = T->bb_ps_adccol[m];
    if(r < 0 || c < 0) {
      std::cerr << "Why is row negative? Or col?" << std::endl;
      continue;
    }
    if(r>= kNrows || c >= kNcols)
      continue;
    if( T->bb_ps_a_time[m]>0 ){
      hPS_int->Fill( double(c+1),double(r+1), T->bb_ps_a_p[m] ); 
      hPS_intEng->Fill( double(c+1),double(r+1), T->bb_ps_a_c[m] );     
    }
  }

  // for(int ihit=0;ihit<T->Ndata_bb_ps_a_p;ihit++){
  //   int r = T->bb_ps_adcrow[ihit];
  //   int c = T->bb_ps_adcrow[ihit];
  //   if(r < 0 || c < 0) {
  //     std::cerr << "Why is row negative? Or col?" << std::endl;
  //     continue;
  //   }
  //   if(r>= kNrows || c >= kNcols)
  //     continue;
  //   hPS_int->Fill( double(c+1),double(r+1), T->bb_ps_a_p[ihit] );
  //   hPS_intEng->Fill( double(c+1),double(r+1), T->bb_ps_a_c[ihit] );     
  // }
  
  // PS Clustering
  cout << "Number of PS clusters in this event= " << T->bb_ps_nclus << endl;
  //cout << "Clus xpos= " << T->bb_ps_x << " ypos= " << T->bb_ps_y << endl;
  cout << "HE blkID (idblk) = " << T->bb_ps_idblk << "  " << "HE blk Row,Col (rowblk+1,colblk)= " << T->bb_ps_rowblk+1 << "," << T->bb_ps_colblk << endl;
  //cout << "HE Blk e_c= " << T->bb_ps_eblk_c << "  " << "HE Blk xpos,ypos= " << T->bb_ps_x << "," << T->bb_ps_y << endl;  
  for( int cl=0; cl<(int)T->bb_ps_nclus; cl++ ){
    //cout << "Clus ID " << T->bb_ps_clus_id[cl] << "\t" << "Clus e_c= " << T->bb_ps_clus_e_c[cl];
    //cout << "\tNo. of blocks involved= " << T->bb_ps_clus_nblk[cl] << endl;
    //cout << "Clus xpos= " << T->bb_ps_x << " ypos= " << T->bb_ps_y << endl;
    int cID = (int)T->bb_ps_clus_id[cl];
    if( cID!=-1 ){
      int rCl = (int)T->bb_ps_clus_row[cl];
      int cCl = (int)T->bb_ps_clus_col[cl];
      hPS_clus_e->Fill( double(cCl+1), double(rCl+1), T->bb_ps_clus_e[cl] );
      if( cl==0 ){
	for( int b = 0; b < (int)T->bb_ps_clus_nblk[cl]; b++ ){
	  int cblkID = (int)T->bb_ps_clus_blk_id[b];
	  int rblkCl = (int)T->bb_ps_clus_blk_row[b];
	  int cblkCl = (int)T->bb_ps_clus_blk_col[b];
	  double xblkCl = T->bb_ps_clus_blk_x[b];
	  double yblkCl = T->bb_ps_clus_blk_y[b];
	  if(cblkID!=cID){
	    hPS_clus_e->Fill( double(cblkCl+1), double(rblkCl+1), T->bb_ps_clus_blk_e[b] );
	  }
	  hPS_xypos->Fill( yblkCl, xblkCl, T->bb_ps_clus_blk_e[b] );
	}
      }
    }
  }
  
  // Cluster position with search radius (Drawing circle)
  double xClposSH = T->bb_sh_x;
  double yClposSH = T->bb_sh_y;
  TEllipse *ClposSH = new TEllipse(yClposSH,xClposSH,.15,.15);
  ClposSH->SetFillStyle(0);
  ClposSH->SetLineWidth(1);
  ClposSH->SetLineColor(kBlack);
  TEllipse *dotClSH = new TEllipse(yClposSH,xClposSH,.008,.008);
  dotClSH->SetFillStyle(1001);
  dotClSH->SetFillColor(1);

  // Tracking stuff (Drawing circle)
  double xTrATsh = T->bb_tr_x[0] + zposSH*T->bb_tr_th[0];
  double yTrATsh = T->bb_tr_y[0] + zposSH*T->bb_tr_ph[0];
  //TEllipse *trackPOSatSH = new TEllipse(yTrATsh,xTrATsh,.05,.05);
  TEllipse *trackPOSatSH = new TEllipse(yClposSH,xClposSH,.05,.05);
  trackPOSatSH->SetFillStyle(0);
  trackPOSatSH->SetLineWidth(2);
  trackPOSatSH->SetLineColor(kRed);
  TEllipse *dotSH = new TEllipse(yTrATsh,xTrATsh,.008,.008);
  dotSH->SetFillStyle(1001);
  dotSH->SetFillColor(2);

  double xTrATps = T->bb_tr_x[0] + zposPS*T->bb_tr_th[0];
  double yTrATps = T->bb_tr_y[0] + zposPS*T->bb_tr_ph[0];
  TEllipse *trackPOSatPS = new TEllipse(yTrATps,xTrATps,.05,.05);
  trackPOSatPS->SetFillStyle(0);
  trackPOSatPS->SetLineWidth(2);
  trackPOSatPS->SetLineColor(kRed);
  TEllipse *dotPS = new TEllipse(yTrATps,xTrATps,.008,.008);
  dotPS->SetFillStyle(1001);
  dotPS->SetFillColor(2);
  
  subCanv[0]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_xypos->SetStats(0);
  hSH_xypos->SetMaximum(0.5);
  hSH_xypos->SetMinimum(0); 
  hSH_xypos->GetYaxis()->SetNdivisions(kNrows);
  hSH_xypos->GetXaxis()->SetNdivisions(kNcols);
  hSH_xypos->Draw("text colz");
  dotSH->Draw("same");
  trackPOSatSH->Draw("same");
  dotClSH->Draw("same");
  ClposSH->Draw("same");
  gPad->Update();
  subCanv[0]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_xypos->SetStats(0);
  hPS_xypos->SetMaximum(0.5);
  hPS_xypos->SetMinimum(0); 
  hPS_xypos->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_xypos->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_xypos->Draw("text colz");
  dotPS->Draw("same");
  trackPOSatPS->Draw("same");
  gPad->Update();

  subCanv[1]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_clus_e->SetStats(0);
  hSH_clus_e->SetMaximum(1.5);
  hSH_clus_e->SetMinimum(0); 
  hSH_clus_e->GetYaxis()->SetNdivisions(kNrows);
  hSH_clus_e->GetXaxis()->SetNdivisions(kNcols);
  hSH_clus_e->Draw("text colz");
  gPad->Update();
  subCanv[1]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_clus_e->SetStats(0);
  hPS_clus_e->SetMaximum(1.5);
  hPS_clus_e->SetMinimum(0); 
  hPS_clus_e->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_clus_e->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_clus_e->Draw("text colz");
  gPad->Update();

  subCanv[2]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_intEng->SetStats(0);
  hSH_intEng->SetMaximum(0.5);
  hSH_intEng->SetMinimum(0); 
  hSH_intEng->GetYaxis()->SetNdivisions(kNrows);
  hSH_intEng->GetXaxis()->SetNdivisions(kNcols);
  hSH_intEng->Draw("text colz");
  gPad->Update();
  subCanv[2]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_clus_e->SetStats(0);
  hSH_clus_e->SetMaximum(1.5);
  hSH_clus_e->SetMinimum(0); 
  hSH_clus_e->GetYaxis()->SetNdivisions(kNrows);
  hSH_clus_e->GetXaxis()->SetNdivisions(kNcols);
  hSH_clus_e->Draw("text colz");
  gPad->Update();

  subCanv[3]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_intEng->SetStats(0);
  hPS_intEng->SetMaximum(0.5);
  hPS_intEng->SetMinimum(0); 
  hPS_intEng->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_intEng->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_intEng->Draw("text colz");
  subCanv[3]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_clus_e->SetStats(0);
  hPS_clus_e->SetMaximum(1.5);
  hPS_clus_e->SetMinimum(0); 
  hPS_clus_e->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_clus_e->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_clus_e->Draw("text colz");
  gPad->Update();

  subCanv[4]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_intEng->SetStats(0);
  hSH_intEng->SetMaximum(0.5);
  hSH_intEng->SetMinimum(0); 
  hSH_intEng->GetYaxis()->SetNdivisions(kNrows);
  hSH_intEng->GetXaxis()->SetNdivisions(kNcols);
  hSH_intEng->Draw("text colz");
  gPad->Update();
  subCanv[4]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_intEng->SetStats(0);
  hPS_intEng->SetMaximum(0.5);
  hPS_intEng->SetMinimum(0); 
  hPS_intEng->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_intEng->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_intEng->Draw("text colz");
  gPad->Update();
}

void clicked_displayNextButton()
{
  //if(gCurrentEntry>gMaxEntries);
  shgui::entryInput->SetIntNumber(++gCurrentEntry);
  displayEvent(gCurrentEntry);
}

void clicked_displayEntryButton()
{
  gCurrentEntry = shgui::entryInput->GetIntNumber();
  displayEvent(gCurrentEntry);
}


Int_t bbcal_clustD(const char* rfile="", Int_t run=290, Int_t event=5)
{
  shgui::SetupGUI();
  gStyle->SetLabelSize(0.05,"XY");
  gStyle->SetTitleFontSize(0.08);

  TChain *C = new TChain("T");
  C->Add(rfile);
  cout << "Opened up tree with nentries=" << C->GetEntries() << endl;

  T = new gmn_tree(C);
  event = -1;

  gCurrentEntry = event;
  while( user_input != "q" ) {
    if(is_number(user_input)) {
      gCurrentEntry = std::stoi(user_input);
    } else {
      gCurrentEntry++;
    }
    displayEvent(gCurrentEntry,run);
    std::cout << "Display options: <enter> == next event, or q to stop." << std::endl;
    //std::cin >> user_input;
    getline(std::cin,user_input);
  }
  return 0;
}

