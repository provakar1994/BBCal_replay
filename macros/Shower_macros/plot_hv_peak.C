/* 
   M. K. Jones  Created
*/

#include <TString.h>
#include <TLegend.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TCutG.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TMath.h>
#include <TProfile.h>
#include <TObjArray.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include<math.h>
using namespace std;

void plot_hv_peak(TString HVfileName, TString basename) {
  gROOT->Reset();
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(11);
  gStyle->SetTitleOffset(1.,"Y");
  gStyle->SetTitleOffset(.7,"X");
  gStyle->SetLabelSize(0.04,"XY");
  gStyle->SetTitleSize(0.06,"XY");
  gStyle->SetPadLeftMargin(0.14);
  //
  static const Int_t shNCol=7;
  static const Int_t shNRow=27;
  TString hv_crate[shNRow][shNCol];
  TString hv_slot[shNRow][shNCol];
  Int_t hv_chan[shNRow][shNCol];
  //
  HVfileName = "/home/daq/slowc/BBCAL/hv_set/"+ HVfileName;
  ifstream file_hv(HVfileName.Data());
  
  for (Int_t nc=0;nc<shNCol;nc++) {
    for (Int_t nr=0;nr<shNRow;nr++) {
      if (nr >= 13)  hv_crate[nr][nc]="rpi18:2001";
      if (nr < 13)  hv_crate[nr][nc]="rpi17:2001";
    }
  }  
  Int_t row_lo[2] ={0,12}; // index of row
  Int_t slot_lo[2]={2,5};
  Int_t slot_hi[2]={9,13};
  Int_t nchan=12;
  Int_t cur_col=0;
  Int_t cur_row=0;
  for (Int_t nc=0;nc<2;nc++) {
    cur_col=0;
    cur_row=row_lo[nc];
    for (Int_t ns=slot_lo[nc];ns<slot_hi[nc]+1;ns++) {
      Int_t nc_start=0;
      if (nc==0 && ns==slot_lo[nc]) nc_start=3;
      for (Int_t nch=nc_start;nch<nchan;nch++) {
	hv_slot[cur_row][cur_col]=ns;
	hv_chan[cur_row][cur_col]=nch;
	//cout << nc << " "<< ns << " "<< nch << " " << cur_row << " "<< cur_col << " "<<endl;
	cur_col++;
	if (cur_col==shNCol) {
	  cur_col=0;
	  cur_row++;
	} 
	if (cur_row==shNRow) break;
      }
    }
  }
  //
  //  
}
