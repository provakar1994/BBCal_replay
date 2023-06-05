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

#include <ctime>
#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <TH2F.h>
#include <TGraph.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TStopwatch.h>

const Double_t Mp = 0.938272081;  // +/- 6E-9 GeV

const Int_t kNcolsSH = 7;   // SH columns
const Int_t kNrowsSH = 27;  // SH rows
const Int_t kNblksSH = 189; // Total # SH blocks/PMTs
const Int_t kNcolsPS = 2;   // PS columns
const Int_t kNrowsPS = 26;  // PS rows
const Int_t kNblksPS = 52;  // Total # PS blocks/PMTs

void Custm1DHisto(TH1F*);
void Custm2DHisto(TH2F*);
TH1F* MakeHisto(Int_t, Int_t, char const *, Int_t, Double_t, Double_t);

TH1F *h_atime_sh[kNrowsSH][kNcolsSH];
TH1F *h_atime_sh_corr[kNrowsSH][kNcolsSH];
TH1F *h_atime_ps[kNrowsPS][kNcolsPS];
TH1F *h_atime_ps_corr[kNrowsPS][kNcolsPS];

vector<Long64_t> goodevents;
Double_t ash_atimeOffs[kNblksSH];
Double_t aps_atimeOffs[kNblksPS];

TCanvas *subCanv[8];
const Int_t kCanvSize = 100;
namespace shgui {
  TGMainFrame *main = 0;
  TGHorizontalFrame *frame1 = 0;
  TGTab *fTab;
  TGLayoutHints *fL3;
  TGCompositeFrame *tf;
  TGTextButton *exitButton;
  TGNumberEntry *entryInput;
  TGLabel *runLabel;

  TRootEmbeddedCanvas *canv[8];

  TGCompositeFrame* AddTabSub(Int_t sub) {
    tf = fTab->AddTab(Form("Tab %d",sub+1));

    TGCompositeFrame *fF5 = new TGCompositeFrame(tf, (12+1)*kCanvSize,(6+1)*kCanvSize , kHorizontalFrame);
    TGLayoutHints *fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
					   kLHintsExpandY, 5, 5, 5, 5);
    TRootEmbeddedCanvas *fEc1 = new TRootEmbeddedCanvas(Form("shSubCanv%d",sub), fF5, 6*kCanvSize,8*kCanvSize);
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
      exitButton = new TGTextButton(frame1, "&Exit", 
				    "gApplication->Terminate(0)");
      TGLayoutHints *frame1LH = new TGLayoutHints(kLHintsTop|kLHintsLeft|
						  kLHintsExpandX,2,2,2,2);
      frame1->AddFrame(runLabel,frame1LH);
      frame1->AddFrame(exitButton,frame1LH);
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
  sw->Start(); sw2->Start();

  //gui setup
  shgui::SetupGUI();

  TChain *C = new TChain("T");
  Int_t SBSconf = 0;   // SBS configuration
  Double_t Ebeam = 0.; // GeV
  Double_t h_atime_bin = 240, h_atime_min = -60., h_atime_max = 60.;

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
      if( skey == "SBSconf" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	SBSconf = sval.Atof();
      }
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
  if(C->GetEntries()==0) {cerr << endl << " --- No ROOT file found!! --- " << endl << endl; exit(1);}
  else cout << endl << "Found " << C->GetEntries() << " events. Starting analysis.. " << endl;
 
  // Setting useful ROOT tree branch addresses
  C->SetBranchStatus("*",0);
  Int_t maxtr=500;
  //shower
  Double_t sh_nclus;     C->SetBranchStatus("bb.sh.nclus",1); C->SetBranchAddress("bb.sh.nclus",&sh_nclus);
  Double_t sh_e;         C->SetBranchStatus("bb.sh.e",1); C->SetBranchAddress("bb.sh.e",&sh_e);
  Double_t sh_rowblk;    C->SetBranchStatus("bb.sh.rowblk",1); C->SetBranchAddress("bb.sh.rowblk",&sh_rowblk);
  Double_t sh_colblk;    C->SetBranchStatus("bb.sh.colblk",1); C->SetBranchAddress("bb.sh.colblk",&sh_colblk);
  Double_t sh_atimeblk;  C->SetBranchStatus("bb.sh.atimeblk",1); C->SetBranchAddress("bb.sh.atimeblk",&sh_atimeblk);
  Double_t sh_nblk;      C->SetBranchStatus("bb.sh.nblk",1); C->SetBranchAddress("bb.sh.nblk",&sh_nblk);
  Double_t sh_clblk_e;   C->SetBranchStatus("bb.sh.clus_blk.e",1); C->SetBranchAddress("bb.sh.clus_blk.e",&sh_clblk_e);
  Double_t sh_idblk;     C->SetBranchStatus("bb.sh.idblk",1); C->SetBranchAddress("bb.sh.idblk",&sh_idblk);
  Double_t sh_clblk_atime[maxtr]; C->SetBranchStatus("bb.sh.clus_blk.atime",1); C->SetBranchAddress("bb.sh.clus_blk.atime",&sh_clblk_atime);
  //preshower
  Double_t ps_nclus;     C->SetBranchStatus("bb.ps.nclus",1); C->SetBranchAddress("bb.ps.nclus",&ps_nclus);
  Double_t ps_e;         C->SetBranchStatus("bb.ps.e",1); C->SetBranchAddress("bb.ps.e",&ps_e);
  Double_t ps_rowblk;    C->SetBranchStatus("bb.ps.rowblk",1); C->SetBranchAddress("bb.ps.rowblk",&ps_rowblk);
  Double_t ps_colblk;    C->SetBranchStatus("bb.ps.colblk",1); C->SetBranchAddress("bb.ps.colblk",&ps_colblk);
  Double_t ps_atimeblk;  C->SetBranchStatus("bb.ps.atimeblk",1); C->SetBranchAddress("bb.ps.atimeblk",&ps_atimeblk);
  Double_t ps_idblk;     C->SetBranchStatus("bb.ps.idblk",1); C->SetBranchAddress("bb.ps.idblk",&ps_idblk);
  Double_t ps_clblk_atime[maxtr]; C->SetBranchStatus("bb.ps.clus_blk.atime",1); C->SetBranchAddress("bb.ps.clus_blk.atime",&ps_clblk_atime);
  //gem
  Double_t p[maxtr];     C->SetBranchStatus("bb.tr.p",1); C->SetBranchAddress("bb.tr.p",&p);
  Double_t pz[maxtr];    C->SetBranchStatus("bb.tr.pz",1); C->SetBranchAddress("bb.tr.pz",&pz);
  Double_t tg_th[maxtr]; C->SetBranchStatus("bb.tr.tg_th",1); C->SetBranchAddress("bb.tr.tg_th",&tg_th);
  Double_t tg_ph[maxtr]; C->SetBranchStatus("bb.tr.tg_ph",1); C->SetBranchAddress("bb.tr.tg_ph",&tg_ph);
  //hodo
  Double_t hodo_nclus;   C->SetBranchStatus("bb.hodotdc.nclus",1); C->SetBranchAddress("bb.hodotdc.nclus",&hodo_nclus);
  Double_t hodo_tmean[maxtr];   C->SetBranchStatus("bb.hodotdc.clus.tmean",1); C->SetBranchAddress("bb.hodotdc.clus.tmean",&hodo_tmean);
  Double_t hodo_trIndex[maxtr]; C->SetBranchStatus("bb.hodotdc.clus.trackindex",1); C->SetBranchAddress("bb.hodotdc.clus.trackindex",&hodo_trIndex);
  // turning on additional branches for the global cut
  C->SetBranchStatus("e.kine.W2", 1);
  C->SetBranchStatus("bb.tr.n", 1);
  C->SetBranchStatus("bb.tr.vz", 1);
  C->SetBranchStatus("bb.gem.track.nhits", 1);
  C->SetBranchStatus("g.trigbits", 1);

  // creating atimeOff histograms per BBCal block
  for(int r = 0; r < kNrowsSH; r++) {
    for(int c = 0; c < kNcolsSH; c++) {
      int blkid = r*kNcolsSH+c;
      ash_atimeOffs[blkid] = -1000;
      h_atime_sh[r][c] = MakeHisto(r, c, "", h_atime_bin, h_atime_min, h_atime_max);
      h_atime_sh_corr[r][c] = MakeHisto(r, c, "_corr", h_atime_bin, h_atime_min, h_atime_max);
    }
  }
  for(int r = 0; r < kNrowsPS; r++) {
    for(int c = 0; c < kNcolsPS; c++) {
      int blkid = r*kNcolsPS+c;
      aps_atimeOffs[blkid] = -1000;
      h_atime_ps[r][c] = MakeHisto(r, c, "", h_atime_bin, h_atime_min, h_atime_max);
      h_atime_ps_corr[r][c] = MakeHisto(r, c, "_corr", h_atime_bin, h_atime_min, h_atime_max);
    }
  }

  // defining raw histograms (we don't wanna write them to file)
  TH2F *h2_atimeOffSH_detview_raw = new TH2F("h2_atimeOffSH_detview_raw","Before offset correction (SH);SH block id;SH ADCtime - TH ClusTmean (ns)",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH); 
  TH2F *h2_atimeOffSH_detview_raw_corr = new TH2F("h2_atimeOffSH_detview_raw_corr","After offset correction (SH);SH block id;SH ADCtime - TH ClusTmean (ns)",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH); 
  TH2F *h2_atimeOffPS_detview_raw = new TH2F("h2_atimeOffPS_detview_raw","Before offset correction (PS);PS block id;PS ADCtime - TH ClusTmean (ns)",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);   
  TH2F *h2_atimeOffPS_detview_raw_corr = new TH2F("h2_atimeOffPS_detview_raw_corr","After offset correction (PS);PS block id;PS ADCtime - TH ClusTmean (ns)",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);   

  // define output files
  TString outFile, outPeaks, toffset_ps, toffset_sh;
  outPeaks = Form("plots/bbcal_atime_offset_sbs%d.pdf",SBSconf);
  outFile = Form("hist/bbcal_atime_offset_sbs%d.root",SBSconf);
  toffset_ps = Form("Output/adctime_offset_sbs%d_ps.txt",SBSconf);
  toffset_sh = Form("Output/adctime_offset_sbs%d_sh.txt",SBSconf);
  ofstream toffset_psdata, toffset_shdata;
  toffset_psdata.open(toffset_ps);
  toffset_shdata.open(toffset_sh);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  // defining important histograms
  TH1F *h_W = new TH1F("h_W","W distribution",200,0.,5.);
  TH1F *h_Q2 = new TH1F("h_Q2","Q2 distribution",300,1.,15.);

  TH1F *h_atimeOffSH = new TH1F("h_atimeOffSH","Peak pos. of (SH ADCtime - TH ClusTmean) dist. (w/ fit error) | Before corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeOffSH_corr = new TH1F("h_atimeOffSH_corr","Peak pos. of (SH ADCtime - TH ClusTmean) dist. (w/ fit error) | After corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeOffPS = new TH1F("h_atimeOffPS","Peak pos. of (PS ADCtime - TH ClusTmean) dist. (w/ fit error) | Before corr.",kNblksPS,0,kNblksPS);
  TH1F *h_atimeOffPS_corr = new TH1F("h_atimeOffPS_corr","Peak pos. of (PS ADCtime - TH ClusTmean) dist. (w/ fit error) | After corr.",kNblksPS,0,kNblksPS);
  Custm1DHisto(h_atimeOffSH); Custm1DHisto(h_atimeOffSH_corr); 
  Custm1DHisto(h_atimeOffPS); Custm1DHisto(h_atimeOffPS_corr);

  TH1F *h_atimeRMSSH = new TH1F("h_atimeRMSSH","Peak RMS of (SH ADCtime - TH ClusTmean) dist. (w/ fit error) | Before corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeRMSSH_corr = new TH1F("h_atimeRMSSH_corr","Peak RMS of (SH ADCtime - TH ClusTmean) dist. (w/ fit error) | After corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeRMSPS = new TH1F("h_atimeRMSPS","Peak RMS of (PS ADCtime - TH ClusTmean) dist. (w/ fit error) | Before corr.",kNblksPS,0,kNblksPS);
  TH1F *h_atimeRMSPS_corr = new TH1F("h_atimeRMSPS_corr","Peak RMS of (PS ADCtime - TH ClusTmean) dist. (w/ fit error) | After corr.",kNblksPS,0,kNblksPS);
  Custm1DHisto(h_atimeRMSSH); Custm1DHisto(h_atimeRMSSH_corr); 
  Custm1DHisto(h_atimeRMSPS); Custm1DHisto(h_atimeRMSPS_corr);

  TH1F *h_atimeOffnRMSSH = new TH1F("h_atimeOffnRMSSH","Peak pos. of (SH ADCtime - TH ClusTmean) dist. (error bar rep. fit RMS) | Before corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeOffnRMSSH_corr = new TH1F("h_atimeOffnRMSSH_corr","Peak pos. of (SH ADCtime - TH ClusTmean) dist. (error bar rep. fit RMS) | After corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeOffnRMSPS = new TH1F("h_atimeOffnRMSPS","Peak pos. of (PS ADCtime - TH ClusTmean) dist. (error bar rep. fit RMS) | Before corr.",kNblksPS,0,kNblksPS);
  TH1F *h_atimeOffnRMSPS_corr = new TH1F("h_atimeOffnRMSPS_corr","Peak pos. of (PS ADCtime - TH ClusTmean) dist. (error bar rep. fit RMS) | After corr.",kNblksPS,0,kNblksPS);
  Custm1DHisto(h_atimeOffnRMSSH); Custm1DHisto(h_atimeOffnRMSSH_corr); 
  Custm1DHisto(h_atimeOffnRMSPS); Custm1DHisto(h_atimeOffnRMSPS_corr);

  TH2F *h2_count_SH = new TH2F("h2_count_SH","# events per SH block;SH columns;SH rows",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH); 
  TH2F *h2_count_PS = new TH2F("h2_count_PS","# events per PS block;PS columns;PS rows",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS); 

  TH2F *h2_atimeOffSH_detview = new TH2F("h2_atimeOffSH_detview","SH ADCtime - TH ClusTmean (ns) | Before corr.;SH columns;SH rows",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH); 
  TH2F *h2_atimeOffSH_detview_corr = new TH2F("h2_atimeOffSH_detview_corr","SH ADCtime - TH ClusTmean (ns) | After corr.;SH columns;SH rows",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH); 
  TH2F *h2_atimeOffPS_detview = new TH2F("h2_atimeOffPS_detview","PS ADCtime - TH ClusTmean (ns) | Before corr.;PS columns;PS rows",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS); 
  TH2F *h2_atimeOffPS_detview_corr = new TH2F("h2_atimeOffPS_detview_corr","PS ADCtime - TH ClusTmean (ns) | After corr.;PS columns;PS rows",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS); 

  TH2F *h2_atimeOffSH_vs_blk = new TH2F("h2_atimeOffSH_vs_blk","Before offset correction (SH);SH block id;SH ADCtime - TH ClusTmean (ns)",kNblksSH,0,kNblksSH,240,-40,40); 
  TH2F *h2_atimeOffSH_vs_blk_corr = new TH2F("h2_atimeOffSH_vs_blk_corr","After offset correction (SH);SH block id;SH ADCtime - TH ClusTmean (ns)",kNblksSH,0,kNblksSH,240,-40,40); 
  TH2F *h2_atimeOffPS_vs_blk = new TH2F("h2_atimeOffPS_vs_blk","Before offset correction (PS);PS block id;PS ADCtime - TH ClusTmean (ns)",kNblksPS,0,kNblksPS,240,-40,40);
  TH2F *h2_atimeOffPS_vs_blk_corr = new TH2F("h2_atimeOffPS_vs_blk_corr","Before offset correction (PS);PS block id;PS ADCtime - TH ClusTmean (ns)",kNblksPS,0,kNblksPS,240,-40,40);

  ///////////////////////////////////////////
  // 1st Loop over all events to calibrate //
  ///////////////////////////////////////////

  cout << endl;  
  Long64_t nevent=0, nevents=C->GetEntries();
  Double_t timekeeper = 0., timeremains = 0.;
  int treenum = 0, currenttreenum = 0;
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

      double sh_atimeOff = hodo_tmean[0] - sh_clblk_atime[0]; //- sh_atimeblk);
      double ps_atimeOff = hodo_tmean[0] - ps_clblk_atime[0]; //- ps_atimeblk);

      h_atime_sh[(int)sh_rowblk][(int)sh_colblk]->Fill(sh_atimeOff);
      h_atime_ps[(int)ps_rowblk][(int)ps_colblk]->Fill(ps_atimeOff);

      h2_count_SH->Fill(sh_colblk, sh_rowblk, 1.);
      h2_atimeOffSH_detview_raw->Fill(sh_colblk, sh_rowblk, sh_atimeOff);
      h2_count_PS->Fill(ps_colblk, ps_rowblk, 1.);
      h2_atimeOffPS_detview_raw->Fill(ps_colblk, ps_rowblk, ps_atimeOff);

      h2_atimeOffSH_vs_blk->Fill(sh_idblk, sh_atimeOff);
      h2_atimeOffPS_vs_blk->Fill(ps_idblk, ps_atimeOff);

      // h_atime_sh[(int)sh_rowblk][(int)sh_colblk]->Fill(sh_atimeblk);
      // h_atime_ps[(int)ps_rowblk][(int)ps_colblk]->Fill(ps_atimeblk);

    } //global cut
  } //while
  cout << endl << endl; 

  h2_atimeOffSH_detview->Divide(h2_atimeOffSH_detview_raw, h2_count_SH); h2_atimeOffSH_detview->GetZaxis()->SetRangeUser(-10,10);
  h2_atimeOffPS_detview->Divide(h2_atimeOffPS_detview_raw, h2_count_PS); h2_atimeOffPS_detview->GetZaxis()->SetRangeUser(-10,10);

  ///////////////////////////////////////////////////
  // Time to calculate and report ADC time offsets //
  ///////////////////////////////////////////////////

  // Let's fit the histograms with Gaussian function 
  TF1 *fgaus = new TF1("fgaus","gaus");

  // fitting SH histograms
  cout << "Fitting SH histograms to calculate offsets..\n";
  int sub = 0;
  for(int r=0; r<kNrowsSH; r++){
    for(int c=0; c<kNcolsSH; c++){
      int blkid = r*kNcolsSH+c;

      sub = r/7;
      subCanv[sub]->cd((r%7)*kNcolsSH + c + 1);

      int maxBin = h_atime_sh[r][c]->GetMaximumBin();
      double maxBinCenter = h_atime_sh[r][c]->GetXaxis()->GetBinCenter( maxBin );
      double maxCount = h_atime_sh[r][c]->GetMaximum();
      double binWidth = h_atime_sh[r][c]->GetBinWidth(maxBin);
      double stdDev = h_atime_sh[r][c]->GetStdDev();
      double lofitlim = h_atime_min + (maxBin)*binWidth - (1.3*stdDev);
      double hifitlim = h_atime_min + (maxBin)*binWidth + (0.8*stdDev);

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
	
	h_atimeOffSH->Fill(r*kNcolsSH+c, fgaus->GetParameter(1));
	h_atimeOffSH->SetBinError(r*kNcolsSH+c, fgaus->GetParError(1));

	h_atimeRMSSH->Fill(r*kNcolsSH+c, fgaus->GetParameter(2));
	h_atimeRMSSH->SetBinError(r*kNcolsSH+c, fgaus->GetParError(2));

	h_atimeOffnRMSSH->Fill(r*kNcolsSH+c, fgaus->GetParameter(1));
	h_atimeOffnRMSSH->SetBinError(r*kNcolsSH+c, fgaus->GetParameter(2));

	toffset_shdata << fgaus->GetParameter(1) << " "; 
	ash_atimeOffs[blkid] = fgaus->GetParameter(1);
      }else{
	toffset_shdata << 0. << " ";
	ash_atimeOffs[blkid] = 0.; 
      }

      h_atime_sh[r][c]->SetTitle(Form("Time Offset | SH%d-%d",r+1,c+1));
      h_atime_sh[r][c]->Draw();
    }
    toffset_shdata << endl;
  }

  //fitting PS histograms
  cout << "Fitting PS histograms to calculate offsets..\n";
  sub = 0;
  for(int r=0; r<kNrowsPS; r++){
    for(int c=0; c<kNcolsPS; c++){
      int blkid = r*kNcolsPS+c;

      sub = r/7 + 4;
      subCanv[sub]->cd((r%7)*kNcolsPS + c + 1);

      int maxBin = h_atime_ps[r][c]->GetMaximumBin();
      double maxBinCenter = h_atime_ps[r][c]->GetXaxis()->GetBinCenter( maxBin );
      double maxCount = h_atime_ps[r][c]->GetMaximum();
      double binWidth = h_atime_ps[r][c]->GetBinWidth(maxBin);
      double stdDev = h_atime_ps[r][c]->GetStdDev();
      double lofitlim = h_atime_min + (maxBin)*binWidth - (1.2*stdDev);
      double hifitlim = h_atime_min + (maxBin)*binWidth + (0.5*stdDev);

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

	h_atimeOffPS->Fill(r*kNcolsPS+c, fgaus->GetParameter(1));
	h_atimeOffPS->SetBinError(r*kNcolsPS+c, fgaus->GetParError(1));

	h_atimeRMSPS->Fill(r*kNcolsPS+c, fgaus->GetParameter(2));
	h_atimeRMSPS->SetBinError(r*kNcolsPS+c, fgaus->GetParError(2));

	h_atimeOffnRMSPS->Fill(r*kNcolsPS+c, fgaus->GetParameter(1));
	h_atimeOffnRMSPS->SetBinError(r*kNcolsPS+c, fgaus->GetParameter(2));

	toffset_psdata << fgaus->GetParameter(1) << " "; 
	aps_atimeOffs[blkid] = fgaus->GetParameter(1);
      }else{
	toffset_psdata << 0. << " "; 
	aps_atimeOffs[blkid] = 0.;
      }

      h_atime_ps[r][c]->SetTitle(Form("Time Offset | PS%d-%d",r+1,c+1));
      h_atime_ps[r][c]->Draw();    
    }
    toffset_psdata << endl;
  }

  /////////////////////////////////////////////////////////////////////
  // 2nd Loop over all events to check the performance of correction //
  /////////////////////////////////////////////////////////////////////

  nevent = 0;
  Long64_t itr = 0; 
  cout << "\nLooping over events again to check corrections..\n" << endl; 
  while(C->GetEntry(nevent++)) {
    // reporting progress
    if (nevent % 100 == 0) cout << nevent << "/" << nevents  << "\r";;
    cout.flush();

    if (nevent == goodevents[itr]) { // choosing good events
      itr++;

      double sh_atimeOff_corr = hodo_tmean[0] - sh_clblk_atime[0] + ash_atimeOffs[(int)sh_idblk];
      double ps_atimeOff_corr = hodo_tmean[0] - ps_clblk_atime[0] + aps_atimeOffs[(int)ps_idblk];

      h_atime_sh_corr[(int)sh_rowblk][(int)sh_colblk]->Fill(sh_atimeOff_corr);
      h_atime_ps_corr[(int)ps_rowblk][(int)ps_colblk]->Fill(ps_atimeOff_corr);

      h2_atimeOffSH_detview_raw_corr->Fill(sh_colblk, sh_rowblk, sh_atimeOff_corr);
      h2_atimeOffPS_detview_raw_corr->Fill(ps_colblk, ps_rowblk, ps_atimeOff_corr);

      h2_atimeOffSH_vs_blk_corr->Fill(sh_idblk, sh_atimeOff_corr);
      h2_atimeOffPS_vs_blk_corr->Fill(ps_idblk, ps_atimeOff_corr);      
    }//global cut
  } //while
  cout << endl << endl;

  h2_atimeOffSH_detview_corr->Divide(h2_atimeOffSH_detview_raw_corr, h2_count_SH); h2_atimeOffSH_detview_corr->GetZaxis()->SetRangeUser(-10,10);
  h2_atimeOffPS_detview_corr->Divide(h2_atimeOffPS_detview_raw_corr, h2_count_PS); h2_atimeOffPS_detview_corr->GetZaxis()->SetRangeUser(-10,10);

  

  TCanvas *c1 = new TCanvas("c1","c1",1200,800);
  c1->Divide(2,2);
  c1->cd(1); //
  h2_atimeOffPS_detview->SetStats(0);
  h2_atimeOffPS_detview->Draw("colz text");
  c1->cd(2); //
  h2_atimeOffPS_detview_corr->SetStats(0);
  h2_atimeOffPS_detview_corr->Draw("colz text");
  c1->cd(3); //
  h2_atimeOffSH_detview->SetStats(0);
  h2_atimeOffSH_detview->Draw("colz text");
  c1->cd(4); //
  h2_atimeOffSH_detview_corr->SetStats(0);
  h2_atimeOffSH_detview_corr->Draw("colz text");
  c1->SaveAs(Form("%s[",outPeaks.Data())); c1->SaveAs(Form("%s",outPeaks.Data())); c1->Write();

  TCanvas *c2 = new TCanvas("c2","c2",1200,800);
  c2->Divide(1,2);
  c2->cd(1); //
  gPad->SetGridy();
  h2_atimeOffPS_vs_blk->SetStats(0);
  h2_atimeOffPS_vs_blk->Draw("colz");
  h_atimeOffnRMSPS->Draw("same");
  c2->cd(2); //
  gPad->SetGridy();
  h2_atimeOffSH_vs_blk->SetStats(0);
  h2_atimeOffSH_vs_blk->Draw("colz");
  h_atimeOffnRMSSH->Draw("same");
  c2->SaveAs(Form("%s",outPeaks.Data())); c2->Write();

  //subCanv[0]->SaveAs(Form("%s[",outPeaks.Data()));
  for(int canC=0; canC<8; canC++) {subCanv[canC]->SaveAs(Form("%s",outPeaks.Data())); subCanv[canC]->Write();}
  subCanv[7]->SaveAs(Form("%s]",outPeaks.Data()));

  fout->Write();
  //fout->Close();
  //fout->Delete();
 
  cout << "Finishing analysis .." << endl;
  cout << " --------- " << endl;
  cout << " Plots saved to : " << outPeaks << endl;
  cout << " Histogram written to : " << outFile << endl;
  cout << " ADCtime offsets for SH written to : " << toffset_sh << endl;
  cout << " ADCtime offsets for PS written to : " << toffset_ps << endl;
  cout << " --------- " << endl;

  sw->Stop(); sw2->Stop();
  cout << "CPU time = " << sw->CpuTime() << "s. Real time = " << sw->RealTime() << "s. \n\n";

}

// Create generic histogram function
TH1F* MakeHisto(Int_t row, Int_t col, char const * suf, Int_t bins, Double_t min=0., Double_t max=50.)
{
  Int_t blkid = row*kNcolsSH+col;
  TH1F *h = new TH1F(TString::Format("h_R%d_C%d_Blk_%d%s",row,col,blkid,suf),"",bins,min,max);
  //h->SetStats(0);
  h->SetLineWidth(1);
  h->SetLineColor(2);
  h->GetYaxis()->SetLabelSize(0.06);
  //h->GetYaxis()->SetLabelOffset(-0.17);
  h->GetYaxis()->SetMaxDigits(3);
  h->GetYaxis()->SetNdivisions(5);
  return h;
}

// Customizes 1D histos
void Custm1DHisto(TH1F* h)
{
  h->GetYaxis()->SetLabelSize(0.045);
  h->GetXaxis()->SetLabelSize(0.045);
  h->SetLineWidth(1);
  h->SetLineColor(kBlack);
  h->SetMarkerSize(0.9);
  h->SetMarkerColor(kRed);
  h->SetMarkerStyle(kFullCircle);
}

// Customizes 2D histos
void Custm2DHisto(TH2F* h)
{
  h->GetYaxis()->SetLabelSize(0.045);
  h->GetXaxis()->SetLabelSize(0.045);
}
