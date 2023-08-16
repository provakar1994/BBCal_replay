/*
  This script has been prepared for the energy calibration of BigBite Calorimeter (BigBite Shower
  + BigBite Preshower) detector. It does so by minimizing the chi2 of the difference between calorimeter
  cluster energy and the reconstructed electron energy. It gets the old adc gain coefficients (GeV/pC) 
  from tree and writes the new adc gain coeffs. and ratios (New/Old) to file. One needs a configfile 
  to execute this script [see cfg/example.cfg]. To execute, do:
  ----
  [a-onl@aonl2 macros]$ pwd
  /adaqfs/home/a-onl/sbs/BBCal_replay/macros
  [a-onl@aonl2 macros]$ root -l 
  root [0] .x Combined_macros/bbcal_eng_calib_w_h2.C("Combined_macros/cfg/example.cfg")
  ----
  P. Datta  <pdbforce@jlab.org>  Created  28 Sep 2022 (Based on test_eng_cal_BBCal.C)
*/

/*
  NOTES:
  1. Global cuts given in the config file (line above "endcut" flag) get applied to all the output
     histograms and output ROOT tree branches (both before & after calib.).
  2. Definition of elastic cut: elastic_cut = cut_on_W || cut_on_PovPel || cut_on_pspot
  3. If "elastic_cut" is True, elastic cut(s) will be applied to all the diagnostic histograms. This
     doesn't include h_W2, h_Q2, all h_PovPel* histograms, and all the output ROOT tree branches.
  4. In addition to all the global cuts (as mentioned above in point 1) the cuts defined under the 
     "other cuts" section of the configfile gets applied to h_W2, h_Q2, & all h_PovPel* histograms but
     they don't get appield to the output ROOT tree branches.
*/

#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>

#include "TCut.h"
#include "TH2D.h"
#include "TMath.h"
#include "TChain.h"
#include "TString.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TStopwatch.h"
#include "TTreeFormula.h"

Double_t const Mp = 0.938272081;  // +/- 6E-9 GeV

Int_t const ncell = 241;          // 189(SH) + 52(PS), Convention: 0-188: SH; 189-240: PS.
Int_t const kNblksSH = 189;       // Total # SH blocks/PMTs
Int_t const kNblksPS = 52;        // Total # PS blocks/PMTs
Int_t const kNcolsSH = 7;         // SH columns
Int_t const kNrowsSH = 27;        // SH rows
Int_t const kNcolsPS = 2;         // PS columns
Int_t const kNrowsPS = 26;        // PS rows
Double_t const zposSH = 1.901952; // m
Double_t const zposPS = 1.695704; // m

string getDate();
void CustmProfHisto(TH1D*);
TString GetOutFileBase(TString);
void ReadGain(TString, Double_t*);
std::vector<std::string> SplitString(char const delim, std::string const myStr);

void bbcal_eng_calib_w_h2(char const *configfilename,
			  bool isdebug=1) //0=False, 1=True
{
  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  TChain *C = new TChain("T");
  //creating base for outfile names
  TString cfgfilebase = GetOutFileBase(configfilename);

  TString macros_dir;
  Int_t Nmin = 10, ppass = 0;
  Double_t Nruns = -99., minMBratio = 0.1, hit_threshold = 0.;
  Double_t E_beam = 0., sbstheta = 0., hcaldist = 0., hcalheight = -0.2897;
  Double_t psE_cut_limit = 0., clusE_cut_limit = 0., EovP_cut_limit = 0.3;
  Double_t p_rec_Offset = 1., p_min_cut = 0., p_max_cut = 0.;
  Double_t W_mean = 0., W_sigma = 0., W_nsigma = 0.;
  Double_t PovPel_mean = 0., PovPel_sigma = 0., PovPel_nsigma = 0.;
  Double_t pspot_dxM = 0., pspot_dxS = 0., pspot_ndxS = 0.; 
  Double_t pspot_dyM = 0., pspot_dyS = 0., pspot_ndyS = 0.;
  bool read_gain = 0, cut_on_EovP = 0, cut_on_pmin = 0, cut_on_pmax = 0;
  bool cut_on_psE = 0, cut_on_clusE = 0;
  bool cut_on_W = 0, cut_on_PovPel = 0, cut_on_pspot = 0; 
  Double_t Corr_Factor_Enrg_Calib_w_Cosmic = 1., cF = 1.;
  Double_t h_W_bin = 200, h_W_min = 0., h_W_max = 5.;
  Double_t h_Q2_bin = 200, h_Q2_min = 0., h_Q2_max = 5.;
  Double_t h_PovPel_bin = 100, h_PovPel_min = -0.5, h_PovPel_max = 0.5;
  Double_t h_EovP_bin = 200, h_EovP_min = 0., h_EovP_max = 5., EovP_fit_width = 1.5;
  Double_t h_clusE_bin = 200, h_clusE_min = 0., h_clusE_max = 5.;
  Double_t h_shE_bin = 200, h_shE_min = 0., h_shE_max = 5.;
  Double_t h_psE_bin = 200, h_psE_min = 0., h_psE_max = 5.;
  Double_t h2_p_bin = 200, h2_p_min = 0., h2_p_max = 5.;
  Double_t h2_pang_bin = 200, h2_pang_min = 0., h2_pang_max = 5.;
  Double_t h2_p_coarse_bin = 25, h2_p_coarse_min = 0., h2_p_coarse_max = 5.;
  Double_t h2_EovP_bin = 200, h2_EovP_min = 0., h2_EovP_max = 5.;
  Double_t h2_dx_bin = 150, h2_dx_min = -2.5, h2_dx_max = 1.;
  Double_t h2_dy_bin = 200, h2_dy_min = -1., h2_dy_max = 1.;
  //parameters to calculate calibrated momentum
  bool mom_calib = 0;
  Double_t A_fit = 0., B_fit = 0., C_fit = 0.;
  Double_t bb_magdist = 1., GEMpitch = 10.;

  TMatrixD M(ncell,ncell), M_inv(ncell,ncell);
  TVectorD B(ncell), CoeffR(ncell);
  
  Double_t E_e = 0;
  Double_t p_rec = 0., px_rec = 0., py_rec = 0., pz_rec = 0.;
  Double_t p_calib = 0., p_calib_Offset = 0.;
  Double_t A[ncell];
  bool badCells[ncell]; // Cells that have events less than Nmin
  Int_t nevents_per_cell[ncell];

  // Define a clock to check macro processing time
  TStopwatch *sw = new TStopwatch();
  TStopwatch *sw2 = new TStopwatch();
  sw->Start(); sw2->Start();

  // Reading config file
  ifstream configfile(configfilename);
  char runlistfile[1000]; 
  TString currentline, readline;
  while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endRunlist") ){
    if( !currentline.BeginsWith("#") ){
      sprintf(runlistfile,"%s",currentline.Data());
      ifstream run_list(runlistfile);
      while( readline.ReadLine( run_list ) && !readline.BeginsWith("endlist") ){
  	if( !readline.BeginsWith("#") ){
	  if( readline.BeginsWith("Nruns") ){ // No. of runs to analyze
	    TObjArray *tokens = readline.Tokenize(" ");
	    Int_t ntokens = tokens->GetEntries();
	    if (ntokens>1) Nruns = ((TObjString*)(*tokens)[1])->GetString().Atof();
	    if (Nruns > 0) std::cout << "\nAnalyzing " << Nruns << " run(s)..\n";
	    else {
	      std::cerr << "ERROR! Illegal no. of runs! Check 'Nruns' variable inside run list!";
	      std::abort();
	    }
	  } else {
	    std::cout << readline << "\n";
	    C->Add(readline);
	  }
  	}
      }   
    } 
  }
  // TString currentline;
  // while( currentline.ReadLine( configfile ) && !currentline.BeginsWith("endRunlist") ){
  //   if( !currentline.BeginsWith("#") ){
  //     C->Add(currentline);
  //   }   
  // } 
  TCut globalcut = ""; TString gcutstr;
  while (currentline.ReadLine(configfile) && !currentline.BeginsWith("endcut")) {
    if (!currentline.BeginsWith("#")) {
      globalcut += currentline;
      gcutstr += currentline;
    }    
  }
  std::vector<std::string> gCutList = SplitString('&', gcutstr.Data());
  TTreeFormula *GlobalCut = new TTreeFormula("GlobalCut", globalcut, C);
  while (currentline.ReadLine(configfile)) {
    if (currentline.BeginsWith("#")) continue;
    TObjArray *tokens = currentline.Tokenize(" ");
    Int_t ntokens = tokens->GetEntries();
    if( ntokens>1 ){
      TString skey = ((TObjString*)(*tokens)[0])->GetString();
      if( skey == "macros_dir" ){
	macros_dir = ((TObjString*)(*tokens)[1])->GetString();
      }
      if( skey == "pre_pass" ){
	ppass = ((TObjString*)(*tokens)[1])->GetString().Atoi();
      }
      if( skey == "read_gain" ){
	read_gain = ((TObjString*)(*tokens)[1])->GetString().Atoi();
      }
      if( skey == "E_beam" ){
	E_beam = ((TObjString*)(*tokens)[1])->GetString().Atof();
      }
      if( skey == "SBS_theta" ){
	sbstheta = ((TObjString*)(*tokens)[1])->GetString().Atof();
	sbstheta *= TMath::DegToRad(); 
      }
      if( skey == "HCAL_dist" ){
	hcaldist = ((TObjString*)(*tokens)[1])->GetString().Atof();
      }
      if( skey == "hit_threshold" ){
	hit_threshold = ((TObjString*)(*tokens)[1])->GetString().Atof();
      }
      if( skey == "Min_Event_Per_Channel" ){
	Nmin = ((TObjString*)(*tokens)[1])->GetString().Atof();
      }
      if( skey == "Min_MB_Ratio" ){
	minMBratio = ((TObjString*)(*tokens)[1])->GetString().Atoi();
      }
      if( skey == "psE_cut" ){
	cut_on_psE = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	psE_cut_limit = ((TObjString*)(*tokens)[2])->GetString().Atof();
      } 
      if( skey == "clusE_cut" ){
	cut_on_clusE = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	clusE_cut_limit = ((TObjString*)(*tokens)[2])->GetString().Atof();
      }      
      if( skey == "pmin_cut" ){
	cut_on_pmin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	p_min_cut = ((TObjString*)(*tokens)[2])->GetString().Atof();
      }
      if( skey == "pmax_cut" ){
	cut_on_pmax = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	p_max_cut = ((TObjString*)(*tokens)[2])->GetString().Atof();
      }
      if( skey == "EovP_cut" ){
	cut_on_EovP = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	EovP_cut_limit = ((TObjString*)(*tokens)[2])->GetString().Atof();
      }
      if( skey == "W_cut" ){
	cut_on_W = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	W_mean = ((TObjString*)(*tokens)[2])->GetString().Atof();
	W_sigma = ((TObjString*)(*tokens)[3])->GetString().Atof();
	W_nsigma = ((TObjString*)(*tokens)[4])->GetString().Atof();
      }
      if( skey == "PovPel_cut" ){
	cut_on_PovPel = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	PovPel_mean = ((TObjString*)(*tokens)[2])->GetString().Atof();
	PovPel_sigma = ((TObjString*)(*tokens)[3])->GetString().Atof();
	PovPel_nsigma = ((TObjString*)(*tokens)[4])->GetString().Atof();
      }
      if( skey == "pspot_cut" ){
	cut_on_pspot = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	pspot_dxM = ((TObjString*)(*tokens)[2])->GetString().Atof();
	pspot_dxS = ((TObjString*)(*tokens)[3])->GetString().Atof();
	pspot_ndxS = ((TObjString*)(*tokens)[4])->GetString().Atof();
	pspot_dyM = ((TObjString*)(*tokens)[5])->GetString().Atof();
	pspot_dyS = ((TObjString*)(*tokens)[6])->GetString().Atof();
	pspot_ndyS = ((TObjString*)(*tokens)[7])->GetString().Atof();
      }
      if( skey == "h_W" ){
	h_W_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h_W_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h_W_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h_Q2" ){
	h_Q2_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h_Q2_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h_Q2_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h_PovPel" ){
	h_PovPel_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h_PovPel_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h_PovPel_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h_EovP" ){
	h_EovP_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h_EovP_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h_EovP_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "EovP_fit_width" ){
	EovP_fit_width = ((TObjString*)(*tokens)[1])->GetString().Atof();
      }
      if( skey == "h_clusE" ){
	h_clusE_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h_clusE_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h_clusE_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h_shE" ){
	h_shE_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h_shE_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h_shE_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h_psE" ){
	h_psE_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h_psE_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h_psE_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h2_p" ){
	h2_p_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h2_p_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h2_p_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h2_pang" ){
	h2_pang_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h2_pang_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h2_pang_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h2_p_coarse" ){
	h2_p_coarse_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h2_p_coarse_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h2_p_coarse_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h2_EovP" ){
	h2_EovP_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h2_EovP_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h2_EovP_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h2_dx" ){
	h2_dx_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h2_dx_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h2_dx_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "h2_dy" ){
	h2_dy_bin = ((TObjString*)(*tokens)[1])->GetString().Atoi();
	h2_dy_min = ((TObjString*)(*tokens)[2])->GetString().Atof();
	h2_dy_max = ((TObjString*)(*tokens)[3])->GetString().Atof();
      }
      if( skey == "p_rec_Offset" ){
	p_rec_Offset = ((TObjString*)(*tokens)[1])->GetString().Atof();
      }
      if( skey == "Corr_Factor_Enrg_Calib_w_Cosmic" ){
	cF = ((TObjString*)(*tokens)[1])->GetString().Atof();
      }
      if( skey == "mom_calib" ){
      	mom_calib = ((TObjString*)(*tokens)[1])->GetString().Atoi();
      	A_fit = ((TObjString*)(*tokens)[2])->GetString().Atof();
      	B_fit = ((TObjString*)(*tokens)[3])->GetString().Atof();
      	C_fit = ((TObjString*)(*tokens)[4])->GetString().Atof();
      	GEMpitch = ((TObjString*)(*tokens)[5])->GetString().Atof();
      	bb_magdist = ((TObjString*)(*tokens)[6])->GetString().Atof();
      }
      if( skey == "*****" ){
	break;
      }
    } 
    delete tokens;
  }
  
  // Sanity checks
  if ((cut_on_clusE&&cut_on_EovP) || (cut_on_clusE&&cut_on_pmin) || (cut_on_pmin&&cut_on_EovP)) 
    std::cout << "*!*[WARNING] Chosen cut combination involving bbcalE, trP, & E/p may introduce bias in the fit\n";
  if (cut_on_W && cut_on_PovPel) {
    std::cerr << "*!*[ERROR] Cutting on W is equivalent to cutting on PovPel! Turn one of them off and retry!\n"; 
    std::exit(1);
  }
  // defining elastic cut
  bool elastic_cut = cut_on_W || cut_on_PovPel || cut_on_pspot;

  // Check for empty rootfiles and set tree branches
  if(C->GetEntries()==0){
    std::cerr << "\n --- No ROOT file found!! --- \n\n";
    throw;
  }else std::cout << "\nFound " << C->GetEntries() << " events. Starting analysis.. \n";

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
  Double_t psAgainblk;         if (!read_gain) C->SetBranchAddress("bb.ps.againblk", &psAgainblk);
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
  Double_t shAgainblk;         if (!read_gain) C->SetBranchAddress("bb.sh.againblk", &shAgainblk);
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
  C->SetBranchStatus("sbs.hcal.e", 1);
  C->SetBranchStatus("bb.gem.track.nhits", 1);

  // Clear arrays
  memset(nevents_per_cell, 0, ncell*sizeof(int));
  memset(badCells, 0, ncell*sizeof(bool));
  
  // Let's read in old gain coefficients for both SH and PS
  std::cout << std::endl;
  Double_t oldADCgainSH[kNblksSH];
  Double_t oldADCgainPS[kNblksPS];
  for (int i=0; i<kNblksSH; i++) { oldADCgainSH[i] = -1000; }  
  for (int i=0; i<kNblksPS; i++) { oldADCgainPS[i] = -1000; }  
  TString adcGain_SH, gainRatio_SH, adcGain_PS, gainRatio_PS;
  if (read_gain) {
    adcGain_SH = Form("%s/Gain/%s_gainCoeff_sh.txt",macros_dir.Data(),cfgfilebase.Data());
    adcGain_PS = Form("%s/Gain/%s_gainCoeff_ps.txt",macros_dir.Data(),cfgfilebase.Data());
    ReadGain(adcGain_SH, oldADCgainSH);
    ReadGain(adcGain_PS, oldADCgainPS);
  }
  
  gStyle->SetOptStat(0);
  TH2D *h2_SHeng_vs_SHblk_raw = new TH2D("h2_SHeng_vs_SHblk_raw","Raw E_clus(SH) per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_raw = new TH2D("h2_EovP_vs_SHblk_raw","Raw E_clus/p_rec per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_raw_calib = new TH2D("h2_EovP_vs_SHblk_raw_calib","Raw E_clus/p_rec per SH block | After Calib.",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_count = new TH2D("h2_count","Count for E_clus/p_rec per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_count_calib = new TH2D("h2_count_calib","Count for E_clus/p_rec per SH block",kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS_raw = new TH2D("h2_EovP_vs_SHblk_trPOS_raw","Raw E_clus/p_rec per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_count_trP = new TH2D("h2_count_trP","Count for E_clus/p_rec per SH block(TrPos)",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_vs_PSblk_raw = new TH2D("h2_PSeng_vs_PSblk_raw","Raw E_clus(PS) per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_raw = new TH2D("h2_EovP_vs_PSblk_raw","Raw E_clus/p_rec per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_raw_calib = new TH2D("h2_EovP_vs_PSblk_raw_calib","Raw E_clus/p_rec per PS block | After Calib.",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_count_PS = new TH2D("h2_count_PS","Count for E_clus/p_rec per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_count_PS_calib = new TH2D("h2_count_PS_calib","Count for E_clus/p_rec per PS block",kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_trPOS_raw = new TH2D("h2_EovP_vs_PSblk_trPOS_raw","Raw E_clus/p_rec per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);
  TH2D *h2_count_trP_PS = new TH2D("h2_count_trP_PS","Count for E_clus/p_rec per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  // Creating output ROOT file to contain histograms
  TString outFile, outPlot;
  char const * debug = isdebug ? "_test" : "";
  char const * elcut = elastic_cut ? "_elcut" : "";
  outFile = Form("%s/hist/%s_prepass%d_bbcal_eng_calib%s%s.root",macros_dir.Data(),cfgfilebase.Data(),ppass,elcut,debug);
  outPlot = Form("%s/plots/%s_prepass%d_bbcal_eng_calib%s%s.pdf",macros_dir.Data(),cfgfilebase.Data(),ppass,elcut,debug);

  //std::unique_ptr<TFile> fout( TFile::Open(outFile, "RECREATE") );
  TFile *fout = new TFile(outFile, "RECREATE");
  fout->cd();

  // Physics histograms
  char const * hecut = elastic_cut ? " (el. cut)" : "";
  TH1D *h_W = new TH1D("h_W",";W (GeV)",h_W_bin,h_W_min,h_W_max);
  TH1D *h_W_pspotcut = new TH1D("h_W_pspotcut",";W (GeV) w/ pspotcut",h_W_bin,h_W_min,h_W_max);
  TH1D *h_Q2 = new TH1D("h_Q2","Q2 distribution",h_Q2_bin,h_Q2_min,h_Q2_max);
  TH1D *h_PovPel = new TH1D("h_PovPel",";p/p_{elastic}(#theta)",h_PovPel_bin,h_PovPel_min,h_PovPel_max);
  TH1D *h_PovPel_pspotcut = new TH1D("h_PovPel_pspotcut","p/p_{elastic}(#theta) w/ p spot cut;p/p_{elastic}(#theta) (w/ p spot cut)",h_PovPel_bin,h_PovPel_min,h_PovPel_max);
  TH1D *h_EovP = new TH1D("h_EovP",Form("E/p (Before Calib.)%s",hecut),h_EovP_bin,h_EovP_min,h_EovP_max);
  TH1D *h_EovP_calib = new TH1D("h_EovP_calib",Form("E/p%s",hecut),h_EovP_bin,h_EovP_min,h_EovP_max);
  TH1D *h_clusE = new TH1D("h_clusE",Form("Best SH+PS cl. eng.%s",hecut),h_clusE_bin,h_clusE_min,h_clusE_max);
  TH1D *h_clusE_calib = new TH1D("h_clusE_calib",Form("Best SH+PS cl. eng. u (sh/ps.e)*%2.2f%s",cF,hecut),h_clusE_bin,h_clusE_min,h_clusE_max);
  TH1D *h_SHclusE = new TH1D("h_SHclusE",Form("Best SH Cluster Energy%s",hecut),h_shE_bin,h_shE_min,h_shE_max);
  TH1D *h_SHclusE_calib = new TH1D("h_SHclusE_calib",Form("Best SH cl. eng. u (sh.e)*%2.2f%s",cF,hecut),h_shE_bin,h_shE_min,h_shE_max);
  TH1D *h_PSclusE = new TH1D("h_PSclusE",Form("Best PS Cluster Energy%s",hecut),h_psE_bin,h_psE_min,h_psE_max);
  TH1D *h_PSclusE_calib = new TH1D("h_PSclusE_calib",Form("Best PS cl. eng. u (ps.e)*%2.2f%s",cF,hecut),h_psE_bin,h_psE_min,h_psE_max);
  TH1D *h_shX_diff = new TH1D("h_shX_diff",Form("Vertical Position Difference%s; sh.x - tr.x (m)",hecut),200,-0.5,0.5);
  TH1D *h_shY_diff = new TH1D("h_shY_diff",Form("Horizontal Position Difference%s; sh.y - tr.y (m)",hecut),200,-0.5,0.5);
  TH1D *h_shX_diff_calib = new TH1D("h_shX_diff_calib",Form("Vertical Pos. Diff. | After Calib.%s; sh.x - tr.x (m)",hecut),200,-0.5,0.5);
  TH1D *h_shY_diff_calib = new TH1D("h_shY_diff_calib",Form("Horizontal Pos. Diff. | After Calib.%s; sh.y - tr.y (m)",hecut),200,-0.5,0.5);
  TH2D *h2_p_rec_vs_etheta = new TH2D("h2_p_rec_vs_etheta",Form("Track p vs Track ang%s",hecut),h2_pang_bin,h2_pang_min,h2_pang_max,h2_p_bin,h2_p_min,h2_p_max);

  TH2D *h2_EovP_vs_P = new TH2D("h2_EovP_vs_P",Form("E/p vs p%s; p (GeV); E/p",hecut),h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h2_EovP_bin,h2_EovP_min,h2_EovP_max);
  TProfile *h2_EovP_vs_P_prof = new TProfile("h2_EovP_vs_P_prof","E/p vs P (Profile)",h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h_EovP_min,h_EovP_max,"S");
  TH2D *h2_EovP_vs_P_calib = new TH2D("h2_EovP_vs_P_calib",Form("E/p vs p | After Calib.%s; p (GeV); E/p",hecut),h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h2_EovP_bin,h2_EovP_min,h2_EovP_max);
  TProfile *h2_EovP_vs_P_calib_prof = new TProfile("h2_EovP_vs_P_calib_prof","E/p vs P (Profile) a clib.",h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,h_EovP_min,h_EovP_max,"S");

  TH2D *h2_SHeng_vs_SHblk = new TH2D("h2_SHeng_vs_SHblk",Form("SH cl. eng. per SH block%s",hecut),kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk = new TH2D("h2_EovP_vs_SHblk",Form("E/p per SH block%s",hecut),kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_calib = new TH2D("h2_EovP_vs_SHblk_calib",Form("E/p per SH block | After Calib.%s",hecut),kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS = new TH2D("h2_EovP_vs_SHblk_trPOS",Form("E/p per SH block (TrPos)%s",hecut),kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);

  TH2D *h2_PSeng_vs_PSblk = new TH2D("h2_PSeng_vs_PSblk",Form("PS cl. eng. per PS block%s",hecut),kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk = new TH2D("h2_EovP_vs_PSblk",Form("E/p per PS block%s",hecut),kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_calib = new TH2D("h2_EovP_vs_PSblk_calib",Form("E/p per PS block | After Calib.%s",hecut),kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_EovP_vs_PSblk_trPOS = new TH2D("h2_EovP_vs_PSblk_trPOS",Form("E/p per PS block (TrPos)%s",hecut),kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  TH1D *h_thetabend = new TH1D("h_thetabend","",100,0.,0.25);

  TH2D *h2_EovP_vs_trX = new TH2D("h2_EovP_vs_trX",Form("E/p vs Track x%s",hecut),200,-0.8,0.8,200,0.4,1.6);
  TH2D *h2_EovP_vs_trX_calib = new TH2D("h2_EovP_vs_trX_calib",Form("E/p vs Track x | After Calib.%s",hecut),200,-0.8,0.8,200,0.4,1.6);
  TH2D *h2_EovP_vs_trY = new TH2D("h2_EovP_vs_trY",Form("E/p vs Track y%s",hecut),200,-0.16,0.16,200,0.4,1.6);
  TH2D *h2_EovP_vs_trY_calib = new TH2D("h2_EovP_vs_trY_calib",Form("E/p vs Track y | After Calib.%s",hecut),200,-0.16,0.16,200,0.4,1.6);
  TH2D *h2_EovP_vs_trTh = new TH2D("h2_EovP_vs_trTh",Form("E/p vs Track theta%s",hecut),200,-0.2,0.2,200,0.4,1.6);
  TH2D *h2_EovP_vs_trTh_calib = new TH2D("h2_EovP_vs_trTh_calib",Form("E/p vs Track theta | After Calib.%s",hecut),200,-0.2,0.2,200,0.4,1.6);
  TH2D *h2_EovP_vs_trPh = new TH2D("h2_EovP_vs_trPh",Form("E/p vs Track phi%s",hecut),200,-0.08,0.08,200,0.4,1.6);
  TH2D *h2_EovP_vs_trPh_calib = new TH2D("h2_EovP_vs_trPh_calib",Form("E/p vs Track phi | After Calib.%s",hecut),200,-0.08,0.08,200,0.4,1.6);
  TH2D *h2_PSeng_vs_trX = new TH2D("h2_PSeng_vs_trX",Form("PS energy vs Track x%s",hecut),200,-0.8,0.8,200,0,4);
  TH2D *h2_PSeng_vs_trX_calib = new TH2D("h2_PSeng_vs_trX_calib",Form("PS energy vs Track x | After Calib.%s",hecut),200,-0.8,0.8,200,0,4);
  TH2D *h2_PSeng_vs_trY = new TH2D("h2_PSeng_vs_trY",Form("PS energy vs Track y%s",hecut),200,-0.16,0.16,200,0,4);
  TH2D *h2_PSeng_vs_trY_calib = new TH2D("h2_PSeng_vs_trY_calib",Form("PS energy vs Track y | After Calib.%s",hecut),200,-0.16,0.16,200,0,4);  

  TH2D *h2_PSclsize_vs_rnum = new TH2D("h2_PSclsize_vs_rnum",Form("PS (best) cluster size vs Run no.%s",hecut),Nruns,0.5,Nruns+0.5,10,0,10);
  TProfile *h2_PSclsize_vs_rnum_prof = new TProfile("h2_PSclsize_vs_rnum_prof","PS (best) cluster size vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,0,10,"S");
  TH2D *h2_PSclmult_vs_rnum = new TH2D("h2_PSclmult_vs_rnum",Form("PS cluster multiplicity vs Run no.%s",hecut),Nruns,0.5,Nruns+0.5,10,0,10);
  TProfile *h2_PSclmult_vs_rnum_prof = new TProfile("h2_PSclmult_vs_rnum_prof","PS cluster multiplicity vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,0,10,"S");

  TH2D *h2_SHclsize_vs_rnum = new TH2D("h2_SHclsize_vs_rnum",Form("SH (best) cluster size vs Run no.%s",hecut),Nruns,0.5,Nruns+0.5,15,0,15);
  TProfile *h2_SHclsize_vs_rnum_prof = new TProfile("h2_SHclsize_vs_rnum_prof","SH (best) cluster size vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,0,15,"S");
  TH2D *h2_SHclmult_vs_rnum = new TH2D("h2_SHclmult_vs_rnum",Form("SH cluster multiplicity vs Run no.%s",hecut),Nruns,0.5,Nruns+0.5,10,0,10);
  TProfile *h2_SHclmult_vs_rnum_prof = new TProfile("h2_SHclmult_vs_rnum_prof","SH cluster multiplicity vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,0,10,"S");

  TH2D *h2_PovPel_vs_rnum_pspotcut = new TH2D("h2_PovPel_vs_rnum_pspotcut","p/p_{elastic}(#theta) vs Run no. w/ pspot cut",Nruns,0.5,Nruns+0.5,200,0.8,1.2);
  TProfile *h2_PovPel_vs_rnum_pspotcut_prof = new TProfile("h2_PovPel_vs_rnum_pspotcut_prof","p/p_{elastic}(#theta) vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,0.8,1.2,"S");

  TH2D *h2_EovP_vs_rnum = new TH2D("h2_EovP_vs_rnum",Form("E/p vs Run no.%s",hecut),Nruns,0.5,Nruns+0.5,200,0.4,1.6);
  TProfile *h2_EovP_vs_rnum_prof = new TProfile("h2_EovP_vs_rnum_prof","E/p vs Run no. (Profile)",Nruns,0.5,Nruns+0.5,0.4,1.6,"S");
  TH2D *h2_EovP_vs_rnum_calib = new TH2D("h2_EovP_vs_rnum_calib",Form("E/p vs Run no. | After Calib.%s",hecut),Nruns,0.5,Nruns+0.5,200,0.4,1.6);
  TProfile *h2_EovP_vs_rnum_calib_prof = new TProfile("h2_EovP_vs_rnum_calib_prof","E/p vs Run no. | After Calib. (Profile)",Nruns,0.5,Nruns+0.5,0.4,1.6,"S");

  TH2D *h2_dxdyHCAL = new TH2D("h2_dxdyHCAL","p Spot cut;#Deltay (m);#Deltax (m)",h2_dy_bin,h2_dy_min,h2_dy_max,h2_dx_bin,h2_dx_min,h2_dx_max);

  // defining output ROOT tree (Set max size to 4GB)
  //auto Tout = std::make_unique<TTree>("Tout", cfgfilebase.Data());
  TTree *Tout = new TTree("Tout", cfgfilebase.Data()); 
  Tout->SetMaxTreeSize(4000000000LL);  
  //
  bool WCut;            Tout->Branch("WCut", &WCut, "WCut/O");
  bool PovPelCut;       Tout->Branch("PovPelCut", &PovPelCut, "PovPelCut/O");
  bool pCut;            Tout->Branch("pCut", &pCut, "pCut/O");
  bool shEdge;          Tout->Branch("shEdge", &shEdge, "shEdge/O");
  //
  UInt_t    T_rnum;     Tout->Branch("rnum", &T_rnum, "rnum/i");
  ULong64_t T_gevnum;   Tout->Branch("gevnum", &T_gevnum, "gevnum/l");
  //
  Double_t T_ebeam;     Tout->Branch("ebeam", &E_beam, "ebeam/D");
  Double_t T_etheta;    Tout->Branch("etheta", &T_etheta, "etheta/D");
  Double_t T_ephi;      Tout->Branch("ephi", &T_ephi, "ephi/D");
  Double_t T_nu;        Tout->Branch("nu", &T_nu, "nu/D");
  Double_t T_W2;        Tout->Branch("W2", &T_W2, "W2/D");
  Double_t T_Q2;        Tout->Branch("Q2", &T_Q2, "Q2/D"); 
  Double_t T_PovPel;    Tout->Branch("PovPel", &T_PovPel, "PovPel/D");
  Double_t T_pelas;     Tout->Branch("pelas", &T_pelas, "pelas/D");
  //
  Double_t T_vz;        Tout->Branch("vz", &T_vz, "vz/D");
  Double_t T_trP;       Tout->Branch("trP", &T_trP, "trP/D");
  Double_t T_trX;       Tout->Branch("trX", &T_trX, "trX/D");
  Double_t T_trY;       Tout->Branch("trY", &T_trY, "trY/D");
  Double_t T_trTh;      Tout->Branch("trTh", &T_trTh, "trTh/D");
  Double_t T_trPh;      Tout->Branch("trPh", &T_trPh, "trPh/D");
  //
  Double_t T_thTdiff;   Tout->Branch("thTdiff", &T_thTdiff, "thTdiff/D");
  Double_t T_thTmean;   Tout->Branch("thTmean", &T_thTmean, "thTmean/D");
  Double_t T_thTOTmean; Tout->Branch("thTOTmean", &T_thTOTmean, "thTOTmean/D");
  //
  Double_t T_psE;       Tout->Branch("psE", &T_psE, "psE/D");
  Double_t T_psX;       Tout->Branch("psX", &T_psX, "psX/D");
  Double_t T_psY;       Tout->Branch("psY", &T_psY, "psY/D");
  Int_t    T_psNblk;    Tout->Branch("psNblk", &T_psNblk, "psNblk/I");    // size of best cluster (PS)
  Int_t    T_psNclus;   Tout->Branch("psNclus", &T_psNclus, "psNclus/I"); // cluster multiplicity (PS)
  Double_t T_psAtime;   Tout->Branch("psAtime", &T_psAtime, "psAtime/D"); // ADC time (PS)
  //
  Double_t T_clusE;     Tout->Branch("clusE", &T_clusE, "clusE/D");
  Double_t T_shX;       Tout->Branch("shX", &T_shX, "shX/D");
  Double_t T_shY;       Tout->Branch("shY", &T_shY, "shY/D");
  Int_t    T_shNblk;    Tout->Branch("shNblk", &T_shNblk, "shNblk/I");    // size of best cluster (SH)
  Int_t    T_shNclus;   Tout->Branch("shNclus", &T_shNclus, "shNclus/I"); // cluster multiplicity (SH)
  Double_t T_shAtime;   Tout->Branch("shAtime", &T_shAtime, "shAtime/D"); // ADC time (SH)
  Double_t T_shX_diff;  Tout->Branch("shX_diff", &T_shX_diff, "shX_diff/D");
  Double_t T_shY_diff;  Tout->Branch("shY_diff", &T_shY_diff, "shY_diff/D");
  //
  Double_t T_hcalE;     Tout->Branch("hcalE", &T_hcalE, "hcalE/D");
  Double_t T_hcalX;     Tout->Branch("hcalX", &T_hcalX, "hcalX/D");
  Double_t T_hcalY;     Tout->Branch("hcalY", &T_hcalY, "hcalY/D");
  Double_t T_hcalAtime; Tout->Branch("hcalAtime", &T_hcalAtime, "hcalAtime/D");
  //
  Double_t T_dx;        Tout->Branch("dx", &T_dx, "dx/D"); 
  Double_t T_dy;        Tout->Branch("dy", &T_dy, "dy/D");

  // calculating HCAL co-ordinates
  TVector3 HCAL_zaxis(sin(-sbstheta),0,cos(-sbstheta));
  TVector3 HCAL_xaxis(0,-1,0);
  TVector3 HCAL_yaxis = HCAL_zaxis.Cross(HCAL_xaxis).Unit();
  TVector3 HCAL_origin = hcaldist*HCAL_zaxis + hcalheight*HCAL_xaxis;

  ///////////////////////////////////////////
  // 1st Loop over all events to calibrate //
  ///////////////////////////////////////////

  std::cout << std::endl;
  Long64_t Ngoodevs=0, Nelasevs=0; 
  Long64_t Nevents = C->GetEntries(), nevent=0; UInt_t runnum=0; 
  Double_t timekeeper=0., timeremains=0.;
  Int_t treenum=0, currenttreenum=0, itrrun=0;
  std::vector<std::string> lrnum;    // list of run numbers

  while(C->GetEntry(nevent++)) {
    // Calculating remaining time 
    sw2->Stop();
    timekeeper += sw2->RealTime();
    if (nevent % 25000 == 0 && nevent != 0) 
      timeremains = timekeeper * (double(Nevents) / double(nevent) - 1.); 
    sw2->Reset();
    sw2->Continue();

    if(nevent % 100 == 0) std::cout << nevent << "/" << Nevents  << ", " << int(timeremains/60.) << "m \r";;
    std::cout.flush();
    // ------

    // get old gain coefficients
    if (!read_gain) {
      oldADCgainSH[int(shIdblk)] = shAgainblk;
      oldADCgainPS[int(psIdblk)] = psAgainblk;
    }

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
      memset(A, 0, ncell*sizeof(double));

      p_calib = trP[0];

      // *---- calculating calibrated momentum (Helps avoiding replay)
      if(mom_calib){
	TVector3 enhat_tgt(trTgth[0], trTgph[0], 1.0);
	enhat_tgt = enhat_tgt.Unit();	
	TVector3 enhat_fp(trRth[0], trRph[0], 1.0);
	enhat_fp = enhat_fp.Unit();
	TVector3 GEMzaxis(-sin(GEMpitch*TMath::DegToRad()),0,cos(GEMpitch*TMath::DegToRad()));
	TVector3 GEMyaxis(0,1,0);
	TVector3 GEMxaxis = (GEMyaxis.Cross(GEMzaxis)).Unit();	
	TVector3 enhat_fp_rot = enhat_fp.X() * GEMxaxis + enhat_fp.Y() * GEMyaxis + enhat_fp.Z() * GEMzaxis;
	double thetabend = acos(enhat_fp_rot.Dot(enhat_tgt));
	h_thetabend->Fill(thetabend);

	p_calib = A_fit * (1. + (B_fit + C_fit*bb_magdist) * trTgth[0]) / thetabend;
      }
      // *----
      
      p_calib_Offset = p_calib / trP[0];

      p_rec = trP[0] * p_calib_Offset * p_rec_Offset; 
      px_rec = trPx[0] * p_calib_Offset * p_rec_Offset; 
      py_rec = trPy[0] * p_calib_Offset * p_rec_Offset; 
      pz_rec = trPz[0] * p_calib_Offset * p_rec_Offset; 

      E_e = p_rec; // Neglecting e- mass. 

      // elastic calculations (Using 4-vector method)
      // Relevant 4-vectors
      /* Reaction    : e + e' -> p + p'
	 Conservation: Pe + Peprime = Pp + Ppprime */
      TVector3 vertex(0,0,trVz[0]);
      TLorentzVector Pe(0,0,E_beam,E_beam);           // incoming e- 4-vector
      TLorentzVector Peprime(px_rec,                  // scattered e- 4-vector
			     py_rec,
			     pz_rec,
			     p_rec);                 
      TLorentzVector Pp(0,0,0,Mp);                    // target nucleon 4-vector
      TLorentzVector Ppprime;                         // Recoil nucleon 4-vector
      TLorentzVector q = Pe - Peprime;                // 4-momentum of virtual photon
      // scattered e-
      Double_t etheta = TMath::ACos(pz_rec / p_rec);
      Double_t ephi = atan2(py_rec,px_rec);
      Double_t pelas = E_beam/(1. + (E_beam/Mp)*(1.0-cos(etheta)));
      // struck nucleon
      Double_t nu = q.E();
      Ppprime = q + Pp;
      TVector3 pNhat = Ppprime.Vect().Unit();
      Double_t Q2 = -q.M2();
      Double_t W2 = Ppprime.M2();
      Double_t W = sqrt(max(0., W2));
      Double_t PovPel = Peprime.E()/pelas;

      // calculating expected hit positions on HCAL
      Double_t sintersect = (HCAL_origin - vertex).Dot(HCAL_zaxis) / (pNhat.Dot(HCAL_zaxis));
      TVector3 HCAL_intersect = vertex + sintersect*pNhat; 
      Double_t hcalX_exp = (HCAL_intersect - HCAL_origin).Dot(HCAL_xaxis);
      Double_t hcalY_exp = (HCAL_intersect - HCAL_origin).Dot(HCAL_yaxis);
      Double_t dx = hcalX - hcalX_exp;
      Double_t dy = hcalY - hcalY_exp;

      // bbcal energy and position projections
      Double_t clusEngBBCal = (shE + psE) * Corr_Factor_Enrg_Calib_w_Cosmic;
      Double_t ClusEngSH = shE * Corr_Factor_Enrg_Calib_w_Cosmic;
      Double_t ClusEngPS = psE * Corr_Factor_Enrg_Calib_w_Cosmic;
      Double_t xtrATsh = trX[0] + zposSH*trTh[0];
      Double_t ytrATsh = trY[0] + zposSH*trPh[0];

      // cut definitions
      // cut on W
      WCut = abs(W - W_mean) <= W_sigma*W_nsigma;
      // cut on PovPel
      PovPelCut = abs(PovPel - PovPel_mean) <= PovPel_sigma*PovPel_nsigma;
      // defining pspot cut
      pCut = pow((dx-pspot_dxM) / (pspot_dxS*pspot_ndxS), 2) + pow((dy-pspot_dyM) / (pspot_dyS*pspot_ndyS), 2) <= 1.;
      // SH active area
      shEdge = shRowblk == 0 || shRowblk == 26 || shColblk == 0 || shColblk == 6;

      // fill out-tree branches before applying elastic cuts
      T_rnum = rnum;
      T_gevnum = gevnum;

      T_ebeam = E_beam;
      T_etheta = etheta;
      T_ephi = ephi;
      T_pelas = pelas;
      T_PovPel = PovPel;

      T_nu = nu;
      T_W2 = W2;
      T_Q2 = Q2;

      T_vz = trVz[0];
      T_trP = p_rec;
      T_trX = trX[0];
      T_trY = trY[0];
      T_trTh = trTh[0];
      T_trPh = trPh[0];

      T_thTdiff = thTdiff[0];
      T_thTmean = thTmean[0];
      T_thTOTmean = thTOTmean[0];

      T_psE = ClusEngPS;
      T_psX = psX;
      T_psY = psY;
      T_psNblk = psNblk;
      T_psNclus = psNclus;
      T_psAtime = psAtime;

      T_clusE = clusEngBBCal;
      T_shX = shX;
      T_shY = shY;
      T_shNblk = shNblk;
      T_shNclus = shNclus;
      T_shAtime = shAtime;
      T_shX_diff = shX - xtrATsh;
      T_shY_diff = shY - ytrATsh;

      T_hcalE = hcalE;
      T_hcalX = hcalX;
      T_hcalY = hcalY;
      T_hcalAtime = hcalAtime;

      T_dx = dx;
      T_dy = dy;

      Tout->Fill();

      /////////////////////
      // Additional cuts //
      /////////////////////

      // cut on p
      if (cut_on_pmin) if(p_rec < p_min_cut) continue;
      if (cut_on_pmax) if(p_rec > p_max_cut) continue;

      // ps cut
      if (cut_on_psE) if (ClusEngPS<psE_cut_limit) continue;
      // bbcal cluster eng. cut
      if (cut_on_clusE) if (clusEngBBCal<clusE_cut_limit) continue;
      // cut on E/p
      if (cut_on_EovP) if(fabs(clusEngBBCal/p_rec - 1.) > EovP_cut_limit) continue;
      Ngoodevs++;

      // filling some histos before cutting on elastics
      h_W->Fill(W);
      h_Q2->Fill(Q2);
      h_PovPel->Fill(PovPel);
      if (pCut) {
	h_W_pspotcut->Fill(W);
	h_PovPel_pspotcut->Fill(PovPel);
	h2_PovPel_vs_rnum_pspotcut->Fill(itrrun, PovPel);
	h2_PovPel_vs_rnum_pspotcut_prof->Fill(itrrun, PovPel, 1.);
      }
      h2_dxdyHCAL->Fill(dy,dx);

      /* elastic cuts */
      if (cut_on_W) if (!WCut) continue;
      if (cut_on_PovPel) if (!PovPelCut) continue;
      if (cut_on_pspot) if (!pCut) continue;
      Nelasevs++;
      /* ------------ */

      // Reject events with max edep on the edge (SH active area cut)
      if (shEdge) continue; 

      /************************
       * Starting calibration *
       ************************/

      // Loop over all the blocks in main cluster and fill in A's
      for(Int_t blk=0; blk<shNblk; blk++){
	Int_t blkID = int(shClBlkId[blk]);
	if (shClBlkE[blk]>hit_threshold) A[blkID] += shClBlkE[blk];
	nevents_per_cell[blkID]++; 
      }
    
      // ****** PreShower ******
      for(Int_t blk=0; blk<psNblk; blk++){
	Int_t blkID = int(psClBlkId[blk]);
	if (psClBlkE[blk]>hit_threshold) A[kNblksSH+blkID] += psClBlkE[blk];
	nevents_per_cell[kNblksSH+blkID]++;
      }

      // filling diagnostic histos
      h_EovP->Fill(clusEngBBCal / p_rec);
      h_clusE->Fill(clusEngBBCal);
      h_SHclusE->Fill(ClusEngSH);
      h_PSclusE->Fill(ClusEngPS);
      h2_p_rec_vs_etheta->Fill(etheta*TMath::RadToDeg(), p_rec);

      h_shX_diff->Fill(T_shX_diff);
      h_shY_diff->Fill(T_shY_diff);
      h2_EovP_vs_SHblk_trPOS_raw->Fill(ytrATsh, xtrATsh, clusEngBBCal/p_rec);
      h2_count_trP->Fill(ytrATsh, xtrATsh, 1.);
 
      //PS
      h2_PSeng_vs_PSblk_raw->Fill(psColblk, psRowblk, ClusEngPS);
      h2_EovP_vs_PSblk_raw->Fill(psColblk, psRowblk, clusEngBBCal/p_rec);
      h2_count_PS->Fill(psColblk, psRowblk, 1.);

      Double_t xtrATps = trX[0] + zposPS*trTh[0];
      Double_t ytrATps = trY[0] + zposPS*trPh[0];
      h2_EovP_vs_PSblk_trPOS_raw->Fill(ytrATps, xtrATps, clusEngBBCal/p_rec);
      h2_count_trP_PS->Fill(ytrATps, xtrATps, 1.);
      // -----

      // Checking to see if there is any bias in track recostruction ----
      //SH
      h2_SHeng_vs_SHblk_raw->Fill(shColblk, shRowblk, ClusEngSH);
      h2_EovP_vs_SHblk_raw->Fill(shColblk, shRowblk, clusEngBBCal/p_rec);
      h2_count->Fill(shColblk, shRowblk, 1.);

      // E/p vs. p
      h2_EovP_vs_P->Fill(p_rec, clusEngBBCal/p_rec);
      h2_EovP_vs_P_prof->Fill(p_rec, clusEngBBCal/p_rec, 1.);

      // histos to check bias in tracking
      h2_EovP_vs_trX->Fill(trX[0], clusEngBBCal/p_rec);
      h2_EovP_vs_trY->Fill(trY[0], clusEngBBCal/p_rec);
      h2_EovP_vs_trTh->Fill(trTh[0], clusEngBBCal/p_rec);
      h2_EovP_vs_trPh->Fill(trPh[0], clusEngBBCal/p_rec);
      h2_PSeng_vs_trX->Fill(trX[0], ClusEngPS);
      h2_PSeng_vs_trY->Fill(trY[0], ClusEngPS);

      // E/p vs. rnum (to check correlations with beam current and/or threshold)
      h2_EovP_vs_rnum->Fill(itrrun, clusEngBBCal/p_rec);
      h2_EovP_vs_rnum_prof->Fill(itrrun, clusEngBBCal/p_rec, 1.);

      // SH & PS cluster variables vs rnum (checking to see rate dependence)
      //PS
      h2_PSclsize_vs_rnum->Fill(itrrun, psNblk);
      h2_PSclsize_vs_rnum_prof->Fill(itrrun, psNblk, 1.);
      h2_PSclmult_vs_rnum->Fill(itrrun, psNclus);
      h2_PSclmult_vs_rnum_prof->Fill(itrrun, psNclus, 1.);
      //SH
      h2_SHclsize_vs_rnum->Fill(itrrun, shNblk);
      h2_SHclsize_vs_rnum_prof->Fill(itrrun, shNblk, 1.);
      h2_SHclmult_vs_rnum->Fill(itrrun, shNclus);
      h2_SHclmult_vs_rnum_prof->Fill(itrrun, shNclus, 1.);

      // Let's costruct the matrix
      for(Int_t icol = 0; icol<ncell; icol++){
	B(icol)+= A[icol];
	for(Int_t irow = 0; irow<ncell; irow++){
	  M(icol,irow)+= A[icol]*A[irow]/E_e;
	} 
      }   
      
    } //global cut
  } //event loop
  h2_EovP_vs_SHblk->Divide(h2_EovP_vs_SHblk_raw, h2_count);
  h2_EovP_vs_PSblk->Divide(h2_EovP_vs_PSblk_raw, h2_count_PS);
  h2_SHeng_vs_SHblk->Divide(h2_SHeng_vs_SHblk_raw, h2_count);
  h2_PSeng_vs_PSblk->Divide(h2_PSeng_vs_PSblk_raw, h2_count_PS);
  h2_EovP_vs_SHblk_trPOS->Divide(h2_EovP_vs_SHblk_trPOS_raw, h2_count_trP);
  h2_EovP_vs_PSblk_trPOS->Divide(h2_EovP_vs_PSblk_trPOS_raw, h2_count_trP_PS);
  std::cout << "\n\n";

  // Let's customize the histogram ranges
  h2_SHeng_vs_SHblk->GetZaxis()->SetRangeUser(0.9,2.0);
  h2_EovP_vs_SHblk->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_EovP_vs_SHblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_PSeng_vs_PSblk->GetZaxis()->SetRangeUser(0.36,1.28);
  h2_EovP_vs_PSblk->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_EovP_vs_PSblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);

  // Customizing profile histograms
  CustmProfHisto(h2_PovPel_vs_rnum_pspotcut_prof);
  CustmProfHisto(h2_EovP_vs_P_prof); CustmProfHisto(h2_EovP_vs_P_calib_prof);
  CustmProfHisto(h2_EovP_vs_rnum_prof); CustmProfHisto(h2_EovP_vs_rnum_calib_prof);
  CustmProfHisto(h2_PSclsize_vs_rnum_prof); CustmProfHisto(h2_PSclmult_vs_rnum_prof);
  CustmProfHisto(h2_SHclsize_vs_rnum_prof); CustmProfHisto(h2_SHclmult_vs_rnum_prof);

  // B.Print();  
  // M.Print();

  ////////////////////////////////////////////////////
  // Time to calculate and report gain coefficients //
  ////////////////////////////////////////////////////

  TH1D *h_nevent_blk_SH = new TH1D("h_nevent_blk_SH", "No. of Good Events; SH Blocks", kNblksSH, 0, kNblksSH);
  TH1D *h_coeff_Ratio_SH = new TH1D("h_coeff_Ratio_SH", "Ratio of Gain Coefficients(new/old); SH Blocks", kNblksSH, 0, kNblksSH);
  TH1D *h_coeff_blk_SH = new TH1D("h_coeff_blk_SH", "ADC Gain Coefficients(GeV/pC); SH Blocks", kNblksSH, 0, kNblksSH);
  TH1D *h_old_coeff_blk_SH = new TH1D("h_old_coeff_blk_SH", "Old ADC Gain Coefficients(GeV/pC); SH Blocks", kNblksSH, 0, kNblksSH);
  TH2D *h2_old_coeff_detView_SH = new TH2D("h2_old_coeff_detView_SH", "Old ADC Gain Coefficients | SH", kNcolsSH, 1, kNcolsSH+1, kNrowsSH, 1, kNrowsSH+1);
  TH2D *h2_coeff_detView_SH = new TH2D("h2_coeff_detView_SH", "New ADC Gain Coefficients | SH", kNcolsSH, 1, kNcolsSH+1, kNrowsSH, 1, kNrowsSH+1);

  TH1D *h_nevent_blk_PS = new TH1D("h_nevent_blk_PS", "No. of Good Events; PS Blocks", kNblksPS, 0, kNblksPS);
  TH1D *h_coeff_Ratio_PS = new TH1D("h_coeff_Ratio_PS", "Ratio of Gain Coefficients(new/old); PS Blocks", kNblksPS, 0, kNblksPS);
  TH1D *h_coeff_blk_PS = new TH1D("h_coeff_blk_PS", "ADC Gain Coefficients(GeV/pC); PS Blocks", kNblksPS, 0, kNblksPS);
  TH1D *h_old_coeff_blk_PS = new TH1D("h_old_coeff_blk_PS", "Old ADC Gain Coefficients(GeV/pC); PS Blocks", kNblksPS, 0, kNblksPS);
  TH2D *h2_old_coeff_detView_PS = new TH2D("h2_old_coeff_detView_PS", "Old ADC Gain Coefficients | PS", kNcolsPS, 1, kNcolsPS+1, kNrowsPS, 1, kNrowsPS+1);
  TH2D *h2_coeff_detView_PS = new TH2D("h2_coeff_detView_PS", "New ADC Gain Coefficients | PS", kNcolsPS, 1, kNcolsPS+1, kNrowsPS, 1, kNrowsPS+1);

  // Leave the bad channels out of the calculation
  for(Int_t j = 0; j<ncell; j++){
    badCells[j]=false;
    if (nevents_per_cell[j] < Nmin || M(j,j) < minMBratio*B(j)) {
      B(j) = 1.;
      M(j, j) = 1.;
      for(Int_t k = 0; k<ncell; k++){
	if(k!=j){
	  M(j, k) = 0.;
	  M(k, j) = 0.;
	}
      }
      badCells[j]=true;
    }
  }  
  
  // Getting coefficients (rather ratios)
  M_inv = M.Invert();
  CoeffR = M_inv*B;

  // SH : Filling diagnostic histograms
  Int_t cell = 0;
  adcGain_SH = Form("%s/Gain/%s_prepass%d_gainCoeff_sh%s%s.txt",macros_dir.Data(),cfgfilebase.Data(),ppass,elcut,debug);
  gainRatio_SH = Form("%s/Gain/%s_prepass%d_gainRatio_sh%s%s.txt",macros_dir.Data(),cfgfilebase.Data(),ppass,elcut,debug);
  Double_t newADCgratioSH[kNcolsSH*kNrowsSH];
  for (int i=0; i<kNblksSH; i++) { newADCgratioSH[i] = -1000; }  
  ofstream adcGainSH_outData, gainRatioSH_outData;
  adcGainSH_outData.open(adcGain_SH);
  gainRatioSH_outData.open(gainRatio_SH);
  for(Int_t shrow = 0; shrow<kNrowsSH; shrow++){
    for(Int_t shcol = 0; shcol<kNcolsSH; shcol++){
      Double_t oldCoeff = oldADCgainSH[shrow*kNcolsSH+shcol];
      if(!badCells[cell]){
	h_coeff_Ratio_SH->Fill(cell, CoeffR(cell));
	h_coeff_blk_SH->Fill(cell, CoeffR(cell) * oldCoeff);
	h_nevent_blk_SH->Fill(cell, nevents_per_cell[cell]);
	h_old_coeff_blk_SH->Fill(cell, oldCoeff);
	h2_old_coeff_detView_SH->Fill(shcol+1, shrow+1, oldCoeff);
	h2_coeff_detView_SH->Fill(shcol+1, shrow+1, CoeffR(cell) * oldCoeff);

	cout << CoeffR(cell) << "  ";
	adcGainSH_outData << CoeffR(cell) * oldCoeff << " ";
	gainRatioSH_outData << CoeffR(cell) << " ";
	newADCgratioSH[cell] = CoeffR(cell);
      }else{
	h_nevent_blk_SH->Fill(cell, nevents_per_cell[cell]);
	h_old_coeff_blk_SH->Fill(cell, oldCoeff);
	h_coeff_Ratio_SH->Fill(cell, 1. * Corr_Factor_Enrg_Calib_w_Cosmic);
	h_coeff_blk_SH->Fill(cell, oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic);
	h2_old_coeff_detView_SH->Fill(shcol+1, shrow+1, oldCoeff);
	h2_coeff_detView_SH->Fill(shcol+1, shrow+1, oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic);

	cout << 1.*Corr_Factor_Enrg_Calib_w_Cosmic << "  ";
	adcGainSH_outData << oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	gainRatioSH_outData << 1. * Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	newADCgratioSH[cell] = 1. * Corr_Factor_Enrg_Calib_w_Cosmic;
      }
      cell++;
    }
    std::cout << std::endl;
    adcGainSH_outData << std::endl;
    gainRatioSH_outData << std::endl;
  }
  std::cout << std::endl;

  // customizing histograms
  h_nevent_blk_SH->SetLineWidth(0); h_nevent_blk_SH->SetMarkerStyle(8);
  h_coeff_Ratio_SH->SetLineWidth(0); h_coeff_Ratio_SH->SetMarkerStyle(8);
  h_coeff_blk_SH->SetLineWidth(0); h_coeff_blk_SH->SetMarkerStyle(8);
  h_old_coeff_blk_SH->SetLineWidth(0); h_old_coeff_blk_SH->SetMarkerStyle(8);

  // PS : Filling diagnostic histograms
  adcGain_PS = Form("%s/Gain/%s_prepass%d_gainCoeff_ps%s%s.txt",macros_dir.Data(),cfgfilebase.Data(),ppass,elcut,debug);
  gainRatio_PS = Form("%s/Gain/%s_prepass%d_gainRatio_ps%s%s.txt",macros_dir.Data(),cfgfilebase.Data(),ppass,elcut,debug);
  Double_t newADCgratioPS[kNcolsPS*kNrowsPS];
  for (int i=0; i<kNblksPS; i++) { newADCgratioPS[i] = -1000; }  
  ofstream adcGainPS_outData, gainRatioPS_outData;
  adcGainPS_outData.open(adcGain_PS);
  gainRatioPS_outData.open(gainRatio_PS);
  for(Int_t psrow = 0; psrow<kNrowsPS; psrow++){
    for(Int_t pscol = 0; pscol<kNcolsPS; pscol++){
      Int_t psBlock = psrow * kNcolsPS + pscol;
      Double_t oldCoeff = oldADCgainPS[psrow*kNcolsPS+pscol];
      if(!badCells[cell]){
	h_coeff_Ratio_PS->Fill(psBlock, CoeffR(cell));
	h_coeff_blk_PS->Fill(psBlock, CoeffR(cell) * oldCoeff);
	h_nevent_blk_PS->Fill(psBlock, nevents_per_cell[cell]);
	h_old_coeff_blk_PS->Fill(psBlock, oldCoeff);
	h2_old_coeff_detView_PS->Fill(pscol+1, psrow+1, oldCoeff);
	h2_coeff_detView_PS->Fill(pscol+1, psrow+1, CoeffR(cell) * oldCoeff);

	cout << CoeffR(cell) << "  ";
	adcGainPS_outData << CoeffR(cell) * oldCoeff << " ";
	gainRatioPS_outData << CoeffR(cell) << " ";
	newADCgratioPS[psBlock] = CoeffR(cell);
      }else{
	h_nevent_blk_PS->Fill(psBlock, nevents_per_cell[cell]);
	h_old_coeff_blk_PS->Fill(psBlock, oldCoeff);
	h_coeff_Ratio_PS->Fill(psBlock, 1. * Corr_Factor_Enrg_Calib_w_Cosmic);
	h_coeff_blk_PS->Fill(psBlock, oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic);
	h2_old_coeff_detView_PS->Fill(pscol+1, psrow+1, oldCoeff);
	h2_coeff_detView_PS->Fill(pscol+1, psrow+1, oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic);

	cout << 1. * Corr_Factor_Enrg_Calib_w_Cosmic << "  ";
	adcGainPS_outData << oldCoeff * Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	gainRatioPS_outData << 1. * Corr_Factor_Enrg_Calib_w_Cosmic << " ";
	newADCgratioPS[psBlock] = 1. * Corr_Factor_Enrg_Calib_w_Cosmic;
      }
      cell++;
    }
    std::cout << std::endl;
    adcGainPS_outData << std::endl;
    gainRatioPS_outData << std::endl;
  }
  std::cout << std::endl;

  // customizing histograms
  h_nevent_blk_PS->SetLineWidth(0); h_nevent_blk_PS->SetMarkerStyle(8);
  h_coeff_Ratio_PS->SetLineWidth(0); h_coeff_Ratio_PS->SetMarkerStyle(8);
  h_coeff_blk_PS->SetLineWidth(0); h_coeff_blk_PS->SetMarkerStyle(8);
  h_old_coeff_blk_PS->SetLineWidth(0); h_old_coeff_blk_PS->SetMarkerStyle(8);

  //////////////////////////////////////////////////////////////////////
  // 2nd Loop over all events to check the performance of calibration //
  //////////////////////////////////////////////////////////////////////

  // add branches to Tout to store values after calibration
  Double_t T_psE_calib;       TBranch *T_psE_c = Tout->Branch("psE_calib", &T_psE_calib, "psE_calib/D");
  Double_t T_clusE_calib;     TBranch *T_clusE_c = Tout->Branch("clusE_calib", &T_clusE_calib, "clusE_calib/D");
  Double_t T_psX_calib;       TBranch *T_psX_c = Tout->Branch("psX_calib", &T_psX_calib, "psX_calib/D");
  Double_t T_psY_calib;       TBranch *T_psY_c = Tout->Branch("psY_calib", &T_psY_calib, "psY_calib/D");
  Double_t T_shX_calib;       TBranch *T_shX_c = Tout->Branch("shX_calib", &T_shX_calib, "shX_calib/D");
  Double_t T_shY_calib;       TBranch *T_shY_c = Tout->Branch("shY_calib", &T_shY_calib, "shY_calib/D");
  Double_t T_shX_diff_calib;  TBranch *T_shX_diff_c = Tout->Branch("shX_diff_calib", &T_shX_diff_calib, "shX_diff_calib/D");
  Double_t T_shY_diff_calib;  TBranch *T_shY_diff_c = Tout->Branch("shY_diff_calib", &T_shY_diff_calib, "shY_diff_calib/D");

  itrrun=0; runnum=0; 
  Nevents = C->GetEntries(), nevent=0;
  std::cout << "Looping over events again to check calibration.." << std::endl; 
  while(C->GetEntry(nevent++)) {
    // Calculating remaining time 
    sw2->Stop();
    timekeeper += sw2->RealTime();
    if (nevent % 25000 == 0 && nevent != 0) 
      timeremains = timekeeper * (double(Nevents) / double(nevent) - 1.); 
    sw2->Reset();
    sw2->Continue();

    if(nevent % 100 == 0) std::cout << nevent << "/" << Nevents  << ", " << int(timeremains/60.) << "m \r";;
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

      p_calib = trP[0];

      // *---- calculating calibrated momentum (Helps avoiding replay)
      if(mom_calib){
	TVector3 enhat_tgt(trTgth[0], trTgph[0], 1.0);
	enhat_tgt = enhat_tgt.Unit();	
	TVector3 enhat_fp(trRth[0], trRph[0], 1.0);
	enhat_fp = enhat_fp.Unit();
	TVector3 GEMzaxis(-sin(GEMpitch*TMath::DegToRad()),0,cos(GEMpitch*TMath::DegToRad()));
	TVector3 GEMyaxis(0,1,0);
	TVector3 GEMxaxis = (GEMyaxis.Cross(GEMzaxis)).Unit();	
	TVector3 enhat_fp_rot = enhat_fp.X() * GEMxaxis + enhat_fp.Y() * GEMyaxis + enhat_fp.Z() * GEMzaxis;
	double thetabend = acos(enhat_fp_rot.Dot(enhat_tgt));
	//h_thetabend->Fill(thetabend);

	p_calib = A_fit * (1. + (B_fit + C_fit*bb_magdist) * trTgth[0]) / thetabend;
      }
      // *----

      p_calib_Offset = p_calib / trP[0];

      p_rec = trP[0] * p_calib_Offset * p_rec_Offset; 
      px_rec = trPx[0] * p_calib_Offset * p_rec_Offset; 
      py_rec = trPy[0] * p_calib_Offset * p_rec_Offset; 
      pz_rec = trPz[0] * p_calib_Offset * p_rec_Offset;

      // elastic calculations (Using 4-vector method)
      // Relevant 4-vectors
      /* Reaction    : e + e' -> p + p'
	 Conservation: Pe + Peprime = Pp + Ppprime */
      TVector3 vertex(0,0,trVz[0]);
      TLorentzVector Pe(0,0,E_beam,E_beam);           // incoming e- 4-vector
      TLorentzVector Peprime(px_rec,                  // scattered e- 4-vector
			     py_rec,
			     pz_rec,
			     p_rec);                 
      TLorentzVector Pp(0,0,0,Mp);                    // target nucleon 4-vector
      TLorentzVector Ppprime;                         // Recoil nucleon 4-vector
      TLorentzVector q = Pe - Peprime;                // 4-momentum of virtual photon
      // scattered e-
      Double_t etheta = TMath::ACos(pz_rec / p_rec);
      Double_t ephi = atan2(py_rec,px_rec);
      Double_t pelas = E_beam/(1. + (E_beam/Mp)*(1.0-cos(etheta)));
      // struck nucleon
      Double_t nu = q.E();
      Ppprime = q + Pp;
      TVector3 pNhat = Ppprime.Vect().Unit();
      Double_t Q2 = -q.M2();
      Double_t W2 = Ppprime.M2();
      Double_t W = sqrt(max(0., W2));
      Double_t PovPel = Peprime.E()/pelas;

      // calculating expected hit positions on HCAL
      Double_t sintersect = (HCAL_origin - vertex).Dot(HCAL_zaxis) / (pNhat.Dot(HCAL_zaxis));
      TVector3 HCAL_intersect = vertex + sintersect*pNhat; 
      Double_t hcalX_exp = (HCAL_intersect - HCAL_origin).Dot(HCAL_xaxis);
      Double_t hcalY_exp = (HCAL_intersect - HCAL_origin).Dot(HCAL_yaxis);
      Double_t dx = hcalX - hcalX_exp;
      Double_t dy = hcalY - hcalY_exp;

      // calculating calibrated BBCAL energy
      // ****** Shower ******
      Double_t shClusE = 0., shX_calib = 0., shY_calib = 0.;
      for(Int_t blk=0; blk<shNblk; blk++){
  	Int_t blkID = int(shClBlkId[blk]);
	// calculating the updated cluster centroid
	shX_calib = (shX_calib*shClusE + shClBlkX[blk]*shClBlkE[blk]) / (shClusE+shClBlkE[blk]);
	shY_calib = (shY_calib*shClusE + shClBlkY[blk]*shClBlkE[blk]) / (shClusE+shClBlkE[blk]);
	 
	Double_t shClBlkE_calib = shClBlkE[blk] * newADCgratioSH[blkID];
 	if (shClBlkE_calib>hit_threshold) shClusE += shClBlkE_calib;
      }
      // ****** PreShower ******
      Double_t psClusE = 0., psX_calib = 0., psY_calib = 0.;
      for(Int_t blk=0; blk<psNblk; blk++){
	Int_t blkID = int(psClBlkId[blk]);
	// calculating the updated cluster centroid
	psX_calib = (psX_calib*psClusE + psClBlkX[blk]*psClBlkE[blk]) / (psClusE+psClBlkE[blk]);
	psY_calib = (psY_calib*psClusE + psClBlkY[blk]*psClBlkE[blk]) / (psClusE+psClBlkE[blk]);

	Double_t psClBlkE_calib = psClBlkE[blk] * newADCgratioPS[blkID];
	if (psClBlkE_calib>hit_threshold) psClusE += psClBlkE_calib;
      }
      Double_t clusEngBBCal = shClusE + psClusE;
      Double_t xtrATsh = trX[0] + zposSH*trTh[0];
      Double_t ytrATsh = trY[0] + zposSH*trPh[0];
      Double_t shX_diff = shX_calib - xtrATsh;
      Double_t shY_diff = shY_calib - ytrATsh;

      // filling tree with calibrated entries
      T_psE_calib = psClusE;        T_psE_c->Fill();
      T_clusE_calib = clusEngBBCal; T_clusE_c->Fill();

      T_shX_calib = shX_calib; T_shX_c->Fill();
      T_shY_calib = shY_calib; T_shY_c->Fill();

      T_shX_diff_calib = shX_diff; T_shX_diff_c->Fill();
      T_shY_diff_calib = shY_diff; T_shY_diff_c->Fill();

      T_psX_calib = psX_calib; T_psX_c->Fill();
      T_psY_calib = psY_calib; T_psY_c->Fill();

      //////////////////////////////////////////////////////
      // Additional cuts before filling diagnostic histos //
      //////////////////////////////////////////////////////

      // cut on p
      if (cut_on_pmin) if(p_rec < p_min_cut) continue;
      if (cut_on_pmax) if(p_rec > p_max_cut) continue;

      // ps cut
      if (cut_on_psE) if (psClusE<psE_cut_limit) continue;
      // bbcal cluster eng. cut
      if (cut_on_clusE) if (clusEngBBCal<clusE_cut_limit) continue;
      // cut on E/p
      if (cut_on_EovP) if(fabs(clusEngBBCal/p_rec - 1.) > EovP_cut_limit) continue;

      /* elastic cuts */
      // cut on W
      bool WCutc = fabs(W - W_mean) <= W_sigma*W_nsigma;
      if (cut_on_W) if (!WCutc) continue;
      // cut on PovPel
      bool PovPelCutc = fabs(PovPel - PovPel_mean) <= PovPel_sigma*PovPel_nsigma;
      if (cut_on_PovPel) if (!PovPelCutc) continue;
      // defining pspot cut
      bool pCutc = pow((dx-pspot_dxM) / (pspot_dxS*pspot_ndxS), 2) + pow((dy-pspot_dyM) / (pspot_dyS*pspot_ndyS), 2) <= 1.;
      if (cut_on_pspot) if (!pCutc) continue;
      /* ------------ */

      // Reject events with max edep on the edge (SH active area cut)
      shEdge = shRowblk == 0 || shRowblk == 26 || shColblk == 0 || shColblk == 6;
      if (shEdge) continue; 

      // Let's fill diagnostic histograms
      h_shX_diff_calib->Fill(shX_diff);
      h_shY_diff_calib->Fill(shY_diff);

      h_EovP_calib->Fill(clusEngBBCal / p_rec);
      h_clusE_calib->Fill(clusEngBBCal);
      h_SHclusE_calib->Fill(shClusE);
      h_PSclusE_calib->Fill(psClusE);

      h2_EovP_vs_P_calib->Fill(p_rec, clusEngBBCal/p_rec);
      h2_EovP_vs_P_calib_prof->Fill(p_rec, clusEngBBCal/p_rec, 1.);

      h2_count_calib->Fill(shColblk, shRowblk, 1.);
      h2_EovP_vs_SHblk_raw_calib->Fill(shColblk, shRowblk, clusEngBBCal/p_rec);

      h2_count_PS_calib->Fill(psColblk, psRowblk, 1.);
      h2_EovP_vs_PSblk_raw_calib->Fill(psColblk, psRowblk, clusEngBBCal/p_rec);

      // histos to check bias in tracking
      h2_EovP_vs_trX_calib->Fill(trX[0], clusEngBBCal/p_rec);
      h2_EovP_vs_trY_calib->Fill(trY[0], clusEngBBCal/p_rec);
      h2_EovP_vs_trTh_calib->Fill(trTh[0], clusEngBBCal/p_rec);
      h2_EovP_vs_trPh_calib->Fill(trPh[0], clusEngBBCal/p_rec);
      h2_PSeng_vs_trX_calib->Fill(trX[0], psClusE);
      h2_PSeng_vs_trY_calib->Fill(trY[0], psClusE);

      // E/p vs. rnum (to check correlations with beam current and/or threshold)
      h2_EovP_vs_rnum_calib->Fill(itrrun, clusEngBBCal/p_rec);
      h2_EovP_vs_rnum_calib_prof->Fill(itrrun, clusEngBBCal/p_rec, 1.);
    }
  }
  h2_EovP_vs_SHblk_calib->Divide(h2_EovP_vs_SHblk_raw_calib, h2_count_calib);
  h2_EovP_vs_PSblk_calib->Divide(h2_EovP_vs_PSblk_raw_calib, h2_count_PS_calib);
  std::cout << "\n\n";

  // Let's customize the histogram ranges
  h2_EovP_vs_SHblk_calib->GetZaxis()->SetRangeUser(0.8,1.2);
  h2_EovP_vs_PSblk_calib->GetZaxis()->SetRangeUser(0.8,1.2);

  /////////////////////////////////
  // Generating diagnostic plots //
  /////////////////////////////////
  /**** Canvas 1 (E/p) ****/
  TCanvas *c1 = new TCanvas("c1","E/p",1500,1200);
  c1->Divide(3,2);
  c1->cd(1); //
  gPad->SetGridx();
  Double_t param[3], param_bc[3], sigerr, sigerr_bc;
  Int_t maxBin_bc = h_EovP->GetMaximumBin();
  Double_t binW_bc = h_EovP->GetBinWidth(maxBin_bc), norm_bc = h_EovP->GetMaximum();
  Double_t mean_bc = h_EovP->GetMean(), stdev_bc = h_EovP->GetStdDev();
  Double_t lower_lim_bc = h_EovP_min + maxBin_bc*binW_bc - EovP_fit_width*stdev_bc;
  Double_t upper_lim_bc = h_EovP_min + maxBin_bc*binW_bc + EovP_fit_width*stdev_bc; 
  TF1* fitg_bc = new TF1("fitg_bc","gaus",h_EovP_min,h_EovP_max);
  fitg_bc->SetRange(lower_lim_bc,upper_lim_bc);
  fitg_bc->SetParameters(norm_bc,mean_bc,stdev_bc);
  fitg_bc->SetLineWidth(2); fitg_bc->SetLineColor(2);
  h_EovP->Fit(fitg_bc,"NO+QR"); fitg_bc->GetParameters(param_bc); sigerr_bc = fitg_bc->GetParError(2);
  h_EovP->SetLineWidth(2); h_EovP->SetLineColor(kGreen+2);
  Int_t maxBin = h_EovP_calib->GetMaximumBin();
  Double_t binW = h_EovP_calib->GetBinWidth(maxBin), norm = h_EovP_calib->GetMaximum();
  Double_t mean = h_EovP_calib->GetMean(), stdev = h_EovP_calib->GetStdDev();
  Double_t lower_lim = h_EovP_min + maxBin*binW - EovP_fit_width*stdev;
  Double_t upper_lim = h_EovP_min + maxBin*binW + EovP_fit_width*stdev; 
  TF1* fitg = new TF1("fitg","gaus",h_EovP_min,h_EovP_max);
  fitg->SetRange(lower_lim,upper_lim);
  fitg->SetParameters(norm,mean,stdev);
  fitg->SetLineWidth(2); fitg->SetLineColor(2);
  h_EovP_calib->Fit(fitg,"QR"); fitg->GetParameters(param); sigerr = fitg->GetParError(2);
  h_EovP_calib->SetLineWidth(2); h_EovP_calib->SetLineColor(1);
  // adjusting histogram height for the legend to fit properly
  h_EovP_calib->GetYaxis()->SetRangeUser(0.,max(norm,norm_bc)*1.2);
  h_EovP_calib->Draw(); h_EovP->Draw("same");
  // draw the legend
  TLegend *l = new TLegend(0.10,0.78,0.90,0.90);
  l->SetTextFont(42);
  l->AddEntry(h_EovP,Form("Before calib., #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param_bc[1],param_bc[2]*100,sigerr_bc*100),"l");
  l->AddEntry(h_EovP_calib,Form("After calib., #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param[1],param[2]*100,sigerr*100),"l");
  l->Draw();
  c1->cd(2); //
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  h2_EovP_vs_P->SetStats(0);
  h2_EovP_vs_P->Draw("colz");
  h2_EovP_vs_P_prof->Draw("same");
  c1->cd(3); //
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  h2_EovP_vs_P_calib->SetStats(0);
  h2_EovP_vs_P_calib->Draw("colz");
  h2_EovP_vs_P_calib_prof->Draw("same");
  c1->cd(4); //
  h2_EovP_vs_SHblk->SetStats(0);
  h2_EovP_vs_SHblk->Draw("colz text");
  c1->cd(5); //
  h2_EovP_vs_SHblk_calib->SetStats(0);
  h2_EovP_vs_SHblk_calib->Draw("colz text");
  c1->cd(6); //
  h2_EovP_vs_PSblk_calib->SetStats(0);
  h2_EovP_vs_PSblk_calib->Draw("colz text");
  c1->SaveAs(Form("%s[",outPlot.Data())); c1->SaveAs(Form("%s",outPlot.Data())); c1->Write();
  //**** -- ***//

  /**** Canvas 2 (Corr. with tr vars.) ****/
  TCanvas *c2 = new TCanvas("c2","tr X,Y,Th",1500,1200);
  c2->Divide(3,2);
  c2->cd(1); //
  gPad->SetGridy();
  h2_EovP_vs_trX->SetStats(0);
  h2_EovP_vs_trX->Draw("colz");
  c2->cd(2); //
  gPad->SetGridy();
  h2_EovP_vs_trY->SetStats(0);
  h2_EovP_vs_trY->Draw("colz");
  c2->cd(3); //
  gPad->SetGridy();
  h2_EovP_vs_trTh->SetStats(0);
  h2_EovP_vs_trTh->Draw("colz");
  c2->cd(4); //
  gPad->SetGridy();
  h2_EovP_vs_trX_calib->SetStats(0);
  h2_EovP_vs_trX_calib->Draw("colz");
  c2->cd(5); //
  gPad->SetGridy();
  h2_EovP_vs_trY_calib->SetStats(0);
  h2_EovP_vs_trY_calib->Draw("colz");
  c2->cd(6); //
  gPad->SetGridy();
  h2_EovP_vs_trTh_calib->SetStats(0);
  h2_EovP_vs_trTh_calib->Draw("colz");
  c2->SaveAs(Form("%s",outPlot.Data())); c2->Write();
  //**** -- ***//

  /**** Canvas 3 (Corr. with tr vars. contd.) ****/
  TCanvas *c3 = new TCanvas("c3","tr Ph,PS",1500,1200);
  c3->Divide(3,2);
  c3->cd(1); //
  gPad->SetGridy();
  h2_EovP_vs_trPh->SetStats(0);
  h2_EovP_vs_trPh->Draw("colz");
  c3->cd(2); //
  gPad->SetGridy();
  h2_PSeng_vs_trX->SetStats(0);
  h2_PSeng_vs_trX->Draw("colz");
  c3->cd(3); //
  gPad->SetGridy();
  h2_PSeng_vs_trY->SetStats(0);
  h2_PSeng_vs_trY->Draw("colz");
  c3->cd(4); //
  gPad->SetGridy();
  h2_EovP_vs_trPh_calib->SetStats(0);
  h2_EovP_vs_trPh_calib->Draw("colz");
  c3->cd(5); //
  gPad->SetGridy();
  h2_PSeng_vs_trX_calib->SetStats(0);
  h2_PSeng_vs_trX_calib->Draw("colz");
  c3->cd(6); //
  gPad->SetGridy();
  h2_PSeng_vs_trY_calib->SetStats(0);
  h2_PSeng_vs_trY_calib->Draw("colz");
  c3->SaveAs(Form("%s",outPlot.Data())); c3->Write();
  //**** -- ***//

  /**** Canvas 4 (position resolution) ****/
  TCanvas *c4 = new TCanvas("c4","pos. res.",1200,1000);
  c4->Divide(2,2);  gStyle->SetOptFit(1111);
  c4->cd(1); //
  TF1* fit_c41 = new TF1("fit_c41","gaus",-0.5,0.5);
  h_shX_diff->Fit(fit_c41,"QR");
  h_shX_diff->SetStats(1);
  c4->cd(2); //
  TF1* fit_c42 = new TF1("fit_c42","gaus",-0.5,0.5);
  h_shY_diff->Fit(fit_c42,"QR");
  h_shY_diff->SetStats(1);
  c4->cd(3); //
  TF1* fit_c43 = new TF1("fit_c43","gaus",-0.5,0.5);
  h_shX_diff_calib->Fit(fit_c43,"QR");
  h_shX_diff_calib->SetStats(1);
  c4->cd(4); //
  TF1* fit_c44 = new TF1("fit_c44","gaus",-0.5,0.5);
  h_shY_diff_calib->Fit(fit_c44,"QR");
  h_shY_diff_calib->SetStats(1);
  c4->SaveAs(Form("%s",outPlot.Data())); c4->Write();
  //**** -- ***//

  /**** Canvas 5 (E/p vs. run number) ****/
  TCanvas *c5 = new TCanvas("c5","E/p vs rnum",1200,1000);
  c5->Divide(1,2);
  // manipulating urnum vector
  std::size_t nrun = lrnum.size();
  if (nrun!=Nruns)
    std::cout << "*!*[WARNING] 'Nruns' value in run list doesn't match with total # runs analyzed!\n\n"; 
  c5->cd(1); //
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  h2_EovP_vs_rnum->SetStats(0);
  h2_EovP_vs_rnum->GetXaxis()->SetLabelSize(0.05);  
  h2_EovP_vs_rnum->GetXaxis()->SetNdivisions(Nruns);
  for (int i=0; i<nrun; i++) h2_EovP_vs_rnum->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
  if (nrun>15) h2_EovP_vs_rnum->LabelsOption("v", "X"); 
  h2_EovP_vs_rnum->Draw("colz");
  h2_EovP_vs_rnum_prof->Draw("same");
  c5->cd(2); //
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  h2_EovP_vs_rnum_calib->SetStats(0);
  h2_EovP_vs_rnum_calib->GetXaxis()->SetLabelSize(0.05);
  h2_EovP_vs_rnum_calib->GetXaxis()->SetNdivisions(Nruns);
  for (int i=0; i<nrun; i++) h2_EovP_vs_rnum_calib->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
  if (nrun>15) h2_EovP_vs_rnum_calib->LabelsOption("v", "X"); 
  h2_EovP_vs_rnum_calib->Draw("colz");
  h2_EovP_vs_rnum_calib_prof->Draw("same");
  c5->SaveAs(Form("%s",outPlot.Data())); c5->Write();
  //**** -- ***//

  /**** Canvas 6 (gain coefficients) ****/
  TCanvas *c6 = new TCanvas("c6","gain Coeff",1200,1000);
  c6->Divide(2,2);
  c6->cd(1); Double_t h_max;
  h_max = h_old_coeff_blk_SH->GetMaximum();
  h2_old_coeff_detView_SH->GetZaxis()->SetRangeUser(0.,h_max); h2_old_coeff_detView_SH->Draw("text col");
  c6->cd(2); //
  h_max = h_coeff_blk_SH->GetMaximum();
  h2_coeff_detView_SH->GetZaxis()->SetRangeUser(0.,h_max); h2_coeff_detView_SH->Draw("text col");
  c6->cd(3); //
  h_max = h_old_coeff_blk_PS->GetMaximum();
  h2_old_coeff_detView_PS->GetZaxis()->SetRangeUser(0.,h_max); h2_old_coeff_detView_PS->Draw("text col");
  c6->cd(4); //
  h_max = h_coeff_blk_PS->GetMaximum();
  h2_coeff_detView_PS->GetZaxis()->SetRangeUser(0.,h_max); h2_coeff_detView_PS->Draw("text col");
  c6->SaveAs(Form("%s",outPlot.Data())); c6->Write();
  //**** -- ***//

  if (elastic_cut) {
    /**** Canvas 7 (elastic cuts) ****/
    TCanvas *c7 = new TCanvas("c7","elastic cuts",1200,1000);
    c7->Divide(1,2);
    TPad* p7 = (TPad*)c7->GetPad(1); p7->Divide(2,1);
    p7->cd(1); //
    if (cut_on_PovPel) {
      h_PovPel->SetLineColor(1);
      h_PovPel->SetTitle("Blue: w/ p spot cut | Red: p/p_{elastic}(#theta) cut region");
      h_PovPel->Draw();
      h_PovPel_pspotcut->SetLineColor(4);
      h_PovPel_pspotcut->Draw("same");
      Double_t x1 = PovPel_mean-PovPel_sigma*PovPel_nsigma;
      Double_t x2 = PovPel_mean+PovPel_sigma*PovPel_nsigma;
      Double_t y1 = 0.;
      Double_t y2 = h_PovPel_pspotcut->GetMaximum();
      TLine L1;
      L1.SetLineColor(2); L1.SetLineWidth(2); L1.SetLineStyle(9);
      L1.DrawLine(x1,y1,x1,y2);
      TLine L2;
      L2.SetLineColor(2); L2.SetLineWidth(2); L2.SetLineStyle(9);
      L2.DrawLine(x2,y1,x2,y2);
    } else if (cut_on_W) {
      h_W->SetLineColor(1);
      h_W->SetTitle("Blue: w/ p spot cut | Red: W cut region");
      h_W->Draw();
      h_W_pspotcut->SetLineColor(4);
      h_W_pspotcut->Draw("same");
      Double_t x1 = W_mean-W_sigma*W_nsigma;
      Double_t x2 = W_mean+W_sigma*W_nsigma;
      Double_t y1 = 0.;
      Double_t y2 = h_W_pspotcut->GetMaximum();
      TLine L1;
      L1.SetLineColor(2); L1.SetLineWidth(2); L1.SetLineStyle(9);
      L1.DrawLine(x1,y1,x1,y2);
      TLine L2;
      L2.SetLineColor(2); L2.SetLineWidth(2); L2.SetLineStyle(9);
      L2.DrawLine(x2,y1,x2,y2);
    }
    p7->cd(2); //
    h2_dxdyHCAL->Draw("colz");
    TEllipse Ep; 
    Ep.SetFillStyle(0);
    Ep.SetLineColor(2);
    Ep.SetLineWidth(2);
    Ep.DrawEllipse(pspot_dyM,pspot_dxM,pspot_ndyS*pspot_dyS,pspot_ndxS*pspot_dxS,0,360,0);
    c7->cd(2); //
    gPad->SetGridy();
    gStyle->SetErrorX(0.0001);
    h2_PovPel_vs_rnum_pspotcut->SetStats(0);
    h2_PovPel_vs_rnum_pspotcut->GetXaxis()->SetLabelSize(0.05);  
    h2_PovPel_vs_rnum_pspotcut->GetXaxis()->SetNdivisions(Nruns);
    for (int i=0; i<nrun; i++) h2_PovPel_vs_rnum_pspotcut->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
    if (nrun>15) h2_PovPel_vs_rnum_pspotcut->LabelsOption("v", "X"); 
    h2_PovPel_vs_rnum_pspotcut->Draw("colz");
    //h2_PovPel_vs_rnum_pspotcut_prof->Draw("same"); #giving misleading values
    c7->SaveAs(Form("%s",outPlot.Data())); c7->Write();
    //**** -- ***//

    /**** Canvas 8 (cluster size) ****/
    TCanvas *c8 = new TCanvas("c8","cl. size vs rnum",1200,1000);
    c8->Divide(1,4);
    c8->cd(1); //
    for (int i=0; i<nrun; i++) h2_PSclsize_vs_rnum->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
    if (nrun>15) h2_PSclsize_vs_rnum->LabelsOption("v", "X"); 
    h2_PSclsize_vs_rnum->Draw("colz");
    h2_PSclsize_vs_rnum_prof->Draw("same");
    c8->cd(2); //
    for (int i=0; i<nrun; i++) h2_PSclmult_vs_rnum->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
    if (nrun>15) h2_PSclmult_vs_rnum->LabelsOption("v", "X"); 
    h2_PSclmult_vs_rnum->Draw("colz");
    h2_PSclmult_vs_rnum_prof->Draw("same");
    c8->cd(3); //
    for (int i=0; i<nrun; i++) h2_SHclsize_vs_rnum->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
    if (nrun>15) h2_SHclsize_vs_rnum->LabelsOption("v", "X"); 
    h2_SHclsize_vs_rnum->Draw("colz");
    h2_SHclsize_vs_rnum_prof->Draw("same");
    c8->cd(4); //
    for (int i=0; i<nrun; i++) h2_SHclmult_vs_rnum->GetXaxis()->SetBinLabel(i+1,lrnum[i].c_str());
    if (nrun>15) h2_SHclmult_vs_rnum->LabelsOption("v", "X"); 
    h2_SHclmult_vs_rnum->Draw("colz");
    h2_SHclmult_vs_rnum_prof->Draw("same");
    c8->SaveAs(Form("%s",outPlot.Data())); c8->Write();
  }
  //**** -- ***//

  /**** Summary Canvas ****/
  TCanvas *cSummary = new TCanvas("cSummary","Summary");
  cSummary->cd();
  TPaveText *pt = new TPaveText(.05,.1,.95,.8);
  pt->AddText(Form(" Date of creation: %s",getDate().c_str()));
  pt->AddText(Form("Configfile: BBCal_replay/macros/Combined_macros/cfg/%s.cfg",cfgfilebase.Data()));
  pt->AddText(Form(" Total # events analyzed: %lld, Preparing for replay pass: %d",Nevents,ppass));
  pt->AddText(Form(" E/p (before calib.) | #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param_bc[1],param_bc[2]*100,sigerr_bc*100));
  pt->AddText(Form(" E/p (after calib.)    | #mu = %.2f, #sigma = (%.3f #pm %.3f) p",param[1],param[2]*100,sigerr*100));
  pt->AddText(" Global cuts: ");
  std::string tmpstr = "";
  for (std::size_t i=0; i<gCutList.size(); i++) {
    if (i>0 && i%3==0) {pt->AddText(Form(" %s",tmpstr.c_str())); tmpstr="";}
    tmpstr += gCutList[i] + ", "; 
  }
  if (!tmpstr.empty()) pt->AddText(Form(" %s",tmpstr.c_str()));
  if (cut_on_psE) pt->AddText(Form(" PS cluster energy > %.1f GeV",psE_cut_limit));
  if (cut_on_clusE) pt->AddText(Form(" BBCAL cluster energy < %.1f GeV",clusE_cut_limit));
  if (cut_on_EovP) pt->AddText(Form(" |E/p - 1| < %.1f",EovP_cut_limit));
  if (cut_on_pmin && cut_on_pmax) pt->AddText(Form(" %.1f < p_recon < %.1f GeV/c",p_min_cut,p_max_cut));
  else if (cut_on_pmin) pt->AddText(Form(" p_recon > %.1f GeV/c",p_min_cut));
  else if (cut_on_pmax) pt->AddText(Form(" p_recon < %.1f GeV/c",p_max_cut));
  pt->AddText(Form(" # events passed global cuts: %lld", Ngoodevs));
  if (elastic_cut) {
    pt->AddText(" Elastic cuts: ");
    if (cut_on_W) pt->AddText(Form(" |W - %.3f| #leq %.1f*%.3f",W_mean,W_nsigma,W_sigma));
    if (cut_on_PovPel) pt->AddText(Form(" |p/p_{el}(#theta) - %.3f| #leq %.1f*%.3f",PovPel_mean,PovPel_nsigma,PovPel_sigma));
    if (cut_on_pspot) pt->AddText(" proton spot cut ranges: ");
    if (cut_on_pspot) pt->AddText(Form("  #Deltax (m): Mean = %.4f, %.1f#sigma = %.4f",pspot_dxM,pspot_ndxS,pspot_dxS));
    if (cut_on_pspot) pt->AddText(Form("  #Deltay (m): Mean = %.4f, %.1f#sigma = %.4f",pspot_dyM,pspot_ndyS,pspot_dyS));
    pt->AddText(Form(" # events passed global & elastic cuts: %lld", Nelasevs));
    TText *tel = pt->GetLineWith(" Elastic"); tel->SetTextColor(kBlue);
  }
  pt->AddText(" Other cuts: ");
  pt->AddText(Form(" Minimum # events per block: %d, (Cluster) hit threshold: %.2f GeV",Nmin,hit_threshold));
  pt->AddText(" Various offsets: ");
  pt->AddText(Form(" Momentum fudge factor: %.2f, BBCAL cluster energy scale factor: %.2f",p_rec_Offset,cF));
  if (mom_calib) pt->AddText(Form(" Momentum calibration factors: A = %.9f, B = %.9f, C = %.1f, #theta^{GEM}_{pitch} = %.1f^{o}, d_{BB} = %.4f m",A_fit,B_fit,C_fit,GEMpitch,bb_magdist));
  sw->Stop(); sw2->Stop();
  pt->AddText(Form("Macro processing time: CPU %.1fs | Real %.1fs",sw->CpuTime(),sw->RealTime()));
  TText *t1 = pt->GetLineWith("Configfile"); t1->SetTextColor(kRed+2);
  TText *t2 = pt->GetLineWith(" E/p (be"); t2->SetTextColor(kRed);
  TText *t3 = pt->GetLineWith(" E/p (af"); t3->SetTextColor(kGreen+2);
  TText *t4 = pt->GetLineWith(" Global"); t4->SetTextColor(kBlue);
  TText *t5 = pt->GetLineWith(" Other"); t5->SetTextColor(kBlue);
  TText *t6 = pt->GetLineWith(" Various"); t6->SetTextColor(kBlue);
  TText *t7 = pt->GetLineWith("Macro"); t7->SetTextColor(kGreen+3);
  pt->Draw();
  cSummary->SaveAs(Form("%s",outPlot.Data())); cSummary->SaveAs(Form("%s]",outPlot.Data())); cSummary->Write();  
  //**** -- ***//

  std::cout << "List of output files:" << "\n";
  std::cout << " --------- " << "\n";
  std::cout << " 1. Summary plots : "        << outPlot << "\n";
  std::cout << " 2. Resulting histograms : " << outFile << "\n";
  std::cout << " 3. Gain ratios (new/old) for SH : " << gainRatio_SH << "\n";
  std::cout << " 4. Gain ratios (new/old) for PS : " << gainRatio_PS << "\n";
  std::cout << " 5. New ADC gain coeffs. (GeV/pC) for SH : " << adcGain_SH << "\n";
  std::cout << " 6. New ADC gain coeffs. (GeV/pC) for PS : " << adcGain_PS << "\n";
  std::cout << " --------- " << "\n";

  std::cout << "CPU time = " << sw->CpuTime() << "s. Real time = " << sw->RealTime() << "s.\n\n";

  ///////////////////////////////////////////////////
  // Write individual memories to file explicitely //
  // to be able to read them using uproot          //
  ///////////////////////////////////////////////////
  Tout->Write("", TObject::kOverwrite);
  // kinematic
  h_Q2->Write();
  h_W->Write(); h_W_pspotcut->Write();
  h_PovPel->Write(); h_PovPel_pspotcut->Write();
  h2_PovPel_vs_rnum_pspotcut->Write();
  //h2_PovPel_vs_rnum_pspotcut_prof->Write();
  h2_p_rec_vs_etheta->Write();
  // main
  h_EovP->Write(); h_EovP_calib->Write();
  h_clusE->Write(); h_clusE_calib->Write();
  h_SHclusE->Write(); h_SHclusE_calib->Write();
  h_PSclusE->Write(); h_PSclusE_calib->Write();
  h2_SHeng_vs_SHblk->Write(); h2_PSeng_vs_PSblk->Write();
  // various corrs. with E/p
  h2_EovP_vs_SHblk_trPOS->Write(); h2_EovP_vs_PSblk_trPOS->Write();
  h2_EovP_vs_P->Write(); h2_EovP_vs_P_calib->Write();
  h2_EovP_vs_P_prof->Write(); h2_EovP_vs_P_calib_prof->Write();
  h2_EovP_vs_rnum->Write(); h2_EovP_vs_rnum_calib->Write();
  h2_EovP_vs_rnum_prof->Write(); h2_EovP_vs_rnum_calib_prof->Write();
  h2_EovP_vs_SHblk->Write(); h2_EovP_vs_SHblk_calib->Write();
  h2_EovP_vs_PSblk->Write(); h2_EovP_vs_PSblk_calib->Write();
  h2_EovP_vs_trX->Write(); h2_EovP_vs_trX_calib->Write();
  h2_EovP_vs_trY->Write(); h2_EovP_vs_trY_calib->Write();
  h2_EovP_vs_trTh->Write(); h2_EovP_vs_trTh_calib->Write();
  h2_EovP_vs_trPh->Write(); h2_EovP_vs_trPh_calib->Write();
  h2_PSeng_vs_trX->Write(); h2_PSeng_vs_trX_calib->Write();
  h2_PSeng_vs_trY->Write(); h2_PSeng_vs_trY_calib->Write();
  // rate dependence
  h2_PSclsize_vs_rnum->Write(); h2_PSclsize_vs_rnum_prof->Write();
  h2_PSclmult_vs_rnum->Write(); h2_PSclmult_vs_rnum_prof->Write();
  h2_SHclsize_vs_rnum->Write(); h2_SHclsize_vs_rnum_prof->Write();
  h2_SHclmult_vs_rnum->Write(); h2_SHclmult_vs_rnum_prof->Write();
  // momentum calib. proxy
  if (mom_calib) h_thetabend->Write();
  // gain coefficients
  h_nevent_blk_SH->Write();
  h_coeff_Ratio_SH->Write();
  h_coeff_blk_SH->Write();
  h_old_coeff_blk_SH->Write();
  h2_old_coeff_detView_SH->Write();
  h2_coeff_detView_SH->Write();
  h_nevent_blk_PS->Write();
  h_coeff_Ratio_PS->Write();
  h_coeff_blk_PS->Write();
  h_old_coeff_blk_PS->Write();
  h2_old_coeff_detView_PS->Write();
  h2_coeff_detView_PS->Write();
  
  /////////////////////////////////////
  // Clear memories & free resources //
  /////////////////////////////////////
  C->Delete();
  CoeffR.Clear();
  M.Clear(); B.Clear();
  adcGainSH_outData.close();
  adcGainPS_outData.close();
  gainRatioSH_outData.close();
  gainRatioPS_outData.close();
  sw->Delete(); sw2->Delete();
}

// **** ========== Useful functions ========== ****  
// returns today's date
std::string getDate(){
  time_t now = time(0);
  tm ltm = *localtime(&now);
  std::string yyyy = to_string(1900 + ltm.tm_year);
  std::string mm = to_string(1 + ltm.tm_mon);
  std::string dd = to_string(ltm.tm_mday);
  std::string date = mm + '/' + dd + '/' + yyyy;
  return date;
}

// reads old ADC gain coefficients from TXT files
void ReadGain(TString adcGain_rfile, Double_t* adcGain){
  ifstream adcGain_data;
  adcGain_data.open(adcGain_rfile);
  std::string readline;
  Int_t elemID=0;
  if(adcGain_data.is_open()){
    std::cout << " Reading ADC gain from : "<< adcGain_rfile << "\n";
    while(getline(adcGain_data,readline)){
      istringstream tokenStream(readline);
      std::string token;
      char delimiter = ' ';
      while(getline(tokenStream,token,delimiter)){
  	TString temptoken=token;
  	adcGain[elemID] = temptoken.Atof();
  	elemID++;
      }
    }
  }else{
    std::cerr << " **!** No file : " << adcGain_rfile << "\n\n";
    std::exit(1);
  }
  adcGain_data.close();
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

// customizes profile histograms
void CustmProfHisto(TH1D* hprof) {
  hprof->SetStats(0);
  hprof->SetMarkerStyle(20);
  hprof->SetMarkerColor(2);
}

/*
  List of input and output files:
  *Input files: 
  1. Gain/<configFileBase>_gainCoeff_sh(ps).txt # Old gain coeff. for SH(PS) [Needed if, "read_gain" = 1]
  *Output files:
  1. plots/<configFileBase>_bbcal_eng_calib.pdf # Contains all the canvases
  2. hist/<configFileBase>_bbcal_eng_calib.root # Contains all the interesting histograms
  3. Gain/<configFileBase>_gainRatio_sh(ps)_calib.txt # Contains gain ratios (new/old) for SH(PS)
  4. Gain/<configFileBase>_gainCoeff_sh(ps)_calib.txt # Contains new gain coeff. for SH(PS)
*/
