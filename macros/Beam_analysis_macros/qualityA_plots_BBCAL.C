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
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TStopwatch.h"

const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows
const Int_t kNcolsPS = 2;  // PS columns
const Int_t kNrowsPS = 26; // PS rows

const Double_t zposSH = 1.901952; //m
const Double_t zposPS = 1.695704; //m

void qualityA_plots_BBCAL(const char *outFile="qualityA_plots_BBCAL.root",
			  const char *configFile="setup_qualityA_plots_BBCAL.cfg")
{
  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings
  
  // creating a TChain
  TChain *C = new TChain("T");

  // Defining variables
  Int_t SBSconfig=4;
  Double_t h_EovP_bin=200, h_EovP_min=0., h_EovP_max=5.;
  Double_t h2_p_coarse_bin=25, h2_p_coarse_min=0., h2_p_coarse_max=5.;

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
  Double_t rowblkPS, colblkPS, xPS, yPS, EPS;
  Double_t rowblkSH, colblkSH, xSH, ySH, ESH;
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

  TH2D *h2_count_PS = new TH2D("h2_count_PS","Count for E_clus/p_rec per per PS block",
			       kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  TH2D *h2_PSeng_vs_PSblk_raw = new TH2D("h2_PSeng_vs_PSblk_raw","Raw E_clus(PS) per PS block",
					kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);
  
  // Creating output ROOT file to contain histograms
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  // Defining physics histograms
  TH1D *h_EovP = new TH1D("h_EovP","E/p",h_EovP_bin,h_EovP_min,h_EovP_max);
  TH2D *h2_EovP_vs_P = new TH2D("h2_EovP_vs_P","E/p vs p; p (GeV); E/p",
				h2_p_coarse_bin,h2_p_coarse_min,h2_p_coarse_max,
				h_EovP_bin,h_EovP_min,h_EovP_max);

  TH2D *h2_EovP_vs_SHblk = new TH2D("h2_EovP_vs_SHblk","E/p per SH block",
				  kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);
  TH2D *h2_EovP_vs_SHblk_trPOS = new TH2D("h2_EovP_vs_SHblk_trPOS","E/p per SH "
					"block u Track Pos.",kNcolsSH,-0.2992,0.2992,kNrowsSH,-1.1542,1.1542);
  TH2D *h2_SHeng_vs_SHblk = new TH2D("h2_SHeng_vs_SHblk","SH energy per SH block",
				     kNcolsSH,0,kNcolsSH,kNrowsSH,0,kNrowsSH);

  TH2D *h2_PSeng_vs_PSblk = new TH2D("h2_PSeng_vs_PSblk","PS energy per PS block",
				    kNcolsPS,0,kNcolsPS,kNrowsPS,0,kNrowsPS);

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

    // Shower block related histos
    h2_SHeng_vs_SHblk_raw->Fill( colblkSH, rowblkSH, ESH );
    h2_EovP_vs_SHblk_raw->Fill( colblkSH, rowblkSH, (clusEngBBCal/prec[0]) );
    h2_count->Fill( colblkSH, rowblkSH, 1.);
    h2_SHeng_vs_SHblk->Divide( h2_SHeng_vs_SHblk_raw, h2_count );
    h2_EovP_vs_SHblk->Divide( h2_EovP_vs_SHblk_raw, h2_count );

    Double_t xtrATsh = xTr[0] + zposSH*thTr[0];
    Double_t ytrATsh = yTr[0] + zposSH*phTr[0];
    h2_EovP_vs_SHblk_trPOS_raw->Fill( ytrATsh, xtrATsh, (clusEngBBCal/prec[0]) );
    h2_count_trP->Fill( ytrATsh, xtrATsh, 1. );
    h2_EovP_vs_SHblk_trPOS->Divide( h2_EovP_vs_SHblk_trPOS_raw, h2_count_trP );
    h2_EovP_vs_SHblk->GetZaxis()->SetRangeUser(0.8,1.2);
    h2_EovP_vs_SHblk_trPOS->GetZaxis()->SetRangeUser(0.8,1.2);

    // PreShower block related histos
    h2_PSeng_vs_PSblk_raw->Fill( colblkPS, rowblkPS, EPS );
    h2_count_PS->Fill( colblkPS, rowblkPS, 1.);
    h2_PSeng_vs_PSblk->Divide( h2_PSeng_vs_PSblk_raw, h2_count_PS );

  } //event loop
  cout << endl << endl;

  // creating a canvas to show all the interesting plots
  TCanvas *c1 = new TCanvas("c1","BBCAL QA plots",1500,1200);
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
  h2_EovP_vs_P->SetStats(0);
  h2_EovP_vs_P->Draw("colz");

  c1->cd(3);
  h2_PSeng_vs_PSblk->SetStats(0);
  h2_PSeng_vs_PSblk->Draw("colz");

  c1->cd(4);
  h2_SHeng_vs_SHblk->SetStats(0);
  h2_SHeng_vs_SHblk->Draw("colz");

  c1->cd(5);
  h2_EovP_vs_SHblk->SetStats(0);
  h2_EovP_vs_SHblk->Draw("colz");

  c1->cd(6);
  h2_EovP_vs_SHblk_trPOS->SetStats(0);
  h2_EovP_vs_SHblk_trPOS->Draw("colz");

  // printing out the canvas
  TString plotsFile = outFile;
  plotsFile.ReplaceAll(".root",".pdf");
  c1->Print(plotsFile.Data(),"pdf");
  
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
