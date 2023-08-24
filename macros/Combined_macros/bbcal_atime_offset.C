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

string getDate();
void Custm1DHisto(TH1F*);
TString GetOutFileBase(TString);
void ReadOffset(TString, Double_t*);
TH1F* MakeHisto(Int_t, Int_t, char const *, Int_t, Double_t, Double_t);
std::vector<std::string> SplitString(char const delim, std::string const myStr);
void Custm2DRnumHisto(TH2F*, std::vector<std::string> const & lrnum);

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
void bbcal_atime_offset (char const * configfilename, bool isdebug = 1) {

  gErrorIgnoreLevel = kError; // Ignore all ROOT warnings

  // Define a clock to check macro processing time
  TStopwatch *sw = new TStopwatch();
  TStopwatch *sw2 = new TStopwatch();
  sw->Start(); sw2->Start();

  //gui setup
  shgui::SetupGUI();

  TChain *C = new TChain("T");
  //creating base for outfile names
  TString cfgfilebase = GetOutFileBase(configfilename);

  TString exp = "unknown";
  Int_t config = -1;       // Experimental configuration
  TString set = "N/A";     // Needed when we have multiple calibration sets within a config
  Int_t ppass = -1;        // Replay pass to get ready for
  Double_t Ebeam = 0.;     // GeV
  Double_t atpos_nom = 0.;   // ns
  Double_t h_atime_blk_bin = 240, h_atime_blk_min = -60., h_atime_blk_max = 60.;
  Double_t h_atime_bin = 240, h_atime_min = -60., h_atime_max = 60.;
  Double_t h_atime_off_bin = 240, h_atime_off_min = -60., h_atime_off_max = 60.;

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
  std::vector<std::string> gCutList = SplitString('&', gcutstr.Data());
  TTreeFormula *GlobalCut = new TTreeFormula("GlobalCut", globalcut, C);
  while( currentline.ReadLine( configfile ) ){
    if( currentline.BeginsWith("#") ) continue;
    TObjArray *tokens = currentline.Tokenize(" ");
    Int_t ntokens = tokens->GetEntries();
    if( ntokens>1 ){
      TString skey = ( (TObjString*)(*tokens)[0] )->GetString();
      if (skey == "exp") exp = ((TObjString*)(*tokens)[1])->GetString();
      if (skey == "config") config = ((TObjString*)(*tokens)[1])->GetString().Atoi();
      if (skey == "set") set = ((TObjString*)(*tokens)[1])->GetString();
      if (skey == "pre_pass") ppass = ((TObjString*)(*tokens)[1])->GetString().Atoi();
      if (skey == "E_beam") Ebeam = ((TObjString*)(*tokens)[1])->GetString().Atof();
      if (skey == "atpos_nom") atpos_nom = ((TObjString*)(*tokens)[1])->GetString().Atof();
      if (skey == "h_atime_blk") {
	h_atime_blk_bin = ((TObjString*)(*tokens)[1])->GetString().Atof();
	h_atime_blk_min = ((TObjString*)(*tokens)[2] )->GetString().Atof();
	h_atime_blk_max = ((TObjString*)(*tokens)[3] )->GetString().Atof();
      }
      if (skey == "h_atime") {
	h_atime_bin = ((TObjString*)(*tokens)[1])->GetString().Atof();
	h_atime_min = ((TObjString*)(*tokens)[2] )->GetString().Atof();
	h_atime_max = ((TObjString*)(*tokens)[3] )->GetString().Atof();
      }
      if (skey == "h_atime_off") {
	h_atime_off_bin = ((TObjString*)(*tokens)[1])->GetString().Atof();
	h_atime_off_min = ((TObjString*)(*tokens)[2] )->GetString().Atof();
	h_atime_off_max = ((TObjString*)(*tokens)[3] )->GetString().Atof();
      }
      if( skey == "*****" ){
	break;
      }
    } 
    delete tokens;
  }
  atpos_nom = -1.*abs(atpos_nom); // Foolproofing - should always be a positive no.

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
  // Event info
  C->SetMakeClass(1);
  C->SetBranchStatus("fEvtHdr.*", 1);
  UInt_t rnum;           C->SetBranchAddress("fEvtHdr.fRun", &rnum);
  UInt_t trigbits;       C->SetBranchAddress("fEvtHdr.fTrigBits", &trigbits);
  ULong64_t gevnum;      C->SetBranchAddress("fEvtHdr.fEvtNum", &gevnum);
  // turning on additional branches for the global cut
  C->SetBranchStatus("e.kine.W2",1);
  C->SetBranchStatus("bb.tr.n",1);
  C->SetBranchStatus("bb.tr.vz",1);
  C->SetBranchStatus("bb.gem.track.nhits",1);
  if (exp=="gmn" && ppass<=2 && config>7) C->SetBranchStatus("g.trigbits",1);

  // creating atimeOff histograms per BBCal block
  for(int r = 0; r < kNrowsSH; r++) {
    for(int c = 0; c < kNcolsSH; c++) {
      int blkid = r*kNcolsSH+c;
      ash_atimeOffs[blkid] = -1000;
      h_atime_sh[r][c] = MakeHisto(r, c, "", h_atime_blk_bin, h_atime_blk_min, h_atime_blk_max);
      h_atime_sh_corr[r][c] = MakeHisto(r, c, "_corr", h_atime_blk_bin, h_atime_blk_min, h_atime_blk_max);
    }
  }
  for(int r = 0; r < kNrowsPS; r++) {
    for(int c = 0; c < kNcolsPS; c++) {
      int blkid = r*kNcolsPS+c;
      aps_atimeOffs[blkid] = -1000;
      h_atime_ps[r][c] = MakeHisto(r, c, "", h_atime_blk_bin, h_atime_blk_min, h_atime_blk_max);
      h_atime_ps_corr[r][c] = MakeHisto(r, c, "_corr", h_atime_blk_bin, h_atime_blk_min, h_atime_blk_max);
    }
  }

  // Let's read in old ADC time offsets for both SH and PS
  std::cout << std::endl;
  Double_t old_ash_atimeOffs[kNblksSH];
  Double_t old_aps_atimeOffs[kNblksPS];
  for (int i=0; i<kNblksSH; i++) { old_ash_atimeOffs[i] = -1000; }  
  for (int i=0; i<kNblksPS; i++) { old_aps_atimeOffs[i] = -1000; }  
  TString atimeOff_sh, atimeOff_ps;
  char const * exptag = "unknown";
  if (exp=="gmn") exptag = "sbs";
  else if (exp=="gen") exptag = "gen";
  int cpass = ppass-1;
  if (exp=="gmn" && ppass==2) cpass = 0;
  char const * setno = set.Atoi() < 0 ? "" : ("_set" + set).Data();
  atimeOff_sh = Form("Output/%s%d%s_prepass%d_atimeOff_sh.txt",exptag,config,setno,cpass);
  atimeOff_ps = Form("Output/%s%d%s_prepass%d_atimeOff_ps.txt",exptag,config,setno,cpass);
  ReadOffset(atimeOff_sh, old_ash_atimeOffs);
  ReadOffset(atimeOff_ps, old_aps_atimeOffs);

  // define output files
  char const * debug = isdebug ? "_test" : "";
  TString outFile, outPeaks, toffset_ps, toffset_sh;
  outPeaks = Form("plots/%s%d%s_prepass%d_atimeOff%s.pdf",exptag,config,setno,ppass,debug);
  outFile = Form("hist/%s%d%s_prepass%d_atimeOff%s.root",exptag,config,setno,ppass,debug);
  toffset_sh = Form("Output/%s%d%s_prepass%d_atimeOff_sh%s.txt",exptag,config,setno,ppass,debug);
  toffset_ps = Form("Output/%s%d%s_prepass%d_atimeOff_ps%s.txt",exptag,config,setno,ppass,debug);
  ofstream toffset_psdata, toffset_shdata;
  toffset_psdata.open(toffset_ps);
  toffset_shdata.open(toffset_sh);
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  // defining important histograms
  TH1F *h_W = new TH1F("h_W","W distribution",200,0.,5.);
  TH1F *h_Q2 = new TH1F("h_Q2","Q2 distribution",300,1.,15.);

  TH1F *h_atimeSH = new TH1F("h_atimeSH","SH ADC time | Before corr.",h_atime_bin,h_atime_min,h_atime_max);
  TH1F *h_atimeSH_corr = new TH1F("h_atimeSH_corr","SH ADC time | After corr.",h_atime_bin,h_atime_min,h_atime_max);
  TH1F *h_atimePS = new TH1F("h_atimePS","PS ADC time | Before corr.",h_atime_bin,h_atime_min,h_atime_max);
  TH1F *h_atimePS_corr = new TH1F("h_atimePS_corr","PS ADC time | After corr.",h_atime_bin,h_atime_min,h_atime_max);

  TH1F *h_atimeOffSH = new TH1F("h_atimeOffSH","Peak pos. of (TH ClusTmean - SH ADCtime) dist. (w/ fit error) | Before corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeOffSH_corr = new TH1F("h_atimeOffSH_corr","Peak pos. of (TH ClusTmean - SH ADCtime) dist. (w/ fit error) | After corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeOffPS = new TH1F("h_atimeOffPS","Peak pos. of (TH ClusTmean - PS ADCtime) dist. (w/ fit error) | Before corr.",kNblksPS,0,kNblksPS);
  TH1F *h_atimeOffPS_corr = new TH1F("h_atimeOffPS_corr","Peak pos. of (TH ClusTmean - PS ADCtime) dist. (w/ fit error) | After corr.",kNblksPS,0,kNblksPS);
  Custm1DHisto(h_atimeOffSH); Custm1DHisto(h_atimeOffSH_corr); 
  Custm1DHisto(h_atimeOffPS); Custm1DHisto(h_atimeOffPS_corr);

  TH1F *h_atimeRMSSH = new TH1F("h_atimeRMSSH","Peak RMS of (TH ClusTmean - SH ADCtime) dist. (w/ fit error) | Before corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeRMSSH_corr = new TH1F("h_atimeRMSSH_corr","Peak RMS of (TH ClusTmean - SH ADCtime) dist. (w/ fit error) | After corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeRMSPS = new TH1F("h_atimeRMSPS","Peak RMS of (TH ClusTmean - PS ADCtime) dist. (w/ fit error) | Before corr.",kNblksPS,0,kNblksPS);
  TH1F *h_atimeRMSPS_corr = new TH1F("h_atimeRMSPS_corr","Peak RMS of (TH ClusTmean - PS ADCtime) dist. (w/ fit error) | After corr.",kNblksPS,0,kNblksPS);
  Custm1DHisto(h_atimeRMSSH); Custm1DHisto(h_atimeRMSSH_corr); 
  Custm1DHisto(h_atimeRMSPS); Custm1DHisto(h_atimeRMSPS_corr);

  TH1F *h_atimeOffnRMSSH = new TH1F("h_atimeOffnRMSSH","Peak pos. of (TH ClusTmean - SH ADCtime) dist. (error bar rep. fit RMS) | Before corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeOffnRMSSH_corr = new TH1F("h_atimeOffnRMSSH_corr","Peak pos. of (TH ClusTmean - SH ADCtime) dist. (error bar rep. fit RMS) | After corr.",kNblksSH,0,kNblksSH);
  TH1F *h_atimeOffnRMSPS = new TH1F("h_atimeOffnRMSPS","Peak pos. of (TH ClusTmean - PS ADCtime) dist. (error bar rep. fit RMS) | Before corr.",kNblksPS,0,kNblksPS);
  TH1F *h_atimeOffnRMSPS_corr = new TH1F("h_atimeOffnRMSPS_corr","Peak pos. of (TH ClusTmean - PS ADCtime) dist. (error bar rep. fit RMS) | After corr.",kNblksPS,0,kNblksPS);
  Custm1DHisto(h_atimeOffnRMSSH); Custm1DHisto(h_atimeOffnRMSSH_corr); 
  Custm1DHisto(h_atimeOffnRMSPS); Custm1DHisto(h_atimeOffnRMSPS_corr);

  TH2F *h2_count_SH = new TH2F("h2_count_SH","# events per SH block;SH columns;SH rows",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH); 
  TH2F *h2_count_PS = new TH2F("h2_count_PS","# events per PS block;PS columns;PS rows",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS); 

  TH2F *h2_atimeOffSH_detview = new TH2F("h2_atimeOffSH_detview","TH ClusTmean - SH ADCtime (ns) | Before corr.;SH columns;SH rows",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH); 
  TH2F *h2_atimeOffSH_detview_corr = new TH2F("h2_atimeOffSH_detview_corr","TH ClusTmean - SH ADCtime (ns) | After corr.;SH columns;SH rows",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH); 
  TH2F *h2_atimeOffPS_detview = new TH2F("h2_atimeOffPS_detview","TH ClusTmean - PS ADCtime (ns) | Before corr.;PS columns;PS rows",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS); 
  TH2F *h2_atimeOffPS_detview_corr = new TH2F("h2_atimeOffPS_detview_corr","TH ClusTmean - PS ADCtime (ns) | After corr.;PS columns;PS rows",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS); 

  TH2F *h2_atimeOffSH_vs_blk = new TH2F("h2_atimeOffSH_vs_blk","Before offset correction (SH);SH block id;TH ClusTmean - SH ADCtime (ns)",kNblksSH,0,kNblksSH,h_atime_off_bin,h_atime_off_min,h_atime_off_max); 
  TH2F *h2_atimeOffSH_vs_blk_corr = new TH2F("h2_atimeOffSH_vs_blk_corr","After offset correction (SH);SH block id;TH ClusTmean - SH ADCtime (ns)",kNblksSH,0,kNblksSH,h_atime_off_bin,h_atime_off_min,h_atime_off_max); 
  TH2F *h2_atimeOffPS_vs_blk = new TH2F("h2_atimeOffPS_vs_blk","Before offset correction (PS);PS block id;TH ClusTmean - PS ADCtime (ns)",kNblksPS,0,kNblksPS,h_atime_off_bin,h_atime_off_min,h_atime_off_max);
  TH2F *h2_atimeOffPS_vs_blk_corr = new TH2F("h2_atimeOffPS_vs_blk_corr","After offset correction (PS);PS block id;TH ClusTmean - PS ADCtime (ns)",kNblksPS,0,kNblksPS,h_atime_off_bin,h_atime_off_min,h_atime_off_max);

  Double_t Nruns = 1000; // Max # runs we anticipate to analyze 
  TH2F *h2_atimeOffSH_vs_rnum = new TH2F("h2_atimeOffSH_vs_rnum","Before offset correction (SH);Run no.;TH ClusTmean - SH ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_off_bin,h_atime_off_min,h_atime_off_max); 
  TH2F *h2_atimeOffSH_vs_rnum_corr = new TH2F("h2_atimeOffSH_vs_rnum_corr","After offset correction (SH);Run no.;TH ClusTmean - SH ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_off_bin,h_atime_off_min,h_atime_off_max); 
  TH2F *h2_atimeOffPS_vs_rnum = new TH2F("h2_atimeOffPS_vs_rnum","Before offset correction (PS);Run no.;TH ClusTmean - PS ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_off_bin,h_atime_off_min,h_atime_off_max);
  TH2F *h2_atimeOffPS_vs_rnum_corr = new TH2F("h2_atimeOffPS_vs_rnum_corr","After offset correction (PS);Run no.;TH ClusTmean - PS ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_off_bin,h_atime_off_min,h_atime_off_max);

  TH2F *h2_atimeSH_vs_rnum = new TH2F("h2_atimeSH_vs_rnum","Before offset correction (SH);Run no.;SH ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_bin,h_atime_min,h_atime_max);
  TH2F *h2_atimeSH_vs_rnum_corr = new TH2F("h2_atimeSH_vs_rnum_corr","After offset correction (SH);Run no.;SH ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_bin,h_atime_min,h_atime_max);
  TH2F *h2_atimePS_vs_rnum = new TH2F("h2_atimePS_vs_rnum","Before offset correction (PS);Run no.;PS ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_bin,h_atime_min,h_atime_max);
  TH2F *h2_atimePS_vs_rnum_corr = new TH2F("h2_atimePS_vs_rnum_corr","After offset correction (PS);Run no.;PS ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_bin,h_atime_min,h_atime_max);

  ///////////////////////////////////////////
  // 1st Loop over all events to calibrate //
  ///////////////////////////////////////////

  cout << endl;  
  Long64_t nevent=0, nevents=C->GetEntries(); UInt_t runnum=0;
  Double_t timekeeper = 0., timeremains = 0.;
  int treenum = 0, currenttreenum = 0, itrrun=0;
  std::vector<std::string> lrnum;    // list of run numbers
  lrnum.reserve(100); 

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

      // track change of runnum
      if (nevent == 1 || rnum != runnum) {
	runnum = rnum; itrrun++;
	lrnum.push_back(to_string(rnum));
      }
    } 
    //lrnum.push_back(to_string(rnum));
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

      // storing good event numbers
      goodevents.push_back(nevent);

      double sh_atimeOff = hodo_tmean[0] - sh_clblk_atime[0];
      double ps_atimeOff = hodo_tmean[0] - ps_clblk_atime[0];

      h_atimeSH->Fill(sh_clblk_atime[0]);
      h_atimePS->Fill(ps_clblk_atime[0]);

      h_atime_sh[(int)sh_rowblk][(int)sh_colblk]->Fill(sh_atimeOff);
      h_atime_ps[(int)ps_rowblk][(int)ps_colblk]->Fill(ps_atimeOff);

      h2_count_SH->Fill(sh_colblk, sh_rowblk, 1.);
      h2_count_PS->Fill(ps_colblk, ps_rowblk, 1.);

      h2_atimeOffSH_vs_blk->Fill(sh_idblk, sh_atimeOff);
      h2_atimeOffPS_vs_blk->Fill(ps_idblk, ps_atimeOff);

      h2_atimeOffSH_vs_rnum->Fill(itrrun, sh_atimeOff);
      h2_atimeOffPS_vs_rnum->Fill(itrrun, ps_atimeOff);

      h2_atimeSH_vs_rnum->Fill(itrrun, sh_clblk_atime[0]);
      h2_atimePS_vs_rnum->Fill(itrrun, ps_clblk_atime[0]);
    } //global cut
  } //while
  cout << endl << endl; 

  // customizing histos with run # on the x-axis
  Custm2DRnumHisto(h2_atimeSH_vs_rnum, lrnum); Custm2DRnumHisto(h2_atimeOffSH_vs_rnum, lrnum); 
  Custm2DRnumHisto(h2_atimePS_vs_rnum, lrnum); Custm2DRnumHisto(h2_atimeOffPS_vs_rnum, lrnum);

  ///////////////////////////////////////////////////
  // Time to calculate and report ADC time offsets //
  ///////////////////////////////////////////////////

  // Let's fit the histograms with Gaussian function 
  TF1 *fgaus = new TF1("fgaus","gaus");

  // fitting SH histograms
  cout << "Fitting SH ADC time histograms to calculate offsets..\n";
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
      double lofitlim = h_atime_blk_min + (maxBin)*binWidth - (1.0*stdDev);
      double hifitlim = h_atime_blk_min + (maxBin)*binWidth + (0.6*stdDev);

      if (h_atime_sh[r][c]->GetEntries()>20 && stdDev>2.*binWidth) {

	// Create fit functions for each module
	fgaus->SetLineColor(1);
	fgaus->SetNpx(1000);

	if (h_atime_sh[r][c]->GetBinContent(maxBin-1) == 0.) {
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

	fgaus->SetParameters(maxCount,maxBinCenter,stdDev);
	fgaus->SetRange(lofitlim, hifitlim);
	h_atime_sh[r][c]->Fit(fgaus,"+RQ");

	double mean = fgaus->GetParameter(1);
	double meanerr = fgaus->GetParError(1);
	double rms = fgaus->GetParameter(2);
	double rmserr = fgaus->GetParError(2);
	
	h_atimeOffSH->Fill(blkid, mean);
	h_atimeOffSH->SetBinError(blkid, meanerr);

	h_atimeRMSSH->Fill(blkid, rms);
	h_atimeRMSSH->SetBinError(blkid, rmserr);

	h_atimeOffnRMSSH->Fill(blkid, mean);
	h_atimeOffnRMSSH->SetBinError(blkid, rms);

	h2_atimeOffSH_detview->Fill(c, r, mean);

	cout << mean + old_ash_atimeOffs[blkid] << " "; 
	toffset_shdata << mean + old_ash_atimeOffs[blkid] << " "; 
	ash_atimeOffs[blkid] = mean;
      }else{
	cout << atpos_nom << " ";
	toffset_shdata << atpos_nom << " ";
	ash_atimeOffs[blkid] = atpos_nom; 
      }

      h_atime_sh[r][c]->SetTitle(Form("Time Offset | SH%d-%d",r+1,c+1));
      h_atime_sh[r][c]->Draw();
    }
    cout << endl;
    toffset_shdata << endl;
  }

  //fitting PS histograms
  cout << "\nFitting PS ADC time histograms to calculate offsets..\n";
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
      double lofitlim = h_atime_blk_min + (maxBin)*binWidth - (0.8*stdDev);
      double hifitlim = h_atime_blk_min + (maxBin)*binWidth + (0.4*stdDev);

      if (h_atime_ps[r][c]->GetEntries()>20 && stdDev>2.*binWidth) {

	// Create fit functions for each module
	fgaus->SetLineColor(1);
	fgaus->SetNpx(1000);

	if (h_atime_ps[r][c]->GetBinContent(maxBin-1) == 0.) {
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

	double mean = fgaus->GetParameter(1);
	double meanerr = fgaus->GetParError(1);
	double rms = fgaus->GetParameter(2);
	double rmserr = rmserr;

	h_atimeOffPS->Fill(blkid, mean);
	h_atimeOffPS->SetBinError(blkid, meanerr);

	h_atimeRMSPS->Fill(blkid, rms);
	h_atimeRMSPS->SetBinError(blkid, rmserr);

	h_atimeOffnRMSPS->Fill(blkid, mean);
	h_atimeOffnRMSPS->SetBinError(blkid, rms);

	h2_atimeOffPS_detview->Fill(c, r, mean);

	cout << mean + old_aps_atimeOffs[blkid] << " "; 
	toffset_psdata << mean + old_aps_atimeOffs[blkid] << " "; 
	aps_atimeOffs[blkid] = mean;
      }else{
	cout << atpos_nom << " "; 
	toffset_psdata << atpos_nom << " "; 
	aps_atimeOffs[blkid] = atpos_nom;
      }

      h_atime_ps[r][c]->SetTitle(Form("Time Offset | PS%d-%d",r+1,c+1));
      h_atime_ps[r][c]->Draw();    
    }
    cout << endl;
    toffset_psdata << endl;
  }

  /////////////////////////////////////////////////////////////////////
  // 2nd Loop over all events to check the performance of correction //
  /////////////////////////////////////////////////////////////////////

  nevent = 0; itrrun=0; runnum=0; 
  cout << "\nLooping over events again to check corrections..\n" << endl; 
  while(C->GetEntry(nevent++)) {
    // Calculating remaining time 
    sw2->Stop();
    timekeeper += sw2->RealTime();
    if (nevent % 25000 == 0 && nevent != 0) 
      timeremains = timekeeper * (double(nevents) / double(nevent) - 1.); 
    sw2->Reset();
    sw2->Continue();

    if(nevent % 100 == 0) std::cout << nevent << "/" << nevents  << ", " << int(timeremains/60.) << "m \r";;
    std::cout.flush();
    // ------

    // apply global cuts efficiently (AJRP method)
    currenttreenum = C->GetTreeNumber();
    if (nevent == 1 || currenttreenum != treenum) {
      treenum = currenttreenum;
      GlobalCut->UpdateFormulaLeaves();

      // track change of runnum
      if (nevent == 1 || rnum != runnum) {
	runnum = rnum; itrrun++;
      }
    } 
    bool passedgCut = GlobalCut->EvalInstance(0) != 0;   
    if (passedgCut) {

      double sh_atimeOff_corr = hodo_tmean[0] - sh_clblk_atime[0] - ash_atimeOffs[(int)sh_idblk];
      double ps_atimeOff_corr = hodo_tmean[0] - ps_clblk_atime[0] - aps_atimeOffs[(int)ps_idblk];

      h_atimeSH_corr->Fill(sh_clblk_atime[0] + ash_atimeOffs[(int)sh_idblk]);
      h_atimePS_corr->Fill(ps_clblk_atime[0] + aps_atimeOffs[(int)ps_idblk]);

      h_atime_sh_corr[(int)sh_rowblk][(int)sh_colblk]->Fill(sh_atimeOff_corr);
      h_atime_ps_corr[(int)ps_rowblk][(int)ps_colblk]->Fill(ps_atimeOff_corr);

      h2_atimeOffSH_vs_blk_corr->Fill(sh_idblk, sh_atimeOff_corr);
      h2_atimeOffPS_vs_blk_corr->Fill(ps_idblk, ps_atimeOff_corr);    

      h2_atimeOffSH_vs_rnum_corr->Fill(itrrun, sh_atimeOff_corr);
      h2_atimeOffPS_vs_rnum_corr->Fill(itrrun, ps_atimeOff_corr);  

      h2_atimeSH_vs_rnum_corr->Fill(itrrun, sh_clblk_atime[0] + ash_atimeOffs[(int)sh_idblk]);
      h2_atimePS_vs_rnum_corr->Fill(itrrun, ps_clblk_atime[0] + aps_atimeOffs[(int)ps_idblk]);
    }//global cut
  } //while
  cout << endl << endl;

  // customizing histos with run # on the x-axis
  Custm2DRnumHisto(h2_atimeSH_vs_rnum_corr, lrnum); Custm2DRnumHisto(h2_atimeOffSH_vs_rnum_corr, lrnum); 
  Custm2DRnumHisto(h2_atimePS_vs_rnum_corr, lrnum); Custm2DRnumHisto(h2_atimeOffPS_vs_rnum_corr, lrnum);

  ///////////////////////////////////////////////////////
  // Time to get the ADC time offsets after correction //
  ///////////////////////////////////////////////////////

  TCanvas *ctemp = new TCanvas("ctemp","",300,300);
  ctemp->cd(); //temporary canvas for the fits

  // fitting SH histograms
  cout << "Fitting SH ADC time histograms again to check the quality of correction..\n";
  sub = 0;
  for(int r=0; r<kNrowsSH; r++){
    for(int c=0; c<kNcolsSH; c++){
      int blkid = r*kNcolsSH+c;

      // sub = r/7;
      // subCanv[sub]->cd((r%7)*kNcolsSH + c + 1);

      int maxBin = h_atime_sh_corr[r][c]->GetMaximumBin();
      double maxBinCenter = h_atime_sh_corr[r][c]->GetXaxis()->GetBinCenter( maxBin );
      double maxCount = h_atime_sh_corr[r][c]->GetMaximum();
      double binWidth = h_atime_sh_corr[r][c]->GetBinWidth(maxBin);
      double stdDev = h_atime_sh_corr[r][c]->GetStdDev();
      double lofitlim = h_atime_blk_min + (maxBin)*binWidth - (1.0*stdDev);
      double hifitlim = h_atime_blk_min + (maxBin)*binWidth + (0.6*stdDev);

      if( h_atime_sh_corr[r][c]->GetEntries()>20 && stdDev>2.*binWidth){

	// Create fit functions for each module
	fgaus->SetLineColor(1);
	fgaus->SetNpx(1000);

	if( h_atime_sh_corr[r][c]->GetBinContent(maxBin-1) == 0. ){
	  while ( h_atime_sh_corr[r][c]->GetBinContent(maxBin+1) < h_atime_sh_corr[r][c]->GetBinContent(maxBin) || 
		  h_atime_sh_corr[r][c]->GetBinContent(maxBin+1) == h_atime_sh_corr[r][c]->GetBinContent(maxBin) ) 
	    {
	      maxBin++;
	    };
	  h_atime_sh_corr[r][c]->GetXaxis()->SetRange( maxBin+1 , h_atime_sh_corr[r][c]->GetNbinsX() );
	  maxBin = h_atime_sh_corr[r][c]->GetMaximumBin();
	  maxBinCenter = h_atime_sh_corr[r][c]->GetXaxis()->GetBinCenter( maxBin );
	  maxCount = h_atime_sh_corr[r][c]->GetMaximum();
	  binWidth = h_atime_sh_corr[r][c]->GetBinWidth(maxBin);
	  stdDev = h_atime_sh_corr[r][c]->GetStdDev();
	}

	fgaus->SetParameters( maxCount,maxBinCenter,stdDev );
	fgaus->SetRange( lofitlim, hifitlim );
	h_atime_sh_corr[r][c]->Fit(fgaus,"+RQ");

	double mean = fgaus->GetParameter(1);
	double meanerr = fgaus->GetParError(1);
	double rms = fgaus->GetParameter(2);
	double rmserr = fgaus->GetParError(2);
	
	h_atimeOffSH_corr->Fill(blkid, mean);
	h_atimeOffSH_corr->SetBinError(blkid, meanerr);

	h_atimeRMSSH_corr->Fill(blkid, rms);
	h_atimeRMSSH_corr->SetBinError(blkid, rmserr);

	h_atimeOffnRMSSH_corr->Fill(blkid, mean);
	h_atimeOffnRMSSH_corr->SetBinError(blkid, rms);

	h2_atimeOffSH_detview_corr->Fill(c, r, mean);

	// cout << mean << " "; 
	// toffset_shdata << mean << " "; 
	// ash_atimeOffs[blkid] = mean;
      }else{ // Needs more thoughts!
	// cout << 0. << " ";
	// toffset_shdata << 0. << " ";
	// ash_atimeOffs[blkid] = 0.; 
      }

      h_atime_sh_corr[r][c]->SetTitle(Form("Time Offset | SH%d-%d",r+1,c+1));
      h_atime_sh_corr[r][c]->Draw();
    }
    // cout << endl;
    // toffset_shdata << endl;
  }

  //fitting PS histograms
  cout << "Fitting PS ADC time histograms again to check the quality of correction..\n";
  sub = 0;
  for(int r=0; r<kNrowsPS; r++){
    for(int c=0; c<kNcolsPS; c++){
      int blkid = r*kNcolsPS+c;

      // sub = r/7 + 4;
      // subCanv[sub]->cd((r%7)*kNcolsPS + c + 1);

      int maxBin = h_atime_ps_corr[r][c]->GetMaximumBin();
      double maxBinCenter = h_atime_ps_corr[r][c]->GetXaxis()->GetBinCenter( maxBin );
      double maxCount = h_atime_ps_corr[r][c]->GetMaximum();
      double binWidth = h_atime_ps_corr[r][c]->GetBinWidth(maxBin);
      double stdDev = h_atime_ps_corr[r][c]->GetStdDev();
      double lofitlim = h_atime_blk_min + (maxBin)*binWidth - (0.8*stdDev);
      double hifitlim = h_atime_blk_min + (maxBin)*binWidth + (0.4*stdDev);

      if( h_atime_ps_corr[r][c]->GetEntries()>20 && stdDev>2.*binWidth){

	// Create fit functions for each module
	fgaus->SetLineColor(1);
	fgaus->SetNpx(1000);

	if( h_atime_ps_corr[r][c]->GetBinContent(maxBin-1) == 0. ){
	  while ( h_atime_ps_corr[r][c]->GetBinContent(maxBin+1) < h_atime_ps_corr[r][c]->GetBinContent(maxBin) || 
		  h_atime_ps_corr[r][c]->GetBinContent(maxBin+1) == h_atime_ps_corr[r][c]->GetBinContent(maxBin) ) 
	    {
	      maxBin++;
	    };
	  h_atime_ps_corr[r][c]->GetXaxis()->SetRange( maxBin+1 , h_atime_ps_corr[r][c]->GetNbinsX() );
	  maxBin = h_atime_ps_corr[r][c]->GetMaximumBin();
	  maxBinCenter = h_atime_ps_corr[r][c]->GetXaxis()->GetBinCenter( maxBin );
	  maxCount = h_atime_ps_corr[r][c]->GetMaximum();
	  binWidth = h_atime_ps_corr[r][c]->GetBinWidth(maxBin);
	  stdDev = h_atime_ps_corr[r][c]->GetStdDev();
	}

	fgaus->SetParameters( maxCount,maxBinCenter,stdDev );
	fgaus->SetRange( lofitlim, hifitlim );
	h_atime_ps_corr[r][c]->Fit(fgaus,"+RQ");

	double mean = fgaus->GetParameter(1);
	double meanerr = fgaus->GetParError(1);
	double rms = fgaus->GetParameter(2);
	double rmserr = fgaus->GetParError(2);

	h_atimeOffPS_corr->Fill(blkid, mean);
	h_atimeOffPS_corr->SetBinError(blkid, meanerr);

	h_atimeRMSPS_corr->Fill(blkid, rms);
	h_atimeRMSPS_corr->SetBinError(blkid, rmserr);

	h_atimeOffnRMSPS_corr->Fill(blkid, mean);
	h_atimeOffnRMSPS_corr->SetBinError(blkid, rms);

	h2_atimeOffPS_detview_corr->Fill(c, r, mean);

	// cout << mean << " "; 
	// toffset_psdata << mean << " "; 
	// aps_atimeOffs[blkid] = mean;
      }else{
	// cout << 0. << " "; 
	// toffset_psdata << 0. << " "; 
	// aps_atimeOffs[blkid] = 0.;
      }

      h_atime_ps_corr[r][c]->SetTitle(Form("Time Offset | PS%d-%d",r+1,c+1));
      h_atime_ps_corr[r][c]->Draw();    
    }
    // cout << endl;
    // toffset_psdata << endl;
  }

  /////////////////////////////////
  // Generating diagnostic plots //
  /////////////////////////////////
  /**** Canvas 1 () ****/
  //Add ADC time resolution before and after correction. - Seems like it will
  //not make much of a sense without elastic cuts.
  //**** -- ***//

  /**** Canvas 2 (SH alignment) ****/
  TCanvas *c2 = new TCanvas("c2","SH align.",1200,800);
  c2->Divide(1,2);
  c2->cd(1); //
  gPad->SetGridy();
  h2_atimeOffSH_vs_blk->SetStats(0);
  h2_atimeOffSH_vs_blk->Draw("colz");
  h_atimeOffnRMSSH->Draw("same");
  c2->cd(2); //
  gPad->SetGridy();
  h2_atimeOffSH_vs_blk_corr->SetStats(0);
  h2_atimeOffSH_vs_blk_corr->Draw("colz");
  h_atimeOffnRMSSH_corr->Draw("same");
  c2->SaveAs(Form("%s[",outPeaks.Data())); c2->SaveAs(Form("%s",outPeaks.Data())); c2->Write();
  //**** -- ***//

  /**** Canvas 3 (PS alignment) ****/
  TCanvas *c3 = new TCanvas("c3","PS align.",1200,800);
  c3->Divide(1,2);
  c3->cd(1); //
  gPad->SetGridy();
  h2_atimeOffPS_vs_blk->SetStats(0);
  h2_atimeOffPS_vs_blk->Draw("colz");
  h_atimeOffnRMSPS->Draw("same");
  c3->cd(2); //
  gPad->SetGridy();
  h2_atimeOffPS_vs_blk_corr->SetStats(0);
  h2_atimeOffPS_vs_blk_corr->Draw("colz");
  h_atimeOffnRMSPS_corr->Draw("same");
  c3->SaveAs(Form("%s",outPeaks.Data())); c3->Write();
  //**** -- ***//

  /**** Canvas 4 (SH off. vs. rnum) ****/
  TCanvas *c4 = new TCanvas("c4","SH off. vs. rnum",1200,800);
  c4->Divide(1,2);
  c4->cd(1); //
  gPad->SetGridy();
  h2_atimeOffSH_vs_rnum->SetStats(0);
  h2_atimeOffSH_vs_rnum->Draw("colz");
  c4->cd(2); //
  gPad->SetGridy();
  h2_atimeOffSH_vs_rnum_corr->SetStats(0);
  h2_atimeOffSH_vs_rnum_corr->Draw("colz");
  c4->SaveAs(Form("%s",outPeaks.Data())); c4->Write();
  //**** -- ***//

  /**** Canvas 5 (PS off. vs. rnum) ****/
  TCanvas *c5 = new TCanvas("c5","PS off. vs. rnum",1200,800);
  c5->Divide(1,2);
  c5->cd(1); //
  gPad->SetGridy();
  h2_atimeOffPS_vs_rnum->SetStats(0);
  h2_atimeOffPS_vs_rnum->Draw("colz");
  c5->cd(2); //
  gPad->SetGridy();
  h2_atimeOffPS_vs_rnum_corr->SetStats(0);
  h2_atimeOffPS_vs_rnum_corr->Draw("colz");
  c5->SaveAs(Form("%s",outPeaks.Data())); c5->Write();
  //**** -- ***//

  /**** Canvas 6 (SH atime vs. rnum) ****/
  TCanvas *c6 = new TCanvas("c6","SH off. vs. rnum",1200,800);
  c6->Divide(1,2);
  c6->cd(1); //
  gPad->SetGridy();
  h2_atimeSH_vs_rnum->SetStats(0);
  h2_atimeSH_vs_rnum->Draw("colz");
  c6->cd(2); //
  gPad->SetGridy();
  h2_atimeSH_vs_rnum_corr->SetStats(0);
  h2_atimeSH_vs_rnum_corr->Draw("colz");
  c6->SaveAs(Form("%s",outPeaks.Data())); c6->Write();
  //**** -- ***//

  /**** Canvas 7 (SH atime vs. rnum) ****/
  TCanvas *c7 = new TCanvas("c7","PS off. vs. rnum",1200,800);
  c7->Divide(1,2);
  c7->cd(1); //
  gPad->SetGridy();
  h2_atimePS_vs_rnum->SetStats(0);
  h2_atimePS_vs_rnum->Draw("colz");
  c7->cd(2); //
  gPad->SetGridy();
  h2_atimePS_vs_rnum_corr->SetStats(0);
  h2_atimePS_vs_rnum_corr->Draw("colz");
  c7->SaveAs(Form("%s",outPeaks.Data())); c7->Write();
  //**** -- ***//

  /**** Canvas 8 (Offsets) ****/
  TCanvas *c8 = new TCanvas("c8","Offsets",1200,800);
  c8->Divide(2,2);
  c8->cd(1); //
  h2_atimeOffPS_detview->SetStats(0);
  h2_atimeOffPS_detview->Draw("colz text");
  h2_atimeOffPS_detview->GetZaxis()->SetRangeUser(-5,5);
  c8->cd(2); //
  h2_atimeOffPS_detview_corr->SetStats(0);
  h2_atimeOffPS_detview_corr->Draw("colz text");
  h2_atimeOffPS_detview_corr->GetZaxis()->SetRangeUser(-5,5);
  c8->cd(3); //
  h2_atimeOffSH_detview->SetStats(0);
  h2_atimeOffSH_detview->Draw("colz text");
  h2_atimeOffSH_detview->GetZaxis()->SetRangeUser(-5,5);
  c8->cd(4); //
  h2_atimeOffSH_detview_corr->SetStats(0);
  h2_atimeOffSH_detview_corr->Draw("colz text");
  h2_atimeOffSH_detview_corr->GetZaxis()->SetRangeUser(-5,5);
  c8->SaveAs(Form("%s",outPeaks.Data())); c8->Write();
  //**** -- ***//

  /**** Summary Canvas ****/
  TCanvas *cSummary = new TCanvas("cSummary","Summary");
  cSummary->cd();
  TPaveText *pt = new TPaveText(.05,.1,.95,.8);
  pt->AddText(Form(" Date of creation: %s", getDate().c_str()));
  pt->AddText(Form("Configfile: BBCal_replay/macros/Combined_macros/%s.cfg",cfgfilebase.Data()));
  set = set.Atoi() < 0 ? "N/A" : set;
  pt->AddText(Form(" %s config.: %d, set: %s, Preparing for replay pass: %d",exptag,config,set.Data(),ppass));
  pt->AddText(Form(" Total # events analyzed: %lld", nevents));
  // pt->AddText(Form(" BBCAL ADC time (before corr.) | #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param_bc[1],param_bc[2]*100,sigerr_bc*100));
  // pt->AddText(Form(" BBCAL ADC time (after corr.) | #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param[1],param[2]*100,sigerr*100));
  pt->AddText(" Global cuts: ");
  std::string tmpstr = "";
  for (std::size_t i=0; i<gCutList.size(); i++) {
    if (i>0 && i%3==0) {pt->AddText(Form(" %s",tmpstr.c_str())); tmpstr="";}
    tmpstr += gCutList[i] + ", "; 
  }
  if (!tmpstr.empty()) pt->AddText(Form(" %s",tmpstr.c_str()));
  pt->AddText(Form(" # events passed global cuts: %zu", goodevents.size()));
  sw->Stop(); sw2->Stop();
  pt->AddText(Form("Macro processing time: CPU %.1fs | Real %.1fs",sw->CpuTime(),sw->RealTime()));
  TText *t1 = pt->GetLineWith("Configfile"); t1->SetTextColor(kRed);
  TText *t2 = pt->GetLineWith(" Global"); t2->SetTextColor(kBlue);
  TText *t3 = pt->GetLineWith("Macro"); t3->SetTextColor(kGreen+3);
  pt->Draw();
  cSummary->SaveAs(Form("%s",outPeaks.Data())); cSummary->Write();  
  //**** -- ***//

  //subCanv[0]->SaveAs(Form("%s[",outPeaks.Data()));
  for(int canC=0; canC<8; canC++) {subCanv[canC]->SaveAs(Form("%s",outPeaks.Data())); subCanv[canC]->Write();}
  subCanv[7]->SaveAs(Form("%s]",outPeaks.Data()));

  fout->Write();
  //fout->Close();
  //fout->Delete();
 
  cout << "\nFinishing analysis .." << endl;
  cout << " --------- " << endl;
  cout << " Plots saved to : " << outPeaks << endl;
  cout << " Histogram written to : " << outFile << endl;
  cout << " ADCtime offsets for SH written to : " << toffset_sh << endl;
  cout << " ADCtime offsets for PS written to : " << toffset_ps << endl;
  cout << " --------- " << endl;

  cout << "CPU time = " << sw->CpuTime() << "s. Real time = " << sw->RealTime() << "s. \n\n";
}

// **** ========== Useful functions ========== ****  
// Returns today's date
string getDate(){
  // returns today's date
  time_t now = time(0);
  tm ltm = *localtime(&now);
  string yyyy = to_string(1900 + ltm.tm_year);
  string mm = to_string(1 + ltm.tm_mon);
  string dd = to_string(ltm.tm_mday);
  string date = mm + '/' + dd + '/' + yyyy;
  return date;
}

// reads old ADC gain coefficients from TXT files
void ReadOffset(TString atimeOff_rfile, Double_t* atimeOff){
  ifstream atimeOff_data;
  atimeOff_data.open(atimeOff_rfile);
  std::string readline;
  Int_t elemID=0;
  if(atimeOff_data.is_open()){
    std::cout << " Reading ADC gain from : "<< atimeOff_rfile << "\n";
    while(getline(atimeOff_data,readline)){
      istringstream tokenStream(readline);
      std::string token;
      char delimiter = ' ';
      while(getline(tokenStream,token,delimiter)){
  	TString temptoken=token;
  	atimeOff[elemID] = temptoken.Atof();
  	elemID++;
      }
    }
  }else{
    std::cerr << " **!** No file : " << atimeOff_rfile << "\n\n";
    std::exit(1);
  }
  atimeOff_data.close();
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

// Customizes 2D histos with run # on the X-axis
void Custm2DRnumHisto(TH2F* h, std::vector<std::string> const & lrnum)
{
  h->GetXaxis()->SetRange(1,lrnum.size());
  for (int i=0; i<lrnum.size(); i++) h->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
  if (lrnum.size()>15) h->LabelsOption("v", "X");
}

// splits a string by a delimiter (doesn't include empty sub-strings)
std::vector<std::string> SplitString(char const delim, std::string const myStr) {
  std::stringstream ss(myStr);
  std::vector<std::string> out;
  while (ss.good()) {
    std::string substr;
    std::getline(ss, substr, delim);
    if (!substr.empty()) out.push_back(substr);
  }
  if (out.empty()) std::cerr << "WARNING! No substrings found!\n";
  return out;
}

// returns output file base from configfilename
TString GetOutFileBase(TString configfilename) {
  std::vector<std::string> result;
  result = SplitString('/',configfilename.Data());
  TString temp = result[result.size() - 1];
  return temp.ReplaceAll(".cfg", "");
}


