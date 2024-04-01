/*
  This script will generate energy of the scattered electron (e-p) taking into consideration
  the angular acceptance of BBSH. Takes three inputs: Beam energy, BB angle, & BB distance
  New additions: Struck nucleon's 1. K.E., 2. Total Energy, and 3. Momentum
  ---------
  P. Datta <pdbforce@jlab.org> Created 10-07-2021
*/

#include <TMath.h>
#include <iostream>
#include <vector>

static const double Mp = 0.938272081; // +/- 6E-9 GeV
static const double Mn = 0.939565413; // +/- 6E-9 GeV

// define useful functions
double E_central (double E_beam, double BB_ang_rad);                  // calculates elastic electron energy (GeV)
double E_kineN (double E_beam, double BB_ang_rad, std::string Ntype); // calculates struck nucleon kinetic energy (GeV)
double p_N (double E_beam, double BB_ang_rad, std::string Ntype);     // calculates struck nucleon momentum (GeV/c)

int elasPeak_expect(double E_beam = 3.7278, double BB_angle = 36., double BB_distance = 1.7988) {
  // get the BB angle in rad
  double BB_ang_rad = BB_angle*TMath::DegToRad();

  // print the inputs
  cout << endl << Form("Inputs: E_beam = %.4fGeV, BBang = %.1fdeg, BBdist = %.2fm", E_beam, BB_angle, BB_distance) << endl;

  // shower position
  double sh_faceDist = 1. + BB_distance;
  double sh_ypos[7] = {-0.2565, -0.171, -.0855, 0.0, 0.0855, 0.171, 0.2565};
 
  // Central elastic electron energy
  double E_cen = E_central(E_beam, BB_ang_rad);
  cout << endl << "----" << endl
       <<  "Central elastic e: Ee = " << E_cen << endl
       << "----" << endl << endl;

  // kinetic energy of the struck nucleon
  double E_kineN_p = E_kineN(E_beam, BB_ang_rad, "p");
  double E_kineN_n = E_kineN(E_beam, BB_ang_rad, "n");
  double p_p = p_N(E_beam, BB_ang_rad, "p");
  double p_n = p_N(E_beam, BB_ang_rad, "n");
  cout << "----" << endl
       << Form("Central elastic p: Ek_p = %.3f | Ep = %.3f | p_p = %.3f", E_kineN_p, E_kineN_p+Mp, p_p) << endl
       << Form("Central elastic n: Ek_n = %.3f | En = %.3f | p_n = %.3f", E_kineN_n, E_kineN_n+Mn, p_n) << endl
       << "----" << endl << endl;

  double deltaBBang = 0.;
  for(int shcol=0; shcol<7; shcol++){
    double effective_BBang = (sh_ypos[shcol]/sh_faceDist) + BB_ang_rad; //rad
    E_cen = E_central(E_beam, effective_BBang);
    double Ep = E_kineN(E_beam, effective_BBang, "p") + Mp;
    double En = E_kineN(E_beam, effective_BBang, "n") + Mn;
    p_p = p_N(E_beam, effective_BBang, "p");
    p_n = p_N(E_beam, effective_BBang, "n");
    effective_BBang *= TMath::RadToDeg();
    cout << Form("SH col %d | BBang = %.1f | Ee = %.3f | Ek_p = %.3f | Ek_n = %.3f | Ep = %.3f | En = %.3f | p_p = %.3f | p_n = %.3f",
		 shcol+1, effective_BBang, E_cen, Ep-Mp, En-Mn, Ep, En, p_p, p_n) << endl;
  }
  cout << endl;
  return 0;
}

//_________________________________________________________________
double E_central (double E_beam, double BB_ang_rad) {
  // Calculates elastically scatterd electron energy
  return E_beam/( 1. + (2.*E_beam / Mp) * pow(TMath::Sin(BB_ang_rad / 2.), 2.) );
}
//_________________________________________________________________
double E_kineN (double E_beam, double BB_ang_rad, std::string Ntype) {
  /* 
     Calculates kinetic energy of struck nucleon (elastic)
     E = sqrt(m0*c^2 + p*c^2)
     M0 = m0*c^2 => Rest mass energy of struck nucleon
     E_kinetic = E - M0
     E_kinetic = (E_beam^2 / M0) * ( (1-cos(BBang)) / ( (E_beam/M0) * (1-cos(BBang)) ) )
  */
  if (Ntype.compare("p") == 0)
    return (pow(E_beam, 2) / Mp) * ((1. - TMath::Cos(BB_ang_rad)) / (1. + (E_beam/Mp) * (1. - TMath::Cos(BB_ang_rad))));
  else if  (Ntype.compare("n") == 0)
    return (pow(E_beam, 2) / Mn) * ((1. - TMath::Cos(BB_ang_rad)) / (1. + (E_beam/Mn) * (1. - TMath::Cos(BB_ang_rad))));
  else {
    cerr << "**!** Enter valid nucleon type!" << endl;
    return -1.;
  }
}
//_________________________________________________________________
double p_N (double E_beam, double BB_ang_rad, std::string Ntype) {
  /*
     Calculates momentum of struck nucleon (elastic)
     M0 = m0 * c^2 => Rest mass energy of struck nucleon
     p*c = sqrt(E^2 - M0^2) = sqrt(E_kinetic^2 + 2*E_kinetic*M0)
  */
  double M_N;
  if (Ntype.compare("p") == 0)
    M_N = Mp;
  else if (Ntype.compare("n") == 0)
    M_N = Mn;
  else 
    cerr << "**!** Enter valid nucleon type!" << endl;
  double e_kine = (pow(E_beam, 2) / M_N) * ((1. - TMath::Cos(BB_ang_rad)) / (1. + (E_beam/M_N) * (1. - TMath::Cos(BB_ang_rad))));
  return sqrt(pow(e_kine,2.) + 2.*e_kine*M_N);
}

