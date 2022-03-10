/* This script calculates xpos and ypos for HCal blocks
   P. Datta <pdbforce@jlab.org> Created 03/10/2022
*/

#include <iostream>

using namespace std;

//defining HCal geometry
const int krowhcal = 24;
const int kcolhcal = 12;
//distance between the centers of two neighboring blocks (including gap)
const double blockoffset = 0.15;

//HCal origin in local co-ordinate system
double x_origin = 0.;
double y_origin = 0.;

int main(){

  cout << " Enter x and y co-ordinates of HCal origin in local co-ordinate system (CoS) " << endl;
  cin >> x_origin >> y_origin;

  //Choice of co-ordinate system (CoS): (All right handed)
  bool CoS = 1;
  // 0 => +X points to hall floor & +Z points along particle ray, same as BBCal
  // 1 => +X points to hall roof & +Z points along particle ray

  cout << " Enter your chois of CoS: 0=>+X to hall floor, 1=> +X to hall roof " << endl;
  cin >> CoS;

  //By convention, 1st HCal block is situated at the top right corner
  //determining its position in the CoS of choice
  double x_1stblk, y_1stblk;
  if(!CoS){
    x_1stblk = -( x_origin + ( ((double)krowhcal-1.)/2. )*blockoffset );
    y_1stblk = -( y_origin + ( ((double)kcolhcal-1.)/2. )*blockoffset );
  }else{
    x_1stblk = x_origin + ( ((double)krowhcal-1.)/2. )*blockoffset;
    y_1stblk = y_origin + ( ((double)kcolhcal-1.)/2. )*blockoffset;
  }

  cout << endl << " Printing out HCal block xpos: " << endl;
  for( int irow=0; irow<krowhcal; irow++ ){
    for( int icol=0; icol<kcolhcal; icol++ ){
	cout << x_1stblk << " ";
    }
    if(!CoS)
      x_1stblk += blockoffset;
    else
      x_1stblk -= blockoffset;
    cout << endl;
  }

  cout << endl << " Printing out HCal block ypos: " << endl;
  for( int irow=0; irow<krowhcal; irow++ ){
    double y_temp = y_1stblk;
    for( int icol=0; icol<kcolhcal; icol++ ){
      cout << y_temp << " ";
      if(!CoS)
	y_temp += blockoffset;
      else
	y_temp -= blockoffset;
    }
    cout << endl;
  }

}


// HCal origin is offset in the Hall CoS by 0.365m in X-axis
// So, for CoS=0, we should use the origin to be at (.365, 0.)
