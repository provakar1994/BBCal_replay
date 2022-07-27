/* 
   This script generates event display with the FADC waveforms for BBSH.
*/

#include <TH2F.h>
#include <TChain.h>
#include <TCanvas.h>
#include <vector>
#include <iostream>
#include <TSystem.h>
#include "fadc_data.h"

const Int_t kNrows = 26;
const Int_t kNcols = 2;

const Int_t kNumModules = kNrows*kNcols;
const Int_t DISP_MIN_SAMPLE = 0;
const Int_t DISP_MAX_SAMPLE = 25;
//const Int_t DISP_FADC_SAMPLES = 200;
const Int_t DISP_FADC_SAMPLES = (DISP_MAX_SAMPLE-DISP_MIN_SAMPLE);
const Int_t numSamples = 25;

const Int_t minADC = 0;
const Int_t maxADC = 4000;
const Int_t kCanvSize = 100;

std::string user_input;


Int_t gCurrentEntry = -1;

TChain *T = 0;
TEventList *elist;
Int_t foundModules = 0;
TCanvas *canvas = 0;

TCanvas *subCanv[4];

void clicked_displayEntryButton();
void clicked_displayNextButton();
namespace psgui {
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

  TRootEmbeddedCanvas *canv[4];

  TGCompositeFrame* AddTabSub(Int_t sub) {
    tf = fTab->AddTab(Form("PS Sub%d",sub+1));

    TGCompositeFrame *fF5 = new TGCompositeFrame(tf, (12+1)*kCanvSize,(6+1)*kCanvSize , kHorizontalFrame);
    TGLayoutHints *fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
        kLHintsExpandY, 5, 5, 5, 5);
    TRootEmbeddedCanvas *fEc1 = new TRootEmbeddedCanvas(Form("psSubCanv%d",sub), fF5, 6*kCanvSize,8*kCanvSize);
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
      ledLabel = new TGLabel(frame1,"LED Bit:    , Count:      ");
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

      // Create Tab1 (PS Sub1)
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
	// if( kNrows<12 || kNcols<12) {
	//   //subCanv[i]->Divide(kNrows,kNcols,0.001,0.001);
	//   subCanv[i]->Divide(kNcols,kNrows,0.001,0.001);
        // } else {
	//   subCanv[i]->Divide(12,6,0.001,0.001);
        // }
	subCanv[i]->Divide(kNcols,7,0.001,0.001);
      }
    }
  }
};


Double_t nhit = 0;
TH1F *histos[kNrows][kNcols];
Bool_t gSaturated[kNrows][kNcols];
TH1F *histos_amp[kNrows][kNcols];


TH1F* MakeHisto(Int_t row, Int_t col, Int_t bins, const char* suf="")
{
  TH1F *h = new TH1F(TString::Format("h%02d%02d%s",row,col,suf),
      TString::Format("%d-%d",row+1,col+1),bins,DISP_MIN_SAMPLE,DISP_MAX_SAMPLE);
  h->SetStats(0);
  h->SetLineWidth(2);
  h->GetYaxis()->SetLabelSize(0.1);
  //h->GetYaxis()->SetLabelOffset(-0.17);
  h->GetYaxis()->SetNdivisions(5);
  return h;
}


bool is_number(const std::string& mystring)
{
  std::string::const_iterator it = mystring.begin();
  while (it != mystring.end() && std::isdigit(*it)) ++it;
  return !mystring.empty() && it == mystring.end();
}

void displayEvent(Int_t entry = -1)
{
  if(entry == -1) {
    gCurrentEntry++;
  } else {
    gCurrentEntry = entry;
  }

  if(gCurrentEntry<0) {
    gCurrentEntry = 0;
  }

  T->GetEntry(elist->GetEntry(gCurrentEntry));
  //T->GetEntry(gCurrentEntry);
  //std::cout << "Displaying event " << gCurrentEntry << std::endl;
  psgui::ledLabel->SetText(TString::Format("LED Bit: %02d, Count: %5d",Int_t(fadc_datat::ledbit),Int_t(fadc_datat::ledcount)));

  Int_t r,c,idx,n,sub;
  // Clear old histograms, just in case modules are not in the tree
  for(r = 0; r < kNrows; r++) {
    for(c = 0; c < kNcols; c++) {
      histos[r][c]->Reset("ICES M");
      histos_amp[r][c]->Reset("ICES M");
      gSaturated[r][c] = false;
    }
  }

  Float_t peak[kNrows][kNcols];
  Double_t adc[kNrows][kNcols];
  Double_t tdc[kNrows][kNcols];
  bool inclus[kNrows][kNcols];
  for(r  = 0; r < kNrows; r++) {
    for(c  = 0; c < kNcols; c++) {
      peak[r][c] = 0.0;
      adc[r][c] = 0.0;
      tdc[r][c] = 0.0;
      inclus[r][c] = 0;
    }
  }
  for(Int_t ihits = 0; ihits < fadc_datat::ndata; ihits++) {
    r = fadc_datat::row[ihits];//-1;
    c = fadc_datat::col[ihits];//-1;
    Int_t m = r*kNcols+c;
 
    if(r < 0 || c < 0) {
      std::cerr << "Why is row negative? Or col?" << std::endl;
      continue;
    }
    if(r>= kNrows || c >= kNcols)
      continue;

    for(Int_t ihit=0;ihit<fadc_datat::cl_ndata;ihit++){
      Int_t nblk = fadc_datat::cl_idblk_PS[ihit];
      if(nblk==m) inclus[r][c]=1;
    }

    idx = fadc_datat::samps_idx[m];
    n = fadc_datat::nsamps[m];
    adc[r][c] = fadc_datat::a[ihits];
    tdc[r][c] = fadc_datat::tdc[ihits];
    //std::cout << "n=" << fadc_datat::nsamps[m] << std::endl;
    bool displayed = false;
    for(Int_t s = DISP_MIN_SAMPLE; s < DISP_MAX_SAMPLE && s < n; s++) {
      displayed = true;
      histos[r][c]->SetBinContent(s+1-DISP_MIN_SAMPLE,fadc_datat::samps[idx+s]);
      histos_amp[r][c]->SetBinContent(int(tdc[r][c]/4.0)+1,fadc_datat::amp[ihits]);
      if(peak[r][c]<fadc_datat::samps[idx+s])
        peak[r][c]=fadc_datat::samps[idx+s];
      if(peak[r][c]>4095) {
        gSaturated[r][c] = true;
      }
      //std::cout << "setting bin content: [" << r+1 << ", " << c+1 << ", " << s << "] = " << fadc_datat::samps[idx+s] << std::endl;
    }
    if(!displayed) {
      std::cerr << "Skipping empty module: " << m << std::endl;
      for(Int_t s = 0;  s < DISP_FADC_SAMPLES; s++) {
        histos[r][c]->SetBinContent(s+1,-404);
      }
    }
  }

  for(r = 0; r < kNrows; r++) {
    for(c = 0; c < kNcols; c++) {
      sub = r/7;
      //subCanv[sub]->cd(c*kNrows + r + 1);
      subCanv[sub]->cd((r%7)*kNcols + c + 1);
      //histos[r][c]->SetTitle(TString::Format("%d-%d (ADC=%g,TDC=%g)",r+1,c+1,adc[r][c],tdc[r][c]));
      histos[r][c]->SetTitle(TString::Format("%d-%d (ADC=%g)",r+1,c+1,adc[r][c]));
      if(gSaturated[r][c])
        histos[r][c]->SetLineColor(kRed+1);
      else
        histos[r][c]->SetLineColor(kBlue+1);
      if(inclus[r][c])
        histos[r][c]->SetLineColor(kGreen+1);
      // TLine* L = new TLine(tdc[r][c]/4.0, 0, tdc[r][c]/4.0, peak[r][c]);
      // L->SetLineColor(kMagenta+1);
      // L->SetLineWidth(2);
      histos_amp[r][c]->SetLineColor(kBlack);
      
      // if(histos_amp[r][c]->GetMaximum()>histos[r][c]->GetMaximum())
      // 	histos[r][c]->SetMaximum(histos_amp[r][c]->GetMaximum()*1.1);
      histos[r][c]->Draw();
      // histos_amp[r][c]->Draw("same");
      // if(tdc[r][c]!=0)L->Draw("same");
      gPad->Update();
      //std::cout << " [" << r << ", " << c << "]=" << peak[r][c];
    }
  }
  std::cout << std::endl;
  //gSystem->mkdir("images/",kTRUE);
  //std::cerr << "Saving canvas!" << std::endl;
  //canvas->SaveAs("images/display_ps.png");
  //  canvVector[i]->SaveAs(TString::Format("images/canvas_%d.png",int(i)));

}

void clicked_displayNextButton()
{
  //if(gCurrentEntry>gMaxEntries);
  psgui::entryInput->SetIntNumber(++gCurrentEntry);
  displayEvent(gCurrentEntry);
}

void clicked_displayEntryButton()
{
  gCurrentEntry = psgui::entryInput->GetIntNumber();
  displayEvent(gCurrentEntry);
}


Int_t display_ps(const char* rfile="", TCut cut="", Int_t run=290)
{
  psgui::SetupGUI();
  gStyle->SetLabelSize(0.05,"XY");
  gStyle->SetTitleFontSize(0.08);
  Int_t event = -1;

  if(!T) { 
    T = new TChain("T");
    //T->Add(TString::Format("~/sbs/Rootfiles/bbshower_%d_%d.root",run,event));
    T->Add(rfile);
    cout << " Opened up tree with nentries = " << T->GetEntries() << endl;
    //T->SetBranchStatus("*",0);
    T->SetBranchStatus("bb.ps.*",1);
    T->SetBranchAddress("bb.ps.nsamps",fadc_datat::nsamps);
    T->SetBranchAddress("bb.ps.a",fadc_datat::a);
    T->SetBranchAddress("bb.ps.a_time",fadc_datat::tdc);
    T->SetBranchAddress("bb.ps.a_amp_p",fadc_datat::amp);
    //T->SetBranchAddress("bb.ps.ledbit",&fadc_datat::ledbit);
    //T->SetBranchAddress("bb.ps.ledcount",&fadc_datat::ledcount);
    T->SetBranchAddress("bb.ps.samps",fadc_datat::samps);
    T->SetBranchAddress("bb.ps.samps_idx",fadc_datat::samps_idx);
    T->SetBranchAddress("bb.ps.adcrow",fadc_datat::row);
    T->SetBranchAddress("bb.ps.adccol",fadc_datat::col);
    T->SetBranchStatus("Ndata.bb.ps.adcrow",1);
    T->SetBranchAddress("Ndata.bb.ps.adcrow",&fadc_datat::ndata);
    T->SetBranchAddress("Ndata.bb.ps.clus_blk.id",&fadc_datat::cl_ndata);
 T->SetBranchAddress("bb.ps.clus_blk.id",&fadc_datat::cl_idblk_PS);

    elist = new TEventList("elist","ABC");
    //T->Draw(">>elist","bb.tr.n==1&&abs(bb.tr.vz[0])<0.08&&abs(bb.tr.tg_th[0])<0.15&&abs(bb.tr.tg_ph[0])<0.3&&bb.gem.track.nhits>3&&bb.tr.p[0]>1.8&&bb.tr.p[0]<2.6&&sbs.hcal.e>0.025&&bb.ps.e>0.22");
    T->Draw(">>elist",cut);
    cout << " No of events passed global cut = " << elist->GetN() << endl;

    for(Int_t r = 0; r < kNrows; r++) {
      for(Int_t c = 0; c < kNcols; c++) {
        histos[r][c] = MakeHisto(r,c,DISP_FADC_SAMPLES);
        histos_amp[r][c] = MakeHisto(r,c,DISP_FADC_SAMPLES,"_amp");
        gSaturated[r][c] = false;
      }
    }
    //canvas = new TCanvas("canvas","canvas",kCanvSize*kNcols,kCanvSize*kNrows);
    //canvas->Divide(kNcols,kNrows);
  }
  return 0;
  gCurrentEntry = event;
  while( user_input != "q" ) {
    if(is_number(user_input)) {
      gCurrentEntry = std::stoi(user_input);
    } else {
      gCurrentEntry++;
    }
    //dis1(event);
    displayEvent(gCurrentEntry);
    std::cout << "Display options: <enter> == next event, or q to stop." << std::endl;
    //std::cin >> user_input;
    getline(std::cin,user_input);
  }
  return 0;
}

