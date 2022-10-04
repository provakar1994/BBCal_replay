/*
  This script generates diagnostic plots for quality assurance of BBCAL calibration.
  -----
  P. Datta <pdbforce@jlab.org> Created 04-21-2022
  -----
*/
#include <iostream>
#include <sstream>
#include <fstream>
#include "TChain.h"
#include "TFile.h"
#include "TCut.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TString.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TProfile.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TStopwatch.h"

const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows
const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows

const Double_t zposSH = 1.901952; //m
const Double_t zposPS = 1.695704; //m

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
  TEventList *elist = new TEventList("elist","Event list for BBCAL energy calibration");
  C->Draw(">>elist",globalcut);  

  // Setting useful ROOT branch status
  Int_t MAXNTRACKS=1000;
  Double_t prec[MAXNTRACKS];
  Double_t xTr[MAXNTRACKS], yTr[MAXNTRACKS];
  Double_t thTr[MAXNTRACKS], phTr[MAXNTRACKS];
  Double_t tmeanHodo[MAXNTRACKS];
  Double_t rowblkPS, colblkPS, xPS, yPS, EPS;
  Double_t atimeblkSH, rowblkSH, colblkSH, xSH, ySH, ESH;
  C->SetBranchStatus("*",0);
  // BigBite track variable
  C->SetBranchStatus("bb.tr.p",1);
  C->SetBranchAddress("bb.tr.p",prec);
  C->SetBranchStatus("bb.tr.x",1);
  C->SetBranchAddress("bb.tr.x",xTr);
  C->SetBranchStatus("bb.tr.y",1);
  C->SetBranchAddress("bb.tr.y",yTr);
  C->SetBranchStatus("bb.tr.th",1);
  C->SetBranchAddress("bb.tr.th",thTr);
  C->SetBranchStatus("bb.tr.ph",1);
  C->SetBranchAddress("bb.tr.ph",phTr);
  // PreShower variable
  C->SetBranchStatus("bb.ps.rowblk",1);
  C->SetBranchAddress("bb.ps.rowblk",&rowblkPS);
  C->SetBranchStatus("bb.ps.colblk",1);
  C->SetBranchAddress("bb.ps.colblk",&colblkPS);
  C->SetBranchStatus("bb.ps.e",1);
  C->SetBranchAddress("bb.ps.e",&EPS);
  C->SetBranchStatus("bb.ps.x",1);
  C->SetBranchAddress("bb.ps.x",&xPS);
  C->SetBranchStatus("bb.ps.y",1);
  C->SetBranchAddress("bb.ps.y",&yPS);
  // Shower variable
  C->SetBranchStatus("bb.sh.atimeblk",1);
  C->SetBranchAddress("bb.sh.atimeblk",&atimeblkSH);
  C->SetBranchStatus("bb.sh.rowblk",1);
  C->SetBranchAddress("bb.sh.rowblk",&rowblkSH);
  C->SetBranchStatus("bb.sh.colblk",1);
  C->SetBranchAddress("bb.sh.colblk",&colblkSH);
  C->SetBranchStatus("bb.sh.e",1);
  C->SetBranchAddress("bb.sh.e",&ESH);
  C->SetBranchStatus("bb.sh.x",1);
  C->SetBranchAddress("bb.sh.x",&xSH);
  C->SetBranchStatus("bb.sh.y",1);
  C->SetBranchAddress("bb.sh.y",&ySH);
  // BBhodo variable
  C->SetBranchStatus("bb.hodotdc.clus.tmean",1);
  C->SetBranchAddress("bb.hodotdc.clus.tmean",&tmeanHodo);

  // Defining temporary histograms (don't wanna write them to files)
  TH2D *h2_SHeng_vs_SHblk_raw = new TH2D("h2_SHeng_vs_SHblk_raw","Raw E_clus(SH) per SH block",
					kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_raw = new TH2D("h2_EovP_vs_SHblk_raw","Raw E_clus/p_rec per SH block",
				      kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_count = new TH2D("h2_count","Count for E_clus/p_rec per per SH block",
			    kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS_raw = new TH2D("h2_EovP_vs_SHblk_trPOS_raw",
					      "Raw E_clus/p_rec per SH block(TrPos)",
					      kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_count_trP = new TH2D("h2_count_trP","Count for E_clus/p_rec per per SH block(TrPos)",
				kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_count_trP_PS = new TH2D("h2_count_trP_PS","Count for E_clus(PS) per per PS block(TrPOS)",
				   kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  TH2D *h2_count_PS = new TH2D("h2_count_PS","Count for E_clus/p_rec per per PS block",
			       kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_PSeng_vs_PSblk_raw = new TH2D("h2_PSeng_vs_PSblk_raw","Raw E_clus(PS) per PS block",
					kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_PSeng_vs_PSblk_trPOS_raw = new TH2D("h2_PSeng_vs_PSblk_trPOS_raw","Raw E_clus(PS) per PS block(TrPos)",kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);
  
  // Creating output ROOT file to contain histograms
  TString outFile = "hist/" + outFileBase;
  TFile *fout = new TFile(outFile, "RECREATE");
  fout->cd();

  // Defining physics histograms
  TH1D *h_EovP = new TH1D("h_EovP","E/p",h_EovP_bin,h_EovP_min,h_EovP_max);
  TH2D *h2_EovP_vs_P = new TH2D("h2_EovP_vs_P","E/p vs p; p (GeV); E/p",
				h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,
				h_EovP_bin,h_EovP_min,h_EovP_max);
  TProfile *h2_EovP_vs_P_prof = new TProfile("h2_EovP_vs_P_prof","E/p vs P (Profile)",
					     h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,
					     h_EovP_min,h_EovP_max);

  TH2D *h2_EovP_vs_SHblk = new TH2D("h2_EovP_vs_SHblk","E/p per SH block",
				  kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS = new TH2D("h2_EovP_vs_SHblk_trPOS","E/p per SH "
					"block u Track Pos.",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_SHeng_vs_SHblk = new TH2D("h2_SHeng_vs_SHblk","SH energy per SH block",
				     kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);

  TH2D *h2_PSeng_vs_PSblk = new TH2D("h2_PSeng_vs_PSblk","PS energy per PS block",
				    kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_PSeng_vs_PSblk_trPOS = new TH2D("h2_PSeng_vs_PSblk_trPOS","PS energy per PS block u Track Pos.",
					   kNcolsPS,-0.3705,0.3705,kNrowsPS,-1.201,1.151);

  TH2D *h2_ADCtime_diff_wrt_Hodo = new TH2D("h2_ADCtime_diff_wrt_Hodo","ADCTime diff. w.r.t. Hodo tmean (ns) vs. SH block",189,0,189,200,-50,50);
  TProfile *h2_ADCtime_diff_wrt_Hodo_prof = new TProfile("h2_ADCtime_diff_wrt_Hodo_prof","ADCTime diff. w.r.t. Hodo tmean (ns) vs. SH block (Profile)",189,0,189,-50,50);

  //histograms to check bias in tracking
  TH2D *h2_EovP_vs_trX = new TH2D("h2_EovP_vs_trX","E/p vs Track x",200,-0.8,0.8,200,0,2);
  TH2D *h2_EovP_vs_trY = new TH2D("h2_EovP_vs_trY","E/p vs Track y",200,-0.16,0.16,200,0,2);
  TH2D *h2_EovP_vs_trTh = new TH2D("h2_EovP_vs_trTh","E/p vs Track theta",200,-0.2,0.2,200,0,2);
  TH2D *h2_EovP_vs_trPh = new TH2D("h2_EovP_vs_trPh","E/p vs Track phi",200,-0.08,0.08,200,0,2);
  TH2D *h2_PSeng_vs_trX = new TH2D("h2_PSeng_vs_trX","PS energy vs Track x",200,-0.8,0.8,200,0,4);
  TH2D *h2_PSeng_vs_trY = new TH2D("h2_PSeng_vs_trY","PS energy vs Track y",200,-0.16,0.16,200,0,4);

  // Looping over good events ================================================================= //
  Long64_t Nevents = elist->GetN(), nevent=0;  
  cout << endl << "Processing " << Nevents << " events.." << endl;

  while( C->GetEntry( elist->GetEntry(nevent++) ) ){

    // progress indicator
    if( nevent % 100 == 0 ) cout << nevent << "/" << Nevents << "\r";
    cout.flush();

    Double_t clusEngBBCal = EPS + ESH;
    // E/p
    h_EovP->Fill( (clusEngBBCal/prec[0]) );

    // E/p vs. p
    h2_EovP_vs_P->Fill( prec[0], clusEngBBCal/prec[0] );
    h2_EovP_vs_P_prof->Fill( prec[0], clusEngBBCal/prec[0], 1. );

    // Shower block related histos
    h2_SHeng_vs_SHblk_raw->Fill( colblkSH, rowblkSH, ESH );
    h2_EovP_vs_SHblk_raw->Fill( colblkSH, rowblkSH, (clusEngBBCal/prec[0]) );
    h2_count->Fill( colblkSH, rowblkSH, 1.);
    h2_SHeng_vs_SHblk->Divide( h2_SHeng_vs_SHblk_raw, h2_count );
    h2_SHeng_vs_SHblk->GetZaxis()->SetRangeUser( h2_SHeng_vs_blk_low, h2_SHeng_vs_blk_up );
    h2_EovP_vs_SHblk->Divide( h2_EovP_vs_SHblk_raw, h2_count );

    Double_t xtrATsh = xTr[0] + zposSH*thTr[0];
    Double_t ytrATsh = yTr[0] + zposSH*phTr[0];
    h2_EovP_vs_SHblk_trPOS_raw->Fill( ytrATsh, xtrATsh, (clusEngBBCal/prec[0]) );
    h2_count_trP->Fill( ytrATsh, xtrATsh, 1. );
    h2_EovP_vs_SHblk_trPOS->Divide( h2_EovP_vs_SHblk_trPOS_raw, h2_count_trP );
    h2_EovP_vs_SHblk->GetZaxis()->SetRangeUser(0.8,1.2);
    h2_EovP_vs_SHblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);

    // PreShower block related histos
    Double_t xtrATps = xTr[0] + zposPS*thTr[0];
    Double_t ytrATps = yTr[0] + zposPS*phTr[0];
    h2_PSeng_vs_PSblk_raw->Fill( colblkPS, rowblkPS, EPS );
    h2_count_PS->Fill( colblkPS, rowblkPS, 1.);
    h2_PSeng_vs_PSblk->Divide( h2_PSeng_vs_PSblk_raw, h2_count_PS );
    h2_PSeng_vs_PSblk->GetZaxis()->SetRangeUser( h2_PSeng_vs_blk_low, h2_PSeng_vs_blk_up );
    h2_PSeng_vs_PSblk_trPOS_raw->Fill( ytrATps, xtrATps, EPS );
    h2_count_trP_PS->Fill( ytrATps, xtrATps, 1. );
    h2_PSeng_vs_PSblk_trPOS->Divide( h2_PSeng_vs_PSblk_trPOS_raw, h2_count_trP_PS );
    h2_PSeng_vs_PSblk_trPOS->GetZaxis()->SetRangeUser( h2_PSeng_vs_blk_low, h2_PSeng_vs_blk_up );

    // ADCTime difference w.r.t. hodo tmean
    Double_t elemSH = rowblkSH*kNcolsSH + colblkSH;
    h2_ADCtime_diff_wrt_Hodo->Fill( elemSH, (tmeanHodo[0]-atimeblkSH) );
    h2_ADCtime_diff_wrt_Hodo_prof->Fill( elemSH, (tmeanHodo[0]-atimeblkSH), 1. );

    // Track related histos
    h2_EovP_vs_trX->Fill( xTr[0], (clusEngBBCal/prec[0]) );
    h2_EovP_vs_trY->Fill( yTr[0], (clusEngBBCal/prec[0]) );
    h2_EovP_vs_trTh->Fill( thTr[0], (clusEngBBCal/prec[0]) );
    h2_EovP_vs_trPh->Fill( phTr[0], (clusEngBBCal/prec[0]) );
    h2_PSeng_vs_trX->Fill( xTr[0], EPS );
    h2_PSeng_vs_trY->Fill( yTr[0], EPS );

  } //event loop
  cout << endl << endl;

  // creating a canvas to show all the interesting plots
  TCanvas *c1 = new TCanvas("c1","BBCAL QA plots: c1",1500,1200);
  c1->Divide(3,2);

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

  c1->cd(2);
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  h2_EovP_vs_P->SetStats(0);
  h2_EovP_vs_P->Draw("colz");
  h2_EovP_vs_P_prof->SetStats(0);
  h2_EovP_vs_P_prof->SetMarkerStyle(20);
  h2_EovP_vs_P_prof->SetMarkerColor(1);
  h2_EovP_vs_P_prof->Draw("same");

  c1->cd(3);
  gPad->SetGridy();
  gStyle->SetErrorX(0.0001);
  // h2_PSeng_vs_PSblk->SetStats(0);
  // h2_PSeng_vs_PSblk->Draw("colz");
  h2_ADCtime_diff_wrt_Hodo->SetStats(0);
  h2_ADCtime_diff_wrt_Hodo->Draw("colz");
  h2_ADCtime_diff_wrt_Hodo_prof->SetMarkerStyle(7);
  h2_ADCtime_diff_wrt_Hodo_prof->SetMarkerColor(2);
  h2_ADCtime_diff_wrt_Hodo_prof->SetStats(0);
  h2_ADCtime_diff_wrt_Hodo_prof->Draw("same");

  c1->cd(4);
  h2_SHeng_vs_SHblk->SetStats(0);
  h2_SHeng_vs_SHblk->Draw("colz");

  c1->cd(5);
  // h2_EovP_vs_SHblk->SetStats(0);
  // h2_EovP_vs_SHblk->Draw("colz");
  h2_PSeng_vs_PSblk_trPOS->SetStats(0);
  h2_PSeng_vs_PSblk_trPOS->Draw("colz");

  c1->cd(6);
  h2_EovP_vs_SHblk_trPOS->SetStats(0);
  h2_EovP_vs_SHblk_trPOS->Draw("colz");

  // creating another canvas to show all the interesting plots
  TCanvas *c2 = new TCanvas("c2","BBCAL QA plots: c2",1500,1200);
  c2->Divide(3,2);

  c2->cd(1);
  gPad->SetGridy();
  h2_EovP_vs_trX->SetStats(0);
  h2_EovP_vs_trX->Draw("colz");

  c2->cd(2);
  gPad->SetGridy();
  h2_EovP_vs_trY->SetStats(0);
  h2_EovP_vs_trY->Draw("colz");

  c2->cd(3);
  gPad->SetGridy();
  h2_EovP_vs_trTh->SetStats(0);
  h2_EovP_vs_trTh->Draw("colz");

  c2->cd(4);
  gPad->SetGridy();
  h2_EovP_vs_trPh->SetStats(0);
  h2_EovP_vs_trPh->Draw("colz");

  c2->cd(5);
  gPad->SetGridy();
  h2_PSeng_vs_trX->SetStats(0);
  h2_PSeng_vs_trX->Draw("colz");

  c2->cd(6);
  gPad->SetGridy();
  h2_PSeng_vs_trY->SetStats(0);
  h2_PSeng_vs_trY->Draw("colz");

  // printing out the canvas
  TString plotsFile = "plots/" + outFileBase.ReplaceAll(".root",".pdf");
  c1->Print(Form("%s[",plotsFile.Data()),"pdf");
  c1->Print(Form("%s",plotsFile.Data()),"pdf");
  c2->Print(Form("%s",plotsFile.Data()),"pdf");
  c2->Print(Form("%s]",plotsFile.Data()),"pdf");
  
  cout << "Finishing analysis..." << endl;
  cout << " --------- " << endl;
  cout << " Resulting histograms written to : " << outFile << endl;
  cout << " Generated plots saved to : " << plotsFile.Data() << endl;
  cout << " --------- " << endl;

  sw->Stop();
  cout << "CPU time elapsed = " << sw->CpuTime() << "s. Real time = " 
       << sw->RealTime() << "s. " << endl << endl;

  fout->Write(); //fout->Close();
  sw->Delete(); C->Delete();
  elist->Delete(); //fout->Delete();
}
