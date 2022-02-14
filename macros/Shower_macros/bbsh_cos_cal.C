/*
  This macro performs the analysis of cosmic data for BigBite Shower.
  For proper execution follow : How-to/Cosmic_data_analysis.pdf
  ----
  P. Datta  <pdbforce@jlab.org>  Created  15 Feb 2021
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <TH2F.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TStopwatch.h>
#include <TGraph.h>
#include <TError.h>
#include "fadc_data.h"
using namespace std;

// Detector geometry
const int kNrows = 27;
const int kNcols = 7;

const double TargetADC = 10.; //mV (Not using it at the moment 11-08-21)

TChain *T = 0;
int gCurrentEntry = -1;

// Declare necessary histograms
TH1F *hADCamp[kNrows][kNcols];
TH1F *hamptointratio[kNrows][kNcols];

// Declare necessary arrayes 
bool gPulse[kNrows+2][kNcols+2];
double Pars[3]={0.};
double ParErrs[3]={0.};
double trigTofadc_ratiosSH[kNrows*kNcols]={0.};

// Declare necessary functions
string getDate();
TH1F* MakeHisto( Int_t, Int_t, Int_t, const char*, Double_t, Double_t );
void processEvent( int, bool );
void goodHistoTest( double, int, int );
void makeSummaryPlots( string, string, bool );
void GetTrigtoFADCratio();

// Declare vectors necessary to make diagnostic plots
double blocks[kNrows*kNcols]={0.}, peakPos[kNrows*kNcols]={0.},
  peakPosErr[kNrows*kNcols]={0.};
double RMS[kNrows*kNcols]={0.}, RMSErr[kNrows*kNcols]={0.}; 
double NinPeak[kNrows*kNcols]={0.}, HVCrrFact[kNrows*kNcols]={0.};
vector<int> runList, eventsReplayed;
vector<double> trigTofadcRatio;
TString OutF_peaks, OutF_diagPlots;

TCanvas *subCanv[4];
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
void bbsh_cos_cal ( int nrun=366, int event=-1, bool userInput=1 ){

  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings 

  shgui::SetupGUI();
  gStyle->SetLabelSize(0.08,"XY");
  gStyle->SetTitleFontSize(0.08);

  // Initializing the parameters [Be very careful before changing]
  int nRuns = 1;
  string runnumber;
  string date = getDate();
  bool trigAmp = 1, multiRuns = 0;
  bool diagPlots = 1, histo_limits = 0;
  int hADCamp_bin = 45, hamptointratio_bin = 18;
  double hADCamp_min = 0., hamptointratio_min = 2.; 
  double hADCamp_max = 45., hamptointratio_max = 5.;

  // Take user inputs
  if(userInput){
    cout << " Multiple Runs? [0=NO, 1=YES] " << endl;
    cin >> multiRuns;
    if(multiRuns){
      cout << " How many runs? " << endl;
      cin >> nRuns;
      int temp1=0, temp2=0;
      for(int nr=0; nr<nRuns; nr++){
	cout << " Run " << nr+1 << "?" << " Events replayed? " << endl;
	cin >> temp1 >> temp2;
	runList.push_back(temp1);
	eventsReplayed.push_back(temp2);
      }
      if(nRuns==2){
	runnumber = to_string(runList[0]) + "&" + to_string(runList[nRuns-1]);
      }else runnumber = to_string(runList[0]) + "->" + 
	      to_string(runList[nRuns-1]);
    }else{
      cout << " Run number? " << endl;
      cin >> nrun;
      cout << " No. of events replayed? [-1 => All] " << endl; 
      cin >> event;
      runList.push_back(nrun);
      eventsReplayed.push_back(event);
      runnumber = to_string(nrun);
    }
    cout << " Want Summary plots? [0=NO, 1=YES] " << endl; 
    cin >> diagPlots;
    cout << " Want trigger amp? [0=NO, 1=YES] " << endl;
    cin >> trigAmp;
    cout << " Need to change histogram settings? [0=NO, 1=YES] " << endl;
    cin >> histo_limits;
    if(histo_limits){
      cout << " nbins? h_min? h_max? [Default: nbins=45,"
	" h_min=0., h_max=45.] " << endl;
      cin >> hADCamp_bin >> hADCamp_min >> hADCamp_max;
    }
  }else{
    runList.push_back(nrun);
    eventsReplayed.push_back(event);
    runnumber = to_string(nrun);
  }

  shgui::runLabel->SetText(TString::Format("Run %s",runnumber.c_str()));

  int ncell = kNrows*kNcols;
  memset(trigTofadc_ratiosSH, 0, ncell*sizeof(double));
  if(trigAmp) GetTrigtoFADCratio();
  
  // Define a clock to check macro processing time
  TStopwatch *st = new TStopwatch();
  st->Start(kTRUE);

  // Out files
  TString OutFile, OutFile2, OutRootFile;
  if(!trigAmp){
    if(!multiRuns){
      OutFile = Form("Output/run_%d_sh_peak_FADC.txt",nrun);
      OutFile2 = Form("Output/fit_results/bbshower_%d_"
		      "FitResults_FADC.txt",nrun);
      OutRootFile = Form("hist/run_%d_sh_peak_FADC.root",nrun);
      OutF_peaks = Form("plots/SH_signal_peak_FADC_%d.pdf",nrun);
      OutF_diagPlots = Form("plots/BBSH_summary_plots_FADC_%d.pdf",nrun);
    }else{
      OutFile = Form("Output/run_%d_%d_sh_peak_FADC.txt",runList[0],runList[nRuns-1]);
      OutFile2 = Form("Output/fit_results/bbshower_%d_%d_FitResults_FADC.txt"
		      ,runList[0],runList[nRuns-1]);
      OutRootFile = Form("hist/run_%d_%d_sh_peak_FADC.root",runList[0],runList[nRuns-1]);
      OutF_peaks = Form("plots/SH_signal_peak_FADC_%d_%d.pdf",runList[0],runList[nRuns-1]);
      OutF_diagPlots = Form("plots/BBSH_summary_plots_FADC_%d_%d.pdf"
			    ,runList[0],runList[nRuns-1]);
    }
  }else{
    if(!multiRuns){
      OutFile = Form("Output/run_%d_sh_peak_Trigger.txt",nrun);
      OutFile2 = Form("Output/fit_results/bbshower_%d_FitResults_Trigger.txt",nrun);
      OutRootFile = Form("hist/run_%d_sh_peak_Trigger.root",nrun);
      OutF_peaks = Form("plots/SH_signal_peak_Trigger_%d.pdf",nrun);
      OutF_diagPlots = Form("plots/BBSH_summary_plots_Trigger_%d.pdf",nrun);
    }else{
      OutFile = Form("Output/run_%d_%d_sh_peak_Trigger.txt",runList[0],runList[nRuns-1]);
      OutFile2 = Form("Output/fit_results/bbpreshower_%d_%d_FitResults_Trigger.txt",
		      runList[0],runList[nRuns-1]);
      OutRootFile = Form("hist/run_%d_%d_sh_peak_Trigger.root",runList[0],runList[nRuns-1]);
      OutF_peaks = Form("plots/SH_signal_peak_Trigger_%d_%d.pdf",runList[0],runList[nRuns-1]);
      OutF_diagPlots = Form("plots/BBSH_summary_plots_Trigger_%d_%d.pdf",
			    runList[0],runList[nRuns-1]);
    }
  }
  ofstream outfile_data, fitData;
  TFile *outhist_data = new TFile(OutRootFile,"RECREATE");
  outfile_data.open(OutFile);
  fitData.open(OutFile2);
  fitData << "*Run Number: " << nrun << " Desired Peak Position: " << TargetADC << endl;
  fitData << "*Block " << " " << " HV Corr " << " " << " Stat " << " " << " ErrStat " << " " << 
    " Peak Pos " << " " << " ErrPPos " << " " << " Peak Width " << " " << " ErrPWid " << " " << 
    " NinPeak " << " " <<  " " << " Flag " << endl;

  // Read in data produced by analyzer in root format
  // cout << "Reading trees from replayed file.." << endl;
  if(!T) { 
    T = new TChain("T");
    //TString dataDIR = gSystem->Getenv("OUT_DIR");
    for(int rn=0; rn<nRuns; rn++){
      TString filename = Form("../../Rootfiles/bbshower_%d_%d.root",runList[rn],eventsReplayed[rn]);
      TString filename_seg = Form("../../Rootfiles/bbshower_%d_%d_seg_*.root", 
				  runList[rn],eventsReplayed[rn]);
      T->Add(filename);
      T->Add(filename_seg);
    }    

    T->SetBranchStatus("*",0);
    T->SetBranchStatus("bb.sh.*",1);
    T->SetBranchAddress("bb.sh.a_p",fadc_datat::a);
    T->SetBranchAddress("bb.sh.a_amp_p",fadc_datat::amp);
    T->SetBranchAddress("bb.sh.a_time",fadc_datat::tdc);
    T->SetBranchAddress("bb.sh.adcrow",fadc_datat::row);
    T->SetBranchAddress("bb.sh.adccol",fadc_datat::col);
    T->SetBranchStatus("Ndata.bb.sh.adcrow",1);
    T->SetBranchAddress("Ndata.bb.sh.adcrow",&fadc_datat::ndata);
    // T->SetBranchAddress("bb.sh.nsamps",fadc_datat::nsamps);
    // T->SetBranchAddress("bb.sh.samps",fadc_datat::samps);
    // T->SetBranchAddress("bb.sh.samps_idx",fadc_datat::samps_idx);    
    for(int r = 0; r < kNrows; r++) {
      for(int c = 0; c < kNcols; c++) {
	hADCamp[r][c] = MakeHisto(r, c, hADCamp_bin, "_i", hADCamp_min, hADCamp_max);
	hamptointratio[r][c] = MakeHisto(r, c, hamptointratio_bin, "_r", 
					 hamptointratio_min, hamptointratio_max);
      }
    }
  }
  
  Long64_t nevents = T->GetEntries();

  // initializing flags and arrays
  for(int r = 0; r < kNrows+2; r++) {
    for(int c = 0; c < kNcols+2; c++) {
      if( r==0 || r==kNrows+1 ){
  	gPulse[r][c] = true;
      } else {
  	gPulse[r][c] = false;
      }
    }  
  }

  cout << endl << "Processing " << nevents << " events ...." << endl;

  // Looping through events
  double progress = 0.;
  while(progress<1.0){
    int barwidth = 70;
    for (int nev = 0; nev < nevents; nev++){ 
      processEvent( nev, trigAmp );
      
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

  // Initialize the vectors
  // blocks.clear(); 
  // peakPos.clear(); 
  // peakPosErr.clear(); 
  // RMS.clear(); 
  // RMSErr.clear(); 
  // NinPeak.clear(); 
  // HVCrrFact.clear();  

  // Let's fit the histograms with Gauss (twice)
  TF1 *fgaus = new TF1("fgaus","gaus");
  TF1 *fgaus2 = new TF1("fgaus2","gaus");

  int sub = 0;
  for(int r=0; r<kNrows; r++){
    for(int c=0; c<kNcols; c++){
      string Flag;
      double HVcorrection = 1.0;
      double_t NinPeakSH =0.0;
      for( int i=0; i<4; i++ ) {
      	Pars[i] = 0.;
      	ParErrs[i] = 0.;
      }

      sub = r/7;
      subCanv[sub]->cd((r%7)*kNcols + c + 1);
      
      int lowerBinC = hADCamp_min;
      int upperBinC = hADCamp_max;
      int maxBin = hADCamp[r][c]->GetMaximumBin();
      double maxBinCenter = hADCamp[r][c]->GetXaxis()->GetBinCenter( maxBin );
      double maxCount = hADCamp[r][c]->GetMaximum();
      double binWidth = hADCamp[r][c]->GetBinWidth(maxBin);
      double stdDev = hADCamp[r][c]->GetStdDev();

      if(hADCamp[r][c]->GetEntries()>20 && stdDev>2.*binWidth){ 

	// Create fit functions for each module
	fgaus->SetLineColor(2);
	fgaus->SetNpx(1000);

	// Reject low energy peak
	if( hADCamp[r][c]->GetBinContent(maxBin-2) < 0.02*hADCamp[r][c]->GetBinContent(maxBin) ){
	  while ( hADCamp[r][c]->GetBinContent(maxBin+1) < hADCamp[r][c]->GetBinContent(maxBin) || 
		  hADCamp[r][c]->GetBinContent(maxBin+1) == hADCamp[r][c]->GetBinContent(maxBin) ) 
	    {
	      maxBin++;
	    };
	  hADCamp[r][c]->GetXaxis()->SetRange( maxBin+1 , hADCamp[r][c]->GetNbinsX() );
	  maxBin = hADCamp[r][c]->GetMaximumBin();
	  maxBinCenter = hADCamp[r][c]->GetXaxis()->GetBinCenter( maxBin );
	  maxCount = hADCamp[r][c]->GetMaximum();
	  binWidth = hADCamp[r][c]->GetBinWidth(maxBin);
	  stdDev = hADCamp[r][c]->GetStdDev();
	}

	// first fit
	fgaus->SetParameters( maxCount,maxBinCenter,stdDev );
	fgaus->SetRange( hADCamp_min, hADCamp_max );
	hADCamp[r][c]->Fit(fgaus,"NO+RQ");
	fgaus->GetParameters(Pars);

  
	// Second fit with tailored range
	lowerBinC = hADCamp_min + (maxBin)*binWidth - (2.1*Pars[2]);
	upperBinC = hADCamp_min + (maxBin)*binWidth + (2.1*Pars[2]);
	if(r==(kNrows-1)){ // To make the fits better for top and bottom rows
	  lowerBinC = hADCamp_min + (maxBin)*binWidth - (2.*Pars[2]);
	  upperBinC = hADCamp_min + (maxBin)*binWidth + (2.5*Pars[2]);
	}
	fgaus->SetParameters( Pars[0],Pars[1],Pars[2] );
	fgaus->SetRange( lowerBinC, upperBinC );

	hADCamp[r][c]->Fit( fgaus,"+RQ" );
	fgaus->GetParameters(Pars);
	for ( int i=0; i<3; i++ ) ParErrs[i] = fgaus->GetParError(i); 

	Flag = "Good"; // States quality of fit
	HVcorrection = pow( (TargetADC/Pars[1]), 0.10); // Correction term for HV
	NinPeakSH = hADCamp[r][c]->Integral( hADCamp[r][c]->FindFixBin(lowerBinC), 
					     hADCamp[r][c]->FindFixBin(upperBinC), "" ); // # of good events
	  
	hADCamp[r][c]->GetXaxis()->SetRange( 2 , hADCamp[r][c]->GetNbinsX() );
	hADCamp[r][c]->SetTitle(TString::Format("SH %d.%d | ADC Amp ",r+1,c+1));
	hADCamp[r][c]->GetYaxis()->SetLabelSize(0.06);
	hADCamp[r][c]->SetLineColor(kBlue+1);
	hADCamp[r][c]->Draw();
      }else hADCamp[r][c]->Draw();

      gPad->Update();

      outhist_data->cd();
      hADCamp[r][c]->Write();

      // fgaus2->SetLineColor(2);
      // fgaus2->SetNpx(1000);
      // fgaus2->SetParameters( maxCount,maxBinCenter,stdDev );
      // fgaus2->SetRange( hamptointratio_min, hamptointratio_max );
      // hamptointratio[r][c]->Fit(fgaus2,"+RQ");

      // abc << r*kNcols+c << "\t" << fgaus->GetParameter(1) << endl;

      // hamptointratio[r][c]->SetTitle(TString::Format("SH %d.%d | Amp/Int  ",r+1,c+1));
      // // hamptointratio[r][c]->GetYaxis()->SetLabelSize(0.06);
      // hamptointratio[r][c]->GetXaxis()->SetLabelSize(0.04);
      // hamptointratio[r][c]->GetYaxis()->SetLabelSize(0.04);
      // hamptointratio[r][c]->SetLineColor(kBlue+1);
      // hamptointratio[r][c]->Draw();
      // gPad->Update();

      // Let's determine how good is the fit
      if( hADCamp[r][c]->GetEntries() < 20 ){
	cout << " ** The histogram for module # " << r+1 << "." << c+1 << " was empty!! " << endl;
	Flag = "No_Data";
      }else if( Pars[2] > 60.0 ){
	cout << " ** Fit for module # " << r+1 << "." << c+1 << " was too wide!! " << endl;
	Flag = "Wide";
	HVcorrection = 1.0;
      }else if( Pars[2] < 0.1 ){
	cout << " ** Fit for module # " << r+1 << "." << c+1 << " was too narrow!! " << endl;
	Flag = "Narrow";
	HVcorrection = 1.0;
      }else if( ParErrs[1] > 20. || ParErrs[2] > 20. ){
	cout << " ** For module # " << r+1 << "." << c+1 << ", error bar was too high!! " << endl;
	Flag = "Big_error";
	HVcorrection = 1.0;
      }

      // Write all the important fit parameters in a text file
      fitData.setf(ios::fixed);
      fitData.setf(ios::showpoint);
      fitData.precision(2);
      fitData.width(5); fitData << kNcols*r+c;
      fitData.width(12); fitData.precision(4); fitData << HVcorrection;
      fitData.width(12); fitData << Pars[0];
      fitData.width(12); fitData << ParErrs[0];
      fitData.width(12); fitData << Pars[1]; 
      fitData.width(12); fitData << ParErrs[1];
      fitData.width(12); fitData << Pars[2]; 
      fitData.width(12); fitData << ParErrs[2];
      fitData.width(12); fitData << NinPeakSH;
      fitData.width(12); fitData << Flag << endl;

      outfile_data << Pars[1] << " "   << ParErrs[1] << " " ; 

      if ( Flag != "Good" ){
	for( int i=0; i<4; i++ ) { Pars[i] = 0.; ParErrs[i] = 0.; }
	NinPeakSH = 0.;
      }

      // Fill in the vectors to create diagnostic plots
      // blocks.push_back( (double)kNcols*(double)r + (double)c );
      // peakPos.push_back( Pars[1] );
      // peakPosErr.push_back( ParErrs[1] );
      // RMS.push_back( Pars[2] );
      // RMSErr.push_back( ParErrs[2] );
      // NinPeak.push_back( NinPeakSH );
      // HVCrrFact.push_back( HVcorrection );

      blocks[kNcols*r+c] = kNcols*r+c;
      peakPos[kNcols*r+c] = Pars[1];
      peakPosErr[kNcols*r+c] = ParErrs[1];
      RMS[kNcols*r+c] = Pars[2];
      RMSErr[kNcols*r+c] = ParErrs[2];
      NinPeak[kNcols*r+c] = NinPeakSH;
      HVCrrFact[kNcols*r+c] = HVcorrection;
	
    }
    outfile_data << endl;
  }

  if(trigAmp){
    subCanv[0]->SaveAs(Form("%s[",OutF_peaks.Data()));
    for( int canC=0; canC<4; canC++ ) subCanv[canC]->SaveAs(Form("%s",OutF_peaks.Data()));
    subCanv[3]->SaveAs(Form("%s]",OutF_peaks.Data()));
  }else{
    subCanv[0]->SaveAs(Form("%s[",OutF_peaks.Data()));
    for( int canC=0; canC<4; canC++ ) subCanv[canC]->SaveAs(Form("%s",OutF_peaks.Data()));
    subCanv[3]->SaveAs(Form("%s]",OutF_peaks.Data()));
  }

  // Generating diagnostic plots
  if( diagPlots ){
    makeSummaryPlots( runnumber, date, trigAmp );
  }

  // Close all the outFiles
  fitData.close();
  outfile_data.close();
  
  // Post analysis reporting
  cout << "Finished loop over run " << runnumber << "." << endl;
  cout << " --------- " << endl;
  cout << " Peak positions written to : " << OutFile << endl;
  cout << " Fit parameters written to : " << OutFile2  << endl;
  cout << " Histograms written to : " << OutRootFile << endl;
  cout << " Signal peaks saved to : " << OutF_peaks << endl;
  cout << " Summary plots saved to : " << OutF_diagPlots << endl;
  cout << " --------- " << endl;

  st->Stop();
  cout << "CPU time elapsed = " << st->CpuTime() << " s. Real time = " 
       << st->RealTime() << " s. " << endl << endl;

} //main

/* ====================================== \/ ===================================== */

// ---------------- Get today's date ----------------
string getDate(){
  time_t now = time(0);
  tm ltm = *localtime(&now);

  string yyyy = to_string(1900 + ltm.tm_year);
  string mm = to_string(1 + ltm.tm_mon);
  string dd = to_string(ltm.tm_mday);
  string date = mm + '/' + dd + '/' + yyyy;

  return date;
} // getDate

// ---------------- Create generic histogram function ----------------
TH1F* MakeHisto(Int_t row, Int_t col, Int_t bins, const char* suf="", Double_t min=0., Double_t max=50.)
{
  TH1F *h = new TH1F(TString::Format("h_R%d_C%d_Blk%02d%s", row+1, col+1, row*kNcols+col, suf),
		     TString::Format("%d_%d", row+1, col+1), bins, min, max);
  h->SetStats(0);
  h->SetLineWidth(2);
  h->GetYaxis()->SetLabelSize(0.1);
  //h->GetYaxis()->SetLabelOffset(-0.17);
  h->GetYaxis()->SetNdivisions(5);
  return h;
}

// ---------------- Process events ----------------
void processEvent( int entry = -1, bool trigAmp = 0 ){
  // Check event increment and increment
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

  int r,c,idx,n,sub;
  // Clear old signal histograms, just in case modules are not in the tree
  for(r = 0; r < kNrows; r++) {
    for(c = 0; c < kNcols; c++) {
      gPulse[r+1][c+1] = false;
    }
  }
  
  
  // Reset signal peak, adc, and tdc arrays
  double adc[kNrows][kNcols];
  double adc_amp[kNrows][kNcols];
  double tdc[kNrows][kNcols];
  for(r  = 0; r < kNrows; r++) {
    for(c  = 0; c < kNcols; c++) {
      adc[r][c] = 0.0;
      adc_amp[r][c] = 0.0;
      tdc[r][c] = 0.0;
    }
  }

  // Process events with module data
  for(int m = 0; m < fadc_datat::ndata; m++) {
    // Define row and column
    r = fadc_datat::row[m]; 
    c = fadc_datat::col[m]; 
    if(r < 0 || c < 0) {
      cerr << "Why is row negative? Or col?" << endl;
      continue;
    }
    
    if(r>= kNrows || c >= kNcols) continue;
    
    // Define index, number of samples, fill adc and tdc arrays, and switch processed marker for error reporting
    adc[r][c] = fadc_datat::a[m];
    adc_amp[r][c] = fadc_datat::amp[m];
    tdc[r][c] = fadc_datat::tdc[m];
    goodHistoTest( tdc[r][c],r,c ); // Setting flag for good events
  }


  // if(trigAmp){
  //   trigTofadcRatio.clear();
  //   GetTrigtoFADCratio();
  // } 
  
  // Implementation of the Tireman ( or, Bogdan? ) cut: The vertical neighbors will have to pass the cut 
  // (defined in "goodHistoTest routine") and at the same time the horizontal neighbors are not allowed
  // to pass the cut.
  for(r = 0; r < kNrows; r++) {
    for(c = 0; c < kNcols; c++) {
      Int_t m = r*kNcols+c;
      hADCamp[r][c]->SetTitle(Form("%d-%d ",r,c));
      if(r==0){  // bottom row
      	if((gPulse[r+2][c+1]&&gPulse[r+3][c+1]&&gPulse[r+4][c+1]) && (!gPulse[r+1][c]&&!gPulse[r+1][c+2])){
	  if(trigAmp) {
	    hADCamp[r][c]->Fill( trigTofadc_ratiosSH[m]*adc_amp[r][c] ); 
	  }else{
	    hADCamp[r][c]->Fill( adc_amp[r][c] );
	  }
	  hamptointratio[r][c]->Fill( adc_amp[r][c]/adc[r][c] ); 
      	}
      }else if(r==(kNrows-1)){  // top row
      	if((gPulse[r][c+1]&&gPulse[r-1][c+1]&&gPulse[r-2][c+1]) && (!gPulse[r+1][c]&&!gPulse[r+1][c+2])){
	  if(trigAmp) {
	    hADCamp[r][c]->Fill( trigTofadc_ratiosSH[m]*adc_amp[r][c] ); 
	  }else{
	    hADCamp[r][c]->Fill( adc_amp[r][c] );
	  }
	  hamptointratio[r][c]->Fill( adc_amp[r][c]/adc[r][c] );  
      	}
      }

      // *** Special case : We need this when a block doesn't have signal -----
      // else if(m==99){
      // 	if( (gPulse[r][c+1]&&gPulse[r+3][c+1]) && (!gPulse[r+1][c]&&!gPulse[r+1][c+2]) ){
      // 	  if(trigAmp) {
      // 	    hADCamp[r][c]->Fill( trigTofadc_ratiosSH[m]*adc_amp[r][c] ); 
      // 	  }else{
      // 	    hADCamp[r][c]->Fill( adc_amp[r][c] );
      // 	  }
      // 	  hamptointratio[r][c]->Fill( adc_amp[r][c]/adc[r][c] );  
      // 	}
      // }else if(m==85){
      // 	if( (gPulse[r-1][c+1]&&gPulse[r+2][c+1]) && (!gPulse[r+1][c]&&!gPulse[r+1][c+2]) ){
      // 	  if(trigAmp) {
      // 	    hADCamp[r][c]->Fill( trigTofadc_ratiosSH[m]*adc_amp[r][c] ); 
      // 	  }else{
      // 	    hADCamp[r][c]->Fill( adc_amp[r][c] );
      // 	  }
      // 	  hamptointratio[r][c]->Fill( adc_amp[r][c]/adc[r][c] );  
      // 	}
      // }
      // ------

      else{  // all other rows
      	if( (gPulse[r][c+1]&&gPulse[r+2][c+1]) && (!gPulse[r+1][c]&&!gPulse[r+1][c+2]) ){
	  if(trigAmp) {
	    hADCamp[r][c]->Fill( trigTofadc_ratiosSH[m]*adc_amp[r][c] ); 
	  }else{
	    hADCamp[r][c]->Fill( adc_amp[r][c] );
	  }
	  hamptointratio[r][c]->Fill( adc_amp[r][c]/adc[r][c] );
      	}
      }
      // hADCamp[r][c]->Fill( adc[r][c] );
      
    }
  }  
} //processEvent

// ---------------- Resetting the flags depending on the cut ----------------
void goodHistoTest( double tdcVal, int row, int col ){
    
  //Switch if passes FADC time cut
  if(tdcVal!=0) {
    gPulse[row+1][col+1]=true;
  }
  
} //goodHistoTest

// ---------------- Create diagnostic plots ----------------
void makeSummaryPlots( string runnumber, string date, bool trigAmp = 0 ){
  char CName[9], CTitle[100];
  TCanvas *CGr[4];
  TGraph *Gr[4];

  // gROOT->SetBatch();
  CGr[0] = new TCanvas("c1SH","pPosvsBlocks",100,10,700,500);
  CGr[1] = new TCanvas("c2SH","pRMSvsBlocks",100,10,700,500);
  CGr[2] = new TCanvas("c3SH","#EvInPeakvsBlocks",100,10,700,500);
  //CGr[3] = new TCanvas("c4SH","HVcorrvsBlocks",100,10,700,500);
 
  int totalBlocks = kNrows*kNcols;
  double xErr[totalBlocks]; // Holds x-error, which is essentially zero for our case
  for( int i=0; i<totalBlocks; i++ ) xErr[i] = 0. ;
	 
  // Gr[0] = new TGraphErrors( totalBlocks, &(blocks[0]), &(peakPos[0]), xErr, &(peakPosErr[0]) );
  // Gr[1] = new TGraphErrors( totalBlocks, &(blocks[0]), &(RMS[0]), xErr, &(RMSErr[0]) );
  // Gr[2] = new TGraph( totalBlocks, &(blocks[0]), &(NinPeak[0]) );
  // Gr[3] = new TGraph( totalBlocks, &(blocks[0]), &(HVCrrFact[0]) );

  Gr[0] = new TGraphErrors( totalBlocks, blocks, peakPos, xErr, peakPosErr );
  Gr[1] = new TGraphErrors( totalBlocks, blocks, RMS, xErr, RMSErr );
  Gr[2] = new TGraph( totalBlocks, blocks, NinPeak );
  //Gr[3] = new TGraph( totalBlocks, blocks, HVCrrFact );

  for( int i = 0; i < 3; i++ ){
    CGr[i]->cd();
    gPad->SetGridy();
    Gr[i]->SetLineColor(2);
    Gr[i]->SetLineWidth(2);
    Gr[i]->SetMarkerColor(1);
    Gr[i]->SetMarkerStyle(20);
    Gr[i]->GetXaxis()->SetLabelSize(0.04);
    Gr[i]->GetYaxis()->SetLabelSize(0.04);
    if(i == 0 ){
      if(!trigAmp){    	
	Gr[i]->SetTitle(Form("Run# %s | Peak Position(FADC) vs. Block No. for SH Blocks | %s",
			     runnumber.c_str(),date.c_str()) );
	Gr[i]->GetYaxis()->SetTitle("Peak Position at FADC (mV)");
      }else{
	Gr[i]->SetTitle(Form("Run# %s | Peak Position(Trigger) vs. Block No. for SH Blocks | %s",
			     runnumber.c_str(),date.c_str()) );
	Gr[i]->GetYaxis()->SetTitle("Peak Position at Trigger (mV)");
      }
      Gr[i]->GetXaxis()->SetTitle("Block Number");
      Gr[i]->GetYaxis()->SetRangeUser(0.,60.);
    } else if (i == 1){
      Gr[i]->SetTitle( Form("Run# %s | Peak RMS vs. Block No. for SH Blocks | %s",
			    runnumber.c_str(),date.c_str()) ); 
      Gr[i]->GetXaxis()->SetTitle("Block Number");
      Gr[i]->GetYaxis()->SetTitle("Peak RMS (mV)");
      Gr[i]->GetYaxis()->SetTitleOffset(1.4);
      Gr[i]->GetYaxis()->SetRangeUser(0.,20.);
    }else if (i == 2){
      Gr[i]->SetTitle( Form("Run# %s | N of Events in Peak(fitted region) vs Block No. for SH | %s",
			    runnumber.c_str(),date.c_str()) );
      Gr[i]->GetXaxis()->SetTitle("Block Number");
      Gr[i]->GetYaxis()->SetTitle("N of Events in Peak(fitted region)");
      Gr[i]->GetYaxis()->SetTitleOffset(1.4);
    }// else if (i == 3){
    // 	Gr[i]->SetTitle( Form("Run# %d | HV Correction Factor vs Block No. for SH blocks | %s",nrun,date.c_str()) );
    // 	Gr[i]->GetXaxis()->SetTitle("Block Number");
    // 	Gr[i]->GetYaxis()->SetTitle("HV Correction Factor");
    // 	Gr[i]->GetYaxis()->SetTitleOffset(1.4);
    // 	Gr[i]->GetYaxis()->SetRangeUser(0.4,1.6);
    // } 
    Gr[i]->Draw("AP");
    CGr[i]->Write();
  }  

  if(trigAmp){
    CGr[0]->SaveAs( Form("%s[",OutF_diagPlots.Data()) );
    for( int i=0; i<3; i++ ) CGr[i]->SaveAs( Form("%s",OutF_diagPlots.Data()) );
    CGr[2]->SaveAs( Form("%s]",OutF_diagPlots.Data()) );
  }else{
    CGr[0]->SaveAs( Form("%s[",OutF_diagPlots.Data()) );
    for( int i=0; i<3; i++ ) CGr[i]->SaveAs( Form("%s",OutF_diagPlots.Data()) );
    CGr[2]->SaveAs( Form("%s]",OutF_diagPlots.Data()) );
  }
}


// ---------------- Reading Trigger to FADC amplitude ratios ----------------
void GetTrigtoFADCratio(){
  string InFile = "Coefficients/trigtoFADCcoef_SH.txt";
  ifstream infile_data;
  infile_data.open(InFile);
  TString currentline;
  if (infile_data.is_open()) {
    // cout << " Reading trigger to FADC ratios from " << InFile << endl;
    TString temp1, temp2;
    while( currentline.ReadLine( infile_data ) ){
      TObjArray *tokens = currentline.Tokenize("\t");
      int ntokens = tokens->GetEntries();
      if( ntokens > 1 ){
	temp1 = ( (TObjString*) (*tokens)[0] )->GetString();
	// elemID.push_back( temp.Atof() );
	temp2 = ( (TObjString*) (*tokens)[1] )->GetString();
	int elemID = temp1.Atoi();
	double theRatio = temp2.Atof();
	//trigTofadcRatio.push_back( theRatio );
	trigTofadc_ratiosSH[elemID] = theRatio;
      }
      delete tokens;
    }
    infile_data.close();
  } else {
    cerr << "--!--" << endl << " No file : " << InFile << endl;
    throw;
  }
}
