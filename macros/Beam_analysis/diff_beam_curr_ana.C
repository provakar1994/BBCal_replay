// - P. Datta

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
#include "gmn_tree.C"

// Detector parameters
const int kNrows = 27;
const int kNcols = 7;

gmn_tree *T;
int gCurrentEntry = -1;

void diff_beam_curr_ana(){

  TFile *f1 = new TFile("","READ");
  TFile *f2 = new TFile("","READ");
  TFile *f4 = new TFile("","READ");
  TFile *f8 = new TFile("","READ");

  TH2F *h2_nclus_nblk = new TH2F("h2_nclus_nblk","",10,0,10,12,0,12);

} 
