/*
  This script Script to determine ADC time offsets for all SH and PS channels w.r.t
  BBTH cluster mean time (bbhodo.clus.tmean). One needs a configfile, setup_bbcal_atime_offset.cfg, 
  to execute this script. To execute, do:
  ----
  [a-onl@aonl2 macros]$ pwd
  /adaqfs/home/a-onl/sbs/BBCal_replay/macros
  [a-onl@aonl2 macros]$ root -l 
  root [0] .x Combined_macros/bbcal_atime_offset.C("Combined_macros/setup_bbcal_atime_offset.cfg")
  ----
  P. Datta  <pdbforce@jlab.org>  Created  16 Feb 2022
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

const Double_t Mp = 0.938272081;  // +/- 6E-9 GeV

const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows
const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows

TH1F* MakeHisto( Int_t, Int_t, Int_t, const char*, Double_t, Double_t );
TH1F *h_atime_sh[kNrowsSH][kNcolsSH];
TH1F *h_atime_ps[kNrowsPS][kNcolsPS];

TCanvas *subCanv[8];
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

  TRootEmbeddedCanvas *canv[8];

  TGCompositeFrame* AddTabSub(Int_t sub) {
    tf = fTab->AddTab(Form("Tab %d",sub+1));

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
      for(Int_t i = 0; i < 8; i++) {
        tf = AddTabSub(i);
      }
      main->AddFrame(fTab, new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
					     kLHintsExpandY, 2, 2, 5, 1));
      main->MapSubwindows();
      main->Resize();   // resize to default size
      main->MapWindow();

      for(Int_t i = 0; i < 4; i++) {
        subCanv[i] = canv[i]->GetCanvas();
	subCanv[i]->Divide(kNcolsSH,7,0.001,0.001);
      }
      for(Int_t i = 4; i < 8; i++) {
        subCanv[i] = canv[i]->GetCanvas();
	subCanv[i]->Divide(kNcolsPS,7,0.001,0.001);
      }
    }
  }
};

// main function
void bbcal_atime_offset( const char *configfilename ){
  gErrorIgnoreLevel = kError; // Ignore all ROOT warnings

  // Define a clock to check macro processing time
  TStopwatch *sw = new TStopwatch();
  TStopwatch *sw2 = new TStopwatch();
  sw->Start();
  sw2->Start();

  //gui setup
  shgui::SetupGUI();
  gStyle->SetLabelSize(0.08,"XY");
  gStyle->SetTitleFontSize(0.08);

  TChain *C = new TChain("T");
  Double_t Ebeam = 0.; // GeV
  Double_t h_atime_bin = 200, h_atime_min = 0., h_atime_max = 5.;

  // Reading configfile
  ifstream configfile(configfilename);
  TString currentline;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endRunlist") ){
    if( !currentline.BeginsWith("#") ){
      C->Add(currentline);
    }   
  } 
  TCut globalcut = ""; TString gcutstr;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endcut") ){
    if( !currentline.BeginsWith("#") ){
      globalcut += currentline;
      gcutstr += currentline;
    }    
  }
  TTreeFormula *GlobalCut = new TTreeFormula("GlobalCut", globalcut, C);
  while( currentline.ReadLine( configfile ) ){
    if( currentline.BeginsWith("#") ) continue;
    TObjArray *tokens = currentline.Tokenize(" ");
    Int_t ntokens = tokens->GetEntries();
    if( ntokens>1 ){
      TString skey = ( (TObjString*)(*tokens)[0] )->GetString();
      if( skey == "E_beam" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	Ebeam = sval.Atof();
      }
      if( skey == "h_atime" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_atime_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_atime_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_atime_max = sval2.Atof();
      }
      if( skey == "*****" ){
	break;
      }
    } 
    delete tokens;
  }

  // Check for empty rootfiles and set tree branches
  if(C->GetEntries()==0){
    cerr << endl << " --- No ROOT file found!! --- " << endl << endl;
    throw;
  }else cout << endl << "Found " << C->GetEntries() << " events. Starting analysis.. " << endl;

  // Setting useful ROOT tree branch addresses
  Int_t maxtr=1000, hodo_trindex=0;
  Double_t sh_nclus, sh_e, sh_rowblk, sh_colblk, sh_nblk, sh_atimeblk;
  Double_t ps_nclus, ps_idblk, ps_e, ps_rowblk, ps_colblk, ps_atimeblk;
  Double_t hodo_nclus;
  Double_t sh_clblk_atime[maxtr], sh_clblk_e[maxtr];
  Double_t ps_clblk_atime[maxtr];
  Double_t p[maxtr], pz[maxtr], tg_th[maxtr], tg_ph[maxtr];
  Double_t hodo_tmean[maxtr], hodo_trIndex[maxtr];

  C->SetBranchStatus("*",0);
  //shower
  C->SetBranchStatus("bb.sh.nclus",1);
  C->SetBranchAddress("bb.sh.nclus",&sh_nclus);
  C->SetBranchStatus("bb.sh.e",1);
  C->SetBranchAddress("bb.sh.e",&sh_e);
  C->SetBranchStatus("bb.sh.rowblk",1);
  C->SetBranchAddress("bb.sh.rowblk",&sh_rowblk);
  C->SetBranchStatus("bb.sh.colblk",1);
  C->SetBranchAddress("bb.sh.colblk",&sh_colblk);
  C->SetBranchStatus("bb.sh.atimeblk",1);
  C->SetBranchAddress("bb.sh.atimeblk",&sh_atimeblk);
  C->SetBranchStatus("bb.sh.nblk",1);
  C->SetBranchAddress("bb.sh.nblk",&sh_nblk);
  C->SetBranchStatus("bb.sh.clus_blk.e",1);
  C->SetBranchAddress("bb.sh.clus_blk.e",&sh_clblk_e);
  C->SetBranchStatus("bb.sh.clus_blk.atime",1);
  C->SetBranchAddress("bb.sh.clus_blk.atime",&sh_clblk_atime);
  //preshower
  C->SetBranchStatus("bb.ps.nclus",1);
  C->SetBranchAddress("bb.ps.nclus",&ps_nclus);
  C->SetBranchStatus("bb.ps.e",1);
  C->SetBranchAddress("bb.ps.e",&ps_e);
  C->SetBranchStatus("bb.ps.rowblk",1);
  C->SetBranchAddress("bb.ps.rowblk",&ps_rowblk);
  C->SetBranchStatus("bb.ps.colblk",1);
  C->SetBranchAddress("bb.ps.colblk",&ps_colblk);
  C->SetBranchStatus("bb.ps.atimeblk",1);
  C->SetBranchAddress("bb.ps.atimeblk",&ps_atimeblk);
  C->SetBranchStatus("bb.ps.idblk",1);
  C->SetBranchAddress("bb.ps.idblk",&ps_idblk);
  C->SetBranchStatus("bb.ps.clus_blk.atime",1);
  C->SetBranchAddress("bb.ps.clus_blk.atime",&ps_clblk_atime);
  //gem
  C->SetBranchStatus("bb.tr.p",1);
  C->SetBranchAddress("bb.tr.p",&p);
  C->SetBranchStatus("bb.tr.pz",1);
  C->SetBranchAddress("bb.tr.pz",&pz);
  C->SetBranchStatus("bb.tr.tg_th",1);
  C->SetBranchAddress("bb.tr.tg_th",&tg_th);
  C->SetBranchStatus("bb.tr.tg_ph",1);
  C->SetBranchAddress("bb.tr.tg_ph",&tg_ph);
  //hodo
  C->SetBranchStatus("bb.hodotdc.nclus",1);
  C->SetBranchAddress("bb.hodotdc.nclus",&hodo_nclus);
  C->SetBranchStatus("bb.hodotdc.clus.tmean",1);
  C->SetBranchAddress("bb.hodotdc.clus.tmean",&hodo_tmean);
  C->SetBranchStatus("bb.hodotdc.clus.trackindex",1);
  C->SetBranchAddress("bb.hodotdc.clus.trackindex",&hodo_trIndex);
  // turning on additional branches for the global cut
  C->SetBranchStatus("bb.tr.n", 1);
  C->SetBranchStatus("bb.tr.vz", 1);
  C->SetBranchStatus("bb.gem.track.nhits", 1);

  //defining atime histograms
  for(int r = 0; r < kNrowsSH; r++) {
    for(int c = 0; c < kNcolsSH; c++) {
      h_atime_sh[r][c] = MakeHisto(r, c, h_atime_bin, "_i", h_atime_min, h_atime_max);
    }
  }
  for(int r = 0; r < kNrowsPS; r++) {
    for(int c = 0; c < kNcolsPS; c++) {
      h_atime_ps[r][c] = MakeHisto(r, c, h_atime_bin, "_i", h_atime_min, h_atime_max);
    }
  }

  // define output files
  TString outFile, outPeaks, toffset_ps, toffset_sh;
  outPeaks = Form("plots/bbcal_atime_offset.pdf");
  outFile = Form("hist/bbcal_atime_offset.root");
  toffset_ps = Form("Output/adctime_offset_ps.txt");
  toffset_sh = Form("Output/adctime_offset_sh.txt");
  ofstream toffset_psdata, toffset_shdata;
  toffset_psdata.open(toffset_ps);
  toffset_shdata.open(toffset_sh);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  TH1F *h_W = new TH1F("h_W","W distribution",200,0.,5.);
  TH1F *h_Q2 = new TH1F("h_Q2","Q2 distribution",300,1.,15.);
  TH1F *h_atime_offset_sh = new TH1F("h_atime_offset_sh","SH ADCtime offset w.r.t. BBHodo time",189,0.,189.);
  //h_atime_offset_sh->GetYaxis()->SetRangeUser(-100.,0.);
  h_atime_offset_sh->GetYaxis()->SetLabelSize(0.045);
  h_atime_offset_sh->GetXaxis()->SetLabelSize(0.045);
  h_atime_offset_sh->SetLineWidth(0);
  h_atime_offset_sh->SetMarkerStyle(kFullCircle);
  TH1F *h_atime_offset_ps = new TH1F("h_atime_offset_ps","PS ADCtime offset w.r.t. BBHodo time",52,0.,52.);
  //h_atime_offset_ps->GetYaxis()->SetRangeUser(-100.,0.);
  h_atime_offset_ps->GetYaxis()->SetLabelSize(0.045);
  h_atime_offset_ps->GetXaxis()->SetLabelSize(0.045);
  h_atime_offset_ps->SetLineWidth(0);
  h_atime_offset_ps->SetMarkerStyle(kFullCircle);

  ///////////////////////////////////////////
  // 1st Loop over all events to calibrate //
  ///////////////////////////////////////////

  cout << endl;  
  Long64_t nevent=0, nevents=C->GetEntries();
  Double_t timekeeper = 0., timeremains = 0.;
  int treenum = 0, currenttreenum = 0;
  vector<Long64_t> goodevents; // list of good events passed all the cuts
  while( C->GetEntry( nevent++ ) ){
    // Calculating remaining time 
    sw2->Stop();
    timekeeper += sw2->RealTime();
    if (nevent % 25000 == 0 && nevent != 0) 
      timeremains = timekeeper * (double(nevents) / double(nevent) - 1.); 
    sw2->Reset();
    sw2->Continue();

    if(nevent % 100 == 0) cout << nevent << "/" << nevents  << ", " << int(timeremains/60.) << "m \r";;
    cout.flush();
    // ------

    // apply global cuts efficiently (AJRP method)
    currenttreenum = C->GetTreeNumber();
    if (nevent == 1 || currenttreenum != treenum) {
      treenum = currenttreenum;
      GlobalCut->UpdateFormulaLeaves();
    } 
    bool passedgCut = GlobalCut->EvalInstance(0) != 0;   
    if (passedgCut) {

      //calculating physics parameters
      Double_t P_ang = 57.3*TMath::ACos(pz[0]/p[0]);
      Double_t Q2 = 4.*Ebeam*p[0]*pow( TMath::Sin(P_ang/57.3/2.),2. );
      Double_t W2 = Mp*Mp + 2.*Mp*(Ebeam-p[0]) - Q2;
      Double_t W = 0.;

      h_Q2->Fill(Q2);
      if( W2>0. ){
	W = TMath::Sqrt(W2);  
	h_W->Fill(W);
      }

      //hodo cut
      if( hodo_trIndex[0]!=0 ) continue;
 
      //avoiding clusters on the edge
      // if(sh_rowblk==0 || sh_rowblk==26 ||
      // 	 sh_colblk==0 || sh_colblk==6) continue; 

      // cut on W
      // if (fabs(W - 0.938) >= 0.2) continue;

      // storing good event numbers for 2nd loop
      goodevents.push_back(nevent);

      h_atime_sh[(int)sh_rowblk][(int)sh_colblk]->Fill(hodo_tmean[0] - sh_clblk_atime[0]); //- sh_atimeblk);
      h_atime_ps[(int)ps_rowblk][(int)ps_colblk]->Fill(hodo_tmean[0] - ps_clblk_atime[0]); //- ps_atimeblk);

      // h_atime_sh[(int)sh_rowblk][(int)sh_colblk]->Fill(sh_atimeblk);
      // h_atime_ps[(int)ps_rowblk][(int)ps_colblk]->Fill(ps_atimeblk);

    } //global cut
  } //while
  cout << endl; 

  // Let's fit the histograms with Gaussian function 
  TF1 *fgaus = new TF1("fgaus","gaus");

  int sub = 0;
  for(int r=0; r<kNrowsSH; r++){
    for(int c=0; c<kNcolsSH; c++){
      sub = r/7;
      subCanv[sub]->cd((r%7)*kNcolsSH + c + 1);

      int maxBin = h_atime_sh[r][c]->GetMaximumBin();
      double maxBinCenter = h_atime_sh[r][c]->GetXaxis()->GetBinCenter( maxBin );
      double maxCount = h_atime_sh[r][c]->GetMaximum();
      double binWidth = h_atime_sh[r][c]->GetBinWidth(maxBin);
      double stdDev = h_atime_sh[r][c]->GetStdDev();
      int lofitlim = h_atime_min + (maxBin)*binWidth - (1.3*stdDev);
      int hifitlim = h_atime_min + (maxBin)*binWidth + (0.8*stdDev);

      if( h_atime_sh[r][c]->GetEntries()>20 && stdDev>2.*binWidth){

	// Create fit functions for each module
	fgaus->SetLineColor(1);
	fgaus->SetNpx(1000);

	if( h_atime_sh[r][c]->GetBinContent(maxBin-1) == 0. ){
	  while ( h_atime_sh[r][c]->GetBinContent(maxBin+1) < h_atime_sh[r][c]->GetBinContent(maxBin) || 
		  h_atime_sh[r][c]->GetBinContent(maxBin+1) == h_atime_sh[r][c]->GetBinContent(maxBin) ) 
	    {
	      maxBin++;
	    };
	  h_atime_sh[r][c]->GetXaxis()->SetRange( maxBin+1 , h_atime_sh[r][c]->GetNbinsX() );
	  maxBin = h_atime_sh[r][c]->GetMaximumBin();
	  maxBinCenter = h_atime_sh[r][c]->GetXaxis()->GetBinCenter( maxBin );
	  maxCount = h_atime_sh[r][c]->GetMaximum();
	  binWidth = h_atime_sh[r][c]->GetBinWidth(maxBin);
	  stdDev = h_atime_sh[r][c]->GetStdDev();
	}

	fgaus->SetParameters( maxCount,maxBinCenter,stdDev );
	fgaus->SetRange( lofitlim, hifitlim );
	h_atime_sh[r][c]->Fit(fgaus,"+RQ");
	
	toffset_shdata << fgaus->GetParameter(1) << " "; 
	h_atime_offset_sh->Fill( r*kNcolsSH+c, fgaus->GetParameter(1) );

      }else{
	toffset_shdata << 0. << " "; 
      }

      h_atime_sh[r][c]->SetTitle(Form("Time Offset | SH%d-%d",r+1,c+1));
      h_atime_sh[r][c]->Draw();
    }
    toffset_shdata << endl;
  }

  //fitting PS histograms
  sub = 0;
  for(int r=0; r<kNrowsPS; r++){
    for(int c=0; c<kNcolsPS; c++){
      sub = r/7 + 4;
      subCanv[sub]->cd((r%7)*kNcolsPS + c + 1);

      int maxBin = h_atime_ps[r][c]->GetMaximumBin();
      double maxBinCenter = h_atime_ps[r][c]->GetXaxis()->GetBinCenter( maxBin );
      double maxCount = h_atime_ps[r][c]->GetMaximum();
      double binWidth = h_atime_ps[r][c]->GetBinWidth(maxBin);
      double stdDev = h_atime_ps[r][c]->GetStdDev();
      int lofitlim = h_atime_min + (maxBin)*binWidth - (1.2*stdDev);
      int hifitlim = h_atime_min + (maxBin)*binWidth + (0.5*stdDev);

      if( h_atime_ps[r][c]->GetEntries()>20 && stdDev>2.*binWidth){

	// Create fit functions for each module
	fgaus->SetLineColor(1);
	fgaus->SetNpx(1000);

	if( h_atime_ps[r][c]->GetBinContent(maxBin-1) == 0. ){
	  while ( h_atime_ps[r][c]->GetBinContent(maxBin+1) < h_atime_ps[r][c]->GetBinContent(maxBin) || 
		  h_atime_ps[r][c]->GetBinContent(maxBin+1) == h_atime_ps[r][c]->GetBinContent(maxBin) ) 
	    {
	      maxBin++;
	    };
	  h_atime_ps[r][c]->GetXaxis()->SetRange( maxBin+1 , h_atime_ps[r][c]->GetNbinsX() );
	  maxBin = h_atime_ps[r][c]->GetMaximumBin();
	  maxBinCenter = h_atime_ps[r][c]->GetXaxis()->GetBinCenter( maxBin );
	  maxCount = h_atime_ps[r][c]->GetMaximum();
	  binWidth = h_atime_ps[r][c]->GetBinWidth(maxBin);
	  stdDev = h_atime_ps[r][c]->GetStdDev();
	}

	fgaus->SetParameters( maxCount,maxBinCenter,stdDev );
	fgaus->SetRange( lofitlim, hifitlim );
	h_atime_ps[r][c]->Fit(fgaus,"+RQ");

	toffset_psdata << fgaus->GetParameter(1) << " "; 
	h_atime_offset_ps->Fill( r*kNcolsPS+c, fgaus->GetParameter(1) );

      }else{
	toffset_psdata << 0. << " "; 
      }

      h_atime_ps[r][c]->SetTitle(Form("Time Offset | PS%d-%d",r+1,c+1));
      h_atime_ps[r][c]->Draw();    
    }
    toffset_psdata << endl;
  }

  subCanv[0]->SaveAs(Form("%s[",outPeaks.Data()));
  for( int canC=0; canC<8; canC++ ) subCanv[canC]->SaveAs(Form("%s",outPeaks.Data()));
  subCanv[7]->SaveAs(Form("%s]",outPeaks.Data()));

  fout->Write();
  fout->Close();
  fout->Delete();
 
  cout << "Finishing analysis .." << endl;
  cout << " --------- " << endl;
  cout << " Plots saved to : " << outPeaks << endl;
  cout << " Histogram written to : " << outFile << endl;
  cout << " ADCtime offsets for SH written to : " << toffset_sh << endl;
  cout << " ADCtime offsets for PS written to : " << toffset_ps << endl;
  cout << " --------- " << endl;

  sw->Stop();
  sw2->Stop();
  cout << "CPU time elapsed = " << sw->CpuTime() << " s. Real time = " << sw->RealTime() << " s. " << endl << endl;

}

// Create generic histogram function
TH1F* MakeHisto(Int_t row, Int_t col, Int_t bins, const char* suf="", Double_t min=0., Double_t max=50.)
{
  TH1F *h = new TH1F(TString::Format("h_R%d_C%d_Blk_%d%s",row,col,row*kNcolsSH+col,suf),
		     "",bins,min,max);
  //h->SetStats(0);
  h->SetLineWidth(1);
  h->SetLineColor(2);
  h->GetYaxis()->SetLabelSize(0.06);
  //h->GetYaxis()->SetLabelOffset(-0.17);
  h->GetYaxis()->SetNdivisions(5);
  return h;
}
