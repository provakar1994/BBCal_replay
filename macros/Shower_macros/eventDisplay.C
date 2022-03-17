/* 
   M. K. Jones Created
*/

#include "Riostream.h"
#include <TFile.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraphErrors.h>
#include <TTree.h>
#include <TROOT.h>
#include <TLegend.h>
#include <string>
#include <TCanvas.h>
#include <TStyle.h>

//Display an array, set a minimum value to display.
void Draw(Double_t data[], vector<Double_t>* means, Double_t min, const char* title, Int_t rows = 27, Int_t cols = 7){

  //Create a 2D histogram to store the data.
  auto display = new TH2I("display",title,cols,0,cols,rows,0,rows);
  
  
  int elem;
  //Fill the bins with data.
  for(int i = 0; i < cols; i++){
    for(int j = 0; j < rows; j++){
      elem = (j*cols)+i;
      if(cols==2)elem = i*rows+j;
      //Only display if value(channel) > min.
      if(data[elem] - means->at(elem) > min){
	
	//Fill the bins
	display->SetBinContent(i+1,j+1,(Int_t)(data[elem] - means->at(elem)));
      } else {
	//Else display 0.
	display->SetBinContent(i+1,j+1,0);
      }
    }
  }
  display->GetYaxis()->SetNdivisions(rows);
  display->GetXaxis()->SetNdivisions(cols);
  
  //Have to completely replace axes to position axis labels where they are	
  display->GetYaxis()->SetLabelSize(0);
  display->GetXaxis()->SetLabelSize(0);	
  
  //Let's make a function to set the axis values
  TF1 *xfunc = new TF1("xfunc","x",1,cols+1);
  TF1 *yfunc = new TF1("yfunc","x",1,rows+1);
  
  //Set colour, box text size and remove the stats box.
  display->SetFillColor(kRed-9);
  display->SetStats(0);
  display->SetMarkerSize(1.2+((7-cols)/1.7));
  
  //Have to make the old axis labels invisible (unable to edit some axis properties directly).
  display->GetYaxis()->SetLabelSize(0);
  display->GetXaxis()->SetLabelSize(0);
  display->SetTitleSize(1/cols);
  
  //Display the event.
  display->Draw("box,text");	
  
  //Create completely new axes to get label in the middle of the divisions)
  TGaxis *x = new TGaxis(0,0,cols,0,"xfunc",cols+1,"M");
  x->SetLabelSize(0.04);
  if(cols==2)x->SetLabelSize(0.06);
  x->SetLabelOffset(0.015);
  //x->SetLabelOffset(0.015*(cols-7));
  x->Draw();
  
  TGaxis *y = new TGaxis(0,0,0,rows,"yfunc",rows+1,"M");
  y->SetLabelSize(0.04);
  if(cols==2)y->SetLabelSize(0.06);
  y->SetLabelOffset(0.015);
  y->Draw();
  
  
  //Vertical lines.
  for (int i = 1; i < cols; i++){
    TLine *line = new TLine(i,0,i,rows);
    line->SetLineStyle(kDotted);
    line->Draw();	
  }

  //Horizontal lines.
  //Vertical lines.
  for (int i = 1; i < rows; i++){
    TLine *line = new TLine(0,i,cols,i);
    line->SetLineStyle(kDotted);
    line->Draw();	
  }

  //Memory clean up.
  //delete gROOT->FindObject("display");
}

void eventDisplay(int run, bool save_option = false){

  //Get the mean and RMS values.
  vector<Double_t>* meanValues;
  vector<Double_t>* rmsValues;
  vector<Double_t>* meanValuesPS;
  vector<Double_t>* rmsValuesPS;
  
  TFile *calibration = TFile::Open(Form("pedestalcalibrated_%d.root", run));
  if(calibration){
    meanValues = (vector<Double_t>*)calibration->Get("Mean");
    rmsValues = (vector<Double_t>*)calibration->Get("RMS");
    meanValuesPS = (vector<Double_t>*)calibration->Get("MeanPS");
    rmsValuesPS = (vector<Double_t>*)calibration->Get("RMSPS");
    calibration->Close();
  }else{
    meanValues = new vector<Double_t>;
    rmsValues = new vector<Double_t>;
    meanValuesPS = new vector<Double_t>;
    rmsValuesPS = new vector<Double_t>;
    for(int i = 0; i<54; i++){
      meanValues->push_back(0);
      rmsValues->push_back(0);
      meanValuesPS->push_back(0);
      rmsValuesPS->push_back(0);
    }
    for(int i = 54; i<189; i++){
      meanValues->push_back(0);
      rmsValues->push_back(0);
    }
  }
  //Create a Canvas
  TCanvas* c1 = new TCanvas("c1","Event Display",700,1200);
  TPad *shower = new TPad("shower","Shower",0.01,0.01,0.599,0.99);
  shower->Draw();
  TPad *preshower = new TPad("preshower","Pre-Shower",0.601,0.01,.99,.99);
  preshower->SetLeftMargin(0.15);
  preshower->Draw();
	
  //Open the file.
  TFile *events = TFile::Open(Form("/home/daq/Analysis/BBcal/rootfiles/bbcal_%d.root", run));

  //Get the Tree.
  TTree* tree = 0;
  events->GetObject("T",tree);

  //Set the variable to hold the values.
  Double_t data[189];
  Double_t dataPS[54];
  tree->SetBranchAddress("bb.sh.a_p",&data);
  tree->SetBranchAddress("bb.ps.a_p",&dataPS);

  //Get the number of events.
  Int_t nEvents =(Int_t) tree->GetEntries();
	
  Int_t event = 0;
  Int_t cell = 0;
  
  bool save;
  
  for(Int_t event = 0; event < nEvents; event++){

    //Clear the data array
    //		for(int i = 0; i < 189;i++){
    //			data[i] = 0.0;
    //			if(i < 54){
    //				dataPS[i] = 0;			
    //			}
    //		}
    //Read in that data.
    tree->GetEntry(event);		

    //Get the total calibrated ADC value
    Double_t sum = 0;
    for(int i = 0; i < 189; i++){
      sum += data[i]-meanValues->at(i);	
    }

    //Don't display events with a total ADC value < 50
    if(sum < 50){
      continue;
    }

    //Create the histogram to draw this event.
    std::string title = "Event ";
    title += std::to_string(event);

    //Display the event
    //c1->cd(1);
    shower->cd();
    Draw(data,meanValues, 20, title.c_str(),27,7);
    //c1->cd(2);
    preshower->cd();
    Draw(dataPS,meanValuesPS, 20, "" ,27,2);
    gPad -> WaitPrimitive();
    if(save_option){
      save = false;
      cout << "save ? (1 - yes; 0 - no)" << endl;
      cin >> save;
      if(save){
	c1->SaveAs(Form("BBCal_run%d_evt%d.pdf", run, event));
      }
    }
    //Memory clean up
    delete gROOT->FindObject("display");
  }
}


