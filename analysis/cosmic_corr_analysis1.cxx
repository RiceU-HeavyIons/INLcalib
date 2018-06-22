#define _FILE_OFFSET_BITS  64

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <TFile.h>
#include <TH1.h>
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
#include <TBenchmark.h>     // ROOT head file for benchmarking applications
#include <TApplication.h>
#include <TH2.h>
#include <TMath.h>

#include <TBenchmark.h>     // ROOT head file for benchmarking applications

using namespace std;
void CreateDir(const string);

int main(int argc, char *argv[])
{

  // time statistic --- start
  TBenchmark* gBenchmark = new TBenchmark();
  //start time of this program
  gBenchmark->Start("main");

  // process commandline
  if(argc<2) {
    cout<<"Usage: cosmic_corr_analysis1 --help to get help"<<endl;
    return -1;
  }
  string datafilename="unknown";
  string inltablename="unknown";
  int totchan=8;
  if (argc>=2){
    for (int i=1;i<(argc);i++){
      if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")){
        cout<<"Usage: ----------->"<<endl;
        cout<<argv[0]<<" -f datafilename "<<endl;
        return 0;
      }
      if (!strcmp(argv[i],"-f"))   {datafilename=argv[++i];}
    }
  }
  char filename[200];
  char tmpchr[200];

  string ComFileName=datafilename;
  int dotpos=ComFileName.find(".dat");  
  if(dotpos>0)ComFileName=ComFileName.erase(dotpos,ComFileName.length());  
  cout<<"corr_analysis: "<<ComFileName<<endl;
  sprintf(filename,"%s",ComFileName.c_str());

  string tdigname1;
  string tdigname="692";

  tdigname1="tdig_ser"+tdigname;
  tdigname="tdig-ser"+tdigname;
  cout<<"board pos: "<<0<<" "<<tdigname<<endl;

  if(inltablename=="unknown")inltablename=filename;

  //create some directories:
  sprintf(tmpchr,"../result/cosmics/%s",filename);CreateDir(tmpchr);
  sprintf(tmpchr,"../result/cosmics/%s/events.dat",filename);
  ofstream cosmic_events(tmpchr);

  double inlcor[3][8][1024];
  for(int itdig=0;itdig<3;itdig++){
     for(int i=0;i<1024;i++){for(int ich=0;ich<8;ich++){inlcor[itdig][ich][i]=0;}}
  }
  cout<<"Reading INL tables"<<endl;
  for(int itdc=0;itdc<3;itdc++){
    for(int ich=0;ich<8;ich++){
      // read in inltable file 
      sprintf(tmpchr,"../result/%s/inl/%s.tdc%d.ch%d.inl",tdigname.c_str(),tdigname.c_str(),itdc,ich);
      ifstream inlinput(tmpchr);
      if(!inlinput){cout<<"ERROR!! Can not find "<<tmpchr<<endl;return -1;}
      float tbin,cor;
      int index=0;
      while(inlinput>>tbin>>cor){
        int ibin = int(tbin-0.5);
        inlcor[itdc][ich][ibin]=cor;
        index++;
      }  
      if(0) {
        if(index<1024) {cout<<"ERROR! Not Enough correction constant in INL table !"<<endl;return -1;}
        cout<<"  First 8 inlcor=";
        for(int i=0;i<8;i++)cout<<dec<<inlcor[itdc][ich][i]<<" ";
        cout<<endl;
        cout<<"  Last  8 inlcor=";
        for(int i=0;i<8;i++)cout<<dec<<inlcor[itdc][ich][1024+i-8]<<" ";
        cout<<endl;
      }
    }
  }


  sprintf(tmpchr,"../rawdata/%s.dat",filename);
  FILE* inputfile=fopen(tmpchr,"rb");
  if(!inputfile) {cout<<"Can not open "<<tmpchr<<endl;return -1;};  
  cout<<"=======================================================================>"<<endl;
  cout<<"Process Data File:============================>> "<<tmpchr<<" "<<endl;

  int numret=0;
  int nwords;
  unsigned int tempv=0;
  int tdcid, tdcchan, edgeid; //always run in board id spot 0
  int misswordevents=0;
  int goodevents=0;
  float time[3][totchan],ntime[3][totchan];
  int lastevents=0;
  int wrongedge=0;

int exist=0;

  while (numret=fread(&nwords,4,1,inputfile)==1) {
    
    if(goodevents%500000==1&&goodevents>lastevents) {lastevents=goodevents;cout<<" processed events "<<goodevents<<endl;}
    
    goodevents++;
exist=0;

    for(int itdc=0;itdc<3;itdc++){for(int ich=0;ich<8;ich++){time[itdc][ich]=0;ntime[itdc][ich]=0;}}

    for(int iword=0;iword<nwords;iword++){
      fread(&tempv,4,1,inputfile);

      if((tempv&0xF0000000)>>28 == 0xE) continue;
      if(tempv>>16==0xa000) continue;
      if(tempv==0xdeadface) continue;
      if((tempv&0xF0000000)>>28 == 0x2) continue;
      if((tempv&0xF0000000)>>28 == 0xC) continue;

      edgeid =int( (tempv & 0xf0000000)>>28 );
      
      if((edgeid != 4 && edgeid != 5)){if(wrongedge<50)cout<<"ERROR!!Wrong edge="<<edgeid<<hex<<" 0x"<<tempv<<endl;wrongedge++;continue;} 
     
      tdcid = ((tempv & 0x0F000000)>>24)%4;

      tdcchan = (tempv&0x00E00000)>>21;

      int itime=((tempv&0x7ffff)<<2)+((tempv>>19)&0x00000003);
      int bin=itime&0x000003ff;
      ntime[tdcid][tdcchan]=itime;
      time[tdcid][tdcchan]=itime+inlcor[tdcid][tdcchan][bin];

if(edgeid==4 || edgeid==5) {
  cosmic_events<<edgeid<<"  "<<tdcid<<"  "<<tdcchan<<"  "<<itime<<"  "<<time[tdcid][tdcchan]<<endl;exist=1;
}

    }

if(exist==1)cosmic_events<<"goodevents= "<<goodevents<<endl;

    // -----------------------
  }// end while loop

  fclose(inputfile);
  cout<<"Good events = "<<goodevents<<" missed word events = "<<misswordevents<<endl;

  //----------------------------------------------------
  cout<<" All done. "<<endl;
  // time statistics.
  gBenchmark->Stop("main");       // run time stop here
  gBenchmark->Show("main");       // print out the time information
  float realtime = gBenchmark->GetRealTime("main");        // real time elapsed
  float cputime  = gBenchmark->GetCpuTime("main");         // CPU time we use
  cout<<" real time = "<<realtime<<" seconds cput="<<cputime<<" seconds"<<endl;
  cout<<" Finished "<<filename<<endl;

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
