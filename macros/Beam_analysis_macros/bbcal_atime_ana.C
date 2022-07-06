#include "TChain.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TString.h"
#include <iostream>
#include <fstream>
//#include "gmn_tree.C"

const Double_t Mp = 0.938272; // GeV
const Double_t Ebeam = 5.965; // GeV

const Int_t kNcolsSH = 7;  // SH columns
const Int_t kNrowsSH = 27; // SH rows

void bbcal_atime_ana( const char *rootfilename, Double_t percentdiff=20. ){

  gErrorIgnoreLevel = kError; // Ignores all ROOT warnings

  TChain *C = new TChain("T");
  //gmn_tree *T = new gmn_tree(C);

  C->Add(rootfilename);

  Int_t maxtr=1000, hodo_trindex=0;
  Double_t sh_nclus, sh_e, sh_rowblk, sh_colblk, sh_nblk;
  Double_t ps_nclus, ps_idblk, ps_e, ps_rowblk, ps_colblk;
  Double_t hodo_nclus;
  Double_t sh_clblk_atime[maxtr], sh_clblk_e[maxtr];
  Double_t p[maxtr], pz[maxtr], tg_th[maxtr], tg_ph[maxtr];
  Double_t hodo_tmean[maxtr], hodo_trIndex[maxtr];

  C->SetBranchStatus("*",0);
  //shower
  C->SetBranchStatus("bb.sh.nclus",1);
  C->SetBranchAddress("bb.sh.nclus",&sh_nclus);
  C->SetBranchStatus("bb.sh.e",1);
  C->SetBranchAddress("bb.sh.e",&sh_e);
  C->SetBranchStatus("bb.sh.rowblk",1);
  C->SetBranchAddress("bb.sh.rowblk",&sh_rowblk);
  C->SetBranchStatus("bb.sh.colblk",1);
  C->SetBranchAddress("bb.sh.colblk",&sh_colblk);
  C->SetBranchStatus("bb.sh.nblk",1);
  C->SetBranchAddress("bb.sh.nblk",&sh_nblk);
  C->SetBranchStatus("bb.sh.clus_blk.e",1);
  C->SetBranchAddress("bb.sh.clus_blk.e",&sh_clblk_e);
  C->SetBranchStatus("bb.sh.clus_blk.atime",1);
  C->SetBranchAddress("bb.sh.clus_blk.atime",&sh_clblk_atime);
  //preshower
  C->SetBranchStatus("bb.ps.nclus",1);
  C->SetBranchAddress("bb.ps.nclus",&ps_nclus);
  C->SetBranchStatus("bb.ps.e",1);
  C->SetBranchAddress("bb.ps.e",&ps_e);
  C->SetBranchStatus("bb.ps.rowblk",1);
  C->SetBranchAddress("bb.ps.rowblk",&ps_rowblk);
  C->SetBranchStatus("bb.ps.colblk",1);
  C->SetBranchAddress("bb.ps.colblk",&ps_colblk);
  C->SetBranchStatus("bb.ps.idblk",1);
  C->SetBranchAddress("bb.ps.idblk",&ps_idblk);
  //gem
  C->SetBranchStatus("bb.tr.p",1);
  C->SetBranchAddress("bb.tr.p",&p);
  C->SetBranchStatus("bb.tr.pz",1);
  C->SetBranchAddress("bb.tr.pz",&pz);
  C->SetBranchStatus("bb.tr.tg_th",1);
  C->SetBranchAddress("bb.tr.tg_th",&tg_th);
  C->SetBranchStatus("bb.tr.tg_ph",1);
  C->SetBranchAddress("bb.tr.tg_ph",&tg_ph);
  //hodo
  C->SetBranchStatus("bb.hodotdc.nclus",1);
  C->SetBranchAddress("bb.hodotdc.nclus",&hodo_nclus);
  C->SetBranchStatus("bb.hodotdc.clus.tmean",1);
  C->SetBranchAddress("bb.hodotdc.clus.tmean",&hodo_tmean);
  C->SetBranchStatus("bb.hodotdc.clus.trackindex",1);
  C->SetBranchAddress("bb.hodotdc.clus.trackindex",&hodo_trIndex);

  TString outFile = Form("hist/bbcal_time_res.root");
  TFile *fout = new TFile(outFile,"RECREATE");
  fout->cd();

  TH1D *h_W = new TH1D("h_W","W distribution",200,0.,5.);
  TH1D *h_Q2 = new TH1D("h_Q2","Q2 distribution",200,0.,10.);
  TH1D *h_blk_count = new TH1D("h_blk_count","",100,0.,10.);
  TH1D *h_blk_count_cut = new TH1D("h_blk_count_cut","",100,0.,10.);
  TH2D *h2_clustime_diff = new TH2D("h2_cltime_diff","",
				    189,0,189,1000,-10.,10.);
  TH2D *h2_clustime_diff_cut = new TH2D("h2_cltime_diff_cut","",
					189,0,189,1000,-10.,10.);
  
  Long64_t nevent=0, nevents=0;
  nevents = C->GetEntries();

  cout << endl;
  while( C->GetEntry( nevent++ ) ){
    if( nevent%1000 == 0 ) cout << nevent << "/" << nevents << "\r";
    cout.flush();

    //calculating W
    Double_t P_ang = 57.3*TMath::ACos(pz[0]/p[0]);
    Double_t Q2 = 4.*Ebeam*p[0]*pow( TMath::Sin(P_ang/57.3/2.),2. );
    Double_t W2 = Mp*Mp + 2.*Mp*(Ebeam-p[0]) - Q2;
    Double_t W = 0.;

    h_Q2->Fill(Q2);
    if( W2>0. ){
      W = TMath::Sqrt(W2);  
      h_W->Fill(W);
    }

    //good cluster cut
    if( sh_nclus==0 || ps_nclus==0 || ps_idblk==-1 || ps_e<0.2 ) continue;

    //good track cut
    if( tg_th[0]>-0.15 && tg_th[0]<0.15 && 
	tg_ph[0]>-0.3 && tg_ph[0]<0 ){
 
      //avoiding clusters on the edge
      if(sh_rowblk==0 || sh_rowblk==26 ||
	 sh_colblk==0 || sh_colblk==6) continue; 

      // looping over clusters
      Int_t nblk = sh_nblk;
      Double_t eng_HEblk = sh_clblk_e[0];
      Double_t atime_HEblk = sh_clblk_atime[0];
      Int_t elemID = sh_rowblk*kNcolsSH + sh_colblk;
      Int_t count=0, count_cut=0;

      // //determine cluster time difference w.r.t. the
      // //ADCtime of HE block:
      // for(Int_t blk=1; blk<nblk; blk++){
      // 	Double_t eng_blk = sh_clblk_e[blk];
      // 	Double_t atime_blk = sh_clblk_atime[blk];

      // 	if( (fabs( eng_HEblk-eng_blk )/eng_HEblk)*100. < percentdiff ){
      // 	  // && fabs(W-0.9515)<0.3 ){
      // 	  h2_clustime_diff_cut->Fill( elemID, atime_HEblk-atime_blk );
      // 	  count_cut += 1;
      // 	}
      // 	h2_clustime_diff->Fill( elemID, atime_HEblk-atime_blk );
      // 	count += 1;
      // }

      //determine cluster time difference w.r.t. the
      //hodoscope mean time:
      for(Int_t blk=0; blk<nblk; blk++){
      	Double_t eng_blk = sh_clblk_e[blk];
      	Double_t atime_blk = sh_clblk_atime[blk];
      	Double_t tmean_hodo = hodo_tmean[0];

      	if( (fabs( eng_HEblk-eng_blk )/eng_HEblk)*100. < percentdiff ){
      	  // && fabs(W-0.9515)<0.3 ){
      	  h2_clustime_diff_cut->Fill( elemID, tmean_hodo-atime_blk );
      	  count_cut += 1;
      	}
      	h2_clustime_diff->Fill( elemID, tmean_hodo-atime_blk );
      	count += 1;
      }
      
      h_blk_count->Fill( count );
      if(count_cut>0) h_blk_count_cut->Fill( count_cut );

    } // if, good track cut


  } //while
  cout << endl; 

  fout->Write();
  fout->Close();
  fout->Delete();
 
  cout << " --------- " << endl;
  cout << " Histogram written to : " << outFile << endl;
  cout << " --------- " << endl;

}
