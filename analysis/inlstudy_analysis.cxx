#include <iostream>
#include <string>
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
#include <TLatex.h>
#include <TROOT.h>
#include <TBenchmark.h>     // ROOT head file for benchmarking applications
#include <TApplication.h>
#include <TH2.h>

#include <TBenchmark.h>     // ROOT head file for benchmarking applications

using namespace std;

int main(int argc, char *argv[])
{

  // need this line to make "gif" files online....do not run code in batch mode.
  TApplication* theApp=new TApplication("app",&argc ,argv);

  // time statistic --- start
  TBenchmark* gBenchmark = new TBenchmark();
  //start time of this program
  gBenchmark->Start("main");

  // process commandline
  if(argc<1) {
    cout<<"Usage: inlstudy_analysis -h or inlstudy_analysis --help to get help"<<endl;
    return -1;
  }
  string datafilename="unknown";
  if (argc>1){
    for (int i=1;i<(argc);i++){
      if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")){
        cout<<"Usage: ----------->"<<endl;
        return 0;
      }
    }
  }
  //
  char tmpchr[200];
  TH2F* ndtimevssigma2h;
  TH2F* avesigmavschan2h;
  TH1F* sigma1h;
  TH1F* sigma2h;

  float maxsigma=60.;
  float minsigma=5.;
  int nboard=18;

  sprintf(tmpchr,"ndtime_vs_sigma");
  ndtimevssigma2h=new TH2F(tmpchr,tmpchr,56*3*nboard,-0.5,3*28*nboard-0.5,500,minsigma,maxsigma-0.1);

  sprintf(tmpchr,"Avesigma_vs_chan");
  avesigmavschan2h=new TH2F(tmpchr,tmpchr,48*nboard,-0.5,nboard*24-0.5,500,minsigma,maxsigma-0.1);

  sprintf(tmpchr,"all_deltaT_sigma");
  sigma1h=new TH1F(tmpchr,tmpchr,250,0.,maxsigma);

  sprintf(tmpchr,"allchan_ave_sigma");
  sigma2h=new TH1F(tmpchr,tmpchr,250,0.,maxsigma);

  TH1F* onechansigma=new TH1F("onechansigma","onechansigma",500,0,150);

  char toppath[200];
  char special[100];
  sprintf(special,"INL512x100");
  sprintf(toppath,"../special_analysis/%s",special);
  //==============================================================================================
  sprintf(tmpchr,"temp.txt");
  ifstream alltdiginput(tmpchr);
  if(!alltdiginput) {cout<<"Error!! Can not open "<<tmpchr<<endl;return -1;}
  string tdigsn;
  int ntdig=0;

  while (alltdiginput>>tdigsn){  // loop all boards.

    string histdig = tdigsn;
    int dot = histdig.find(".");
    if(dot>0) histdig = histdig.erase(dot,histdig.length());
    int dash = histdig.find("-");
    if(dash>0) histdig = histdig.erase(0,dash+1);
    // cout<<" tdigsn = "<<tdigsn<<" hisdig="<<histdig<<endl;

    TFile* hfile[3];
    for(int itdc=0;itdc<3;itdc++){  // loop all TDCs.
      sprintf(tmpchr,"%s/%s/root/%s.tdc%d.corr8.root",toppath,tdigsn.c_str(),tdigsn.c_str(),itdc);

      cout<<"Open: "<<tmpchr<<" ntdig="<<ntdig<<endl;
      hfile[itdc]=new TFile(tmpchr,"READ");

      hfile[itdc]->cd();
      int basenpoint=ntdig*28*3+itdc*28;
      int npoint=0;
      for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
          if(j==i) continue;
          sprintf(tmpchr,"tdig_%s_tdc%d_deltaT_ch%d_ch%d",histdig.c_str(),itdc,i,j);

          TH1F* temp0h=(TH1F*)hfile[itdc]->Get(tmpchr);
	  //cout<<tmpchr<<endl;
          float mean0=temp0h->GetMean();
          float sigma0=temp0h->GetRMS(); 
          temp0h->SetAxisRange(mean0-30,mean0+30,"X");
          sigma0=temp0h->GetRMS(); 
          float sigma0ps=sigma0*1000.*24.5/1024.;
          if(sigma0>3||sigma0<0.4) {
            cout<<"ERROR! "<<tdigsn<<" sigma0="<<sigma0<<" "<<tmpchr<<endl;
            //continue;
          }    // to get rid of some big RMS due to bad/dead channel 
          onechansigma->Fill(sigma0ps);
          if(j>i) {
            sigma1h->Fill(sigma0ps);
            //cout<<"nn="<<basenpoint+npoint<<" itdc="<<itdc<<" i="<<i<<" j="<<j<<" "<<mean0<<" "<<sigma0<<" "<<basenpoint+npoint<<endl;
            ndtimevssigma2h->Fill(float(basenpoint+npoint),sigma0ps);
            npoint++;
	  }
        }
        avesigmavschan2h->Fill(i+ntdig*24+itdc*8,onechansigma->GetMean());
        sigma2h->Fill(onechansigma->GetMean());
        onechansigma->Reset();
      }
      hfile[itdc]->Close();
    } //  TDC loop
    ntdig++;
  }  // end wile alltdig boards

  cout<<" Processed "<<ntdig<<" boards"<<endl;
  // write everything to root file

  sprintf(tmpchr,"./inlstudy_%s.root",special);
  cout<<"Write to root file :"<<tmpchr<<endl;
  TFile* fFile=new TFile(tmpchr,"recreate");
  ndtimevssigma2h->SetMarkerStyle(5);
  ndtimevssigma2h->SetMarkerColor(2);
  ndtimevssigma2h->SetMarkerSize(0.5);

  avesigmavschan2h->SetMarkerStyle(5);
  avesigmavschan2h->SetMarkerColor(2);
  avesigmavschan2h->SetMarkerSize(0.5);

  ndtimevssigma2h->Write();
  avesigmavschan2h->Write();
  sigma1h->Write();
  sigma2h->Write();

  fFile->Close();

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

