#include "TChain.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TString.h"
#include <iostream>
#include <fstream>

const double Mp = 0.938272;
const double Mn = 0.939565;

void plot_BB_HCAL_correlations( const char *rootfilename1, const char *rootfilename2, const char *outfilename, double ebeam=5.965, 
				double bbtheta=26.5, double sbstheta=29.9, double hcaldist=11.0, double dx0=0.0, double dy0=0.0, double dxsigma=0.08, double dysigma=0.08, double Wmin=0.6, double Wmax=1.2, double dpel_min=-0.06, double dpel_max=0.06 ){
  //ifstream infile(configfilename);

  TChain *C = new TChain("T");

  C->Add(rootfilename1);
  C->Add(rootfilename2);

  // TString currentline;

  // while( currentline.ReadLine(infile) && !currentline.BeginsWith("endlist") ){
  //   if( !currentline.BeginsWith("#") ){
  //     C->Add(currentline.Data());
  //   }
  // }

  // double ebeam = 3.7278;
  bbtheta *= TMath::DegToRad();
  sbstheta *= TMath::DegToRad();
  //double hcaldist = 11.0; //m
  double hcalheight = 0.365; //m (we are guessing that this is the height of the center of HCAL above beam height:

  //The following are the positions of the "first" row and column from HCAL database (top right block as viewed from upstream)
  double xoff_hcal = 0.92835;
  double yoff_hcal = 0.47305;
  
  double blockspace_hcal = 0.15254;

  int nrows_hcal=24;
  int ncols_hcal=12;
  
  
  double Wmin_elastic = Wmin;
  double Wmax_elastic = Wmax;
  
  //HCAL variables:
  double xHCAL, yHCAL, EHCAL;
  
  //BigBite track variables:
  double ntrack;
  int MAXNTRACKS=1000;
  //double trackX[MAXNTRACKS], trackY[MAXNTRACKS], trackXp[MAXNTRACKS],
  double px[MAXNTRACKS], py[MAXNTRACKS], pz[MAXNTRACKS], p[MAXNTRACKS];
  
  double vx[MAXNTRACKS], vy[MAXNTRACKS], vz[MAXNTRACKS];

  double tracknhits[MAXNTRACKS];

  //BigBite shower/preshower variables:
  double Eps_BB, Esh_BB;
  double xps_BB, yps_BB, xsh_BB, ysh_BB;

  C->SetBranchStatus("*",0);
  C->SetBranchStatus("sbs.hcal.x",1);
  C->SetBranchStatus("sbs.hcal.y",1);
  C->SetBranchStatus("sbs.hcal.e",1);
  C->SetBranchStatus("bb.tr.n",1);
  C->SetBranchStatus("bb.gem.track.nhits",1);
  C->SetBranchStatus("bb.tr.px",1);
  C->SetBranchStatus("bb.tr.py",1);
  C->SetBranchStatus("bb.tr.pz",1);
  C->SetBranchStatus("bb.tr.vx",1);
  C->SetBranchStatus("bb.tr.vy",1);
  C->SetBranchStatus("bb.tr.vz",1);
  C->SetBranchStatus("bb.ps.e",1);
  C->SetBranchStatus("bb.ps.x",1);
  C->SetBranchStatus("bb.ps.y",1);
  C->SetBranchStatus("bb.sh.e",1);
  C->SetBranchStatus("bb.sh.x",1);
  C->SetBranchStatus("bb.sh.y",1);

  C->SetBranchAddress("sbs.hcal.x",&xHCAL);
  C->SetBranchAddress("sbs.hcal.y",&yHCAL);
  C->SetBranchAddress("sbs.hcal.e",&EHCAL);

  C->SetBranchAddress("bb.tr.n",&ntrack);
  C->SetBranchAddress("bb.gem.track.nhits",&tracknhits);
  C->SetBranchAddress("bb.tr.px",px);
  C->SetBranchAddress("bb.tr.py",py);
  C->SetBranchAddress("bb.tr.pz",pz);
  C->SetBranchAddress("bb.tr.p",p);
  C->SetBranchAddress("bb.tr.vx",vx);
  C->SetBranchAddress("bb.tr.vy",vy);
  C->SetBranchAddress("bb.tr.vz",vz);
  C->SetBranchAddress("bb.ps.e",&Eps_BB);
  C->SetBranchAddress("bb.sh.e",&Esh_BB);
  C->SetBranchAddress("bb.ps.x",&xps_BB);
  C->SetBranchAddress("bb.ps.y",&yps_BB);
  C->SetBranchAddress("bb.sh.x",&xsh_BB);
  C->SetBranchAddress("bb.sh.y",&ysh_BB);

  TFile *fout = new TFile(outfilename,"RECREATE");

  // TTree *Tout = new TTree("Tout","BigBite HCAL elastic ep correlation");

  // double 

  TH1D *hdpel = new TH1D("hdpel",";p/p_{elastic}(#theta)-1;", 250, -1.0, 0.5);
  TH1D *hW = new TH1D("hW",";W (GeV);", 400,0.0,4.0);

  TH1D *hdpel_cutBBCAL = new TH1D("hdpel_cutBBCAL","",250, -1.0,0.5);
  TH1D *hW_cutBBCAL = new TH1D("hW_cutBBCAL","",400,0.0,4.0);
  
  TH1D *hdx_HCAL = new TH1D("hdx_HCAL",";x_{HCAL}-x_{expect} (m);", 500, -2.5, 2.5);
  TH1D *hdy_HCAL = new TH1D("hdy_HCAL",";y_{HCAL}-y_{expect} (m);", 500, -1.25, 1.25);
  TH2D *hdxdy_HCAL = new TH2D("hdxdy_HCAL",";y_{HCAL}-y_{expect} (m); x_{HCAL}-x_{expect} (m)", 250, -1.25, 1.25, 250, -2.5, 2.5 );
  TH2D *hxcorr_HCAL = new TH2D("hxcorr_HCAL",";x_{expect} (m);x_{HCAL} (m)", 250, -2.5, 2.5, 250, -2.5, 2.5 );
  TH2D *hycorr_HCAL = new TH2D("hycorr_HCAL",";y_{expect} (m);y_{HCAL} (m)", 250, -1.25, 1.25, 250, -1.25, 1.25);

  TH1D *hvz = new TH1D("hvz","",250,-0.15,0.15);

  TH2D *hdy_HCAL_vs_z = new TH2D("hdy_HCAL_vs_z","",250,-0.15,0.15,250,-1.25,1.25);
  TH2D *hdy_HCAL_vs_ptheta = new TH2D("hdy_HCAL_vs_ptheta","",250,sbstheta-0.3,sbstheta+0.3,250,-1.25,1.25);
  
  TH1D *hE_HCAL = new TH1D("hE_HCAL",";HCAL E (GeV);",250,0.0,1.0);
  TH1D *hE_HCAL_cut = new TH1D("hE_HCAL_cut",";HCAL E (GeV);",250,0.0,1.0);
  
  TH1D *hW_cut_HCAL = new TH1D("hW_cut_HCAL",";W (GeV);", 400,0.0,4.0);
  TH1D *hdpel_cut_HCAL = new TH1D("hdpel_cut_HCAL",";p/p_{elastic}(#theta)-1;", 250,-0.5,0.5);

  TH1D *hE_preshower = new TH1D("hE_preshower",";E_{PS} (GeV);",250,0.0,1.25);
  TH1D *hEoverP = new TH1D("hEoverP",";E/p;",250,0.0,2.0);
  TH2D *hEoverP_vs_preshower = new TH2D("hEoverP_vs_preshower",";E_{PS} (GeV);E/p",125,0.0,1.25,125,0.0,2.0);

  TH1D *hE_preshower_cut = new TH1D("hE_preshower_cut",";E_{PS} (GeV);",250,0.0,1.25);
  TH1D *hEoverP_cut = new TH1D("hEoverP_cut",";E/p;",250,0.0,2.0);
  TH2D *hEoverP_vs_preshower_cut = new TH2D("hEoverP_vs_preshower_cut",";E_{PS} (GeV);E/p",125,0.0,1.25,125,0.0,2.0);

  TH2D *hdxdy_HCAL_cut = new TH2D("hdxdy_HCAL_cut",";y_{HCAL}-y_{expect} (m); x_{HCAL}-x_{expect} (m)", 250, -1.25, 1.25, 250, -2.5, 2.5 );

  TH1D *hdx_HCAL_cut = new TH1D("hdx_HCAL_cut",";x_{HCAL}-x_{expect} (m);", 500, -2.5, 2.5);
  TH1D *hdy_HCAL_cut = new TH1D("hdy_HCAL_cut",";y_{HCAL}-y_{expect} (m);", 500, -1.25, 1.25);

  TH1D *hdeltaphi = new TH1D("hdeltaphi",";#phi_{p}-#phi_{e}-#pi;", 250, -0.25,0.25 );
  TH1D *hthetapq = new TH1D("hthetapq",";#theta_{pq};", 250,0.0,0.5);
  TH1D *hpmiss_perp = new TH1D("hpmiss_perp",";p_{miss,#perp} (GeV);", 250,0.0,0.5);
  TH1D *hdeltaptheta = new TH1D("hdeltaptheta",";#theta_{p}-#theta_{p,expect} (rad);", 250, -0.25,0.25);
  
  //For these only apply W and preshower cuts:
  TH1D *hdeltaphi_cut = new TH1D("hdeltaphi_cut",";#phi_{p}-#phi_{e}-#pi;", 250, -0.5,0.5 );
  TH1D *hthetapq_cut = new TH1D("hthetapq_cut",";#theta_{pq};", 250,0.0,0.5);
  TH1D *hpmiss_perp_cut = new TH1D("hpmiss_perp_cut",";p_{miss,#perp} (GeV);", 250,0.0,0.5);
  TH1D *hdeltaptheta_cut = new TH1D("hdeltaptheta_cut",";#theta_{p}-#theta_{p,expect} (rad);", 250, -0.25,0.25);

  TH1D *hetheta_cut = new TH1D("hetheta_cut","#theta_{e} (deg)",250,bbtheta*TMath::RadToDeg()-10.0,bbtheta*TMath::RadToDeg()+10.0);
  TH1D *hephi_cut = new TH1D("hephi_cut","#phi_{e} (deg)",250,-45,45);
  TH1D *hpphi_cut = new TH1D("hpphi_cut","#phi_{p} (deg)",250,135,225);

  double pel_central = ebeam/(1.+ebeam/Mp*(1.-cos(bbtheta)));
  double Q2_central = 2.*ebeam*pel_central*(1.-cos(bbtheta));
  
  TH1D *hep_cut = new TH1D("hep_cut",";p_{e} (GeV);",250,0.5*pel_central,1.5*pel_central);
  TH1D *hQ2_cut = new TH1D("hQ2_cut",";Q^{2} (GeV^{2});",250,0.5*Q2_central, 1.5*Q2_central );

  //TH1D *hetheta_cut = new TH1D("hetheta_cut","",250,bbtheta*TMath::RadTo
  
  TH1D *hptheta_cut = new TH1D("hptheta_cut",";#theta_{p} (deg);",250,sbstheta*TMath::RadToDeg()-15.0,sbstheta*TMath::RadToDeg()+15.0);

  double pp_central = sqrt(Q2_central*(1.+Q2_central/(4.*Mp*Mp)));
  
  TH1D *hpp_cut = new TH1D("hpp_cut","p_{N,expect} (GeV)",250,0.5*pp_central,1.5*pp_central);

  double Tcentral = sqrt(pow(pp_central,2)+pow(Mp,2))-Mp;
  
  TH1D *hpEkin_cut = new TH1D("hpEkin_cut","E_{kin,expect} (GeV)",250,0.5*Tcentral,1.5*Tcentral);

  TH1D *hEHCALoverEkin = new TH1D("hEHCALoverEkin","E_{HCAL}/E_{kin,expect}",250,0.0,1.0);

  TH1D *hTrackNhits_cut = new TH1D("hTrackNhits_cut",";Num. hits on track;",6,-0.5,5.5);

  TH1D *hvz_cut = new TH1D("hvz_cut",";vertex z (m);", 250,-0.125,0.125);

  long nevent = 0;

  while( C->GetEntry( nevent++ ) ){
    if( nevent % 1000 == 0 ) cout << nevent << endl;

    if( ntrack > 0 ){
      double etheta = acos( pz[0]/p[0] );
      double ephi = atan2( py[0], px[0] );

      TVector3 vertex(0,0,vz[0]);

      TLorentzVector Pbeam(0,0,ebeam,ebeam);
      TLorentzVector kprime(px[0],py[0],pz[0],p[0]);
      TLorentzVector Ptarg(0,0,0,Mp);

      TLorentzVector q = Pbeam - kprime;

      TLorentzVector PgammaN = Ptarg + q; //(-px, -py, ebeam - pz, Mp + ebeam - p)
      
      double pel = ebeam/(1.+ebeam/Mp*(1.-cos(etheta)));

      hdpel->Fill( p[0]/pel - 1.0 );

      hW->Fill( PgammaN.M() );

      hvz->Fill( vertex.Z() );

      //Now project to HCAL and compare to best HCAL cluster:
      //Assume neutron (straight-line): 
      //Also assume quasi-elastic kinematics:

      //if( PgammaN.M() >= Wmin_elastic && PgammaN.M() <= Wmax_elastic && Eps_BB>0.15 && 
      //	  fabs( vertex.Z() )<=0.08 ){
	//TVector3 pnucleon_expect = PgammaN.Vect();
	//TVector3 pNhat = pnucleon_expect.Unit();
      
	//pnucleon_expect gives suspect results. Not clear why. Let's try direct calculation from the momentum:
      double nu = ebeam - p[0];
      //double nu = ebeam - pel;
      double pp = sqrt(pow(nu,2)+2.*Mp*nu);
      double phinucleon = ephi + TMath::Pi(); //assume coplanarity
      
      // ** linear momentum conservation along z direction
      double thetanucleon = acos( (ebeam - p[0]*cos(etheta))/pp ); //use elastic constraint on nucleon kinematics
      
      // ** unit radius vector along nucleon momentum
      TVector3 pNhat( sin(thetanucleon)*cos(phinucleon),sin(thetanucleon)*sin(phinucleon),cos(thetanucleon));
      
      TVector3 HCAL_zaxis(-sin(sbstheta),0,cos(sbstheta));
      TVector3 HCAL_xaxis(0,1,0);
      TVector3 HCAL_yaxis = HCAL_zaxis.Cross(HCAL_xaxis).Unit();
      
      TVector3 HCAL_origin = hcaldist * HCAL_zaxis + hcalheight * HCAL_xaxis;
      
      TVector3 TopRightBlockPos_DB(xoff_hcal,yoff_hcal,0);
      
      TVector3 TopRightBlockPos_Hall( hcalheight + (nrows_hcal/2-0.5)*blockspace_hcal,
				      (ncols_hcal/2-0.5)*blockspace_hcal, 0 );
      
      
      //Assume that HCAL origin is at the vertical and horizontal midpoint of HCAL
      
      xHCAL += TopRightBlockPos_Hall.X() - TopRightBlockPos_DB.X();
      yHCAL += TopRightBlockPos_Hall.Y() - TopRightBlockPos_DB.Y();
      
      
      double sintersect = ( HCAL_origin - vertex ).Dot( HCAL_zaxis ) / (pNhat.Dot( HCAL_zaxis ) );
      
      TVector3 HCAL_intersect = vertex + sintersect * pNhat;
      
      double yexpect_HCAL = (HCAL_intersect - HCAL_origin).Dot( HCAL_yaxis );
      double xexpect_HCAL = (HCAL_intersect - HCAL_origin).Dot( HCAL_xaxis );
      
      hdx_HCAL->Fill( xHCAL - xexpect_HCAL );
      hdy_HCAL->Fill( yHCAL - yexpect_HCAL );
      
      hdxdy_HCAL->Fill( yHCAL - yexpect_HCAL, xHCAL - xexpect_HCAL );
      
      hxcorr_HCAL->Fill( xexpect_HCAL, xHCAL );
      hycorr_HCAL->Fill( yexpect_HCAL, yHCAL );
      
      hdy_HCAL_vs_z->Fill( vertex.Z(), yHCAL - yexpect_HCAL );
      hdy_HCAL_vs_ptheta->Fill( thetanucleon, yHCAL - yexpect_HCAL );

      TVector3 HCALpos(xHCAL,yHCAL,0);
      TVector3 HCALpos_global = HCAL_origin + HCALpos.X() * HCAL_xaxis + HCALpos.Y() * HCAL_yaxis;

      TVector3 HCAL_ray = HCALpos_global - vertex;
      HCAL_ray = HCAL_ray.Unit();

      double ptheta_recon = HCAL_ray.Theta();
      double pphi_recon = HCAL_ray.Phi();

      if( pphi_recon < 0.0 ) pphi_recon += 2.0*TMath::Pi();
      if( pphi_recon > 2.0*TMath::Pi() ) pphi_recon -= 2.0*TMath::Pi();

      hdeltaphi->Fill( pphi_recon - phinucleon );
      hdeltaptheta->Fill( ptheta_recon - thetanucleon );
      
      
      double thetapq = acos( HCAL_ray.Dot( pNhat ) );
      hthetapq->Fill( thetapq );

      TVector3 pNrecon = pp*HCAL_ray;

      double Enucleon = sqrt(pow(pp,2)+pow(Mp,2));

      TLorentzVector PNrecon( pNrecon,Enucleon );
      //pmiss = P + q - PNrecon

      TLorentzVector Pmiss = PgammaN - PNrecon;

      double pmiss_perp = (Pmiss.Vect() - Pmiss.Vect().Dot(q.Vect().Unit())*q.Vect().Unit()).Mag();

      hpmiss_perp->Fill( pmiss_perp );
      
      
      hE_HCAL->Fill( EHCAL );

      if( pow( (xHCAL-xexpect_HCAL - dx0)/dxsigma,2) + pow( (yHCAL-yexpect_HCAL - dy0)/dysigma,2) <= pow(2.5,2) ){


	if( Eps_BB >= 0.15 && abs( (Eps_BB+Esh_BB)/p[0] - 1. ) < 0.25 ){

	  hW_cut_HCAL->Fill( PgammaN.M() );
	  hdpel_cut_HCAL->Fill( p[0]/pel - 1.0 );
	  
	  
	  hE_HCAL_cut->Fill( EHCAL );

	  //for the following histograms make aggressive cuts:
	  if( Wmin < PgammaN.M() && PgammaN.M() < Wmax && dpel_min < p[0]/pel-1.&&p[0]/pel-1. < dpel_max ){ 
	    hep_cut->Fill( p[0] );
	    hQ2_cut->Fill( 2.*ebeam*p[0]*(1.-cos(etheta)) );
	    hptheta_cut->Fill( thetanucleon * TMath::RadToDeg() );
	    hetheta_cut->Fill( etheta* TMath::RadToDeg() );
	    hpp_cut->Fill( pp );
	    hpEkin_cut->Fill( nu );
	    hEHCALoverEkin->Fill( EHCAL/nu );
	    
	    hephi_cut->Fill( ephi * TMath::RadToDeg() );
	    hpphi_cut->Fill( pphi_recon * TMath::RadToDeg() );

	    hTrackNhits_cut->Fill( tracknhits[0] );

	    hvz_cut->Fill( vz[0] );


	  }
	}

	//cut on elastic peak for EoverP and preshower cut plots:
	if( Wmin < PgammaN.M() && PgammaN.M() < Wmax && dpel_min < p[0]/pel-1.&&p[0]/pel-1. < dpel_max ){ 
	  hE_preshower_cut->Fill( Eps_BB );
	  hEoverP_cut->Fill( (Eps_BB+Esh_BB)/p[0] );
	  hEoverP_vs_preshower_cut->Fill( Eps_BB,  (Eps_BB+Esh_BB)/p[0] );
	  
	}

      }

      if( Eps_BB >= 0.15 && abs( (Eps_BB+Esh_BB)/p[0] - 1.0 ) <= 0.3 ){
	hdpel_cutBBCAL->Fill( p[0]/pel - 1.0 );
	hW_cutBBCAL->Fill( PgammaN.M() );
	if( Wmin <= PgammaN.M() && PgammaN.M() <= Wmax && 
	    dpel_min <= p[0]/pel - 1. && p[0]/pel - 1. < dpel_max ){
	  hdx_HCAL_cut->Fill( xHCAL - xexpect_HCAL );
	  hdy_HCAL_cut->Fill( yHCAL - yexpect_HCAL );
	  hdxdy_HCAL_cut->Fill( yHCAL - yexpect_HCAL, xHCAL - xexpect_HCAL );

	  hdeltaphi_cut->Fill( pphi_recon - phinucleon );
	  hthetapq_cut->Fill( thetapq );
	  hpmiss_perp_cut->Fill( pmiss_perp );
	  hdeltaptheta_cut->Fill( ptheta_recon - thetanucleon );
	}
      }

      hE_preshower->Fill( Eps_BB );
      hEoverP->Fill( (Eps_BB+Esh_BB)/p[0] );
      hEoverP_vs_preshower->Fill( Eps_BB,  (Eps_BB+Esh_BB)/p[0] );
   

    }
  }
  
  fout->Write();


}
