//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Sep 19 13:30:08 2022 by ROOT version 6.24/06
// from TTree T/Hall A Analyzer Output DST
// found on file: bbshower_1475_1000000_10.root
//////////////////////////////////////////////////////////

#ifndef cosmic_tree_h
#define cosmic_tree_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

class cosmic_tree {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           Ndata_bb_bbtrig_Ref_a;
   Double_t        bb_bbtrig_Ref_a[1000];   //[Ndata.bb.bbtrig.Ref.a]
   Int_t           Ndata_bb_bbtrig_Ref_a_amp;
   Double_t        bb_bbtrig_Ref_a_amp[1000];   //[Ndata.bb.bbtrig.Ref.a_amp]
   Int_t           Ndata_bb_bbtrig_Ref_a_amp_c;
   Double_t        bb_bbtrig_Ref_a_amp_c[1000];   //[Ndata.bb.bbtrig.Ref.a_amp_c]
   Int_t           Ndata_bb_bbtrig_Ref_a_amp_p;
   Double_t        bb_bbtrig_Ref_a_amp_p[1000];   //[Ndata.bb.bbtrig.Ref.a_amp_p]
   Int_t           Ndata_bb_bbtrig_Ref_a_c;
   Double_t        bb_bbtrig_Ref_a_c[1000];   //[Ndata.bb.bbtrig.Ref.a_c]
   Int_t           Ndata_bb_bbtrig_Ref_a_mult;
   Double_t        bb_bbtrig_Ref_a_mult[1000];   //[Ndata.bb.bbtrig.Ref.a_mult]
   Int_t           Ndata_bb_bbtrig_Ref_a_p;
   Double_t        bb_bbtrig_Ref_a_p[1000];   //[Ndata.bb.bbtrig.Ref.a_p]
   Int_t           Ndata_bb_bbtrig_Ref_a_time;
   Double_t        bb_bbtrig_Ref_a_time[1000];   //[Ndata.bb.bbtrig.Ref.a_time]
   Int_t           Ndata_bb_bbtrig_Ref_adcelemID;
   Double_t        bb_bbtrig_Ref_adcelemID[1000];   //[Ndata.bb.bbtrig.Ref.adcelemID]
   Int_t           Ndata_bb_bbtrig_Ref_ped;
   Double_t        bb_bbtrig_Ref_ped[1000];   //[Ndata.bb.bbtrig.Ref.ped]
   Int_t           Ndata_bb_bbtrig_Ref_tdc;
   Double_t        bb_bbtrig_Ref_tdc[1000];   //[Ndata.bb.bbtrig.Ref.tdc]
   Int_t           Ndata_bb_bbtrig_Ref_tdc_mult;
   Double_t        bb_bbtrig_Ref_tdc_mult[1000];   //[Ndata.bb.bbtrig.Ref.tdc_mult]
   Int_t           Ndata_bb_bbtrig_Ref_tdc_te;
   Double_t        bb_bbtrig_Ref_tdc_te[1000];   //[Ndata.bb.bbtrig.Ref.tdc_te]
   Int_t           Ndata_bb_bbtrig_Ref_tdc_tot;
   Double_t        bb_bbtrig_Ref_tdc_tot[1000];   //[Ndata.bb.bbtrig.Ref.tdc_tot]
   Int_t           Ndata_bb_bbtrig_Ref_tdcelemID;
   Double_t        bb_bbtrig_Ref_tdcelemID[1000];   //[Ndata.bb.bbtrig.Ref.tdcelemID]
   Int_t           Ndata_bb_bbtrig_a;
   Double_t        bb_bbtrig_a[1000];   //[Ndata.bb.bbtrig.a]
   Int_t           Ndata_bb_bbtrig_a_amp;
   Double_t        bb_bbtrig_a_amp[1000];   //[Ndata.bb.bbtrig.a_amp]
   Int_t           Ndata_bb_bbtrig_a_amp_c;
   Double_t        bb_bbtrig_a_amp_c[1000];   //[Ndata.bb.bbtrig.a_amp_c]
   Int_t           Ndata_bb_bbtrig_a_amp_p;
   Double_t        bb_bbtrig_a_amp_p[1000];   //[Ndata.bb.bbtrig.a_amp_p]
   Int_t           Ndata_bb_bbtrig_a_amptrig_c;
   Double_t        bb_bbtrig_a_amptrig_c[1000];   //[Ndata.bb.bbtrig.a_amptrig_c]
   Int_t           Ndata_bb_bbtrig_a_amptrig_p;
   Double_t        bb_bbtrig_a_amptrig_p[1000];   //[Ndata.bb.bbtrig.a_amptrig_p]
   Int_t           Ndata_bb_bbtrig_a_c;
   Double_t        bb_bbtrig_a_c[1000];   //[Ndata.bb.bbtrig.a_c]
   Int_t           Ndata_bb_bbtrig_a_mult;
   Double_t        bb_bbtrig_a_mult[1000];   //[Ndata.bb.bbtrig.a_mult]
   Int_t           Ndata_bb_bbtrig_a_p;
   Double_t        bb_bbtrig_a_p[1000];   //[Ndata.bb.bbtrig.a_p]
   Int_t           Ndata_bb_bbtrig_a_time;
   Double_t        bb_bbtrig_a_time[1000];   //[Ndata.bb.bbtrig.a_time]
   Int_t           Ndata_bb_bbtrig_adccol;
   Double_t        bb_bbtrig_adccol[1000];   //[Ndata.bb.bbtrig.adccol]
   Int_t           Ndata_bb_bbtrig_adcelemID;
   Double_t        bb_bbtrig_adcelemID[1000];   //[Ndata.bb.bbtrig.adcelemID]
   Int_t           Ndata_bb_bbtrig_adclayer;
   Double_t        bb_bbtrig_adclayer[1000];   //[Ndata.bb.bbtrig.adclayer]
   Int_t           Ndata_bb_bbtrig_adcrow;
   Double_t        bb_bbtrig_adcrow[1000];   //[Ndata.bb.bbtrig.adcrow]
   Int_t           Ndata_bb_bbtrig_ped;
   Double_t        bb_bbtrig_ped[1000];   //[Ndata.bb.bbtrig.ped]
   Int_t           Ndata_bb_bbtrig_tdc;
   Double_t        bb_bbtrig_tdc[1000];   //[Ndata.bb.bbtrig.tdc]
   Int_t           Ndata_bb_bbtrig_tdc_mult;
   Double_t        bb_bbtrig_tdc_mult[1000];   //[Ndata.bb.bbtrig.tdc_mult]
   Int_t           Ndata_bb_bbtrig_tdc_te;
   Double_t        bb_bbtrig_tdc_te[1000];   //[Ndata.bb.bbtrig.tdc_te]
   Int_t           Ndata_bb_bbtrig_tdc_tot;
   Double_t        bb_bbtrig_tdc_tot[1000];   //[Ndata.bb.bbtrig.tdc_tot]
   Int_t           Ndata_bb_bbtrig_tdccol;
   Double_t        bb_bbtrig_tdccol[1000];   //[Ndata.bb.bbtrig.tdccol]
   Int_t           Ndata_bb_bbtrig_tdcelemID;
   Double_t        bb_bbtrig_tdcelemID[1000];   //[Ndata.bb.bbtrig.tdcelemID]
   Int_t           Ndata_bb_bbtrig_tdclayer;
   Double_t        bb_bbtrig_tdclayer[1000];   //[Ndata.bb.bbtrig.tdclayer]
   Int_t           Ndata_bb_bbtrig_tdcrow;
   Double_t        bb_bbtrig_tdcrow[1000];   //[Ndata.bb.bbtrig.tdcrow]
   Int_t           Ndata_bb_ps_a;
   Double_t        bb_ps_a[1000];   //[Ndata.bb.ps.a]
   Int_t           Ndata_bb_ps_a_amp;
   Double_t        bb_ps_a_amp[1000];   //[Ndata.bb.ps.a_amp]
   Int_t           Ndata_bb_ps_a_amp_c;
   Double_t        bb_ps_a_amp_c[1000];   //[Ndata.bb.ps.a_amp_c]
   Int_t           Ndata_bb_ps_a_amp_p;
   Double_t        bb_ps_a_amp_p[1000];   //[Ndata.bb.ps.a_amp_p]
   Int_t           Ndata_bb_ps_a_amptrig_c;
   Double_t        bb_ps_a_amptrig_c[1000];   //[Ndata.bb.ps.a_amptrig_c]
   Int_t           Ndata_bb_ps_a_amptrig_p;
   Double_t        bb_ps_a_amptrig_p[1000];   //[Ndata.bb.ps.a_amptrig_p]
   Int_t           Ndata_bb_ps_a_c;
   Double_t        bb_ps_a_c[1000];   //[Ndata.bb.ps.a_c]
   Int_t           Ndata_bb_ps_a_mult;
   Double_t        bb_ps_a_mult[1000];   //[Ndata.bb.ps.a_mult]
   Int_t           Ndata_bb_ps_a_p;
   Double_t        bb_ps_a_p[1000];   //[Ndata.bb.ps.a_p]
   Int_t           Ndata_bb_ps_a_time;
   Double_t        bb_ps_a_time[1000];   //[Ndata.bb.ps.a_time]
   Int_t           Ndata_bb_ps_adccol;
   Double_t        bb_ps_adccol[1000];   //[Ndata.bb.ps.adccol]
   Int_t           Ndata_bb_ps_adcelemID;
   Double_t        bb_ps_adcelemID[1000];   //[Ndata.bb.ps.adcelemID]
   Int_t           Ndata_bb_ps_adclayer;
   Double_t        bb_ps_adclayer[1000];   //[Ndata.bb.ps.adclayer]
   Int_t           Ndata_bb_ps_adcrow;
   Double_t        bb_ps_adcrow[1000];   //[Ndata.bb.ps.adcrow]
   Int_t           Ndata_bb_ps_clus_atime;
   Double_t        bb_ps_clus_atime[1000];   //[Ndata.bb.ps.clus.atime]
   Int_t           Ndata_bb_ps_clus_col;
   Double_t        bb_ps_clus_col[1000];   //[Ndata.bb.ps.clus.col]
   Int_t           Ndata_bb_ps_clus_e;
   Double_t        bb_ps_clus_e[1000];   //[Ndata.bb.ps.clus.e]
   Int_t           Ndata_bb_ps_clus_e_c;
   Double_t        bb_ps_clus_e_c[1000];   //[Ndata.bb.ps.clus.e_c]
   Int_t           Ndata_bb_ps_clus_eblk;
   Double_t        bb_ps_clus_eblk[1000];   //[Ndata.bb.ps.clus.eblk]
   Int_t           Ndata_bb_ps_clus_eblk_c;
   Double_t        bb_ps_clus_eblk_c[1000];   //[Ndata.bb.ps.clus.eblk_c]
   Int_t           Ndata_bb_ps_clus_id;
   Double_t        bb_ps_clus_id[1000];   //[Ndata.bb.ps.clus.id]
   Int_t           Ndata_bb_ps_clus_nblk;
   Double_t        bb_ps_clus_nblk[1000];   //[Ndata.bb.ps.clus.nblk]
   Int_t           Ndata_bb_ps_clus_row;
   Double_t        bb_ps_clus_row[1000];   //[Ndata.bb.ps.clus.row]
   Int_t           Ndata_bb_ps_clus_tdctime;
   Double_t        bb_ps_clus_tdctime[1000];   //[Ndata.bb.ps.clus.tdctime]
   Int_t           Ndata_bb_ps_clus_x;
   Double_t        bb_ps_clus_x[1000];   //[Ndata.bb.ps.clus.x]
   Int_t           Ndata_bb_ps_clus_y;
   Double_t        bb_ps_clus_y[1000];   //[Ndata.bb.ps.clus.y]
   Int_t           Ndata_bb_ps_clus_blk_atime;
   Double_t        bb_ps_clus_blk_atime[4];   //[Ndata.bb.ps.clus_blk.atime]
   Int_t           Ndata_bb_ps_clus_blk_col;
   Double_t        bb_ps_clus_blk_col[4];   //[Ndata.bb.ps.clus_blk.col]
   Int_t           Ndata_bb_ps_clus_blk_e;
   Double_t        bb_ps_clus_blk_e[4];   //[Ndata.bb.ps.clus_blk.e]
   Int_t           Ndata_bb_ps_clus_blk_e_c;
   Double_t        bb_ps_clus_blk_e_c[4];   //[Ndata.bb.ps.clus_blk.e_c]
   Int_t           Ndata_bb_ps_clus_blk_id;
   Double_t        bb_ps_clus_blk_id[4];   //[Ndata.bb.ps.clus_blk.id]
   Int_t           Ndata_bb_ps_clus_blk_row;
   Double_t        bb_ps_clus_blk_row[4];   //[Ndata.bb.ps.clus_blk.row]
   Int_t           Ndata_bb_ps_clus_blk_tdctime;
   Double_t        bb_ps_clus_blk_tdctime[4];   //[Ndata.bb.ps.clus_blk.tdctime]
   Int_t           Ndata_bb_ps_clus_blk_x;
   Double_t        bb_ps_clus_blk_x[4];   //[Ndata.bb.ps.clus_blk.x]
   Int_t           Ndata_bb_ps_clus_blk_y;
   Double_t        bb_ps_clus_blk_y[4];   //[Ndata.bb.ps.clus_blk.y]
   Int_t           Ndata_bb_ps_e_res;
   Double_t        bb_ps_e_res[1000];   //[Ndata.bb.ps.e_res]
   Int_t           Ndata_bb_ps_goodblock_atime;
   Double_t        bb_ps_goodblock_atime[1000];   //[Ndata.bb.ps.goodblock.atime]
   Int_t           Ndata_bb_ps_goodblock_col;
   Double_t        bb_ps_goodblock_col[1000];   //[Ndata.bb.ps.goodblock.col]
   Int_t           Ndata_bb_ps_goodblock_e;
   Double_t        bb_ps_goodblock_e[1000];   //[Ndata.bb.ps.goodblock.e]
   Int_t           Ndata_bb_ps_goodblock_id;
   Double_t        bb_ps_goodblock_id[1000];   //[Ndata.bb.ps.goodblock.id]
   Int_t           Ndata_bb_ps_goodblock_row;
   Double_t        bb_ps_goodblock_row[1000];   //[Ndata.bb.ps.goodblock.row]
   Int_t           Ndata_bb_ps_goodblock_tdctime;
   Double_t        bb_ps_goodblock_tdctime[1000];   //[Ndata.bb.ps.goodblock.tdctime]
   Int_t           Ndata_bb_ps_goodblock_x;
   Double_t        bb_ps_goodblock_x[1000];   //[Ndata.bb.ps.goodblock.x]
   Int_t           Ndata_bb_ps_goodblock_y;
   Double_t        bb_ps_goodblock_y[1000];   //[Ndata.bb.ps.goodblock.y]
   Int_t           Ndata_bb_ps_hits_a;
   Double_t        bb_ps_hits_a[1000];   //[Ndata.bb.ps.hits.a]
   Int_t           Ndata_bb_ps_hits_a_amp;
   Double_t        bb_ps_hits_a_amp[1000];   //[Ndata.bb.ps.hits.a_amp]
   Int_t           Ndata_bb_ps_hits_a_time;
   Double_t        bb_ps_hits_a_time[1000];   //[Ndata.bb.ps.hits.a_time]
   Int_t           Ndata_bb_ps_nsamps;
   Double_t        bb_ps_nsamps[1000];   //[Ndata.bb.ps.nsamps]
   Int_t           Ndata_bb_ps_ped;
   Double_t        bb_ps_ped[1000];   //[Ndata.bb.ps.ped]
   Int_t           Ndata_bb_ps_samps;
   Double_t        bb_ps_samps[1248];   //[Ndata.bb.ps.samps]
   Int_t           Ndata_bb_ps_samps_elemID;
   Double_t        bb_ps_samps_elemID[1000];   //[Ndata.bb.ps.samps_elemID]
   Int_t           Ndata_bb_ps_samps_idx;
   Double_t        bb_ps_samps_idx[1000];   //[Ndata.bb.ps.samps_idx]
   Int_t           Ndata_bb_ps_x_res;
   Double_t        bb_ps_x_res[1000];   //[Ndata.bb.ps.x_res]
   Int_t           Ndata_bb_ps_y_res;
   Double_t        bb_ps_y_res[1000];   //[Ndata.bb.ps.y_res]
   Int_t           Ndata_bb_sh_a;
   Double_t        bb_sh_a[1000];   //[Ndata.bb.sh.a]
   Int_t           Ndata_bb_sh_a_amp;
   Double_t        bb_sh_a_amp[1000];   //[Ndata.bb.sh.a_amp]
   Int_t           Ndata_bb_sh_a_amp_c;
   Double_t        bb_sh_a_amp_c[1000];   //[Ndata.bb.sh.a_amp_c]
   Int_t           Ndata_bb_sh_a_amp_p;
   Double_t        bb_sh_a_amp_p[1000];   //[Ndata.bb.sh.a_amp_p]
   Int_t           Ndata_bb_sh_a_amptrig_c;
   Double_t        bb_sh_a_amptrig_c[1000];   //[Ndata.bb.sh.a_amptrig_c]
   Int_t           Ndata_bb_sh_a_amptrig_p;
   Double_t        bb_sh_a_amptrig_p[1000];   //[Ndata.bb.sh.a_amptrig_p]
   Int_t           Ndata_bb_sh_a_c;
   Double_t        bb_sh_a_c[1000];   //[Ndata.bb.sh.a_c]
   Int_t           Ndata_bb_sh_a_mult;
   Double_t        bb_sh_a_mult[1000];   //[Ndata.bb.sh.a_mult]
   Int_t           Ndata_bb_sh_a_p;
   Double_t        bb_sh_a_p[1000];   //[Ndata.bb.sh.a_p]
   Int_t           Ndata_bb_sh_a_time;
   Double_t        bb_sh_a_time[1000];   //[Ndata.bb.sh.a_time]
   Int_t           Ndata_bb_sh_adccol;
   Double_t        bb_sh_adccol[1000];   //[Ndata.bb.sh.adccol]
   Int_t           Ndata_bb_sh_adcelemID;
   Double_t        bb_sh_adcelemID[1000];   //[Ndata.bb.sh.adcelemID]
   Int_t           Ndata_bb_sh_adclayer;
   Double_t        bb_sh_adclayer[1000];   //[Ndata.bb.sh.adclayer]
   Int_t           Ndata_bb_sh_adcrow;
   Double_t        bb_sh_adcrow[1000];   //[Ndata.bb.sh.adcrow]
   Int_t           Ndata_bb_sh_clus_atime;
   Double_t        bb_sh_clus_atime[1000];   //[Ndata.bb.sh.clus.atime]
   Int_t           Ndata_bb_sh_clus_col;
   Double_t        bb_sh_clus_col[1000];   //[Ndata.bb.sh.clus.col]
   Int_t           Ndata_bb_sh_clus_e;
   Double_t        bb_sh_clus_e[1000];   //[Ndata.bb.sh.clus.e]
   Int_t           Ndata_bb_sh_clus_e_c;
   Double_t        bb_sh_clus_e_c[1000];   //[Ndata.bb.sh.clus.e_c]
   Int_t           Ndata_bb_sh_clus_eblk;
   Double_t        bb_sh_clus_eblk[1000];   //[Ndata.bb.sh.clus.eblk]
   Int_t           Ndata_bb_sh_clus_eblk_c;
   Double_t        bb_sh_clus_eblk_c[1000];   //[Ndata.bb.sh.clus.eblk_c]
   Int_t           Ndata_bb_sh_clus_id;
   Double_t        bb_sh_clus_id[1000];   //[Ndata.bb.sh.clus.id]
   Int_t           Ndata_bb_sh_clus_nblk;
   Double_t        bb_sh_clus_nblk[1000];   //[Ndata.bb.sh.clus.nblk]
   Int_t           Ndata_bb_sh_clus_row;
   Double_t        bb_sh_clus_row[1000];   //[Ndata.bb.sh.clus.row]
   Int_t           Ndata_bb_sh_clus_tdctime;
   Double_t        bb_sh_clus_tdctime[1000];   //[Ndata.bb.sh.clus.tdctime]
   Int_t           Ndata_bb_sh_clus_x;
   Double_t        bb_sh_clus_x[1000];   //[Ndata.bb.sh.clus.x]
   Int_t           Ndata_bb_sh_clus_y;
   Double_t        bb_sh_clus_y[1000];   //[Ndata.bb.sh.clus.y]
   Int_t           Ndata_bb_sh_clus_blk_atime;
   Double_t        bb_sh_clus_blk_atime[1000];   //[Ndata.bb.sh.clus_blk.atime]
   Int_t           Ndata_bb_sh_clus_blk_col;
   Double_t        bb_sh_clus_blk_col[1000];   //[Ndata.bb.sh.clus_blk.col]
   Int_t           Ndata_bb_sh_clus_blk_e;
   Double_t        bb_sh_clus_blk_e[1000];   //[Ndata.bb.sh.clus_blk.e]
   Int_t           Ndata_bb_sh_clus_blk_e_c;
   Double_t        bb_sh_clus_blk_e_c[1000];   //[Ndata.bb.sh.clus_blk.e_c]
   Int_t           Ndata_bb_sh_clus_blk_id;
   Double_t        bb_sh_clus_blk_id[1000];   //[Ndata.bb.sh.clus_blk.id]
   Int_t           Ndata_bb_sh_clus_blk_row;
   Double_t        bb_sh_clus_blk_row[1000];   //[Ndata.bb.sh.clus_blk.row]
   Int_t           Ndata_bb_sh_clus_blk_tdctime;
   Double_t        bb_sh_clus_blk_tdctime[1000];   //[Ndata.bb.sh.clus_blk.tdctime]
   Int_t           Ndata_bb_sh_clus_blk_x;
   Double_t        bb_sh_clus_blk_x[1000];   //[Ndata.bb.sh.clus_blk.x]
   Int_t           Ndata_bb_sh_clus_blk_y;
   Double_t        bb_sh_clus_blk_y[1000];   //[Ndata.bb.sh.clus_blk.y]
   Int_t           Ndata_bb_sh_e_res;
   Double_t        bb_sh_e_res[1000];   //[Ndata.bb.sh.e_res]
   Int_t           Ndata_bb_sh_goodblock_atime;
   Double_t        bb_sh_goodblock_atime[1000];   //[Ndata.bb.sh.goodblock.atime]
   Int_t           Ndata_bb_sh_goodblock_col;
   Double_t        bb_sh_goodblock_col[1000];   //[Ndata.bb.sh.goodblock.col]
   Int_t           Ndata_bb_sh_goodblock_e;
   Double_t        bb_sh_goodblock_e[1000];   //[Ndata.bb.sh.goodblock.e]
   Int_t           Ndata_bb_sh_goodblock_id;
   Double_t        bb_sh_goodblock_id[1000];   //[Ndata.bb.sh.goodblock.id]
   Int_t           Ndata_bb_sh_goodblock_row;
   Double_t        bb_sh_goodblock_row[1000];   //[Ndata.bb.sh.goodblock.row]
   Int_t           Ndata_bb_sh_goodblock_tdctime;
   Double_t        bb_sh_goodblock_tdctime[1000];   //[Ndata.bb.sh.goodblock.tdctime]
   Int_t           Ndata_bb_sh_goodblock_x;
   Double_t        bb_sh_goodblock_x[1000];   //[Ndata.bb.sh.goodblock.x]
   Int_t           Ndata_bb_sh_goodblock_y;
   Double_t        bb_sh_goodblock_y[1000];   //[Ndata.bb.sh.goodblock.y]
   Int_t           Ndata_bb_sh_hits_a;
   Double_t        bb_sh_hits_a[1000];   //[Ndata.bb.sh.hits.a]
   Int_t           Ndata_bb_sh_hits_a_amp;
   Double_t        bb_sh_hits_a_amp[1000];   //[Ndata.bb.sh.hits.a_amp]
   Int_t           Ndata_bb_sh_hits_a_time;
   Double_t        bb_sh_hits_a_time[1000];   //[Ndata.bb.sh.hits.a_time]
   Int_t           Ndata_bb_sh_nsamps;
   Double_t        bb_sh_nsamps[1000];   //[Ndata.bb.sh.nsamps]
   Int_t           Ndata_bb_sh_ped;
   Double_t        bb_sh_ped[1000];   //[Ndata.bb.sh.ped]
   Int_t           Ndata_bb_sh_samps;
   Double_t        bb_sh_samps[4536];   //[Ndata.bb.sh.samps]
   Int_t           Ndata_bb_sh_samps_elemID;
   Double_t        bb_sh_samps_elemID[1000];   //[Ndata.bb.sh.samps_elemID]
   Int_t           Ndata_bb_sh_samps_idx;
   Double_t        bb_sh_samps_idx[1000];   //[Ndata.bb.sh.samps_idx]
   Int_t           Ndata_bb_sh_x_res;
   Double_t        bb_sh_x_res[1000];   //[Ndata.bb.sh.x_res]
   Int_t           Ndata_bb_sh_y_res;
   Double_t        bb_sh_y_res[1000];   //[Ndata.bb.sh.y_res]
   Double_t        bb_bbtrig_ngoodADChits;
   Double_t        bb_bbtrig_ngoodTDChits;
   Double_t        bb_bbtrig_nhits;
   Double_t        bb_bbtrig_nrefhits;
   Double_t        bb_ps_atimeblk;
   Double_t        bb_ps_colblk;
   Double_t        bb_ps_e;
   Double_t        bb_ps_e_c;
   Double_t        bb_ps_e_m_res;
   Double_t        bb_ps_eblk;
   Double_t        bb_ps_eblk_c;
   Double_t        bb_ps_idblk;
   Double_t        bb_ps_index;
   Double_t        bb_ps_nblk;
   Double_t        bb_ps_nclus;
   Double_t        bb_ps_ngoodADChits;
   Double_t        bb_ps_ngoodTDChits;
   Double_t        bb_ps_nhits;
   Double_t        bb_ps_nrefhits;
   Double_t        bb_ps_rowblk;
   Double_t        bb_ps_tdctimeblk;
   Double_t        bb_ps_x;
   Double_t        bb_ps_x_m_res;
   Double_t        bb_ps_y;
   Double_t        bb_ps_y_m_res;
   Double_t        bb_sh_atimeblk;
   Double_t        bb_sh_colblk;
   Double_t        bb_sh_e;
   Double_t        bb_sh_e_c;
   Double_t        bb_sh_e_m_res;
   Double_t        bb_sh_eblk;
   Double_t        bb_sh_eblk_c;
   Double_t        bb_sh_idblk;
   Double_t        bb_sh_index;
   Double_t        bb_sh_nblk;
   Double_t        bb_sh_nclus;
   Double_t        bb_sh_ngoodADChits;
   Double_t        bb_sh_ngoodTDChits;
   Double_t        bb_sh_nhits;
   Double_t        bb_sh_nrefhits;
   Double_t        bb_sh_rowblk;
   Double_t        bb_sh_tdctimeblk;
   Double_t        bb_sh_x;
   Double_t        bb_sh_x_m_res;
   Double_t        bb_sh_y;
   Double_t        bb_sh_y_m_res;
   Double_t        g_datatype;
   Double_t        g_evlen;
   Double_t        g_evnum;
   Double_t        g_evtime;
   Double_t        g_evtyp;
   Double_t        g_runnum;
   Double_t        g_runtime;
   Double_t        g_runtype;
   Double_t        g_trigbits;
 //THaEvent        *Event_Branch;
   ULong64_t       fEvtHdr_fEvtTime;
   UInt_t          fEvtHdr_fEvtNum;
   UInt_t          fEvtHdr_fEvtType;
   UInt_t          fEvtHdr_fEvtLen;
   Int_t           fEvtHdr_fHelicity;
   UInt_t          fEvtHdr_fTrigBits;
   UInt_t          fEvtHdr_fRun;

   // List of branches
   TBranch        *b_Ndata_bb_bbtrig_Ref_a;   //!
   TBranch        *b_bb_bbtrig_Ref_a;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_a_amp;   //!
   TBranch        *b_bb_bbtrig_Ref_a_amp;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_a_amp_c;   //!
   TBranch        *b_bb_bbtrig_Ref_a_amp_c;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_a_amp_p;   //!
   TBranch        *b_bb_bbtrig_Ref_a_amp_p;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_a_c;   //!
   TBranch        *b_bb_bbtrig_Ref_a_c;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_a_mult;   //!
   TBranch        *b_bb_bbtrig_Ref_a_mult;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_a_p;   //!
   TBranch        *b_bb_bbtrig_Ref_a_p;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_a_time;   //!
   TBranch        *b_bb_bbtrig_Ref_a_time;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_adcelemID;   //!
   TBranch        *b_bb_bbtrig_Ref_adcelemID;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_ped;   //!
   TBranch        *b_bb_bbtrig_Ref_ped;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_tdc;   //!
   TBranch        *b_bb_bbtrig_Ref_tdc;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_tdc_mult;   //!
   TBranch        *b_bb_bbtrig_Ref_tdc_mult;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_tdc_te;   //!
   TBranch        *b_bb_bbtrig_Ref_tdc_te;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_tdc_tot;   //!
   TBranch        *b_bb_bbtrig_Ref_tdc_tot;   //!
   TBranch        *b_Ndata_bb_bbtrig_Ref_tdcelemID;   //!
   TBranch        *b_bb_bbtrig_Ref_tdcelemID;   //!
   TBranch        *b_Ndata_bb_bbtrig_a;   //!
   TBranch        *b_bb_bbtrig_a;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_amp;   //!
   TBranch        *b_bb_bbtrig_a_amp;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_amp_c;   //!
   TBranch        *b_bb_bbtrig_a_amp_c;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_amp_p;   //!
   TBranch        *b_bb_bbtrig_a_amp_p;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_amptrig_c;   //!
   TBranch        *b_bb_bbtrig_a_amptrig_c;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_amptrig_p;   //!
   TBranch        *b_bb_bbtrig_a_amptrig_p;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_c;   //!
   TBranch        *b_bb_bbtrig_a_c;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_mult;   //!
   TBranch        *b_bb_bbtrig_a_mult;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_p;   //!
   TBranch        *b_bb_bbtrig_a_p;   //!
   TBranch        *b_Ndata_bb_bbtrig_a_time;   //!
   TBranch        *b_bb_bbtrig_a_time;   //!
   TBranch        *b_Ndata_bb_bbtrig_adccol;   //!
   TBranch        *b_bb_bbtrig_adccol;   //!
   TBranch        *b_Ndata_bb_bbtrig_adcelemID;   //!
   TBranch        *b_bb_bbtrig_adcelemID;   //!
   TBranch        *b_Ndata_bb_bbtrig_adclayer;   //!
   TBranch        *b_bb_bbtrig_adclayer;   //!
   TBranch        *b_Ndata_bb_bbtrig_adcrow;   //!
   TBranch        *b_bb_bbtrig_adcrow;   //!
   TBranch        *b_Ndata_bb_bbtrig_ped;   //!
   TBranch        *b_bb_bbtrig_ped;   //!
   TBranch        *b_Ndata_bb_bbtrig_tdc;   //!
   TBranch        *b_bb_bbtrig_tdc;   //!
   TBranch        *b_Ndata_bb_bbtrig_tdc_mult;   //!
   TBranch        *b_bb_bbtrig_tdc_mult;   //!
   TBranch        *b_Ndata_bb_bbtrig_tdc_te;   //!
   TBranch        *b_bb_bbtrig_tdc_te;   //!
   TBranch        *b_Ndata_bb_bbtrig_tdc_tot;   //!
   TBranch        *b_bb_bbtrig_tdc_tot;   //!
   TBranch        *b_Ndata_bb_bbtrig_tdccol;   //!
   TBranch        *b_bb_bbtrig_tdccol;   //!
   TBranch        *b_Ndata_bb_bbtrig_tdcelemID;   //!
   TBranch        *b_bb_bbtrig_tdcelemID;   //!
   TBranch        *b_Ndata_bb_bbtrig_tdclayer;   //!
   TBranch        *b_bb_bbtrig_tdclayer;   //!
   TBranch        *b_Ndata_bb_bbtrig_tdcrow;   //!
   TBranch        *b_bb_bbtrig_tdcrow;   //!
   TBranch        *b_Ndata_bb_ps_a;   //!
   TBranch        *b_bb_ps_a;   //!
   TBranch        *b_Ndata_bb_ps_a_amp;   //!
   TBranch        *b_bb_ps_a_amp;   //!
   TBranch        *b_Ndata_bb_ps_a_amp_c;   //!
   TBranch        *b_bb_ps_a_amp_c;   //!
   TBranch        *b_Ndata_bb_ps_a_amp_p;   //!
   TBranch        *b_bb_ps_a_amp_p;   //!
   TBranch        *b_Ndata_bb_ps_a_amptrig_c;   //!
   TBranch        *b_bb_ps_a_amptrig_c;   //!
   TBranch        *b_Ndata_bb_ps_a_amptrig_p;   //!
   TBranch        *b_bb_ps_a_amptrig_p;   //!
   TBranch        *b_Ndata_bb_ps_a_c;   //!
   TBranch        *b_bb_ps_a_c;   //!
   TBranch        *b_Ndata_bb_ps_a_mult;   //!
   TBranch        *b_bb_ps_a_mult;   //!
   TBranch        *b_Ndata_bb_ps_a_p;   //!
   TBranch        *b_bb_ps_a_p;   //!
   TBranch        *b_Ndata_bb_ps_a_time;   //!
   TBranch        *b_bb_ps_a_time;   //!
   TBranch        *b_Ndata_bb_ps_adccol;   //!
   TBranch        *b_bb_ps_adccol;   //!
   TBranch        *b_Ndata_bb_ps_adcelemID;   //!
   TBranch        *b_bb_ps_adcelemID;   //!
   TBranch        *b_Ndata_bb_ps_adclayer;   //!
   TBranch        *b_bb_ps_adclayer;   //!
   TBranch        *b_Ndata_bb_ps_adcrow;   //!
   TBranch        *b_bb_ps_adcrow;   //!
   TBranch        *b_Ndata_bb_ps_clus_atime;   //!
   TBranch        *b_bb_ps_clus_atime;   //!
   TBranch        *b_Ndata_bb_ps_clus_col;   //!
   TBranch        *b_bb_ps_clus_col;   //!
   TBranch        *b_Ndata_bb_ps_clus_e;   //!
   TBranch        *b_bb_ps_clus_e;   //!
   TBranch        *b_Ndata_bb_ps_clus_e_c;   //!
   TBranch        *b_bb_ps_clus_e_c;   //!
   TBranch        *b_Ndata_bb_ps_clus_eblk;   //!
   TBranch        *b_bb_ps_clus_eblk;   //!
   TBranch        *b_Ndata_bb_ps_clus_eblk_c;   //!
   TBranch        *b_bb_ps_clus_eblk_c;   //!
   TBranch        *b_Ndata_bb_ps_clus_id;   //!
   TBranch        *b_bb_ps_clus_id;   //!
   TBranch        *b_Ndata_bb_ps_clus_nblk;   //!
   TBranch        *b_bb_ps_clus_nblk;   //!
   TBranch        *b_Ndata_bb_ps_clus_row;   //!
   TBranch        *b_bb_ps_clus_row;   //!
   TBranch        *b_Ndata_bb_ps_clus_tdctime;   //!
   TBranch        *b_bb_ps_clus_tdctime;   //!
   TBranch        *b_Ndata_bb_ps_clus_x;   //!
   TBranch        *b_bb_ps_clus_x;   //!
   TBranch        *b_Ndata_bb_ps_clus_y;   //!
   TBranch        *b_bb_ps_clus_y;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_atime;   //!
   TBranch        *b_bb_ps_clus_blk_atime;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_col;   //!
   TBranch        *b_bb_ps_clus_blk_col;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_e;   //!
   TBranch        *b_bb_ps_clus_blk_e;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_e_c;   //!
   TBranch        *b_bb_ps_clus_blk_e_c;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_id;   //!
   TBranch        *b_bb_ps_clus_blk_id;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_row;   //!
   TBranch        *b_bb_ps_clus_blk_row;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_tdctime;   //!
   TBranch        *b_bb_ps_clus_blk_tdctime;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_x;   //!
   TBranch        *b_bb_ps_clus_blk_x;   //!
   TBranch        *b_Ndata_bb_ps_clus_blk_y;   //!
   TBranch        *b_bb_ps_clus_blk_y;   //!
   TBranch        *b_Ndata_bb_ps_e_res;   //!
   TBranch        *b_bb_ps_e_res;   //!
   TBranch        *b_Ndata_bb_ps_goodblock_atime;   //!
   TBranch        *b_bb_ps_goodblock_atime;   //!
   TBranch        *b_Ndata_bb_ps_goodblock_col;   //!
   TBranch        *b_bb_ps_goodblock_col;   //!
   TBranch        *b_Ndata_bb_ps_goodblock_e;   //!
   TBranch        *b_bb_ps_goodblock_e;   //!
   TBranch        *b_Ndata_bb_ps_goodblock_id;   //!
   TBranch        *b_bb_ps_goodblock_id;   //!
   TBranch        *b_Ndata_bb_ps_goodblock_row;   //!
   TBranch        *b_bb_ps_goodblock_row;   //!
   TBranch        *b_Ndata_bb_ps_goodblock_tdctime;   //!
   TBranch        *b_bb_ps_goodblock_tdctime;   //!
   TBranch        *b_Ndata_bb_ps_goodblock_x;   //!
   TBranch        *b_bb_ps_goodblock_x;   //!
   TBranch        *b_Ndata_bb_ps_goodblock_y;   //!
   TBranch        *b_bb_ps_goodblock_y;   //!
   TBranch        *b_Ndata_bb_ps_hits_a;   //!
   TBranch        *b_bb_ps_hits_a;   //!
   TBranch        *b_Ndata_bb_ps_hits_a_amp;   //!
   TBranch        *b_bb_ps_hits_a_amp;   //!
   TBranch        *b_Ndata_bb_ps_hits_a_time;   //!
   TBranch        *b_bb_ps_hits_a_time;   //!
   TBranch        *b_Ndata_bb_ps_nsamps;   //!
   TBranch        *b_bb_ps_nsamps;   //!
   TBranch        *b_Ndata_bb_ps_ped;   //!
   TBranch        *b_bb_ps_ped;   //!
   TBranch        *b_Ndata_bb_ps_samps;   //!
   TBranch        *b_bb_ps_samps;   //!
   TBranch        *b_Ndata_bb_ps_samps_elemID;   //!
   TBranch        *b_bb_ps_samps_elemID;   //!
   TBranch        *b_Ndata_bb_ps_samps_idx;   //!
   TBranch        *b_bb_ps_samps_idx;   //!
   TBranch        *b_Ndata_bb_ps_x_res;   //!
   TBranch        *b_bb_ps_x_res;   //!
   TBranch        *b_Ndata_bb_ps_y_res;   //!
   TBranch        *b_bb_ps_y_res;   //!
   TBranch        *b_Ndata_bb_sh_a;   //!
   TBranch        *b_bb_sh_a;   //!
   TBranch        *b_Ndata_bb_sh_a_amp;   //!
   TBranch        *b_bb_sh_a_amp;   //!
   TBranch        *b_Ndata_bb_sh_a_amp_c;   //!
   TBranch        *b_bb_sh_a_amp_c;   //!
   TBranch        *b_Ndata_bb_sh_a_amp_p;   //!
   TBranch        *b_bb_sh_a_amp_p;   //!
   TBranch        *b_Ndata_bb_sh_a_amptrig_c;   //!
   TBranch        *b_bb_sh_a_amptrig_c;   //!
   TBranch        *b_Ndata_bb_sh_a_amptrig_p;   //!
   TBranch        *b_bb_sh_a_amptrig_p;   //!
   TBranch        *b_Ndata_bb_sh_a_c;   //!
   TBranch        *b_bb_sh_a_c;   //!
   TBranch        *b_Ndata_bb_sh_a_mult;   //!
   TBranch        *b_bb_sh_a_mult;   //!
   TBranch        *b_Ndata_bb_sh_a_p;   //!
   TBranch        *b_bb_sh_a_p;   //!
   TBranch        *b_Ndata_bb_sh_a_time;   //!
   TBranch        *b_bb_sh_a_time;   //!
   TBranch        *b_Ndata_bb_sh_adccol;   //!
   TBranch        *b_bb_sh_adccol;   //!
   TBranch        *b_Ndata_bb_sh_adcelemID;   //!
   TBranch        *b_bb_sh_adcelemID;   //!
   TBranch        *b_Ndata_bb_sh_adclayer;   //!
   TBranch        *b_bb_sh_adclayer;   //!
   TBranch        *b_Ndata_bb_sh_adcrow;   //!
   TBranch        *b_bb_sh_adcrow;   //!
   TBranch        *b_Ndata_bb_sh_clus_atime;   //!
   TBranch        *b_bb_sh_clus_atime;   //!
   TBranch        *b_Ndata_bb_sh_clus_col;   //!
   TBranch        *b_bb_sh_clus_col;   //!
   TBranch        *b_Ndata_bb_sh_clus_e;   //!
   TBranch        *b_bb_sh_clus_e;   //!
   TBranch        *b_Ndata_bb_sh_clus_e_c;   //!
   TBranch        *b_bb_sh_clus_e_c;   //!
   TBranch        *b_Ndata_bb_sh_clus_eblk;   //!
   TBranch        *b_bb_sh_clus_eblk;   //!
   TBranch        *b_Ndata_bb_sh_clus_eblk_c;   //!
   TBranch        *b_bb_sh_clus_eblk_c;   //!
   TBranch        *b_Ndata_bb_sh_clus_id;   //!
   TBranch        *b_bb_sh_clus_id;   //!
   TBranch        *b_Ndata_bb_sh_clus_nblk;   //!
   TBranch        *b_bb_sh_clus_nblk;   //!
   TBranch        *b_Ndata_bb_sh_clus_row;   //!
   TBranch        *b_bb_sh_clus_row;   //!
   TBranch        *b_Ndata_bb_sh_clus_tdctime;   //!
   TBranch        *b_bb_sh_clus_tdctime;   //!
   TBranch        *b_Ndata_bb_sh_clus_x;   //!
   TBranch        *b_bb_sh_clus_x;   //!
   TBranch        *b_Ndata_bb_sh_clus_y;   //!
   TBranch        *b_bb_sh_clus_y;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_atime;   //!
   TBranch        *b_bb_sh_clus_blk_atime;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_col;   //!
   TBranch        *b_bb_sh_clus_blk_col;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_e;   //!
   TBranch        *b_bb_sh_clus_blk_e;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_e_c;   //!
   TBranch        *b_bb_sh_clus_blk_e_c;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_id;   //!
   TBranch        *b_bb_sh_clus_blk_id;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_row;   //!
   TBranch        *b_bb_sh_clus_blk_row;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_tdctime;   //!
   TBranch        *b_bb_sh_clus_blk_tdctime;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_x;   //!
   TBranch        *b_bb_sh_clus_blk_x;   //!
   TBranch        *b_Ndata_bb_sh_clus_blk_y;   //!
   TBranch        *b_bb_sh_clus_blk_y;   //!
   TBranch        *b_Ndata_bb_sh_e_res;   //!
   TBranch        *b_bb_sh_e_res;   //!
   TBranch        *b_Ndata_bb_sh_goodblock_atime;   //!
   TBranch        *b_bb_sh_goodblock_atime;   //!
   TBranch        *b_Ndata_bb_sh_goodblock_col;   //!
   TBranch        *b_bb_sh_goodblock_col;   //!
   TBranch        *b_Ndata_bb_sh_goodblock_e;   //!
   TBranch        *b_bb_sh_goodblock_e;   //!
   TBranch        *b_Ndata_bb_sh_goodblock_id;   //!
   TBranch        *b_bb_sh_goodblock_id;   //!
   TBranch        *b_Ndata_bb_sh_goodblock_row;   //!
   TBranch        *b_bb_sh_goodblock_row;   //!
   TBranch        *b_Ndata_bb_sh_goodblock_tdctime;   //!
   TBranch        *b_bb_sh_goodblock_tdctime;   //!
   TBranch        *b_Ndata_bb_sh_goodblock_x;   //!
   TBranch        *b_bb_sh_goodblock_x;   //!
   TBranch        *b_Ndata_bb_sh_goodblock_y;   //!
   TBranch        *b_bb_sh_goodblock_y;   //!
   TBranch        *b_Ndata_bb_sh_hits_a;   //!
   TBranch        *b_bb_sh_hits_a;   //!
   TBranch        *b_Ndata_bb_sh_hits_a_amp;   //!
   TBranch        *b_bb_sh_hits_a_amp;   //!
   TBranch        *b_Ndata_bb_sh_hits_a_time;   //!
   TBranch        *b_bb_sh_hits_a_time;   //!
   TBranch        *b_Ndata_bb_sh_nsamps;   //!
   TBranch        *b_bb_sh_nsamps;   //!
   TBranch        *b_Ndata_bb_sh_ped;   //!
   TBranch        *b_bb_sh_ped;   //!
   TBranch        *b_Ndata_bb_sh_samps;   //!
   TBranch        *b_bb_sh_samps;   //!
   TBranch        *b_Ndata_bb_sh_samps_elemID;   //!
   TBranch        *b_bb_sh_samps_elemID;   //!
   TBranch        *b_Ndata_bb_sh_samps_idx;   //!
   TBranch        *b_bb_sh_samps_idx;   //!
   TBranch        *b_Ndata_bb_sh_x_res;   //!
   TBranch        *b_bb_sh_x_res;   //!
   TBranch        *b_Ndata_bb_sh_y_res;   //!
   TBranch        *b_bb_sh_y_res;   //!
   TBranch        *b_bb_bbtrig_ngoodADChits;   //!
   TBranch        *b_bb_bbtrig_ngoodTDChits;   //!
   TBranch        *b_bb_bbtrig_nhits;   //!
   TBranch        *b_bb_bbtrig_nrefhits;   //!
   TBranch        *b_bb_ps_atimeblk;   //!
   TBranch        *b_bb_ps_colblk;   //!
   TBranch        *b_bb_ps_e;   //!
   TBranch        *b_bb_ps_e_c;   //!
   TBranch        *b_bb_ps_e_m_res;   //!
   TBranch        *b_bb_ps_eblk;   //!
   TBranch        *b_bb_ps_eblk_c;   //!
   TBranch        *b_bb_ps_idblk;   //!
   TBranch        *b_bb_ps_index;   //!
   TBranch        *b_bb_ps_nblk;   //!
   TBranch        *b_bb_ps_nclus;   //!
   TBranch        *b_bb_ps_ngoodADChits;   //!
   TBranch        *b_bb_ps_ngoodTDChits;   //!
   TBranch        *b_bb_ps_nhits;   //!
   TBranch        *b_bb_ps_nrefhits;   //!
   TBranch        *b_bb_ps_rowblk;   //!
   TBranch        *b_bb_ps_tdctimeblk;   //!
   TBranch        *b_bb_ps_x;   //!
   TBranch        *b_bb_ps_x_m_res;   //!
   TBranch        *b_bb_ps_y;   //!
   TBranch        *b_bb_ps_y_m_res;   //!
   TBranch        *b_bb_sh_atimeblk;   //!
   TBranch        *b_bb_sh_colblk;   //!
   TBranch        *b_bb_sh_e;   //!
   TBranch        *b_bb_sh_e_c;   //!
   TBranch        *b_bb_sh_e_m_res;   //!
   TBranch        *b_bb_sh_eblk;   //!
   TBranch        *b_bb_sh_eblk_c;   //!
   TBranch        *b_bb_sh_idblk;   //!
   TBranch        *b_bb_sh_index;   //!
   TBranch        *b_bb_sh_nblk;   //!
   TBranch        *b_bb_sh_nclus;   //!
   TBranch        *b_bb_sh_ngoodADChits;   //!
   TBranch        *b_bb_sh_ngoodTDChits;   //!
   TBranch        *b_bb_sh_nhits;   //!
   TBranch        *b_bb_sh_nrefhits;   //!
   TBranch        *b_bb_sh_rowblk;   //!
   TBranch        *b_bb_sh_tdctimeblk;   //!
   TBranch        *b_bb_sh_x;   //!
   TBranch        *b_bb_sh_x_m_res;   //!
   TBranch        *b_bb_sh_y;   //!
   TBranch        *b_bb_sh_y_m_res;   //!
   TBranch        *b_g_datatype;   //!
   TBranch        *b_g_evlen;   //!
   TBranch        *b_g_evnum;   //!
   TBranch        *b_g_evtime;   //!
   TBranch        *b_g_evtyp;   //!
   TBranch        *b_g_runnum;   //!
   TBranch        *b_g_runtime;   //!
   TBranch        *b_g_runtype;   //!
   TBranch        *b_g_trigbits;   //!
   TBranch        *b_Event_Branch_fEvtHdr_fEvtTime;   //!
   TBranch        *b_Event_Branch_fEvtHdr_fEvtNum;   //!
   TBranch        *b_Event_Branch_fEvtHdr_fEvtType;   //!
   TBranch        *b_Event_Branch_fEvtHdr_fEvtLen;   //!
   TBranch        *b_Event_Branch_fEvtHdr_fHelicity;   //!
   TBranch        *b_Event_Branch_fEvtHdr_fTrigBits;   //!
   TBranch        *b_Event_Branch_fEvtHdr_fRun;   //!

   cosmic_tree(TTree *tree=0);
   virtual ~cosmic_tree();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef cosmic_tree_cxx
cosmic_tree::cosmic_tree(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("bbshower_1475_1000000_10.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("bbshower_1475_1000000_10.root");
      }
      f->GetObject("T",tree);

   }
   Init(tree);
}

cosmic_tree::~cosmic_tree()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t cosmic_tree::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t cosmic_tree::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void cosmic_tree::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.a", &Ndata_bb_bbtrig_Ref_a, &b_Ndata_bb_bbtrig_Ref_a);
   fChain->SetBranchAddress("bb.bbtrig.Ref.a", &bb_bbtrig_Ref_a, &b_bb_bbtrig_Ref_a);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.a_amp", &Ndata_bb_bbtrig_Ref_a_amp, &b_Ndata_bb_bbtrig_Ref_a_amp);
   fChain->SetBranchAddress("bb.bbtrig.Ref.a_amp", &bb_bbtrig_Ref_a_amp, &b_bb_bbtrig_Ref_a_amp);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.a_amp_c", &Ndata_bb_bbtrig_Ref_a_amp_c, &b_Ndata_bb_bbtrig_Ref_a_amp_c);
   fChain->SetBranchAddress("bb.bbtrig.Ref.a_amp_c", &bb_bbtrig_Ref_a_amp_c, &b_bb_bbtrig_Ref_a_amp_c);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.a_amp_p", &Ndata_bb_bbtrig_Ref_a_amp_p, &b_Ndata_bb_bbtrig_Ref_a_amp_p);
   fChain->SetBranchAddress("bb.bbtrig.Ref.a_amp_p", &bb_bbtrig_Ref_a_amp_p, &b_bb_bbtrig_Ref_a_amp_p);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.a_c", &Ndata_bb_bbtrig_Ref_a_c, &b_Ndata_bb_bbtrig_Ref_a_c);
   fChain->SetBranchAddress("bb.bbtrig.Ref.a_c", &bb_bbtrig_Ref_a_c, &b_bb_bbtrig_Ref_a_c);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.a_mult", &Ndata_bb_bbtrig_Ref_a_mult, &b_Ndata_bb_bbtrig_Ref_a_mult);
   fChain->SetBranchAddress("bb.bbtrig.Ref.a_mult", &bb_bbtrig_Ref_a_mult, &b_bb_bbtrig_Ref_a_mult);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.a_p", &Ndata_bb_bbtrig_Ref_a_p, &b_Ndata_bb_bbtrig_Ref_a_p);
   fChain->SetBranchAddress("bb.bbtrig.Ref.a_p", &bb_bbtrig_Ref_a_p, &b_bb_bbtrig_Ref_a_p);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.a_time", &Ndata_bb_bbtrig_Ref_a_time, &b_Ndata_bb_bbtrig_Ref_a_time);
   fChain->SetBranchAddress("bb.bbtrig.Ref.a_time", &bb_bbtrig_Ref_a_time, &b_bb_bbtrig_Ref_a_time);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.adcelemID", &Ndata_bb_bbtrig_Ref_adcelemID, &b_Ndata_bb_bbtrig_Ref_adcelemID);
   fChain->SetBranchAddress("bb.bbtrig.Ref.adcelemID", &bb_bbtrig_Ref_adcelemID, &b_bb_bbtrig_Ref_adcelemID);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.ped", &Ndata_bb_bbtrig_Ref_ped, &b_Ndata_bb_bbtrig_Ref_ped);
   fChain->SetBranchAddress("bb.bbtrig.Ref.ped", &bb_bbtrig_Ref_ped, &b_bb_bbtrig_Ref_ped);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.tdc", &Ndata_bb_bbtrig_Ref_tdc, &b_Ndata_bb_bbtrig_Ref_tdc);
   fChain->SetBranchAddress("bb.bbtrig.Ref.tdc", bb_bbtrig_Ref_tdc, &b_bb_bbtrig_Ref_tdc);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.tdc_mult", &Ndata_bb_bbtrig_Ref_tdc_mult, &b_Ndata_bb_bbtrig_Ref_tdc_mult);
   fChain->SetBranchAddress("bb.bbtrig.Ref.tdc_mult", bb_bbtrig_Ref_tdc_mult, &b_bb_bbtrig_Ref_tdc_mult);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.tdc_te", &Ndata_bb_bbtrig_Ref_tdc_te, &b_Ndata_bb_bbtrig_Ref_tdc_te);
   fChain->SetBranchAddress("bb.bbtrig.Ref.tdc_te", bb_bbtrig_Ref_tdc_te, &b_bb_bbtrig_Ref_tdc_te);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.tdc_tot", &Ndata_bb_bbtrig_Ref_tdc_tot, &b_Ndata_bb_bbtrig_Ref_tdc_tot);
   fChain->SetBranchAddress("bb.bbtrig.Ref.tdc_tot", bb_bbtrig_Ref_tdc_tot, &b_bb_bbtrig_Ref_tdc_tot);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.Ref.tdcelemID", &Ndata_bb_bbtrig_Ref_tdcelemID, &b_Ndata_bb_bbtrig_Ref_tdcelemID);
   fChain->SetBranchAddress("bb.bbtrig.Ref.tdcelemID", bb_bbtrig_Ref_tdcelemID, &b_bb_bbtrig_Ref_tdcelemID);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a", &Ndata_bb_bbtrig_a, &b_Ndata_bb_bbtrig_a);
   fChain->SetBranchAddress("bb.bbtrig.a", bb_bbtrig_a, &b_bb_bbtrig_a);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_amp", &Ndata_bb_bbtrig_a_amp, &b_Ndata_bb_bbtrig_a_amp);
   fChain->SetBranchAddress("bb.bbtrig.a_amp", bb_bbtrig_a_amp, &b_bb_bbtrig_a_amp);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_amp_c", &Ndata_bb_bbtrig_a_amp_c, &b_Ndata_bb_bbtrig_a_amp_c);
   fChain->SetBranchAddress("bb.bbtrig.a_amp_c", bb_bbtrig_a_amp_c, &b_bb_bbtrig_a_amp_c);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_amp_p", &Ndata_bb_bbtrig_a_amp_p, &b_Ndata_bb_bbtrig_a_amp_p);
   fChain->SetBranchAddress("bb.bbtrig.a_amp_p", bb_bbtrig_a_amp_p, &b_bb_bbtrig_a_amp_p);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_amptrig_c", &Ndata_bb_bbtrig_a_amptrig_c, &b_Ndata_bb_bbtrig_a_amptrig_c);
   fChain->SetBranchAddress("bb.bbtrig.a_amptrig_c", bb_bbtrig_a_amptrig_c, &b_bb_bbtrig_a_amptrig_c);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_amptrig_p", &Ndata_bb_bbtrig_a_amptrig_p, &b_Ndata_bb_bbtrig_a_amptrig_p);
   fChain->SetBranchAddress("bb.bbtrig.a_amptrig_p", bb_bbtrig_a_amptrig_p, &b_bb_bbtrig_a_amptrig_p);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_c", &Ndata_bb_bbtrig_a_c, &b_Ndata_bb_bbtrig_a_c);
   fChain->SetBranchAddress("bb.bbtrig.a_c", bb_bbtrig_a_c, &b_bb_bbtrig_a_c);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_mult", &Ndata_bb_bbtrig_a_mult, &b_Ndata_bb_bbtrig_a_mult);
   fChain->SetBranchAddress("bb.bbtrig.a_mult", bb_bbtrig_a_mult, &b_bb_bbtrig_a_mult);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_p", &Ndata_bb_bbtrig_a_p, &b_Ndata_bb_bbtrig_a_p);
   fChain->SetBranchAddress("bb.bbtrig.a_p", bb_bbtrig_a_p, &b_bb_bbtrig_a_p);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.a_time", &Ndata_bb_bbtrig_a_time, &b_Ndata_bb_bbtrig_a_time);
   fChain->SetBranchAddress("bb.bbtrig.a_time", bb_bbtrig_a_time, &b_bb_bbtrig_a_time);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.adccol", &Ndata_bb_bbtrig_adccol, &b_Ndata_bb_bbtrig_adccol);
   fChain->SetBranchAddress("bb.bbtrig.adccol", bb_bbtrig_adccol, &b_bb_bbtrig_adccol);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.adcelemID", &Ndata_bb_bbtrig_adcelemID, &b_Ndata_bb_bbtrig_adcelemID);
   fChain->SetBranchAddress("bb.bbtrig.adcelemID", bb_bbtrig_adcelemID, &b_bb_bbtrig_adcelemID);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.adclayer", &Ndata_bb_bbtrig_adclayer, &b_Ndata_bb_bbtrig_adclayer);
   fChain->SetBranchAddress("bb.bbtrig.adclayer", bb_bbtrig_adclayer, &b_bb_bbtrig_adclayer);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.adcrow", &Ndata_bb_bbtrig_adcrow, &b_Ndata_bb_bbtrig_adcrow);
   fChain->SetBranchAddress("bb.bbtrig.adcrow", bb_bbtrig_adcrow, &b_bb_bbtrig_adcrow);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.ped", &Ndata_bb_bbtrig_ped, &b_Ndata_bb_bbtrig_ped);
   fChain->SetBranchAddress("bb.bbtrig.ped", bb_bbtrig_ped, &b_bb_bbtrig_ped);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.tdc", &Ndata_bb_bbtrig_tdc, &b_Ndata_bb_bbtrig_tdc);
   fChain->SetBranchAddress("bb.bbtrig.tdc", bb_bbtrig_tdc, &b_bb_bbtrig_tdc);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.tdc_mult", &Ndata_bb_bbtrig_tdc_mult, &b_Ndata_bb_bbtrig_tdc_mult);
   fChain->SetBranchAddress("bb.bbtrig.tdc_mult", bb_bbtrig_tdc_mult, &b_bb_bbtrig_tdc_mult);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.tdc_te", &Ndata_bb_bbtrig_tdc_te, &b_Ndata_bb_bbtrig_tdc_te);
   fChain->SetBranchAddress("bb.bbtrig.tdc_te", bb_bbtrig_tdc_te, &b_bb_bbtrig_tdc_te);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.tdc_tot", &Ndata_bb_bbtrig_tdc_tot, &b_Ndata_bb_bbtrig_tdc_tot);
   fChain->SetBranchAddress("bb.bbtrig.tdc_tot", bb_bbtrig_tdc_tot, &b_bb_bbtrig_tdc_tot);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.tdccol", &Ndata_bb_bbtrig_tdccol, &b_Ndata_bb_bbtrig_tdccol);
   fChain->SetBranchAddress("bb.bbtrig.tdccol", bb_bbtrig_tdccol, &b_bb_bbtrig_tdccol);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.tdcelemID", &Ndata_bb_bbtrig_tdcelemID, &b_Ndata_bb_bbtrig_tdcelemID);
   fChain->SetBranchAddress("bb.bbtrig.tdcelemID", bb_bbtrig_tdcelemID, &b_bb_bbtrig_tdcelemID);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.tdclayer", &Ndata_bb_bbtrig_tdclayer, &b_Ndata_bb_bbtrig_tdclayer);
   fChain->SetBranchAddress("bb.bbtrig.tdclayer", bb_bbtrig_tdclayer, &b_bb_bbtrig_tdclayer);
   fChain->SetBranchAddress("Ndata.bb.bbtrig.tdcrow", &Ndata_bb_bbtrig_tdcrow, &b_Ndata_bb_bbtrig_tdcrow);
   fChain->SetBranchAddress("bb.bbtrig.tdcrow", bb_bbtrig_tdcrow, &b_bb_bbtrig_tdcrow);
   fChain->SetBranchAddress("Ndata.bb.ps.a", &Ndata_bb_ps_a, &b_Ndata_bb_ps_a);
   fChain->SetBranchAddress("bb.ps.a", bb_ps_a, &b_bb_ps_a);
   fChain->SetBranchAddress("Ndata.bb.ps.a_amp", &Ndata_bb_ps_a_amp, &b_Ndata_bb_ps_a_amp);
   fChain->SetBranchAddress("bb.ps.a_amp", bb_ps_a_amp, &b_bb_ps_a_amp);
   fChain->SetBranchAddress("Ndata.bb.ps.a_amp_c", &Ndata_bb_ps_a_amp_c, &b_Ndata_bb_ps_a_amp_c);
   fChain->SetBranchAddress("bb.ps.a_amp_c", bb_ps_a_amp_c, &b_bb_ps_a_amp_c);
   fChain->SetBranchAddress("Ndata.bb.ps.a_amp_p", &Ndata_bb_ps_a_amp_p, &b_Ndata_bb_ps_a_amp_p);
   fChain->SetBranchAddress("bb.ps.a_amp_p", bb_ps_a_amp_p, &b_bb_ps_a_amp_p);
   fChain->SetBranchAddress("Ndata.bb.ps.a_amptrig_c", &Ndata_bb_ps_a_amptrig_c, &b_Ndata_bb_ps_a_amptrig_c);
   fChain->SetBranchAddress("bb.ps.a_amptrig_c", bb_ps_a_amptrig_c, &b_bb_ps_a_amptrig_c);
   fChain->SetBranchAddress("Ndata.bb.ps.a_amptrig_p", &Ndata_bb_ps_a_amptrig_p, &b_Ndata_bb_ps_a_amptrig_p);
   fChain->SetBranchAddress("bb.ps.a_amptrig_p", bb_ps_a_amptrig_p, &b_bb_ps_a_amptrig_p);
   fChain->SetBranchAddress("Ndata.bb.ps.a_c", &Ndata_bb_ps_a_c, &b_Ndata_bb_ps_a_c);
   fChain->SetBranchAddress("bb.ps.a_c", bb_ps_a_c, &b_bb_ps_a_c);
   fChain->SetBranchAddress("Ndata.bb.ps.a_mult", &Ndata_bb_ps_a_mult, &b_Ndata_bb_ps_a_mult);
   fChain->SetBranchAddress("bb.ps.a_mult", bb_ps_a_mult, &b_bb_ps_a_mult);
   fChain->SetBranchAddress("Ndata.bb.ps.a_p", &Ndata_bb_ps_a_p, &b_Ndata_bb_ps_a_p);
   fChain->SetBranchAddress("bb.ps.a_p", bb_ps_a_p, &b_bb_ps_a_p);
   fChain->SetBranchAddress("Ndata.bb.ps.a_time", &Ndata_bb_ps_a_time, &b_Ndata_bb_ps_a_time);
   fChain->SetBranchAddress("bb.ps.a_time", bb_ps_a_time, &b_bb_ps_a_time);
   fChain->SetBranchAddress("Ndata.bb.ps.adccol", &Ndata_bb_ps_adccol, &b_Ndata_bb_ps_adccol);
   fChain->SetBranchAddress("bb.ps.adccol", bb_ps_adccol, &b_bb_ps_adccol);
   fChain->SetBranchAddress("Ndata.bb.ps.adcelemID", &Ndata_bb_ps_adcelemID, &b_Ndata_bb_ps_adcelemID);
   fChain->SetBranchAddress("bb.ps.adcelemID", bb_ps_adcelemID, &b_bb_ps_adcelemID);
   fChain->SetBranchAddress("Ndata.bb.ps.adclayer", &Ndata_bb_ps_adclayer, &b_Ndata_bb_ps_adclayer);
   fChain->SetBranchAddress("bb.ps.adclayer", bb_ps_adclayer, &b_bb_ps_adclayer);
   fChain->SetBranchAddress("Ndata.bb.ps.adcrow", &Ndata_bb_ps_adcrow, &b_Ndata_bb_ps_adcrow);
   fChain->SetBranchAddress("bb.ps.adcrow", bb_ps_adcrow, &b_bb_ps_adcrow);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.atime", &Ndata_bb_ps_clus_atime, &b_Ndata_bb_ps_clus_atime);
   fChain->SetBranchAddress("bb.ps.clus.atime", bb_ps_clus_atime, &b_bb_ps_clus_atime);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.col", &Ndata_bb_ps_clus_col, &b_Ndata_bb_ps_clus_col);
   fChain->SetBranchAddress("bb.ps.clus.col", bb_ps_clus_col, &b_bb_ps_clus_col);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.e", &Ndata_bb_ps_clus_e, &b_Ndata_bb_ps_clus_e);
   fChain->SetBranchAddress("bb.ps.clus.e", bb_ps_clus_e, &b_bb_ps_clus_e);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.e_c", &Ndata_bb_ps_clus_e_c, &b_Ndata_bb_ps_clus_e_c);
   fChain->SetBranchAddress("bb.ps.clus.e_c", bb_ps_clus_e_c, &b_bb_ps_clus_e_c);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.eblk", &Ndata_bb_ps_clus_eblk, &b_Ndata_bb_ps_clus_eblk);
   fChain->SetBranchAddress("bb.ps.clus.eblk", bb_ps_clus_eblk, &b_bb_ps_clus_eblk);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.eblk_c", &Ndata_bb_ps_clus_eblk_c, &b_Ndata_bb_ps_clus_eblk_c);
   fChain->SetBranchAddress("bb.ps.clus.eblk_c", bb_ps_clus_eblk_c, &b_bb_ps_clus_eblk_c);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.id", &Ndata_bb_ps_clus_id, &b_Ndata_bb_ps_clus_id);
   fChain->SetBranchAddress("bb.ps.clus.id", bb_ps_clus_id, &b_bb_ps_clus_id);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.nblk", &Ndata_bb_ps_clus_nblk, &b_Ndata_bb_ps_clus_nblk);
   fChain->SetBranchAddress("bb.ps.clus.nblk", bb_ps_clus_nblk, &b_bb_ps_clus_nblk);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.row", &Ndata_bb_ps_clus_row, &b_Ndata_bb_ps_clus_row);
   fChain->SetBranchAddress("bb.ps.clus.row", bb_ps_clus_row, &b_bb_ps_clus_row);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.tdctime", &Ndata_bb_ps_clus_tdctime, &b_Ndata_bb_ps_clus_tdctime);
   fChain->SetBranchAddress("bb.ps.clus.tdctime", bb_ps_clus_tdctime, &b_bb_ps_clus_tdctime);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.x", &Ndata_bb_ps_clus_x, &b_Ndata_bb_ps_clus_x);
   fChain->SetBranchAddress("bb.ps.clus.x", bb_ps_clus_x, &b_bb_ps_clus_x);
   fChain->SetBranchAddress("Ndata.bb.ps.clus.y", &Ndata_bb_ps_clus_y, &b_Ndata_bb_ps_clus_y);
   fChain->SetBranchAddress("bb.ps.clus.y", bb_ps_clus_y, &b_bb_ps_clus_y);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.atime", &Ndata_bb_ps_clus_blk_atime, &b_Ndata_bb_ps_clus_blk_atime);
   fChain->SetBranchAddress("bb.ps.clus_blk.atime", bb_ps_clus_blk_atime, &b_bb_ps_clus_blk_atime);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.col", &Ndata_bb_ps_clus_blk_col, &b_Ndata_bb_ps_clus_blk_col);
   fChain->SetBranchAddress("bb.ps.clus_blk.col", bb_ps_clus_blk_col, &b_bb_ps_clus_blk_col);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.e", &Ndata_bb_ps_clus_blk_e, &b_Ndata_bb_ps_clus_blk_e);
   fChain->SetBranchAddress("bb.ps.clus_blk.e", bb_ps_clus_blk_e, &b_bb_ps_clus_blk_e);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.e_c", &Ndata_bb_ps_clus_blk_e_c, &b_Ndata_bb_ps_clus_blk_e_c);
   fChain->SetBranchAddress("bb.ps.clus_blk.e_c", bb_ps_clus_blk_e_c, &b_bb_ps_clus_blk_e_c);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.id", &Ndata_bb_ps_clus_blk_id, &b_Ndata_bb_ps_clus_blk_id);
   fChain->SetBranchAddress("bb.ps.clus_blk.id", bb_ps_clus_blk_id, &b_bb_ps_clus_blk_id);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.row", &Ndata_bb_ps_clus_blk_row, &b_Ndata_bb_ps_clus_blk_row);
   fChain->SetBranchAddress("bb.ps.clus_blk.row", bb_ps_clus_blk_row, &b_bb_ps_clus_blk_row);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.tdctime", &Ndata_bb_ps_clus_blk_tdctime, &b_Ndata_bb_ps_clus_blk_tdctime);
   fChain->SetBranchAddress("bb.ps.clus_blk.tdctime", bb_ps_clus_blk_tdctime, &b_bb_ps_clus_blk_tdctime);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.x", &Ndata_bb_ps_clus_blk_x, &b_Ndata_bb_ps_clus_blk_x);
   fChain->SetBranchAddress("bb.ps.clus_blk.x", bb_ps_clus_blk_x, &b_bb_ps_clus_blk_x);
   fChain->SetBranchAddress("Ndata.bb.ps.clus_blk.y", &Ndata_bb_ps_clus_blk_y, &b_Ndata_bb_ps_clus_blk_y);
   fChain->SetBranchAddress("bb.ps.clus_blk.y", bb_ps_clus_blk_y, &b_bb_ps_clus_blk_y);
   fChain->SetBranchAddress("Ndata.bb.ps.e_res", &Ndata_bb_ps_e_res, &b_Ndata_bb_ps_e_res);
   fChain->SetBranchAddress("bb.ps.e_res", bb_ps_e_res, &b_bb_ps_e_res);
   fChain->SetBranchAddress("Ndata.bb.ps.goodblock.atime", &Ndata_bb_ps_goodblock_atime, &b_Ndata_bb_ps_goodblock_atime);
   fChain->SetBranchAddress("bb.ps.goodblock.atime", bb_ps_goodblock_atime, &b_bb_ps_goodblock_atime);
   fChain->SetBranchAddress("Ndata.bb.ps.goodblock.col", &Ndata_bb_ps_goodblock_col, &b_Ndata_bb_ps_goodblock_col);
   fChain->SetBranchAddress("bb.ps.goodblock.col", bb_ps_goodblock_col, &b_bb_ps_goodblock_col);
   fChain->SetBranchAddress("Ndata.bb.ps.goodblock.e", &Ndata_bb_ps_goodblock_e, &b_Ndata_bb_ps_goodblock_e);
   fChain->SetBranchAddress("bb.ps.goodblock.e", bb_ps_goodblock_e, &b_bb_ps_goodblock_e);
   fChain->SetBranchAddress("Ndata.bb.ps.goodblock.id", &Ndata_bb_ps_goodblock_id, &b_Ndata_bb_ps_goodblock_id);
   fChain->SetBranchAddress("bb.ps.goodblock.id", bb_ps_goodblock_id, &b_bb_ps_goodblock_id);
   fChain->SetBranchAddress("Ndata.bb.ps.goodblock.row", &Ndata_bb_ps_goodblock_row, &b_Ndata_bb_ps_goodblock_row);
   fChain->SetBranchAddress("bb.ps.goodblock.row", bb_ps_goodblock_row, &b_bb_ps_goodblock_row);
   fChain->SetBranchAddress("Ndata.bb.ps.goodblock.tdctime", &Ndata_bb_ps_goodblock_tdctime, &b_Ndata_bb_ps_goodblock_tdctime);
   fChain->SetBranchAddress("bb.ps.goodblock.tdctime", bb_ps_goodblock_tdctime, &b_bb_ps_goodblock_tdctime);
   fChain->SetBranchAddress("Ndata.bb.ps.goodblock.x", &Ndata_bb_ps_goodblock_x, &b_Ndata_bb_ps_goodblock_x);
   fChain->SetBranchAddress("bb.ps.goodblock.x", bb_ps_goodblock_x, &b_bb_ps_goodblock_x);
   fChain->SetBranchAddress("Ndata.bb.ps.goodblock.y", &Ndata_bb_ps_goodblock_y, &b_Ndata_bb_ps_goodblock_y);
   fChain->SetBranchAddress("bb.ps.goodblock.y", bb_ps_goodblock_y, &b_bb_ps_goodblock_y);
   fChain->SetBranchAddress("Ndata.bb.ps.hits.a", &Ndata_bb_ps_hits_a, &b_Ndata_bb_ps_hits_a);
   fChain->SetBranchAddress("bb.ps.hits.a", &bb_ps_hits_a, &b_bb_ps_hits_a);
   fChain->SetBranchAddress("Ndata.bb.ps.hits.a_amp", &Ndata_bb_ps_hits_a_amp, &b_Ndata_bb_ps_hits_a_amp);
   fChain->SetBranchAddress("bb.ps.hits.a_amp", &bb_ps_hits_a_amp, &b_bb_ps_hits_a_amp);
   fChain->SetBranchAddress("Ndata.bb.ps.hits.a_time", &Ndata_bb_ps_hits_a_time, &b_Ndata_bb_ps_hits_a_time);
   fChain->SetBranchAddress("bb.ps.hits.a_time", &bb_ps_hits_a_time, &b_bb_ps_hits_a_time);
   fChain->SetBranchAddress("Ndata.bb.ps.nsamps", &Ndata_bb_ps_nsamps, &b_Ndata_bb_ps_nsamps);
   fChain->SetBranchAddress("bb.ps.nsamps", bb_ps_nsamps, &b_bb_ps_nsamps);
   fChain->SetBranchAddress("Ndata.bb.ps.ped", &Ndata_bb_ps_ped, &b_Ndata_bb_ps_ped);
   fChain->SetBranchAddress("bb.ps.ped", bb_ps_ped, &b_bb_ps_ped);
   fChain->SetBranchAddress("Ndata.bb.ps.samps", &Ndata_bb_ps_samps, &b_Ndata_bb_ps_samps);
   fChain->SetBranchAddress("bb.ps.samps", bb_ps_samps, &b_bb_ps_samps);
   fChain->SetBranchAddress("Ndata.bb.ps.samps_elemID", &Ndata_bb_ps_samps_elemID, &b_Ndata_bb_ps_samps_elemID);
   fChain->SetBranchAddress("bb.ps.samps_elemID", bb_ps_samps_elemID, &b_bb_ps_samps_elemID);
   fChain->SetBranchAddress("Ndata.bb.ps.samps_idx", &Ndata_bb_ps_samps_idx, &b_Ndata_bb_ps_samps_idx);
   fChain->SetBranchAddress("bb.ps.samps_idx", bb_ps_samps_idx, &b_bb_ps_samps_idx);
   fChain->SetBranchAddress("Ndata.bb.ps.x_res", &Ndata_bb_ps_x_res, &b_Ndata_bb_ps_x_res);
   fChain->SetBranchAddress("bb.ps.x_res", bb_ps_x_res, &b_bb_ps_x_res);
   fChain->SetBranchAddress("Ndata.bb.ps.y_res", &Ndata_bb_ps_y_res, &b_Ndata_bb_ps_y_res);
   fChain->SetBranchAddress("bb.ps.y_res", bb_ps_y_res, &b_bb_ps_y_res);
   fChain->SetBranchAddress("Ndata.bb.sh.a", &Ndata_bb_sh_a, &b_Ndata_bb_sh_a);
   fChain->SetBranchAddress("bb.sh.a", bb_sh_a, &b_bb_sh_a);
   fChain->SetBranchAddress("Ndata.bb.sh.a_amp", &Ndata_bb_sh_a_amp, &b_Ndata_bb_sh_a_amp);
   fChain->SetBranchAddress("bb.sh.a_amp", bb_sh_a_amp, &b_bb_sh_a_amp);
   fChain->SetBranchAddress("Ndata.bb.sh.a_amp_c", &Ndata_bb_sh_a_amp_c, &b_Ndata_bb_sh_a_amp_c);
   fChain->SetBranchAddress("bb.sh.a_amp_c", bb_sh_a_amp_c, &b_bb_sh_a_amp_c);
   fChain->SetBranchAddress("Ndata.bb.sh.a_amp_p", &Ndata_bb_sh_a_amp_p, &b_Ndata_bb_sh_a_amp_p);
   fChain->SetBranchAddress("bb.sh.a_amp_p", bb_sh_a_amp_p, &b_bb_sh_a_amp_p);
   fChain->SetBranchAddress("Ndata.bb.sh.a_amptrig_c", &Ndata_bb_sh_a_amptrig_c, &b_Ndata_bb_sh_a_amptrig_c);
   fChain->SetBranchAddress("bb.sh.a_amptrig_c", bb_sh_a_amptrig_c, &b_bb_sh_a_amptrig_c);
   fChain->SetBranchAddress("Ndata.bb.sh.a_amptrig_p", &Ndata_bb_sh_a_amptrig_p, &b_Ndata_bb_sh_a_amptrig_p);
   fChain->SetBranchAddress("bb.sh.a_amptrig_p", bb_sh_a_amptrig_p, &b_bb_sh_a_amptrig_p);
   fChain->SetBranchAddress("Ndata.bb.sh.a_c", &Ndata_bb_sh_a_c, &b_Ndata_bb_sh_a_c);
   fChain->SetBranchAddress("bb.sh.a_c", bb_sh_a_c, &b_bb_sh_a_c);
   fChain->SetBranchAddress("Ndata.bb.sh.a_mult", &Ndata_bb_sh_a_mult, &b_Ndata_bb_sh_a_mult);
   fChain->SetBranchAddress("bb.sh.a_mult", bb_sh_a_mult, &b_bb_sh_a_mult);
   fChain->SetBranchAddress("Ndata.bb.sh.a_p", &Ndata_bb_sh_a_p, &b_Ndata_bb_sh_a_p);
   fChain->SetBranchAddress("bb.sh.a_p", bb_sh_a_p, &b_bb_sh_a_p);
   fChain->SetBranchAddress("Ndata.bb.sh.a_time", &Ndata_bb_sh_a_time, &b_Ndata_bb_sh_a_time);
   fChain->SetBranchAddress("bb.sh.a_time", bb_sh_a_time, &b_bb_sh_a_time);
   fChain->SetBranchAddress("Ndata.bb.sh.adccol", &Ndata_bb_sh_adccol, &b_Ndata_bb_sh_adccol);
   fChain->SetBranchAddress("bb.sh.adccol", bb_sh_adccol, &b_bb_sh_adccol);
   fChain->SetBranchAddress("Ndata.bb.sh.adcelemID", &Ndata_bb_sh_adcelemID, &b_Ndata_bb_sh_adcelemID);
   fChain->SetBranchAddress("bb.sh.adcelemID", bb_sh_adcelemID, &b_bb_sh_adcelemID);
   fChain->SetBranchAddress("Ndata.bb.sh.adclayer", &Ndata_bb_sh_adclayer, &b_Ndata_bb_sh_adclayer);
   fChain->SetBranchAddress("bb.sh.adclayer", bb_sh_adclayer, &b_bb_sh_adclayer);
   fChain->SetBranchAddress("Ndata.bb.sh.adcrow", &Ndata_bb_sh_adcrow, &b_Ndata_bb_sh_adcrow);
   fChain->SetBranchAddress("bb.sh.adcrow", bb_sh_adcrow, &b_bb_sh_adcrow);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.atime", &Ndata_bb_sh_clus_atime, &b_Ndata_bb_sh_clus_atime);
   fChain->SetBranchAddress("bb.sh.clus.atime", bb_sh_clus_atime, &b_bb_sh_clus_atime);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.col", &Ndata_bb_sh_clus_col, &b_Ndata_bb_sh_clus_col);
   fChain->SetBranchAddress("bb.sh.clus.col", bb_sh_clus_col, &b_bb_sh_clus_col);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.e", &Ndata_bb_sh_clus_e, &b_Ndata_bb_sh_clus_e);
   fChain->SetBranchAddress("bb.sh.clus.e", bb_sh_clus_e, &b_bb_sh_clus_e);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.e_c", &Ndata_bb_sh_clus_e_c, &b_Ndata_bb_sh_clus_e_c);
   fChain->SetBranchAddress("bb.sh.clus.e_c", bb_sh_clus_e_c, &b_bb_sh_clus_e_c);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.eblk", &Ndata_bb_sh_clus_eblk, &b_Ndata_bb_sh_clus_eblk);
   fChain->SetBranchAddress("bb.sh.clus.eblk", bb_sh_clus_eblk, &b_bb_sh_clus_eblk);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.eblk_c", &Ndata_bb_sh_clus_eblk_c, &b_Ndata_bb_sh_clus_eblk_c);
   fChain->SetBranchAddress("bb.sh.clus.eblk_c", bb_sh_clus_eblk_c, &b_bb_sh_clus_eblk_c);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.id", &Ndata_bb_sh_clus_id, &b_Ndata_bb_sh_clus_id);
   fChain->SetBranchAddress("bb.sh.clus.id", bb_sh_clus_id, &b_bb_sh_clus_id);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.nblk", &Ndata_bb_sh_clus_nblk, &b_Ndata_bb_sh_clus_nblk);
   fChain->SetBranchAddress("bb.sh.clus.nblk", bb_sh_clus_nblk, &b_bb_sh_clus_nblk);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.row", &Ndata_bb_sh_clus_row, &b_Ndata_bb_sh_clus_row);
   fChain->SetBranchAddress("bb.sh.clus.row", bb_sh_clus_row, &b_bb_sh_clus_row);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.tdctime", &Ndata_bb_sh_clus_tdctime, &b_Ndata_bb_sh_clus_tdctime);
   fChain->SetBranchAddress("bb.sh.clus.tdctime", bb_sh_clus_tdctime, &b_bb_sh_clus_tdctime);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.x", &Ndata_bb_sh_clus_x, &b_Ndata_bb_sh_clus_x);
   fChain->SetBranchAddress("bb.sh.clus.x", bb_sh_clus_x, &b_bb_sh_clus_x);
   fChain->SetBranchAddress("Ndata.bb.sh.clus.y", &Ndata_bb_sh_clus_y, &b_Ndata_bb_sh_clus_y);
   fChain->SetBranchAddress("bb.sh.clus.y", bb_sh_clus_y, &b_bb_sh_clus_y);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.atime", &Ndata_bb_sh_clus_blk_atime, &b_Ndata_bb_sh_clus_blk_atime);
   fChain->SetBranchAddress("bb.sh.clus_blk.atime", bb_sh_clus_blk_atime, &b_bb_sh_clus_blk_atime);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.col", &Ndata_bb_sh_clus_blk_col, &b_Ndata_bb_sh_clus_blk_col);
   fChain->SetBranchAddress("bb.sh.clus_blk.col", bb_sh_clus_blk_col, &b_bb_sh_clus_blk_col);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.e", &Ndata_bb_sh_clus_blk_e, &b_Ndata_bb_sh_clus_blk_e);
   fChain->SetBranchAddress("bb.sh.clus_blk.e", bb_sh_clus_blk_e, &b_bb_sh_clus_blk_e);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.e_c", &Ndata_bb_sh_clus_blk_e_c, &b_Ndata_bb_sh_clus_blk_e_c);
   fChain->SetBranchAddress("bb.sh.clus_blk.e_c", bb_sh_clus_blk_e_c, &b_bb_sh_clus_blk_e_c);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.id", &Ndata_bb_sh_clus_blk_id, &b_Ndata_bb_sh_clus_blk_id);
   fChain->SetBranchAddress("bb.sh.clus_blk.id", bb_sh_clus_blk_id, &b_bb_sh_clus_blk_id);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.row", &Ndata_bb_sh_clus_blk_row, &b_Ndata_bb_sh_clus_blk_row);
   fChain->SetBranchAddress("bb.sh.clus_blk.row", bb_sh_clus_blk_row, &b_bb_sh_clus_blk_row);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.tdctime", &Ndata_bb_sh_clus_blk_tdctime, &b_Ndata_bb_sh_clus_blk_tdctime);
   fChain->SetBranchAddress("bb.sh.clus_blk.tdctime", bb_sh_clus_blk_tdctime, &b_bb_sh_clus_blk_tdctime);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.x", &Ndata_bb_sh_clus_blk_x, &b_Ndata_bb_sh_clus_blk_x);
   fChain->SetBranchAddress("bb.sh.clus_blk.x", bb_sh_clus_blk_x, &b_bb_sh_clus_blk_x);
   fChain->SetBranchAddress("Ndata.bb.sh.clus_blk.y", &Ndata_bb_sh_clus_blk_y, &b_Ndata_bb_sh_clus_blk_y);
   fChain->SetBranchAddress("bb.sh.clus_blk.y", bb_sh_clus_blk_y, &b_bb_sh_clus_blk_y);
   fChain->SetBranchAddress("Ndata.bb.sh.e_res", &Ndata_bb_sh_e_res, &b_Ndata_bb_sh_e_res);
   fChain->SetBranchAddress("bb.sh.e_res", bb_sh_e_res, &b_bb_sh_e_res);
   fChain->SetBranchAddress("Ndata.bb.sh.goodblock.atime", &Ndata_bb_sh_goodblock_atime, &b_Ndata_bb_sh_goodblock_atime);
   fChain->SetBranchAddress("bb.sh.goodblock.atime", bb_sh_goodblock_atime, &b_bb_sh_goodblock_atime);
   fChain->SetBranchAddress("Ndata.bb.sh.goodblock.col", &Ndata_bb_sh_goodblock_col, &b_Ndata_bb_sh_goodblock_col);
   fChain->SetBranchAddress("bb.sh.goodblock.col", bb_sh_goodblock_col, &b_bb_sh_goodblock_col);
   fChain->SetBranchAddress("Ndata.bb.sh.goodblock.e", &Ndata_bb_sh_goodblock_e, &b_Ndata_bb_sh_goodblock_e);
   fChain->SetBranchAddress("bb.sh.goodblock.e", bb_sh_goodblock_e, &b_bb_sh_goodblock_e);
   fChain->SetBranchAddress("Ndata.bb.sh.goodblock.id", &Ndata_bb_sh_goodblock_id, &b_Ndata_bb_sh_goodblock_id);
   fChain->SetBranchAddress("bb.sh.goodblock.id", bb_sh_goodblock_id, &b_bb_sh_goodblock_id);
   fChain->SetBranchAddress("Ndata.bb.sh.goodblock.row", &Ndata_bb_sh_goodblock_row, &b_Ndata_bb_sh_goodblock_row);
   fChain->SetBranchAddress("bb.sh.goodblock.row", bb_sh_goodblock_row, &b_bb_sh_goodblock_row);
   fChain->SetBranchAddress("Ndata.bb.sh.goodblock.tdctime", &Ndata_bb_sh_goodblock_tdctime, &b_Ndata_bb_sh_goodblock_tdctime);
   fChain->SetBranchAddress("bb.sh.goodblock.tdctime", bb_sh_goodblock_tdctime, &b_bb_sh_goodblock_tdctime);
   fChain->SetBranchAddress("Ndata.bb.sh.goodblock.x", &Ndata_bb_sh_goodblock_x, &b_Ndata_bb_sh_goodblock_x);
   fChain->SetBranchAddress("bb.sh.goodblock.x", bb_sh_goodblock_x, &b_bb_sh_goodblock_x);
   fChain->SetBranchAddress("Ndata.bb.sh.goodblock.y", &Ndata_bb_sh_goodblock_y, &b_Ndata_bb_sh_goodblock_y);
   fChain->SetBranchAddress("bb.sh.goodblock.y", bb_sh_goodblock_y, &b_bb_sh_goodblock_y);
   fChain->SetBranchAddress("Ndata.bb.sh.hits.a", &Ndata_bb_sh_hits_a, &b_Ndata_bb_sh_hits_a);
   fChain->SetBranchAddress("bb.sh.hits.a", &bb_sh_hits_a, &b_bb_sh_hits_a);
   fChain->SetBranchAddress("Ndata.bb.sh.hits.a_amp", &Ndata_bb_sh_hits_a_amp, &b_Ndata_bb_sh_hits_a_amp);
   fChain->SetBranchAddress("bb.sh.hits.a_amp", &bb_sh_hits_a_amp, &b_bb_sh_hits_a_amp);
   fChain->SetBranchAddress("Ndata.bb.sh.hits.a_time", &Ndata_bb_sh_hits_a_time, &b_Ndata_bb_sh_hits_a_time);
   fChain->SetBranchAddress("bb.sh.hits.a_time", &bb_sh_hits_a_time, &b_bb_sh_hits_a_time);
   fChain->SetBranchAddress("Ndata.bb.sh.nsamps", &Ndata_bb_sh_nsamps, &b_Ndata_bb_sh_nsamps);
   fChain->SetBranchAddress("bb.sh.nsamps", bb_sh_nsamps, &b_bb_sh_nsamps);
   fChain->SetBranchAddress("Ndata.bb.sh.ped", &Ndata_bb_sh_ped, &b_Ndata_bb_sh_ped);
   fChain->SetBranchAddress("bb.sh.ped", bb_sh_ped, &b_bb_sh_ped);
   fChain->SetBranchAddress("Ndata.bb.sh.samps", &Ndata_bb_sh_samps, &b_Ndata_bb_sh_samps);
   fChain->SetBranchAddress("bb.sh.samps", bb_sh_samps, &b_bb_sh_samps);
   fChain->SetBranchAddress("Ndata.bb.sh.samps_elemID", &Ndata_bb_sh_samps_elemID, &b_Ndata_bb_sh_samps_elemID);
   fChain->SetBranchAddress("bb.sh.samps_elemID", bb_sh_samps_elemID, &b_bb_sh_samps_elemID);
   fChain->SetBranchAddress("Ndata.bb.sh.samps_idx", &Ndata_bb_sh_samps_idx, &b_Ndata_bb_sh_samps_idx);
   fChain->SetBranchAddress("bb.sh.samps_idx", bb_sh_samps_idx, &b_bb_sh_samps_idx);
   fChain->SetBranchAddress("Ndata.bb.sh.x_res", &Ndata_bb_sh_x_res, &b_Ndata_bb_sh_x_res);
   fChain->SetBranchAddress("bb.sh.x_res", bb_sh_x_res, &b_bb_sh_x_res);
   fChain->SetBranchAddress("Ndata.bb.sh.y_res", &Ndata_bb_sh_y_res, &b_Ndata_bb_sh_y_res);
   fChain->SetBranchAddress("bb.sh.y_res", bb_sh_y_res, &b_bb_sh_y_res);
   fChain->SetBranchAddress("bb.bbtrig.ngoodADChits", &bb_bbtrig_ngoodADChits, &b_bb_bbtrig_ngoodADChits);
   fChain->SetBranchAddress("bb.bbtrig.ngoodTDChits", &bb_bbtrig_ngoodTDChits, &b_bb_bbtrig_ngoodTDChits);
   fChain->SetBranchAddress("bb.bbtrig.nhits", &bb_bbtrig_nhits, &b_bb_bbtrig_nhits);
   fChain->SetBranchAddress("bb.bbtrig.nrefhits", &bb_bbtrig_nrefhits, &b_bb_bbtrig_nrefhits);
   fChain->SetBranchAddress("bb.ps.atimeblk", &bb_ps_atimeblk, &b_bb_ps_atimeblk);
   fChain->SetBranchAddress("bb.ps.colblk", &bb_ps_colblk, &b_bb_ps_colblk);
   fChain->SetBranchAddress("bb.ps.e", &bb_ps_e, &b_bb_ps_e);
   fChain->SetBranchAddress("bb.ps.e_c", &bb_ps_e_c, &b_bb_ps_e_c);
   fChain->SetBranchAddress("bb.ps.e_m_res", &bb_ps_e_m_res, &b_bb_ps_e_m_res);
   fChain->SetBranchAddress("bb.ps.eblk", &bb_ps_eblk, &b_bb_ps_eblk);
   fChain->SetBranchAddress("bb.ps.eblk_c", &bb_ps_eblk_c, &b_bb_ps_eblk_c);
   fChain->SetBranchAddress("bb.ps.idblk", &bb_ps_idblk, &b_bb_ps_idblk);
   fChain->SetBranchAddress("bb.ps.index", &bb_ps_index, &b_bb_ps_index);
   fChain->SetBranchAddress("bb.ps.nblk", &bb_ps_nblk, &b_bb_ps_nblk);
   fChain->SetBranchAddress("bb.ps.nclus", &bb_ps_nclus, &b_bb_ps_nclus);
   fChain->SetBranchAddress("bb.ps.ngoodADChits", &bb_ps_ngoodADChits, &b_bb_ps_ngoodADChits);
   fChain->SetBranchAddress("bb.ps.ngoodTDChits", &bb_ps_ngoodTDChits, &b_bb_ps_ngoodTDChits);
   fChain->SetBranchAddress("bb.ps.nhits", &bb_ps_nhits, &b_bb_ps_nhits);
   fChain->SetBranchAddress("bb.ps.nrefhits", &bb_ps_nrefhits, &b_bb_ps_nrefhits);
   fChain->SetBranchAddress("bb.ps.rowblk", &bb_ps_rowblk, &b_bb_ps_rowblk);
   fChain->SetBranchAddress("bb.ps.tdctimeblk", &bb_ps_tdctimeblk, &b_bb_ps_tdctimeblk);
   fChain->SetBranchAddress("bb.ps.x", &bb_ps_x, &b_bb_ps_x);
   fChain->SetBranchAddress("bb.ps.x_m_res", &bb_ps_x_m_res, &b_bb_ps_x_m_res);
   fChain->SetBranchAddress("bb.ps.y", &bb_ps_y, &b_bb_ps_y);
   fChain->SetBranchAddress("bb.ps.y_m_res", &bb_ps_y_m_res, &b_bb_ps_y_m_res);
   fChain->SetBranchAddress("bb.sh.atimeblk", &bb_sh_atimeblk, &b_bb_sh_atimeblk);
   fChain->SetBranchAddress("bb.sh.colblk", &bb_sh_colblk, &b_bb_sh_colblk);
   fChain->SetBranchAddress("bb.sh.e", &bb_sh_e, &b_bb_sh_e);
   fChain->SetBranchAddress("bb.sh.e_c", &bb_sh_e_c, &b_bb_sh_e_c);
   fChain->SetBranchAddress("bb.sh.e_m_res", &bb_sh_e_m_res, &b_bb_sh_e_m_res);
   fChain->SetBranchAddress("bb.sh.eblk", &bb_sh_eblk, &b_bb_sh_eblk);
   fChain->SetBranchAddress("bb.sh.eblk_c", &bb_sh_eblk_c, &b_bb_sh_eblk_c);
   fChain->SetBranchAddress("bb.sh.idblk", &bb_sh_idblk, &b_bb_sh_idblk);
   fChain->SetBranchAddress("bb.sh.index", &bb_sh_index, &b_bb_sh_index);
   fChain->SetBranchAddress("bb.sh.nblk", &bb_sh_nblk, &b_bb_sh_nblk);
   fChain->SetBranchAddress("bb.sh.nclus", &bb_sh_nclus, &b_bb_sh_nclus);
   fChain->SetBranchAddress("bb.sh.ngoodADChits", &bb_sh_ngoodADChits, &b_bb_sh_ngoodADChits);
   fChain->SetBranchAddress("bb.sh.ngoodTDChits", &bb_sh_ngoodTDChits, &b_bb_sh_ngoodTDChits);
   fChain->SetBranchAddress("bb.sh.nhits", &bb_sh_nhits, &b_bb_sh_nhits);
   fChain->SetBranchAddress("bb.sh.nrefhits", &bb_sh_nrefhits, &b_bb_sh_nrefhits);
   fChain->SetBranchAddress("bb.sh.rowblk", &bb_sh_rowblk, &b_bb_sh_rowblk);
   fChain->SetBranchAddress("bb.sh.tdctimeblk", &bb_sh_tdctimeblk, &b_bb_sh_tdctimeblk);
   fChain->SetBranchAddress("bb.sh.x", &bb_sh_x, &b_bb_sh_x);
   fChain->SetBranchAddress("bb.sh.x_m_res", &bb_sh_x_m_res, &b_bb_sh_x_m_res);
   fChain->SetBranchAddress("bb.sh.y", &bb_sh_y, &b_bb_sh_y);
   fChain->SetBranchAddress("bb.sh.y_m_res", &bb_sh_y_m_res, &b_bb_sh_y_m_res);
   fChain->SetBranchAddress("g.datatype", &g_datatype, &b_g_datatype);
   fChain->SetBranchAddress("g.evlen", &g_evlen, &b_g_evlen);
   fChain->SetBranchAddress("g.evnum", &g_evnum, &b_g_evnum);
   fChain->SetBranchAddress("g.evtime", &g_evtime, &b_g_evtime);
   fChain->SetBranchAddress("g.evtyp", &g_evtyp, &b_g_evtyp);
   fChain->SetBranchAddress("g.runnum", &g_runnum, &b_g_runnum);
   fChain->SetBranchAddress("g.runtime", &g_runtime, &b_g_runtime);
   fChain->SetBranchAddress("g.runtype", &g_runtype, &b_g_runtype);
   fChain->SetBranchAddress("g.trigbits", &g_trigbits, &b_g_trigbits);
   fChain->SetBranchAddress("fEvtHdr.fEvtTime", &fEvtHdr_fEvtTime, &b_Event_Branch_fEvtHdr_fEvtTime);
   fChain->SetBranchAddress("fEvtHdr.fEvtNum", &fEvtHdr_fEvtNum, &b_Event_Branch_fEvtHdr_fEvtNum);
   fChain->SetBranchAddress("fEvtHdr.fEvtType", &fEvtHdr_fEvtType, &b_Event_Branch_fEvtHdr_fEvtType);
   fChain->SetBranchAddress("fEvtHdr.fEvtLen", &fEvtHdr_fEvtLen, &b_Event_Branch_fEvtHdr_fEvtLen);
   fChain->SetBranchAddress("fEvtHdr.fHelicity", &fEvtHdr_fHelicity, &b_Event_Branch_fEvtHdr_fHelicity);
   fChain->SetBranchAddress("fEvtHdr.fTrigBits", &fEvtHdr_fTrigBits, &b_Event_Branch_fEvtHdr_fTrigBits);
   fChain->SetBranchAddress("fEvtHdr.fRun", &fEvtHdr_fRun, &b_Event_Branch_fEvtHdr_fRun);
   Notify();
}

Bool_t cosmic_tree::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void cosmic_tree::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t cosmic_tree::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef cosmic_tree_cxx
