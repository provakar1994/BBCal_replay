#include <iostream>
#include "TSystem.h"
#include "TString.h"
#include "TFile.h"
#include "THaShower.h"
#include "THaEvent.h"
#include "THaEvData.h"
#include "THaRun.h"
#include "THaAnalyzer.h"
#include "THaVarList.h"
#include "SBSBigBite.h"
#include "SBSBBTotalShower.h"
#include "SBSBBShower.h"

// Simple example replay script
//
// Ole Hansen, 11 April 2016
void replay_BBCal(int run_number = 124, uint nev = -1, TString start_name = "e1209019", uint nseg = 0)
{
  //load SBS-offline
  // gSystem->Load("libsbs.so");
  //--- Define the experimental configuration, i.e. spectrometers, detectors ---

  //THaHRS* bb = new THaHRS("R", "Right HRS" );
  //hrs->AddDetector( new THaVDC("vdc", "Vertical Drift Chambers") );
  // gHaApps->Add(hrs);
  
  SBSBigBite* bigbite = new SBSBigBite("bb", "BigBite spectrometer" );
  //bigbite->AddDetector( new THaShower("ps", "BigBite preshower") );
  SBSBBTotalShower* ts= new SBSBBTotalShower("ts", "sh", "ps", "BigBite shower");
  ts->SetDataOutputLevel(0);
  bigbite->AddDetector( ts );
  ts->SetStoreEmptyElements(kFALSE);
  ts->GetShower()->SetStoreEmptyElements(kFALSE);
  ts->GetPreShower()->SetStoreEmptyElements(kFALSE);

  SBSGenericDetector* bbtrig= new SBSGenericDetector("bbtrig","BigBite shower ADC trig");
  bbtrig->SetModeADC(SBSModeADC::kADC);
  bbtrig->SetModeTDC(SBSModeTDC::kTDC);
  bbtrig->SetStoreEmptyElements(kFALSE);
  bigbite->AddDetector( bbtrig );
  gHaApps->Add(bigbite);

  // SBSGenericDetector* trig= new SBSGenericDetector("trig","BigBite shower trig");
  // trig->SetModeADC(SBSModeADC::kWaveform);
  // bigbite->AddDetector( trig );
  // bigbite->AddDetector( new SBSBBShower("ps", "BigBite preshower") );
  // bigbite->AddDetector( new SBSBBShower("sh", "BigBite shower") );
  // gHaApps->Add(bigbite);
  
  // Ideal beam (perfect normal incidence and centering)
  // THaIdealBeam* ib = new THaIdealBeam("IB", "Ideal beam");
  // gHaApps->Add(ib);

  //--- Set up the run we want to replay ---

  // This often requires a bit of coding to search directories, test
  // for non-existent files, etc.
  TString exp = "bbshower";
  // Create file name patterns.
  string firstname = "bbshower_%d";
  string endname = Form(".evio.0.%d",nseg);
  //string endname = Form(".evio");
  string combined(string(firstname)+endname);
  const char* RunFileNamePattern = combined.c_str();
  vector<TString> pathList;
  pathList.push_back("/cache/halla/sbs/raw");
  //pathList.push_back("/adaqeb1/data1");
  pathList.push_back(Form("%s/data","/adaqfs/home/a-onl/sbs"));

  THaAnalyzer* analyzer = new THaAnalyzer;
  THaEvent* event = new THaEvent;
  THaRun* run = 0;
  int seg = 0;
  bool seg_ok = true;
  while(seg_ok) {
    TString data_fname;
    data_fname = TString::Format("%s/%s_%d.evio.0.%d",getenv("DATA_DIR"),start_name.Data(),run_number,seg);
    //new THaRun( pathList, Form(RunFileNamePattern, run_number) );
    //  for (UInt_t nn=0; nn < pathList.size(); nn++) {
    // data_fname = TString::Format("%s/%s_%d.evio.%d",pathList[nn].Data(),start_name.Data(),run_number,seg);
    std::cout << "Looking for segment " << seg << " file " << data_fname.Data() << std::endl;
    if( gSystem->AccessPathName(data_fname)) {
      seg_ok = false;
      std::cout << "Segment " << seg << " not found. Exiting" << std::endl;
      continue;
    }
    //  }
    run = new THaRun(data_fname);
    run->SetLastEvent(nev);

    run->SetDataRequired(0);//for the time being
    run->SetDate(TDatime());
  
  

    analyzer->SetEvent( event );
    TString out_dir = gSystem->Getenv("OUT_DIR");
    if( out_dir.IsNull() )  out_dir = ".";
    TString out_file = out_dir + "/" + exp + Form("_%d_%d.root", run_number,nev);

    analyzer->SetOutFile( out_file );
  
    analyzer->SetCutFile( "replay_BBCal.cdef" );
    analyzer->SetOdefFile( "replay_BBCal.odef" );

    analyzer->SetVerbosity(2);  // write cut summary to stdout
    analyzer->EnableBenchmarks();

    //--- Analyze the run ---
    // Here, one could replay more than one run with
    // a succession of Process calls. The results would all go into the
    // same ROOT output file

    run->Print();
  

    analyzer->Process(run);
    // Cleanup this run segment
    delete run;
    
    seg++; // Increment for next search
  }

  // Clean up

  analyzer->Close();
  delete analyzer;
  //gHaCuts->Clear();
  gHaVars->Clear();
  gHaPhysics->Delete();
  gHaApps->Delete();

  // Open the ROOT file so that a user doing interactive analysis can 
  // immediately look at the results. Of course, don't do this in batch jobs!
  // To close the file later, simply type "delete rootfile" or just exit

  //TFile* rootfile = new TFile( out_file, "READ" );
}
