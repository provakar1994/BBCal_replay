/*
  This script generates diagnostic plots for quality assurance of BBCAL calibration.
  -----
  P. Datta <pdbforce@jlab.org> Created 04-21-2022
  -----
*/
#include <iostream>
#include <sstream>
#include <fstream>

#include "TCut.h"
#include "TH2F.h"
#include "TMath.h"
#include "TChain.h"
#include "TString.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TStopwatch.h"
#include "TTreeFormula.h"

const Int_t kNcolsSH = 7;   // SH columns
const Int_t kNrowsSH = 27;  // SH rows
const Int_t kNblksSH = 189; // Total # SH blocks/PMTs
const Int_t kNcolsPS = 2;   // PS columns
const Int_t kNrowsPS = 26;  // PS rows
const Int_t kNblksPS = 52;  // Total # PS blocks/PMTs

const Double_t zposSH = 1.901952; //m
const Double_t zposPS = 1.695704; //m

string GetDate();
double GetNDC(double x);
void CustmProfHisto(TProfile*);
void Custm2DRnumHisto(TH2F*, std::vector<std::string> const & lrnum);

void qualityA_plots_BBCAL(TString outFileBase = "qulaityA_plots_BBCAL.root",
			  const char *configFile="Beam_analysis_macros/setup_qualityA_plots_BBCAL.cfg")
{
  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings
  
  // creating a TChain
  TChain *C = new TChain("T");
  
  // Defining variables
  Int_t SBSconfig=4;
  Double_t h_EovP_bin=200, h_EovP_min=0., h_EovP_max=5.;
  Double_t h2_p_coarse_bin=25, h2_p_coarse_min=0., h2_p_coarse_max=5.;
  Double_t h2_SHeng_vs_blk_low=0., h2_SHeng_vs_blk_up=4.;
  Double_t h2_PSeng_vs_blk_low=0., h2_PSeng_vs_blk_up=4.;
  Double_t bbcal_atppos=0., hcal_atppos=0.;

  // Define a stopwatch to measure macro processing time
  TStopwatch *sw = new TStopwatch();
  sw->Start();

  // reading config file
  ifstream configfile(configFile);
  TString currentline;
  cout << endl << "Chaining all the ROOT files.." << endl;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endRunlist") ){
    if( !currentline.BeginsWith("#") ){
      C->Add(currentline);
    }
  }
  TCut globalcut="";
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endcut") ){
    if( !currentline.BeginsWith("#") ){
      globalcut += currentline;
    }
  }
  TTreeFormula *GlobalCut = new TTreeFormula("GlobalCut", globalcut, C);
  while( currentline.ReadLine( configfile ) ){
    if( currentline.BeginsWith("#") ) continue;
    TObjArray *tokens = currentline.Tokenize(" ");
    Int_t ntokens = tokens->GetEntries();
    if( ntokens>1 ){
      TString skey = ( (TObjString*)(*tokens)[0] )->GetString();
      if( skey == "h_EovP" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h_EovP_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h_EovP_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h_EovP_max = sval2.Atof();
      }
      if( skey == "h2_p_coarse" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h2_p_coarse_bin = sval.Atoi();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h2_p_coarse_min = sval1.Atof();
	TString sval2 = ( (TObjString*)(*tokens)[3] )->GetString();
	h2_p_coarse_max = sval2.Atof();
      }
      if( skey == "h2_SHeng_vs_blk" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h2_SHeng_vs_blk_low = sval.Atof();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h2_SHeng_vs_blk_up = sval1.Atof();
      }
      if( skey == "h2_PSeng_vs_blk" ){
	TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
	h2_PSeng_vs_blk_low = sval.Atof();
	TString sval1 = ( (TObjString*)(*tokens)[2] )->GetString();
	h2_PSeng_vs_blk_up = sval1.Atof();
      }
      if (skey == "bbcal_atppos") bbcal_atppos = ((TObjString*)(*tokens)[1])->GetString().Atof();
      if (skey == "hcal_atppos") hcal_atppos = ((TObjString*)(*tokens)[1])->GetString().Atof();
      if( skey == "*****" ){
	break;
      }
    }
    delete tokens;
  }

  // Implementing global cuts
  if(C->GetEntries()==0){
    cerr << endl << " --- No ROOT file found!! ---" << endl << endl;
    throw;
  }else{
    cout << endl << "Found " << C->GetEntries() << " events. Implementing global cuts.." << endl;
  }

  // Setting branch addresses for various tree variables for analysis
  int maxNtr = 200;
  C->SetBranchStatus("*", 0);
  // bb.ps branches
  C->SetBranchStatus("bb.ps.*", 1);
  Double_t psNclus;            C->SetBranchAddress("bb.ps.nclus", &psNclus);
  Double_t psIdblk;            C->SetBranchAddress("bb.ps.idblk", &psIdblk);
  Double_t psRowblk;           C->SetBranchAddress("bb.ps.rowblk", &psRowblk);
  Double_t psColblk;           C->SetBranchAddress("bb.ps.colblk", &psColblk);
  Double_t psNblk;             C->SetBranchAddress("bb.ps.nblk", &psNblk);
  Double_t psAtime;            C->SetBranchAddress("bb.ps.atimeblk", &psAtime);
  Double_t psE;                C->SetBranchAddress("bb.ps.e", &psE);
  Double_t psX;                C->SetBranchAddress("bb.ps.x", &psX);
  Double_t psY;                C->SetBranchAddress("bb.ps.y", &psY);
  Double_t psClBlkId[maxNtr];  C->SetBranchAddress("bb.ps.clus_blk.id", &psClBlkId);
  Double_t psClBlkE[maxNtr];   C->SetBranchAddress("bb.ps.clus_blk.e", &psClBlkE);
  Double_t psClBlkX[maxNtr];   C->SetBranchAddress("bb.ps.clus_blk.x", &psClBlkX);
  Double_t psClBlkY[maxNtr];   C->SetBranchAddress("bb.ps.clus_blk.y", &psClBlkY);
  Double_t psClBlkAtime[maxNtr]; C->SetBranchAddress("bb.ps.clus_blk.atime", &psClBlkAtime);
  Double_t psAgainblk;         C->SetBranchAddress("bb.ps.againblk", &psAgainblk);
  // bb.sh branches
  C->SetBranchStatus("bb.sh.*", 1);
  Double_t shNclus;            C->SetBranchAddress("bb.sh.nclus", &shNclus);
  Double_t shIdblk;            C->SetBranchAddress("bb.sh.idblk", &shIdblk);
  Double_t shRowblk;           C->SetBranchAddress("bb.sh.rowblk", &shRowblk);
  Double_t shColblk;           C->SetBranchAddress("bb.sh.colblk", &shColblk);
  Double_t shNblk;             C->SetBranchAddress("bb.sh.nblk", &shNblk);
  Double_t shAtime;            C->SetBranchAddress("bb.sh.atimeblk", &shAtime);
  Double_t shE;                C->SetBranchAddress("bb.sh.e", &shE);
  Double_t shX;                C->SetBranchAddress("bb.sh.x", &shX);
  Double_t shY;                C->SetBranchAddress("bb.sh.y", &shY);
  Double_t shClBlkId[maxNtr];  C->SetBranchAddress("bb.sh.clus_blk.id", &shClBlkId);
  Double_t shClBlkE[maxNtr];   C->SetBranchAddress("bb.sh.clus_blk.e", &shClBlkE);
  Double_t shClBlkX[maxNtr];   C->SetBranchAddress("bb.sh.clus_blk.x", &shClBlkX);
  Double_t shClBlkY[maxNtr];   C->SetBranchAddress("bb.sh.clus_blk.y", &shClBlkY);
  Double_t shClBlkAtime[maxNtr]; C->SetBranchAddress("bb.sh.clus_blk.atime", &shClBlkAtime);
  Double_t shAgainblk;         C->SetBranchAddress("bb.sh.againblk", &shAgainblk);
  // sbs.hcal branches
  Double_t hcalE;              C->SetBranchStatus("sbs.hcal.e",1); C->SetBranchAddress("sbs.hcal.e", &hcalE);
  Double_t hcalX;              C->SetBranchStatus("sbs.hcal.x",1); C->SetBranchAddress("sbs.hcal.x", &hcalX);
  Double_t hcalY;              C->SetBranchStatus("sbs.hcal.y",1); C->SetBranchAddress("sbs.hcal.y", &hcalY); 
  Double_t hcalAtime;          C->SetBranchStatus("sbs.hcal.atimeblk",1); C->SetBranchAddress("sbs.hcal.atimeblk", &hcalAtime); 
  // bb.tr branches
  C->SetBranchStatus("bb.tr.*", 1);
  Double_t trN;                C->SetBranchAddress("bb.tr.n", &trN);
  Double_t trP[maxNtr];        C->SetBranchAddress("bb.tr.p", &trP);
  Double_t trPx[maxNtr];       C->SetBranchAddress("bb.tr.px", &trPx);
  Double_t trPy[maxNtr];       C->SetBranchAddress("bb.tr.py", &trPy);
  Double_t trPz[maxNtr];       C->SetBranchAddress("bb.tr.pz", &trPz);
  Double_t trX[maxNtr];        C->SetBranchAddress("bb.tr.x", &trX);
  Double_t trY[maxNtr];        C->SetBranchAddress("bb.tr.y", &trY);
  Double_t trTh[maxNtr];       C->SetBranchAddress("bb.tr.th", &trTh);
  Double_t trPh[maxNtr];       C->SetBranchAddress("bb.tr.ph", &trPh);
  Double_t trVz[maxNtr];       C->SetBranchAddress("bb.tr.vz", &trVz);
  Double_t trVy[maxNtr];       C->SetBranchAddress("bb.tr.vy", &trVy);
  Double_t trTgth[maxNtr];     C->SetBranchAddress("bb.tr.tg_th", &trTgth);
  Double_t trTgph[maxNtr];     C->SetBranchAddress("bb.tr.tg_ph", &trTgph);
  Double_t trRx[maxNtr];       C->SetBranchAddress("bb.tr.r_x", &trRx);
  Double_t trRy[maxNtr];       C->SetBranchAddress("bb.tr.r_y", &trRy);
  Double_t trRth[maxNtr];      C->SetBranchAddress("bb.tr.r_th", &trRth);
  Double_t trRph[maxNtr];      C->SetBranchAddress("bb.tr.r_ph", &trRph);
  // bb.hodotdc branches
  Double_t thTdiff[maxNtr];    C->SetBranchStatus("bb.hodotdc.clus.tdiff",1); C->SetBranchAddress("bb.hodotdc.clus.tdiff", &thTdiff);
  Double_t thTmean[maxNtr];    C->SetBranchStatus("bb.hodotdc.clus.tmean",1); C->SetBranchAddress("bb.hodotdc.clus.tmean", &thTmean);
  Double_t thTOTmean[maxNtr];  C->SetBranchStatus("bb.hodotdc.clus.totmean",1); C->SetBranchAddress("bb.hodotdc.clus.totmean", &thTOTmean);
  // Event info
  C->SetMakeClass(1);
  C->SetBranchStatus("fEvtHdr.*", 1);
  UInt_t rnum;                 C->SetBranchAddress("fEvtHdr.fRun", &rnum);
  UInt_t trigbits;             C->SetBranchAddress("fEvtHdr.fTrigBits", &trigbits);
  ULong64_t gevnum;            C->SetBranchAddress("fEvtHdr.fEvtNum", &gevnum);
  // turning on additional branches for the global cut
  C->SetBranchStatus("e.kine.W2", 1);
  C->SetBranchStatus("sbs.hcal.e", 1);
  C->SetBranchStatus("bb.gem.track.nhits", 1);
  C->SetBranchStatus("bb.gem.track.ngoodhits", 1);
  C->SetBranchStatus("bb.gem.track.chi2ndf", 1);
  C->SetBranchStatus("bb.grinch_tdc.clus.trackindex", 1);
  C->SetBranchStatus("bb.grinch_tdc.clus.size", 1);

  // Defining temporary histograms (don't wanna write them to files)
  TH2F *h2_SHeng_vs_SHblk_raw = new TH2F("h2_SHeng_vs_SHblk_raw","Raw E_clus(SH) per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2F *h2_EovP_vs_SHblk_raw = new TH2F("h2_EovP_vs_SHblk_raw","Raw E_clus/p_rec per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2F *h2_EovP_vs_PSblk_raw = new TH2F("h2_EovP_vs_PSblk_raw","Raw E_clus/p_rec per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2F *h2_count = new TH2F("h2_count","Count for E_clus/p_rec per per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2F *h2_EovP_vs_SHblk_trPOS_raw = new TH2F("h2_EovP_vs_SHblk_trPOS_raw","Raw E_clus/p_rec per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2F *h2_count_trP = new TH2F("h2_count_trP","Count for E_clus/p_rec per per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2F *h2_count_trP_PS = new TH2F("h2_count_trP_PS","Count for E_clus(PS) per per PS block(TrPOS)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  TH2F *h2_count_PS = new TH2F("h2_count_PS","Count for E_clus/p_rec per per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2F *h2_PSeng_vs_PSblk_raw = new TH2F("h2_PSeng_vs_PSblk_raw","Raw E_clus(PS) per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2F *h2_PSeng_vs_PSblk_trPOS_raw = new TH2F("h2_PSeng_vs_PSblk_trPOS_raw","Raw E_clus(PS) per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);
  
  // Creating output ROOT file to contain histograms
  TString outFile = "hist/" + outFileBase;
  TFile *fout = new TFile(outFile, "RECREATE");
  fout->cd();

  // Defining physics histograms
  TH1F *h_EovP = new TH1F("h_EovP","E/p",h_EovP_bin,h_EovP_min,h_EovP_max);
  TH2F *h2_EovP_vs_P = new TH2F("h2_EovP_vs_P","E/p vs p; p (GeV); E/p",h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h_EovP_bin,h_EovP_min,h_EovP_max);
  TProfile *h2_EovP_vs_P_prof = new TProfile("h2_EovP_vs_P_prof","E/p vs P (Profile)",h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h_EovP_min,h_EovP_max,"S");

  TH2F *h2_EovP_vs_SHblk = new TH2F("h2_EovP_vs_SHblk","E/p per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2F *h2_EovP_vs_PSblk = new TH2F("h2_EovP_vs_PSblk","E/p per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2F *h2_EovP_vs_SHblk_trPOS = new TH2F("h2_EovP_vs_SHblk_trPOS","E/p per SH block u Track Pos.",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2F *h2_SHeng_vs_SHblk = new TH2F("h2_SHeng_vs_SHblk","SH energy per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);

  TH2F *h2_PSeng_vs_PSblk = new TH2F("h2_PSeng_vs_PSblk","PS energy per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2F *h2_PSeng_vs_PSblk_trPOS = new TH2F("h2_PSeng_vs_PSblk_trPOS","PS energy per PS block u Track Pos.",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  // ADC time related histograms
  // TH2F *h2_ADCtime_diff_wrt_Hodo = new TH2F("h2_ADCtime_diff_wrt_Hodo","ADCTime diff. w.r.t. Hodo tmean (ns) vs. SH block",189,0,189,200,-50,50);
  // TProfile *h2_ADCtime_diff_wrt_Hodo_prof = new TProfile("h2_ADCtime_diff_wrt_Hodo_prof","ADCTime diff. w.r.t. Hodo tmean (ns) vs. SH block (Profile)",189,0,189,-50,50);
  Double_t h_atime_bin=200, h_atime_min=bbcal_atppos-25., h_atime_max=bbcal_atppos+20.;
  Double_t h2_ThShCoin_bin=200, h2_ThShCoin_min=-bbcal_atppos-25., h2_ThShCoin_max=-bbcal_atppos+25.;
  Double_t ShHcalCoin = bbcal_atppos - hcal_atppos;
  Double_t h2_ShHcalCoin_bin=200, h2_ShHcalCoin_min=ShHcalCoin-25., h2_ShHcalCoin_max=ShHcalCoin+25.;

  Double_t Nruns = 1000; // Max # runs we anticipate to analyze 
  TH2F *h2_EovP_vs_rnum = new TH2F("h2_EovP_vs_rnum","E/p vs Run no",Nruns,0.5,Nruns+0.5,200,0.4,1.6);
  TProfile *h2_EovP_vs_rnum_prof = new TProfile("h2_EovP_vs_rnum_prof","E/p vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,0.4,1.6,"S");

  // TH1F *h_atimeSH = new TH1F("h_atimeSH","SH ADC time | Before corr.",h_atime_bin,h_atime_min,h_atime_max);
  // TH1F *h_atimePS = new TH1F("h_atimePS","PS ADC time | Before corr.",h_atime_bin,h_atime_min,h_atime_max);

  TH2F *h2_atimeSH_vs_rnum = new TH2F("h2_atimeSH_vs_rnum","SH ADC time vs Run no.;Run no.;SH ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_bin,h_atime_min,h_atime_max);
  TProfile *h2_atimeSH_vs_rnum_prof = new TProfile("h2_atimeSH_vs_rnum_prof","SH ADC time vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,-bbcal_atppos-3.,-bbcal_atppos+3.,"S");
  TH2F *h2_atimePS_vs_rnum = new TH2F("h2_atimePS_vs_rnum","PS ADC time vs Run no.;Run no.;PS ADCtime (ns)",Nruns,0.5,Nruns+0.5,h_atime_bin,h_atime_min,h_atime_max);
  TProfile *h2_atimePS_vs_rnum_prof = new TProfile("h2_atimePS_vs_rnum_prof","PS ADC time vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,-bbcal_atppos-3.,-bbcal_atppos+3.,"S");

  TH2F *h2_ThShCoin_vs_blk = new TH2F("h2_ThShCoin_vs_blk","TH-SH Coin vs SH blocks;SH block id;TH ClusTmean - SH ADCtime (ns)",kNblksSH,0,kNblksSH,h2_ThShCoin_bin,h2_ThShCoin_min,h2_ThShCoin_max); 
  TProfile *h2_ThShCoin_vs_blk_prof = new TProfile("h2_ThShCoin_vs_blk_prof","TH-SH Coin vs SH blocks (Profile)",189,0,189,-bbcal_atppos-3.,-bbcal_atppos+3.,"S");
  TH2F *h2_ThPsCoin_vs_blk = new TH2F("h2_ThPsCoin_vs_blk","TH-PS Coin vs PS blocks;PS block id;TH ClusTmean - PS ADCtime (ns)",kNblksPS,0,kNblksPS,h2_ThShCoin_bin,h2_ThShCoin_min,h2_ThShCoin_max);
  TProfile *h2_ThPsCoin_vs_blk_prof = new TProfile("h2_ThPsCoin_vs_blk_prof","TH-PS Coin vs PS blocks (Profile)",52,0,52,-bbcal_atppos-3.,-bbcal_atppos+3.,"S");

  TH2F *h2_ThShCoin_vs_rnum = new TH2F("h2_ThShCoin_vs_rnum","TH-SH Coin vs Run No.;Run no.;TH ClusTmean - SH ADCtime (ns)",Nruns,0.5,Nruns+0.5,h2_ThShCoin_bin,h2_ThShCoin_min,h2_ThShCoin_max); 
  TProfile *h2_ThShCoin_vs_rnum_prof = new TProfile("h2_ThShCoin_vs_rnum_prof","TH-SH Coin vs Run No. (Profile)",Nruns,0.5,Nruns+0.5,-bbcal_atppos-3.,-bbcal_atppos+3.,"S");
  TH2F *h2_ThPsCoin_vs_rnum = new TH2F("h2_ThPsCoin_vs_rnum","TH-PS Coin vs Run No.;Run no.;TH ClusTmean - PS ADCtime (ns)",Nruns,0.5,Nruns+0.5,h2_ThShCoin_bin,h2_ThShCoin_min,h2_ThShCoin_max);
  TProfile *h2_ThPsCoin_vs_rnum_prof = new TProfile("h2_ThPsCoin_vs_rnum_prof","TH-PS Coin vs Run No. (Profile)",Nruns,0.5,Nruns+0.5,-bbcal_atppos-3.,-bbcal_atppos+3.,"S");

  TH2F *h2_ShHcalCoin_vs_rnum = new TH2F("h2_ShHcalCoin_vs_rnum","SH-HCAL Coin vs Run No.;Run no.;SH ADCtime - HCAL ADCtime (ns)",Nruns,0.5,Nruns+0.5,h2_ShHcalCoin_bin,h2_ShHcalCoin_min,h2_ShHcalCoin_max); 
  TProfile *h2_ShHcalCoin_vs_rnum_prof = new TProfile("h2_ShHcalCoin_vs_rnum_prof","SH-HCAL Coin vs Run No. (Profile)",Nruns,0.5,Nruns+0.5,ShHcalCoin-5.,ShHcalCoin+5.,"S");
  TH2F *h2_PsHcalCoin_vs_rnum = new TH2F("h2_PsHcalCoin_vs_rnum","PS-HCAL Coin vs Run No.;Run no.;PS ADCtime - HCAL ADCtime (ns)",Nruns,0.5,Nruns+0.5,h2_ShHcalCoin_bin,h2_ShHcalCoin_min,h2_ShHcalCoin_max);
  TProfile *h2_PsHcalCoin_vs_rnum_prof = new TProfile("h2_PsHcalCoin_vs_rnum_prof","PS-HCAL Coin vs Run No. (Profile)",Nruns,0.5,Nruns+0.5,ShHcalCoin-5.,ShHcalCoin+5.,"S");

  //histograms to check bias in tracking
  TH2F *h2_EovP_vs_trX = new TH2F("h2_EovP_vs_trX","E/p vs Track x",200,-0.8,0.8,200,0,2);
  TH2F *h2_EovP_vs_trY = new TH2F("h2_EovP_vs_trY","E/p vs Track y",200,-0.16,0.16,200,0,2);
  TH2F *h2_EovP_vs_trTh = new TH2F("h2_EovP_vs_trTh","E/p vs Track theta",200,-0.2,0.2,200,0,2);
  TH2F *h2_EovP_vs_trPh = new TH2F("h2_EovP_vs_trPh","E/p vs Track phi",200,-0.08,0.08,200,0,2);
  // TH2F *h2_PSeng_vs_trX = new TH2F("h2_PSeng_vs_trX","PS energy vs Track x",200,-0.8,0.8,200,0,4);
  // TH2F *h2_PSeng_vs_trY = new TH2F("h2_PSeng_vs_trY","PS energy vs Track y",200,-0.16,0.16,200,0,4);
  TH2D *h2_PSeng_vs_trXatPS = new TH2D("h2_PSeng_vs_trXatPS","PS energy vs Track x (proj. at PS)",200,-1.,1.,200,0,4);
  TH2D *h2_PSeng_vs_trYatPS = new TH2D("h2_PSeng_vs_trYatPS","PS energy vs Track y (proj. at PS)",200,-0.3,0.3,200,0,4);

  // Looping over good events ================================================================= //
  Long64_t Nevents = C->GetEntries(), nevent=0;  
  cout << endl << "Processing " << Nevents << " events.." << endl;

  Int_t treenum=0, currenttreenum=0, itrrun=0; UInt_t runnum=0; 
  std::vector<std::string> lrnum;    // list of run numbers

  while(C->GetEntry(nevent++)) {

    // progress indicator
    if( nevent % 100 == 0 ) cout << nevent << "/" << Nevents << "\r";
    cout.flush();

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
    bool passedgCut = GlobalCut->EvalInstance(0) != 0;   
    if (passedgCut) {

      Double_t clusEngBBCal = psE + shE;
      // E/p
      h_EovP->Fill( (clusEngBBCal/trP[0]) );

      // E/p vs. p
      h2_EovP_vs_P->Fill( trP[0], clusEngBBCal/trP[0] );
      h2_EovP_vs_P_prof->Fill( trP[0], clusEngBBCal/trP[0], 1. );

      // Shower block related histos
      h2_SHeng_vs_SHblk_raw->Fill( shColblk, shRowblk, shE );
      h2_EovP_vs_SHblk_raw->Fill( shColblk, shRowblk, (clusEngBBCal/trP[0]) );
      h2_EovP_vs_PSblk_raw->Fill( psColblk, psRowblk, (clusEngBBCal/trP[0]) );
      h2_count->Fill( shColblk, shRowblk, 1.);

      Double_t xtrATsh = trX[0] + zposSH*trTh[0];
      Double_t ytrATsh = trY[0] + zposSH*trPh[0];
      h2_EovP_vs_SHblk_trPOS_raw->Fill( ytrATsh, xtrATsh, (clusEngBBCal/trP[0]) );
      h2_count_trP->Fill( ytrATsh, xtrATsh, 1. );

      // PreShower block related histos
      Double_t xtrATps = trX[0] + zposPS*trTh[0];
      Double_t ytrATps = trY[0] + zposPS*trPh[0];
      h2_PSeng_vs_PSblk_raw->Fill( psColblk, psRowblk, psE );
      h2_count_PS->Fill( psColblk, psRowblk, 1.);
      h2_PSeng_vs_PSblk_trPOS_raw->Fill( ytrATps, xtrATps, psE );
      h2_count_trP_PS->Fill( ytrATps, xtrATps, 1. );

      h2_EovP_vs_rnum->Fill(itrrun, clusEngBBCal/trP[0]);
      h2_EovP_vs_rnum_prof->Fill(itrrun, clusEngBBCal/trP[0], 1.);

      // ADCTime related histos
      h2_atimeSH_vs_rnum->Fill(itrrun, shAtime);
      h2_atimeSH_vs_rnum_prof->Fill(itrrun, shAtime, 1.);
      h2_atimePS_vs_rnum->Fill(itrrun, psAtime);
      h2_atimePS_vs_rnum_prof->Fill(itrrun, psAtime, 1.);

      Double_t sh_atimeOff = thTmean[0]-shAtime;
      Double_t ps_atimeOff = thTmean[0]-psAtime;

      h2_ThShCoin_vs_blk->Fill(shIdblk, sh_atimeOff);
      h2_ThShCoin_vs_blk_prof->Fill(shIdblk, sh_atimeOff, 1.);

      h2_ThPsCoin_vs_blk->Fill(psIdblk, ps_atimeOff);
      h2_ThPsCoin_vs_blk_prof->Fill(psIdblk, ps_atimeOff, 1.);

      h2_ThShCoin_vs_rnum->Fill(itrrun, sh_atimeOff);
      h2_ThShCoin_vs_rnum_prof->Fill(itrrun, sh_atimeOff, 1.);

      h2_ThPsCoin_vs_rnum->Fill(itrrun, ps_atimeOff);
      h2_ThPsCoin_vs_rnum_prof->Fill(itrrun, ps_atimeOff, 1.);

      h2_ShHcalCoin_vs_rnum->Fill(itrrun, shAtime-hcalAtime);
      h2_ShHcalCoin_vs_rnum_prof->Fill(itrrun, shAtime-hcalAtime, 1.);

      h2_PsHcalCoin_vs_rnum->Fill(itrrun, psAtime-hcalAtime);
      h2_PsHcalCoin_vs_rnum_prof->Fill(itrrun, psAtime-hcalAtime, 1.);

      // Track related histos
      h2_EovP_vs_trX->Fill( trX[0], (clusEngBBCal/trP[0]) );
      h2_EovP_vs_trY->Fill( trY[0], (clusEngBBCal/trP[0]) );
      h2_EovP_vs_trTh->Fill( trTh[0], (clusEngBBCal/trP[0]) );
      h2_EovP_vs_trPh->Fill( trPh[0], (clusEngBBCal/trP[0]) );
      // h2_PSeng_vs_trX->Fill( trX[0], psE );
      // h2_PSeng_vs_trY->Fill( trY[0], psE );
      h2_PSeng_vs_trXatPS->Fill( xtrATps, psE);
      h2_PSeng_vs_trYatPS->Fill( ytrATps, psE);
    }

  } //event loop
  cout << endl << endl;

  // customizing histo ranges
  h2_SHeng_vs_SHblk->Divide( h2_SHeng_vs_SHblk_raw, h2_count );
  h2_SHeng_vs_SHblk->GetZaxis()->SetRangeUser( h2_SHeng_vs_blk_low, h2_SHeng_vs_blk_up );
  //
  h2_EovP_vs_SHblk->Divide( h2_EovP_vs_SHblk_raw, h2_count );
  h2_EovP_vs_SHblk->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_EovP_vs_SHblk_trPOS->Divide( h2_EovP_vs_SHblk_trPOS_raw, h2_count_trP );
  h2_EovP_vs_SHblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);
  // PS
  h2_EovP_vs_PSblk->Divide( h2_EovP_vs_PSblk_raw, h2_count_PS );
  h2_EovP_vs_PSblk->GetZaxis()->SetRangeUser(0.8,1.2);
  //
  h2_PSeng_vs_PSblk->Divide( h2_PSeng_vs_PSblk_raw, h2_count_PS );
  h2_PSeng_vs_PSblk->GetZaxis()->SetRangeUser( h2_PSeng_vs_blk_low, h2_PSeng_vs_blk_up );
  //
  h2_PSeng_vs_PSblk_trPOS->Divide( h2_PSeng_vs_PSblk_trPOS_raw, h2_count_trP_PS );
  h2_PSeng_vs_PSblk_trPOS->GetZaxis()->SetRangeUser( h2_PSeng_vs_blk_low, h2_PSeng_vs_blk_up );

  // customizing histos with run # on the x-axis
  Custm2DRnumHisto(h2_atimeSH_vs_rnum, lrnum); Custm2DRnumHisto(h2_atimePS_vs_rnum, lrnum); 
  Custm2DRnumHisto(h2_ThShCoin_vs_rnum, lrnum); Custm2DRnumHisto(h2_ThPsCoin_vs_rnum, lrnum);
  Custm2DRnumHisto(h2_ShHcalCoin_vs_rnum, lrnum); Custm2DRnumHisto(h2_PsHcalCoin_vs_rnum, lrnum);
  CustmProfHisto(h2_EovP_vs_P_prof); CustmProfHisto(h2_EovP_vs_rnum_prof); 
  CustmProfHisto(h2_atimeSH_vs_rnum_prof); CustmProfHisto(h2_atimePS_vs_rnum_prof); 
  CustmProfHisto(h2_ThShCoin_vs_blk_prof);  CustmProfHisto(h2_ThShCoin_vs_rnum_prof);
  CustmProfHisto(h2_ThPsCoin_vs_blk_prof);  CustmProfHisto(h2_ThPsCoin_vs_rnum_prof);
  CustmProfHisto(h2_ShHcalCoin_vs_rnum_prof);  CustmProfHisto(h2_PsHcalCoin_vs_rnum_prof);

  // creating a canvas to show all the interesting plots
  // printing out the canvas
  TString plotsFile = "plots/" + outFileBase.ReplaceAll(".root",".pdf");

  TCanvas *c1 = new TCanvas("c1","E/p",1500,1200);
  c1->Divide(3,2);
  //
  c1->cd(1);
  gPad->SetGridx();
  gStyle->SetOptFit(1111);
  Int_t maxBin = h_EovP->GetMaximumBin();
  Double_t binW = h_EovP->GetBinWidth(maxBin),norm = h_EovP->GetMaximum();
  Double_t mean = h_EovP->GetMean(), stdev = h_EovP->GetStdDev();
  Double_t lower_lim = h_EovP_min + maxBin*binW - 1.*stdev;
  Double_t upper_lim = h_EovP_min + maxBin*binW + 1.*stdev; 
  TF1* fitg = new TF1("fitg","gaus",h_EovP_min,h_EovP_max);
  fitg->SetRange(lower_lim,upper_lim);
  fitg->SetParameters(norm,mean,stdev);
  fitg->SetLineWidth(2); fitg->SetLineColor(2);
  h_EovP->Fit(fitg,"QR");
  h_EovP->SetLineWidth(2); h_EovP->SetLineColor(1);
  h_EovP->Draw();
  //
  c1->cd(2);
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  h2_EovP_vs_P->SetStats(0);
  h2_EovP_vs_P->Draw("colz");
  h2_EovP_vs_P_prof->Draw("same");
  //
  // c1->cd(3);
  // gPad->SetGridy();
  // gStyle->SetErrorX(0.0001);
  // // h2_PSeng_vs_PSblk->SetStats(0);
  // // h2_PSeng_vs_PSblk->Draw("colz");
  // h2_ADCtime_diff_wrt_Hodo->SetStats(0);
  // h2_ADCtime_diff_wrt_Hodo->Draw("colz");
  // h2_ADCtime_diff_wrt_Hodo_prof->SetMarkerStyle(7);
  // h2_ADCtime_diff_wrt_Hodo_prof->SetMarkerColor(2);
  // h2_ADCtime_diff_wrt_Hodo_prof->SetStats(0);
  // h2_ADCtime_diff_wrt_Hodo_prof->Draw("same")
  //
  c1->cd(3);
  h2_EovP_vs_SHblk->SetStats(0);
  h2_EovP_vs_SHblk->Draw("colz text");
  //
  c1->cd(4);
  // h2_EovP_vs_SHblk->SetStats(0);
  // h2_EovP_vs_SHblk->Draw("colz");
  h2_EovP_vs_PSblk->SetStats(0);
  h2_EovP_vs_PSblk->Draw("colz text");
  //
  c1->cd(5);
  h2_SHeng_vs_SHblk->SetStats(0);
  h2_SHeng_vs_SHblk->Draw("colz text");
  //
  c1->cd(6);
  h2_PSeng_vs_PSblk->SetStats(0);
  h2_PSeng_vs_PSblk->Draw("colz text");
  c1->SaveAs(Form("%s[",plotsFile.Data()),"pdf");
  c1->SaveAs(Form("%s",plotsFile.Data()),"pdf"); c1->Write();
  // ***** 

  // creating another canvas to show all the interesting plots
  TCanvas *c2 = new TCanvas("c2","E/p vs Track params",1500,1200);
  c2->Divide(3,2);
  //
  c2->cd(1);
  gPad->SetGridy();
  h2_EovP_vs_trX->SetStats(0);
  h2_EovP_vs_trX->Draw("colz");
  //
  c2->cd(2);
  gPad->SetGridy();
  h2_EovP_vs_trY->SetStats(0);
  h2_EovP_vs_trY->Draw("colz");
  //
  c2->cd(3);
  gPad->SetGridy();
  h2_EovP_vs_trTh->SetStats(0);
  h2_EovP_vs_trTh->Draw("colz");
  //
  c2->cd(4);
  gPad->SetGridy();
  h2_EovP_vs_trPh->SetStats(0);
  h2_EovP_vs_trPh->Draw("colz");
  //
  c2->cd(5);
  gPad->SetGridy();
  // h2_PSeng_vs_trX->SetStats(0);
  // h2_PSeng_vs_trX->Draw("colz");
  h2_PSeng_vs_trXatPS->SetStats(0);
  h2_PSeng_vs_trXatPS->Draw("colz");
  //
  c2->cd(6);
  gPad->SetGridy();
  // h2_PSeng_vs_trY->SetStats(0);
  // h2_PSeng_vs_trY->Draw("colz");
  h2_PSeng_vs_trYatPS->SetStats(0);
  h2_PSeng_vs_trYatPS->Draw("colz");
  c2->SaveAs(Form("%s",plotsFile.Data()),"pdf"); c2->Write();
  // *****

  /**** Canvas 3 (E/p vs. run number) ****/
  TCanvas *c3 = new TCanvas("c3","E/p vs rnum",1200,1000);
  c3->Divide(1,2); 
  c3->cd(1); //
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001); 
  Custm2DRnumHisto(h2_EovP_vs_rnum,lrnum);
  h2_EovP_vs_rnum->Draw("colz");
  h2_EovP_vs_rnum_prof->Draw("same");
  c3->SaveAs(Form("%s",plotsFile.Data()),"pdf"); c3->Write();

  /**** Canvas 4 (SH off. vs. rnum) ****/
  TCanvas *c4 = new TCanvas("c4","TH-BBCAL coin vs blk",1200,800);
  c4->Divide(1,2);
  c4->cd(1); //
  gPad->SetGridy();
  h2_ThShCoin_vs_blk->SetStats(0);
  h2_ThShCoin_vs_blk->Draw("colz");
  h2_ThShCoin_vs_blk_prof->Draw("same");
  c4->cd(2); //
  gPad->SetGridy();
  h2_ThPsCoin_vs_blk->SetStats(0);
  h2_ThPsCoin_vs_blk->Draw("colz");
  h2_ThPsCoin_vs_blk_prof->Draw("same");
  c4->SaveAs(Form("%s",plotsFile.Data())); c4->Write();
  //**** -- ***//

  /**** Canvas 5 (PS off. vs. rnum) ****/
  TCanvas *c5 = new TCanvas("c5","TH-BBCAL coin vs rnum",1200,800);
  c5->Divide(1,2);
  c5->cd(1); //
  gPad->SetGridy();
  h2_ThShCoin_vs_rnum->SetStats(0);
  h2_ThShCoin_vs_rnum->Draw("colz");
  h2_ThShCoin_vs_rnum_prof->Draw("same");
  c5->cd(2); //
  gPad->SetGridy();
  h2_ThPsCoin_vs_rnum->SetStats(0);
  h2_ThPsCoin_vs_rnum->Draw("colz");
  h2_ThPsCoin_vs_rnum_prof->Draw("same");
  c5->SaveAs(Form("%s",plotsFile.Data())); c5->Write();
  //**** -- ***//

  /**** Canvas 6 (HCAL-SH coin vs. rnum) ****/
  TCanvas *c6 = new TCanvas("c6","HCAL-BBCAL coin vs rnum",1200,800);
  c6->Divide(1,2);
  c6->cd(1); //
  gPad->SetGridy();
  h2_ShHcalCoin_vs_rnum->SetStats(0);
  h2_ShHcalCoin_vs_rnum->Draw("colz");
  h2_ShHcalCoin_vs_rnum_prof->Draw("same");
  c6->cd(2); //
  gPad->SetGridy();
  h2_PsHcalCoin_vs_rnum->SetStats(0);
  h2_PsHcalCoin_vs_rnum->Draw("colz");
  h2_PsHcalCoin_vs_rnum_prof->Draw("same");
  c6->SaveAs(Form("%s",plotsFile.Data())); c6->Write();

  /**** Canvas 9 (SH atime vs. rnum) ****/
  TCanvas *c7 = new TCanvas("c7","BBCAL atime vs rnum",1200,800);
  c7->Divide(1,2);
  c7->cd(1); //
  gPad->SetGridy();
  h2_atimeSH_vs_rnum->SetStats(0);
  h2_atimeSH_vs_rnum->Draw("colz");
  h2_atimeSH_vs_rnum_prof->Draw("same");
  c7->cd(2); //
  gPad->SetGridy();
  h2_atimePS_vs_rnum->SetStats(0);
  h2_atimePS_vs_rnum->Draw("colz");
  h2_atimePS_vs_rnum_prof->Draw("same");
  c7->SaveAs(Form("%s]",plotsFile.Data())); c7->Write();
  
  cout << "Finishing analysis..." << endl;
  cout << " --------- " << endl;
  cout << " Resulting histograms written to : " << outFile << endl;
  cout << " Generated plots saved to : " << plotsFile.Data() << endl;
  cout << " --------- " << endl;

  sw->Stop();
  std::cout << "CPU time = " << sw->CpuTime() << "s. Real time = " << sw->RealTime() << "s.\n\n";

  fout->Write();
  sw->Delete(); C->Delete();
}

// **** ========== Useful functions ========== ****  
// returns today's date
std::string GetDate(){
  time_t now = time(0);
  tm ltm = *localtime(&now);
  std::string yyyy = to_string(1900 + ltm.tm_year);
  std::string mm = to_string(1 + ltm.tm_mon);
  std::string dd = to_string(ltm.tm_mday);
  std::string date = mm + '/' + dd + '/' + yyyy;
  return date;
}

// customizes profile histograms
void CustmProfHisto(TProfile* hprof) {
  hprof->SetStats(0);
  hprof->SetMarkerStyle(20);
  hprof->SetMarkerColor(2);
}

// Customizes 2D histos with run # on the X-axis
void Custm2DRnumHisto(TH2F* h, std::vector<std::string> const & lrnum)
{
  h->SetStats(0);
  h->GetXaxis()->SetLabelSize(0.05);
  h->GetXaxis()->SetRange(1,lrnum.size());
  for (int i=0; i<lrnum.size(); i++) h->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
  if (lrnum.size()>15) h->LabelsOption("v", "X");
}

// Returns NDC value for a given abscissa
double GetNDC(double x) {
  gPad->Update();
  return (x - gPad->GetX1())/(gPad->GetX2()-gPad->GetX1());
}
