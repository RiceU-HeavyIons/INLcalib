 #include <iostream>
#include <string>
#include <fstream>
#include <iomanip>

#include <TMath.h>
#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TBenchmark.h>     // ROOT head file for benchmarking applications

using namespace std;

int main(int argc, char *argv[])
{
  //
  // time statistic --- start
  TBenchmark* gBenchmark = new TBenchmark();
  //start time of this program
  gBenchmark->Start("main");

  // process commandline
  if(argc<2) {
    cout<<"Usage: checkdata --help to get help"<<endl;
    return -1;
  }
  string datafilename="unknown";
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
  char tmpchr[200],hisname[200];
  //-----------------
  sprintf(tmpchr,"%s",datafilename.c_str());
  FILE* inputfile=fopen(tmpchr,"r");
  if(!inputfile) {cout<<"Can not open "<<tmpchr<<endl;return -1;};  
  cout<<"Process Data File:"<<tmpchr<<" "<<endl;
  sprintf(tmpchr,"%s",datafilename.c_str());

  //==================================================================================
  //--read in INL tables
  char inltablename[200];
  sprintf(inltablename,"tdig-ser48");
  double inlcor[3][8][1024];
  for(int i=0;i<1024;i++){for(int ich=0;ich<8;ich++){for(int itdc=0;itdc<3;itdc++)inlcor[itdc][ich][i]=0;}}

  for(int itdc=0;itdc<3;itdc++){
    for(int ich=0;ich<8;ich++){
      // read in inltable file 
      sprintf(tmpchr,"../result/%s/inl/%s.tdc%d.ch%d.inl",inltablename,inltablename,itdc,ich);
      ifstream inlinput(tmpchr);
      if(!inlinput){cout<<"ERROR!! Can not find "<<tmpchr<<endl;return -1;}
      cout<<"INL table:"<<tmpchr<<endl;

      float tbin,cor;
      int index=0;
      while(inlinput>>tbin>>cor){
        int ibin = int(tbin-0.5);
        inlcor[itdc][ich][ibin]=cor;
        index++;
      }  
    }   
  }
  // end read INL tables

  TH1F* allToT1h;
  TH1F* all2hitsToT1h;
  allToT1h=new TH1F("allToT1h","allToT1h",400,0,50);
  all2hitsToT1h=new TH1F("all2hitsToT1h","all2hitsToT1h",400,0,50);
  TH1F* hitsperevent1h=new TH1F("hitsperevent","hitsperevent",40,0,40);

  TH1F* ToT1h[10];
  TH1F* twohitsToT1h[10];
  for(int i=0;i<10;i++){
    sprintf(hisname,"ToT_ch%d",i);
    ToT1h[i]=new TH1F(hisname,hisname,400,0,50);
    sprintf(hisname,"twohitsToT_ch%d",i);
    twohitsToT1h[i]=new TH1F(hisname,hisname,400,0,50);
  }

  TH2F* ToTvschan2h=new TH2F("ToTvschan","ToTvschan",20,0,10,200,0,40);
  TH2F* aveToTvschan2h=new TH2F("aveToTvschan","aveToTvschan",20,0,10,100,0,40);

  //======================================================================= 
  unsigned int itime,tempv,tempv1;
  float ftime;
  int numret;
  int tdig,edgeid,tdcid,tdcchan,leglobalchan,teglobalchan;
  int letdig[50],letdcid[50],letdcchan[50],lechan[50];
  int tetdig[50],tetdcid[50],tetdcchan[50],techan[50];
  float letime[50],tetime[50],gletime[50],gtetime[50];
 
  int hitevents=0,wrongedgeevents=0;

  float lasttime=0;
  int lehits=0,tehits=0;;
  int lostleevents=0,lostteevents=0;
  while ((numret=fscanf(inputfile,"%x",&tempv1))==1) {
    fscanf(inputfile,"%x",&tempv);
    if(tempv>>16==0xe000) {cout<<" ----------- Find a Seperator: "<<hex<<tempv<<endl;continue;}

    edgeid =int( (tempv & 0xf0000000)>>28 );
    if(edgeid==2) {
       if(lehits+tehits>0){
         hitevents++;hitsperevent1h->Fill(lehits+tehits);
         if(lehits<tehits) lostleevents++;
         if(tehits<lehits) lostteevents++;
       };
       lehits=0;tehits=0;
       continue;
    }

    if((edgeid != 4) && (edgeid != 5)){
      //cout<<"ERROR!!----------------Wrong edge="<<edgeid<<" value=0x"<<hex<<tempv<<endl;
      wrongedgeevents++;
      continue;
    } 

    tdcid = (tempv & 0x0F000000)>>24; 
    tdig = tdcid/4; tdcid = tdcid%4;
    tdcchan = (tempv&0x00E00000)>>21;


    itime=((tempv&0x7ffff)<<2)+((tempv>>19)&0x00000003);
    int bin=itime&0x000003ff;
    //ftime= (itime+inlcor[tdcid][tdcchan][bin])*25./1024.;       
    ftime= (itime)*25./1024.;       


    if(edgeid==4) {
      if(tdig==0&&tdcid==0&&tdcchan==0) leglobalchan=0;
      if(tdig==0&&tdcid==0&&tdcchan==2) leglobalchan=1;
      if(tdig==0&&tdcid==0&&tdcchan==3) leglobalchan=2;
      if(tdig==0&&tdcid==2&&tdcchan==0) leglobalchan=3;
      if(tdig==0&&tdcid==2&&tdcchan==6) leglobalchan=4;

      if(tdig==1&&tdcid==0&&tdcchan==0) leglobalchan=5;
      if(tdig==1&&tdcid==0&&tdcchan==2) leglobalchan=6;
      if(tdig==1&&tdcid==0&&tdcchan==3) leglobalchan=7;
      if(tdig==1&&tdcid==2&&tdcchan==0) leglobalchan=8;
      if(tdig==1&&tdcid==2&&tdcchan==6) leglobalchan=9;

      letdig[lehits]=tdig;letdcid[lehits]=tdcid;letdcchan[lehits]=tdcchan;letime[lehits]=ftime;lechan[lehits]=leglobalchan;

      lehits++;
    }
    if(edgeid==5) {

      if(tdig==0&&tdcid==1&&tdcchan==7) teglobalchan=0;
      if(tdig==0&&tdcid==1&&tdcchan==3) teglobalchan=1;
      if(tdig==0&&tdcid==1&&tdcchan==4) teglobalchan=2;
      if(tdig==0&&tdcid==1&&tdcchan==5) teglobalchan=3;
      if(tdig==0&&tdcid==1&&tdcchan==1) teglobalchan=4;

      if(tdig==1&&tdcid==1&&tdcchan==7) teglobalchan=5;
      if(tdig==1&&tdcid==1&&tdcchan==3) teglobalchan=6;
      if(tdig==1&&tdcid==1&&tdcchan==4) teglobalchan=7;
      if(tdig==1&&tdcid==1&&tdcchan==5) teglobalchan=8;
      if(tdig==1&&tdcid==1&&tdcchan==1) teglobalchan=9;

      tetdig[tehits]=tdig;tetdcid[tehits]=tdcid;tetdcchan[tehits]=tdcchan;tetime[tehits]=ftime;techan[tehits]=teglobalchan;
      tehits++;
    }

    float aToT=0;
    if(lehits==1&&tehits==1) {
      aToT=tetime[0]-letime[0];
      allToT1h->Fill(aToT);
      all2hitsToT1h->Fill(aToT);
      ToT1h[teglobalchan]->Fill(aToT);
      twohitsToT1h[teglobalchan]->Fill(aToT);
      ToTvschan2h->Fill(teglobalchan,aToT);

    } else {
      for(int ite=0;ite<tehits;ite++){
        int tech=techan[ite];
        for(int ile=0;ile<lehits;ile++){
          int lech=lechan[ile];
          if(lech==tech) {
            aToT=tetime[ite]-letime[ile];
            allToT1h->Fill(aToT);
            ToT1h[lech]->Fill(aToT);
            ToTvschan2h->Fill(lech,aToT);
          }
        }
      }
    }
    
    cout<<"events="<<hitevents<<" value:0x"<<hex<<tempv<<dec<<" TDIG="<<tdig<<" edge="<<edgeid<<" tdcid="<<tdcid<<" tdcch="<<tdcchan<<" time="<<itime<<" ftime="<<ftime<<endl; 

  }
  fclose(inputfile);

  for(int i=0;i<10;i++){
    aveToTvschan2h->Fill(i,twohitsToT1h[i]->GetMean());
  }

  TFile* outfile=new TFile("cosmic_analysis.root","recreate");

  allToT1h->Write();
  all2hitsToT1h->Write();
  hitsperevent1h->Write();
  for(int i=0;i<10;i++){ToT1h[i]->Write();}
  ToTvschan2h->Write();
  aveToTvschan2h->Write();
  outfile->Close();

  cout<<"Processed total hits= "<<hitevents<<" , with wrong edge events="<<wrongedgeevents<<endl;
  cout<<" lost le events="<<lostleevents<<" lost te events="<<lostteevents<<endl;
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
