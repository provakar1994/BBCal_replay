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

void diff_beam_curr_ana(int nrun=100, int nevent=-1, 
			int fseg=0, int mseg=4, bool SHorHCAL=1 ){ // 0=BBSH, 1=HCAL

  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  TChain *C = new TChain("T");
  gmn_tree *T = new gmn_tree(C);

  // TString filename = Form("/volatile/halla/sbs/datta/GMN_REPLAYS/rootfiles"
  //			  "/bbshower_gmn_%d_%d_seg%d_%d.root"
  //			  ,nrun,nevent,fseg,mseg);
  TString filename = Form("../../Rootfiles/bbshower_gmn_%d_%d"
  			  "_stream0_seg%d_%d.root",nrun,nevent,fseg,mseg);
  C->Add(filename);

  TString outFile;
  if(SHorHCAL)
    outFile = Form("hist/nclus_nblk_e_%d_%d_hcal.root",nrun,nevent);
  else
    outFile = Form("hist/nclus_nblk_e_%d_%d_bbsh.root",nrun,nevent);

  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  gStyle->SetOptStat(0);
  TH2F *h2_nclus_nblk = new TH2F("h2_nclus_nblk","Number of block vs"
				 " Cluster ID; Cluster ID [0=>Best]; "
				 "Number of Blocks involved",12,0,12,12,1,13);
  TH2F *h2_nclus_eng = new TH2F("h2_nclus_eng","Energy of Cluster vs"
				 " Cluster ID; Cluster ID [0=>Best]; "
				 "Energy of Cluster",25,0,25,200,0.,2.);
  TH2F *h2_nclus_seed_eng = new TH2F("h2_nclus_seed_eng","Energy of HE block vs"
				 " Cluster ID; Cluster ID [0=>Best]; "
				 "Energy of HE block",25,0,25,200,0.,2.);
  
  Long64_t nevents = C->GetEntries();
  cout << endl << "Processing " << nevents << " events... " << endl;

  // Looping through events
  double progress = 0.;
  while(progress<1.0){
    int barwidth = 70;
    for (int nev = 0; nev < nevents; nev++){ 
 
      T->GetEntry(nev);
      
      if(!SHorHCAL){ // BBSH Clustering
	
	for( int cl=0; cl<(int)T->bb_sh_nclus; cl++ ){
	
	  double clus_ID = T->bb_sh_clus_id[cl]; 
	  double num_Block = T->bb_sh_clus_nblk[cl];
	  double clus_eng = T->bb_sh_clus_e[cl];
	  double HEblk_eng = T->bb_sh_clus_eblk[cl];

	  h2_nclus_nblk->Fill(cl,num_Block);
	  h2_nclus_eng->Fill(cl,clus_eng);
	  h2_nclus_seed_eng->Fill(cl,HEblk_eng);
	}
      }     
      else{ // HCAL Clustering
	
	for( int cl=0; cl<(int)T->sbs_hcal_nclus; cl++ ){

	  double clus_ID = T->sbs_hcal_clus_id[cl]; 
	  double num_Block = T->sbs_hcal_clus_nblk[cl];
	  double clus_eng = T->sbs_hcal_clus_e[cl];
	  double HEblk_eng = T->sbs_hcal_clus_eblk[cl];

	  h2_nclus_nblk->Fill(cl,num_Block);
	  h2_nclus_eng->Fill(cl,clus_eng);
	  h2_nclus_seed_eng->Fill(cl,HEblk_eng);
	}
      }


      // ---- \/ ----     
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
  cout << endl << endl << "------" << endl;
  cout << " Histograms written to: " << outFile << endl;
  cout << "------" << endl << endl;

  fout->Write();
  fout->Close();
  fout->Delete();

} 
