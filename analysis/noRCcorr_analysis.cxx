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

  // need this line to make "gif" files online....do not run code in batch mode.
  //  TApplication* theApp=new TApplication("app",&argc ,argv);
  TApplication* theApp=new TApplication("app",0 ,0);

  // time statistic --- start
  TBenchmark* gBenchmark = new TBenchmark();
  //start time of this program
  gBenchmark->Start("main");

  // process commandline
  if(argc<2) {
    cout<<"Usage: corr_analysis --help to get help"<<endl;
    return -1;
  }
  string datafilename="unknown";
  string inltablename="unknown";
  string processmode="8";
  int processboard =-1;
  int totchan=8;
  if (argc>=2){
    for (int i=1;i<(argc);i++){
      if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")){
        cout<<"Usage: ----------->"<<endl;
        cout<<argv[0]<<" -f datafilename "<<endl;
        cout<<argv[0]<<" -c inltablename -- Apply INL correction by inlfilename"<<endl;
        cout<<argv[0]<<" -mode processmode, correct INL by 1 or 8 table"<<endl;
        cout<<argv[0]<<" -board 0,1,4,5 or -1 for all "<<endl;
        return 0;
      }

      if (!strcmp(argv[i],"-f"))   {datafilename=argv[++i];}
      if (!strcmp(argv[i],"-c"))   {inltablename=argv[++i];}
      if (!strcmp(argv[i],"-mode")) {processmode=argv[++i];}
      if (!strcmp(argv[i],"-board"))   {processboard=atoi(argv[++i]);}
    }
  }
  char filename[200];
  char tmpchr[200];

  string ComFileName=datafilename;
  int dotpos=ComFileName.find(".dat");  
  if(dotpos>0)ComFileName=ComFileName.erase(dotpos,ComFileName.length());  
  cout<<"corr_analysis: "<<ComFileName<<endl;
  sprintf(filename,"%s",ComFileName.c_str());

  int activeboard[8]={0,1,-1,-1,4,5,-1,-1};
  string tdigname1[8];
  string tdigname[8];
 
  string tempstr=ComFileName;
  // int dashpos = tempstr.find("-");
  //tempstr= tempstr.erase(0,dashpos+1); 

  // File name configuration: tdig-pos0-pos1-pos4-pos5.
  // TCPU: J4: should be 4, 5, but we connected as: 0,1, 5/29/2008, will keep this configuration in calibration.
  // TCPU: J5: should be 0, 1, but we connected as: 4,5, 5/29/2008, will keep this configuration in calibration.
  // should reverse ??  confirm?
  // should be:
  //tdigname[0].assign(tempstr,5,3);
  //tdigname[1].assign(tempstr,9,3);
  //tdigname[4].assign(tempstr,13,3);
  //tdigname[5].assign(tempstr,17,3);
  // but we connected them like this:
  //tdigname[4].assign(tempstr,5,3);
  //tdigname[5].assign(tempstr,9,3);
  //tdigname[0].assign(tempstr,13,3);
  //tdigname[1].assign(tempstr,17,3);

  string extid="";
  string thisstr=ComFileName;
  istringstream ss(thisstr);
  string buf;
  int ndash=0;
  while (getline(ss,buf,'-')){
    if(ndash==1) tdigname[4]=buf;    
    if(ndash==2) tdigname[5]=buf;    
    if(ndash==3) tdigname[0]=buf;    
    if(ndash==4) tdigname[1]=buf;    
    ndash++;
  }
  int pos=tdigname[1].rfind(".");
  if(pos>0) {extid.assign(tdigname[1],pos,tdigname[1].length());tdigname[1]=tdigname[1].erase(pos,thisstr.length());}
  //cout<<tdigname[4]<<" "<<tdigname[5]<<" "<<tdigname[0]<<" "<<tdigname[1]<<" extid="<<extid<<endl;

  int nactiveboard=0;
  for(int i=0;i<8;i++){
    if(activeboard[i]<0) continue;
    tdigname1[i]="tdig_ser"+tdigname[i];
    tdigname[i]="tdig-ser"+tdigname[i]+extid;
    cout<<"board pos: "<<i<<" "<<tdigname[i]<<endl;
    nactiveboard++;
  }
  //
  if(inltablename=="unknown")inltablename=filename;

  //create some directory, if not exist
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
   
    //create some directory:
    sprintf(tmpchr,"../result/inlXcheck/%s",tdigname[iboard].c_str());CreateDir(tmpchr);
    sprintf(tmpchr,"../result/inlXcheck/%s/inl",tdigname[iboard].c_str());CreateDir(tmpchr);
    sprintf(tmpchr,"../result/inlXcheck/%s/root",tdigname[iboard].c_str());CreateDir(tmpchr);
    sprintf(tmpchr,"../result/inlXcheck/%s/gif",tdigname[iboard].c_str());CreateDir(tmpchr);
  }

  double inlcor[8][3][8][1024];
  for(int iboard=0;iboard<8;iboard++){
    for(int itdig=0;itdig<3;itdig++){
       for(int i=0;i<1024;i++){for(int ich=0;ich<8;ich++){inlcor[iboard][itdig][ich][i]=0;}}
    }
  }
  
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
    for(int itdc=0;itdc<3;itdc++){
      for(int ich=0;ich<8;ich++){
        // read in inltable file 
        sprintf(tmpchr,"../result/%s.RC/inl/%s.RC.tdc%d.inl",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc);
        if(processmode=="8")sprintf(tmpchr,"../result/%s.RC/inl/%s.RC.tdc%d.ch%d.inl",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc,ich);
        ifstream inlinput(tmpchr);
        if(!inlinput){cout<<"ERROR!! Can not find "<<tmpchr<<endl;return -1;}
        cout<<"Read in INL table:"<<tmpchr<<endl;
        //
        float tbin,cor;
        int index=0;
        while(inlinput>>tbin>>cor){
          int ibin = int(tbin-0.5);
          inlcor[iboard][itdc][ich][ibin]=cor;
          index++;
        }  
        if(0) {
          if(index<1024) {cout<<"ERROR! Not Enough correction constant in INL table !"<<endl;return -1;}
          cout<<"  First 8 inlcor=";
          for(int i=0;i<8;i++)cout<<dec<<inlcor[iboard][itdc][ich][i]<<" ";
          cout<<endl;
          cout<<"  Last  8 inlcor=";
          for(int i=0;i<8;i++)cout<<dec<<inlcor[iboard][itdc][ich][1024+i-8]<<" ";
          cout<<endl;
        }
      }
    }
  }
  

  char hisname[200];
  TH1F* deltatimeh[8][3][8][8];
  TH1F* deltatime0h[8][3][8][8];
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0)continue;
    for(int itdc=0;itdc<3;itdc++){
      for(int i=0;i<totchan;i++){
        for(int j=0;j<totchan;j++){
          sprintf(hisname,"%s_tdc%d_deltaT_ch%d_ch%d",tdigname1[iboard].c_str(),itdc,i,j);
          deltatimeh[iboard][itdc][i][j]=new TH1F(hisname,hisname,40000,0,20000);
          sprintf(hisname,"%s_tdc%d_deltaT0_ch%d_ch%d",tdigname1[iboard].c_str(),itdc,i,j);
          deltatime0h[iboard][itdc][i][j]=new TH1F(hisname,hisname,40000,0,20000);
        }
      }
    }
  }

  TH2F* meanvssigma2h[8][3];   
  TH1F* sigma1h[8][3];
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
    for(int itdc=0;itdc<3;itdc++){
      sprintf(tmpchr,"%s_mean_vs_sigma_tdc%d",tdigname1[iboard].c_str(),itdc);
      meanvssigma2h[iboard][itdc]=new TH2F(tmpchr,tmpchr,2000,0,20000,60,0.3,3.3);
      sprintf(tmpchr,"%s_all_sigma_tdc%d",tdigname1[iboard].c_str(),itdc);
      sigma1h[iboard][itdc]=new TH1F(tmpchr,tmpchr,54,0.3,3.);
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
  int tdcid, tdcchan, tdigboardid,edgeid;
  int misswordevents=0;
  int goodevents=0;
  float time[8][3][totchan],ntime[8][3][totchan];
  int lastevents=0;
  int wrongedge=0;
  int halftrayid=-1;
  while (numret=fread(&nwords,4,1,inputfile)==1) {
    //if(goodevents>10000) continue;
    //cout<<"-----------------------------------------------------------------------------"<<endl;
    
    if(goodevents%500000==1&&goodevents>lastevents) {lastevents=goodevents;cout<<" processed events "<<goodevents<<endl;}
    if(nwords != 0x66) {misswordevents++;}
    if(nwords > 0x66) {continue;}
    
    goodevents++;
    //
    for(int iboard=0;iboard<8;iboard++){for(int itdc=0;itdc<3;itdc++){for(int ich=0;ich<8;ich++){time[iboard][itdc][ich]=0;ntime[iboard][itdc][ich]=0;}}}

    for(int iword=0;iword<nwords;iword++){
      fread(&tempv,4,1,inputfile);
      //
      if( (tempv&0xF0000000)>>28 == 0xE) continue;
      if(tempv>>16==0xa000) continue;
      if(tempv    ==0xdeadface) continue;
      if( (tempv&0xF0000000)>>28 == 0x2) {continue;}
      if( (tempv&0xF0000000)>>28 == 0xC) {halftrayid = tempv&0x01; continue;}

      edgeid =int( (tempv & 0xf0000000)>>28 );
      if((edgeid != 4)){if(wrongedge<50)cout<<"ERROR!!Wrong edge="<<edgeid<<hex<<"0x"<<tempv<<endl;wrongedge++;continue;} 
     
      tdcid = (tempv & 0x0F000000)>>24;  
      tdigboardid = tdcid/4 + halftrayid*4;
      tdcid = tdcid % 4;
      //if(tdcid >=3) {cout<<"TDC ID =="<<tdcid<<" check your configuration !!"<<endl;return -1;}
      tdcchan = (tempv&0x00E00000)>>21;
      //if(tdcchan<0 || tdcchan >8){cout<<"TDCid="<<tdcid<<" TDCchan="<<tdcchan<<" ERROR!!"<<endl;return -1;}
      int itime=((tempv&0x7ffff)<<2)+((tempv>>19)&0x00000003);
      int bin=itime&0x000003ff;
      ntime[tdigboardid][tdcid][tdcchan]=itime;
      time[tdigboardid][tdcid][tdcchan]=itime+inlcor[tdigboardid][tdcid][tdcchan][bin];

      //if(goodevents<5)cout<<"value:0x"<<hex<<tempv<<dec<<" TDIG="<<tdigboardid<<" edge="<<edgeid<<" tdcid="<<tdcid<<" tdcchan="<<tdcchan<<" itime="<<itime<<endl; 

    }

    for(int iboard=0;iboard<8;iboard++){
      if(activeboard[iboard]<0) continue;
      for(int itdc=0;itdc<3;itdc++){
        for(int i=0;i<totchan;i++){
          for(int j=0;j<totchan;j++){
            float deltatime=TMath::Abs(time[iboard][itdc][i]-time[iboard][itdc][j]);
            if(deltatime>1.e6)  deltatime=2097152-deltatime;   
            float deltatime0=TMath::Abs(float(ntime[iboard][itdc][i])-float(ntime[iboard][itdc][j]));
            if(deltatime0>1.e6)  deltatime0=2097152-deltatime0;   
            deltatimeh[iboard][itdc][i][j]->Fill(deltatime);
            deltatime0h[iboard][itdc][i][j]->Fill(deltatime0);
          }
        }
      }
    }

    // -----------------------
  }// end while loop

  fclose(inputfile);
  cout<<"Good events = "<<goodevents<<" missed word events = "<<misswordevents<<endl;
	 
  //make mean-vs-sigma plot.
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
    for(int itdc=0;itdc<3;itdc++){	 
      for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
          if(j<=i) continue;
          float mean=deltatimeh[iboard][itdc][i][j]->GetMean();
          float sigma=deltatimeh[iboard][itdc][i][j]->GetRMS(); 
          meanvssigma2h[iboard][itdc]->Fill(mean,sigma);
          sigma1h[iboard][itdc]->Fill(sigma);

          meanvssigma2h[iboard][itdc]->SetMarkerColor(1+itdc);
          meanvssigma2h[iboard][itdc]->SetMarkerStyle(20+itdc);
          meanvssigma2h[iboard][itdc]->SetMarkerSize(1);
  
          sigma1h[iboard][itdc]->SetLineColor(1+itdc);
          sigma1h[iboard][itdc]->SetLineWidth(2);
	}
      }   

      // write everything to root file
      sprintf(tmpchr,"../result/inlXcheck/%s",tdigname[iboard].c_str());CreateDir(tmpchr);
      sprintf(tmpchr,"../result/inlXcheck/%s/root",tdigname[iboard].c_str());CreateDir(tmpchr);
      sprintf(tmpchr,"../result/inlXcheck/%s/gif",tdigname[iboard].c_str()); CreateDir(tmpchr);

      sprintf(tmpchr,"../result/inlXcheck/%s/root/%s.tdc%d.corr1.root",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc);
      if(processmode=="8")sprintf(tmpchr,"../result/inlXcheck/%s/root/%s.tdc%d.corr8.root",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc);
      cout<<"Write to root file :"<<tmpchr<<endl;
      TFile* fFile=new TFile(tmpchr,"recreate");
      for(int i=0;i<totchan;i++){for(int j=0;j<totchan;j++){deltatime0h[iboard][itdc][i][j]->Write();}}
      for(int i=0;i<totchan;i++){for(int j=0;j<totchan;j++){deltatimeh[iboard][itdc][i][j]->Write();}}
      meanvssigma2h[iboard][itdc]->Write(); sigma1h[iboard][itdc]->Write();

     fFile->Close();
     //----------------
    }
  }

  //=======================================
  // make gif file----
  //-- The following is a copy from plotdeltatime.C
  //
  gROOT->SetStyle("Plain");
  // Turn off all borders
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetDrawBorder(0);
  gStyle->SetCanvasBorderSize(0);
  gStyle->SetFrameBorderSize(0);
  gStyle->SetPadBorderSize(0);
  gStyle->SetTitleBorderSize(1);

  gStyle->SetOptTitle(0);

  gStyle->SetNdivisions(505,"xyz");

  gStyle->SetLabelSize(0.08,"xyz");
  gStyle->SetLabelSize(0.06,"y");
  gStyle->SetLabelSize(0.08,"x");
  gStyle->SetLabelOffset(0.01,"x");
  gStyle->SetLabelOffset(0.01,"y");
  gStyle->SetTickLength(0.09);
  gStyle->SetTitleColor(2,"xyz");
  gStyle->SetTitleOffset(1.3,"y");
  gStyle->SetTitleX(0.); gStyle->SetTitleY(1.);
  gStyle->SetTitleW(0.98); gStyle->SetTitleH(0.08);

  gStyle->SetStatX(0.91); gStyle->SetStatY(0.91);
  gStyle->SetStatW(0.28); gStyle->SetStatH(0.22);
  gStyle->SetStatFormat("12.6g");
  gStyle->SetOptFit(11);
  gStyle->SetOptStat("nemruo");
  gStyle->SetPadGridX(0);
  gStyle->SetPadGridY(0);
  //----
  TCanvas* c1=new TCanvas("c1","c1",0,0,750,650);
  c1->SetFillColor(0);c1->Divide(2,2,0.01,0.01);
  //TGaxis* A=new TGaxis(); A->SetMaxDigits(2);
  gROOT->ForceStyle();

  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
    for(int itdc=0;itdc<3;itdc++){
      int ipage=0;
      int ifig=0;
      int lastpage=0;
      for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
          if(j<=i) continue;
          float mean=deltatimeh[iboard][itdc][i][j]->GetMean();
          deltatimeh[iboard][itdc][i][j]->SetAxisRange(mean-11,mean+22,"X");
          deltatimeh[iboard][itdc][i][j]->SetLineColor(4);
          deltatimeh[iboard][itdc][i][j]->SetLineWidth(2);
          c1->cd(ifig%4+1);
          deltatimeh[iboard][itdc][i][j]->Draw();
          ifig++;
          ipage=ifig/4;
          if(lastpage !=ipage) {
            sprintf(tmpchr,"../result/inlXcheck/%s/gif/%s-tdc%d-page%d-deltatime.gif",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc,ipage);
            c1->Print(tmpchr);
            lastpage=ipage;
          }
        }
      }
    }
  }
  // end plot

  //----------------------------------------------------
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
