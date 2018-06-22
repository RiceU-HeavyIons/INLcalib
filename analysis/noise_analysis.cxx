#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TPaveLabel.h>
#include <TVirtualPad.h>
#include <TGaxis.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TLatex.h>
#include <TBenchmark.h>     // ROOT head file for benchmarking applications
#include <TApplication.h>

using namespace std;
void CreateDir(const string);
int tdcchan2mrpcchan(int globaltdcchan);
int mrpcchan2tdcchan(int globalmrpcchan);

int main(int argc, char *argv[])
{

  // need this line to make "gif" files online....do not run code in batch mode.
  TApplication* theApp=new TApplication("app",0 ,0);
  //
  // time statistic --- start
  TBenchmark* gBenchmark = new TBenchmark();
  //start time of this program
  gBenchmark->Start("main");

  // process commandline
  if(argc<2) {
    cout<<"Usage: noise_analysis --help to get help"<<endl;
    return -1;
  }
  string datafilename="unknown";
  if (argc>1){
    for (int i=1;i<(argc);i++){
      if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")){
        cout<<"Usage: ----------->"<<endl;
        cout<<argv[0]<<" -f datafilename "<<endl;
        return 0;
      }
      if (!strcmp(argv[i],"-f"))   {datafilename=argv[++i];}
    }
  }
  cout<<"filename = "<<datafilename<<endl;

  string ComFileName=datafilename;
  int dotpos=ComFileName.find(".dat");  
  if(dotpos>0)ComFileName=ComFileName.erase(dotpos,ComFileName.length());  
  cout<<"noise_analysis: "<<ComFileName<<endl;

  char tmpchr[200];
  char filename[200];
  sprintf(filename,"%s",ComFileName.c_str());
  
  cout<<"filename="<<filename<<endl;

  TH2F* noiseratele2h[8];
  TH2F* noiseratete2h[8];
  for(int iboard=0;iboard<4;iboard++){
    float xx=iboard*4;
    sprintf(tmpchr,"lehits_board%d",iboard);
    noiseratele2h[iboard]=new TH2F(tmpchr,tmpchr,3,xx-0.5,3+xx-0.5,8,-0.5,7.5);
    sprintf(tmpchr,"tehits_board%d",iboard);
    noiseratete2h[iboard]=new TH2F(tmpchr,tmpchr,3,xx-0.5,3+xx-0.5,8,-0.5,7.5);
  } 
  for(int iboard=4;iboard<8;iboard++){
    float xx=(iboard-4)*4; 
    sprintf(tmpchr,"lehits_board%d",iboard);
    noiseratele2h[iboard]=new TH2F(tmpchr,tmpchr,3,xx-0.5,3+xx-0.5,8,-0.5,7.5);
    sprintf(tmpchr,"tehits_board%d",iboard);
    noiseratete2h[iboard]=new TH2F(tmpchr,tmpchr,3,xx-0.5,3+xx-0.5,8,-0.5,7.5);
  } 
  TH2F* modulenoiseratele2h[32];
  TH2F* modulenoiseratete2h[32];
  for(int imrpc=0;imrpc<32;imrpc++){
    float xx=imrpc;
    sprintf(tmpchr,"lehits_mrpc%d",imrpc);
    modulenoiseratele2h[imrpc]=new TH2F(tmpchr,tmpchr,1,xx-0.5,xx+0.5,6,-0.5,5.5);
    sprintf(tmpchr,"tehits_mrpc%d",imrpc);
    modulenoiseratete2h[imrpc]=new TH2F(tmpchr,tmpchr,1,xx-0.5,xx+0.5,6,-0.5,5.5);
  }
  TH1F* modulenoisele1h=new TH1F("module_noise_le","module_noise_le",192,-0.5,191.5);
  TH1F* modulenoisete1h=new TH1F("module_noise_te","module_noise_te",192,-0.5,191.5);


  //-----------------
  sprintf(tmpchr,"%s.dat",filename);
  FILE* inputfile=fopen(tmpchr,"r");
  if(!inputfile) {cout<<"Can not open "<<tmpchr<<endl;return -1;};  
  cout<<"Process Data File:"<<tmpchr<<" "<<endl;

  int hitperchan[8][3][8]; for(int i=0;i<8;i++){for(int j=0;j<3;j++){for(int k=0;k<8;k++){hitperchan[i][j][k]=0;}}}
  unsigned int tempv;
  int numret=0;
  int tdcid, tdcchan, tdig,edgeid, otdcid;
  int lsb;
  int lastword=0;
  int totalwords=0,goodwords=0;
  int bin=0;
  unsigned int itime;
  float time;
  int wrongedge=0;

  int halftrayid=0;
  int trayid=0;

  while ((numret=fscanf(inputfile,"%x",&tempv))==1) {
   
    //cout<<"-----------------------------------------------------------------------------"<<endl;
    if(totalwords%10000==1&&goodwords>lastword){lastword=goodwords;cout<<" processed data words: "<<dec<<goodwords<<endl;}
    totalwords++;

    if(tempv>>28==0xe) continue;  // seperators
    if(tempv>>28==0x2) continue;  // headers

    if( (tempv & 0xF0000000)>>28 == 0xC) {
      halftrayid = tempv & 0x01;    // get halftray id!
      trayid = tempv & 0x0FE>>1;     
      continue;
    }

    edgeid =int( (tempv & 0xf0000000)>>28 );
    if((edgeid != 4) && (edgeid != 5)){if(wrongedge<50)cout<<"ERROR!!Wrong edge="<<edgeid<<" value=0x"<<hex<<tempv<<endl;wrongedge++;continue;} 
    otdcid = (tempv & 0x0F000000)>>24;  
    tdcid = (tempv & 0x0F000000)>>24;  
    tdig = tdcid/4 +halftrayid*4;
    tdcid = tdcid % 4;
    if(tdig<0 || tdig >=8) {cout<<"ERROR!! TDIG ID ="<<tdig<<" check your configuration !!"<<endl;return -1;}
    tdcchan = (tempv&0x00e00000)>>21;
    if(tdcchan<0 || tdcchan >8){cout<<"ERROR!! TDCchan="<<tdcchan<<endl;return -1;}

    goodwords++;

    itime=((tempv&0x7ffff)<<2)+((tempv>>19)&0x00000003);
    //if(goodwords<300)cout<<"value:0x"<<hex<<tempv<<dec<<" TDIG="<<tdig<<" edge="<<edgeid<<" tdcid="<<otdcid<<" tdcchan="<<tdcchan<<" itime="<<itime<<" goodwords="<<goodwords<<endl; 

    lsb=((tempv&0x1ff)<<2)+((tempv>>19)&0x00000003);
    bin=itime&0x000003ff;
    time=itime ;
    hitperchan[tdig][tdcid][tdcchan]++;
    if(edgeid==4)noiseratele2h[tdig]->Fill(otdcid,tdcchan);
    if(edgeid==5)noiseratele2h[tdig]->Fill(otdcid,tdcchan);
    int globaltdcchan  = tdig * 24 + (tdcid%4)*8 + tdcchan;
    int globalmrpcchan = tdcchan2mrpcchan(globaltdcchan);
    int nmrpc= globalmrpcchan/6;
    int mrpcchan = globalmrpcchan%6;
    //cout<<"globaltdcchan="<<globaltdcchan<<" globalmrpcchan="<<globalmrpcchan<<" nmrpc="<<nmrpc<<" mrpcchan="<<mrpcchan<<endl;
    if(edgeid==4){
      modulenoiseratele2h[nmrpc]->Fill(nmrpc,mrpcchan);
      modulenoisele1h->Fill(nmrpc*6+mrpcchan);
    }
    if(edgeid==5) {
      modulenoiseratete2h[nmrpc]->Fill(nmrpc,mrpcchan);
      modulenoisete1h->Fill(nmrpc*6+mrpcchan);
    }

 }
  fclose(inputfile);
  cout<<"Process:  ="<<dec<<totalwords<<" good words "<<goodwords<<" wrong edge words ="<<wrongedge<<endl;

//=====================================================
  gROOT->SetStyle("Plain");

  gStyle->SetCanvasBorderMode(0);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetDrawBorder(0);
  gStyle->SetCanvasBorderSize(0);
  gStyle->SetFrameBorderSize(0);
  gStyle->SetPadBorderSize(0);
  gStyle->SetStatBorderSize(1);
  gStyle->SetTitleBorderSize(0);

  // Say it in black and white!
  gStyle->SetLabelFont(62,"xyz");
  gStyle->SetTitleOffset(1.25,"Y");
  gStyle->SetTitleOffset(1.,"X");
  gStyle->SetTitleSize(0.06);
  gStyle->SetOptFile(1);

  // Set Line Widths
  gStyle->SetFrameLineWidth(2);
  gStyle->SetFuncWidth(2);

  // Set margins -- I like to shift the plot a little up and to the
  // right to make more room for axis labels
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetPadBottomMargin(0.12);
  gStyle->SetPadLeftMargin(0.12);
  gStyle->SetPadRightMargin(0.08);
  //
  // Set Data/Stat/... and other options
  // Set tick marks and turn off grids
  //gStyle->SetNdivisions(505,"xyz");
  gStyle->SetPadGridX(0);
  gStyle->SetPadGridY(0);
  // Adjust size and placement of axis labels
  gStyle->SetLabelSize(0.12,"x");
  gStyle->SetLabelSize(0.09,"y");
  gStyle->SetLabelOffset(0.005,"x");
  gStyle->SetLabelOffset(0.01,"y");
  // Set paper size for life in the US
  gStyle->SetPaperSize(TStyle::kUSLetter);

  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);

  gStyle->SetPaintTextFormat("4.1f");

  gStyle->SetPalette(1);
  //
  gROOT->ForceStyle();


  TCanvas* c1=new TCanvas("c1","c1",0,0,1000,500);c1->SetFillColor(0);
  c1->Divide(8,1,0,0);
  TCanvas* c2=new TCanvas("c2","c2",0,0,1000,500);c2->SetFillColor(0);
  c2->Divide(32,1,0,0);

  float triggerfreq = 40;
  float occupancy = 0.000025 * triggerfreq;
  float totaltime =  20*60 * occupancy;
  cout<<" INT time = "<<totaltime<<endl;

  for(int i=0;i<8;i++){
    //noiseratele2h[i]->Scale(1./totaltime);
  }
  float maxnhit=0;
  for(int i=0;i<8;i++){
    if(noiseratele2h[i]->GetMaximum()>maxnhit) maxnhit=noiseratele2h[i]->GetMaximum();
  }
  for(int i=0;i<8;i++){
    noiseratele2h[i]->SetMaximum(maxnhit);
  }

  float maxnhit1=0;
  for(int i=0;i<32;i++){
    modulenoiseratele2h[i]->Scale(1./totaltime);
  }
  for(int i=0;i<32;i++){
    if(modulenoiseratele2h[i]->GetMaximum()>maxnhit1) maxnhit1=modulenoiseratele2h[i]->GetMaximum();
  }
  for(int i=0;i<32;i++){
    modulenoiseratele2h[i]->SetMaximum(maxnhit1);
  }
  modulenoisele1h->Scale(1./totaltime);
  modulenoisete1h->Scale(1./totaltime);

  cout<<" INT time = "<<totaltime<<" max hit = "<<maxnhit<<endl;
 
  TText* tex;
  for(int i=0;i<8;i++){
    c1->cd(i+1);

    noiseratele2h[i]->SetMarkerSize(5);
    noiseratele2h[i]->Draw("col text");


    sprintf(tmpchr,"%d",i);
    tex=new TText((i%4)*4+0.75,-1.25,tmpchr);
    tex->SetTextSize(0.25);
    tex->SetTextColor(2);
    tex->Draw();
  }
  c1->cd();
    sprintf(tmpchr,"%s",filename);
    tex=new TText(4.5,-1.8,tmpchr);
    tex->SetTextSize(0.3);
    tex->SetTextColor(4);
    tex->Draw();

  for(int i=0;i<32;i++){
    c2->cd(i+1);
    modulenoiseratele2h[i]->SetMarkerSize(18);
    modulenoiseratele2h[i]->Draw("col text");

    sprintf(tmpchr,"%d",i);
    tex=new TText(i-0.1,-0.8,tmpchr);
    tex->SetTextSize(0.5);
    tex->SetTextColor(2);
    tex->Draw();
  }
   

  // write histograms into root file.

  sprintf(tmpchr,"%s.noise.root",filename);
  cout<<"Write to root file :"<<tmpchr<<endl;
  TFile* fFile=new TFile(tmpchr,"recreate");
  for(int iboard=0;iboard<8;iboard++){noiseratele2h[iboard]->Write();noiseratete2h[iboard]->Write();}
  for(int imrpc=0;imrpc<32;imrpc++){modulenoiseratele2h[imrpc]->Write();modulenoiseratete2h[imrpc]->Write();}
  modulenoisele1h->Write();
  modulenoisete1h->Write();

  fFile->Close();

  theApp->Run();  
        
  //=================================================================================
  cout<<" All done. "<<endl;
  // time statistics.
  gBenchmark->Stop("main");       // run time stop here
  gBenchmark->Show("main");       // print out the time information
  float realtime = gBenchmark->GetRealTime("main");        // real time elapsed
  float cputime  = gBenchmark->GetCpuTime("main");         // CPU time we use
  cout<<" real time = "<<realtime<<" seconds cput="<<cputime<<" seconds"<<endl;

  return 0;
}


//-------------------------------
void CreateDir(const string directoryname)
{
  string command;
  // if the directoryname does not exist, creat it
  if(gSystem->AccessPathName(directoryname.c_str())){
    command = "mkdir "+directoryname; 
    cout<<"mkdir :"<<directoryname<<endl;
    gSystem->Exec(command.c_str()); //execute shell command to cread directory
  }
}

int tdcchan2mrpcchan(int globaltdcchan)
{
  if(globaltdcchan<0 || globaltdcchan>191) {cout<<"Wrong global tdc chan: "<<globaltdcchan<<endl; return -1;}

  int tdcidmap[4][6] = { {0,1,0,1,0,1}, {2,0,2,0,1,0}, {1,2,0,2,0,2}, {2,1,2,1,2,1}};
  int tdcchanmap[4][6]={ {7,7,0,2,5,6}, {7,4,4,2,3,6}, {0,2,3,3,1,6}, {0,5,1,4,5,1}};

  int theglobalmodulechan[192];
  int theglobaltdcchan[192];

  for(int isec=0;isec<8;isec++){
    for(int imodule=0;imodule<4;imodule++){
      for(int ipad=0;ipad<6;ipad++){
        int globalmodule  =  isec*24 + imodule*6 + ipad;
        int globaltdc =  isec*24 + tdcidmap[imodule][ipad]*8+tdcchanmap[imodule][ipad];
        theglobalmodulechan[globalmodule]=globalmodule;
        theglobaltdcchan[globalmodule]=globaltdc;
        //cout<<"global module chan="<<globalmodule<<" global tdc chan="<<globaltdc<<endl;
     }
    }
  }
  int returnthis=0;

  //int thistdcchan=tdig*24+(tdcid%4)*8+tdcchan;
  int thistdcchan=globaltdcchan;
  for(int i=0;i<192;i++){
    if(thistdcchan == theglobaltdcchan[i]) {returnthis = i;break;}
  }
  return returnthis;
}

int mrpcchan2tdcchan(int globalmrpcchan)
{
  if(globalmrpcchan<0 || globalmrpcchan>191) {cout<<"Wrong global mrpc chan: "<<globalmrpcchan<<endl; return -1;}

  int tdcidmap[4][6] = { {0,1,0,1,0,1}, {2,0,2,0,1,0}, {1,2,0,2,0,2}, {2,1,2,1,2,1}};
  int tdcchanmap[4][6]={ {7,7,0,2,5,6}, {7,4,4,2,3,6}, {0,2,3,3,1,6}, {0,5,1,4,5,1}};

  int theglobalmodulechan[192];
  int theglobaltdcchan[192];

  for(int isec=0;isec<8;isec++){
    for(int imodule=0;imodule<4;imodule++){
      for(int ipad=0;ipad<6;ipad++){
        int globalmodule  =  isec*24 + imodule*6 + ipad;
        int globaltdc =  isec*24 + tdcidmap[imodule][ipad]*8+tdcchanmap[imodule][ipad];
        theglobalmodulechan[globalmodule]=globalmodule;
        theglobaltdcchan[globalmodule]=globaltdc;
     }
    }
  }
  return theglobaltdcchan[globalmrpcchan];
}

