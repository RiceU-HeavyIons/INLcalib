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
    cout<<"Usage: allsummary_analysis -h or summary_analysis --help to get help"<<endl;
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
  TH1F* inl_max1h;
  TH1F* inl_min1h;

  int nboard=50;

  sprintf(tmpchr,"max_inl");
  inl_max1h=new TH1F(tmpchr,tmpchr,200,-20,20);
  sprintf(tmpchr,"min_inl");
  inl_min1h=new TH1F(tmpchr,tmpchr,200,-20,20);

  char toppath[200];
  sprintf(toppath,"/var/www/html/production-test/");
  //==============================================================================================
  sprintf(tmpchr,"01082008-tdig.txt");
  ifstream alltdiginput(tmpchr);
  if(!alltdiginput) {cout<<"Error!! Can not open "<<tmpchr<<endl;return -1;}
  string tdigsn;
  int ntdig=0;

  while (alltdiginput>>tdigsn){  // loop all boards.

    TFile* hfile;

    sprintf(tmpchr,"%s/%s/root/%s.inl.root",toppath,tdigsn.c_str(),tdigsn.c_str());

    cout<<"Open: "<<tmpchr<<endl;
    hfile=new TFile(tmpchr,"READ");

    hfile->cd();
    for(int itdc=0;itdc<3;itdc++){
      for(int ich=0;ich<8;ich++){
        sprintf(tmpchr,"TDC%dch%d_inl1024h",itdc,ich);
        TH1F* temp0h=(TH1F*)hfile->Get(tmpchr);
        float mean0=temp0h->GetMean();
        if(mean0<1) continue;
        if(tdigsn=="tdig-ser32" && itdc==0 && ich==2) continue;
        if(tdigsn=="tdig-ser134" && itdc==0 && ich==7) continue;
        float highest=temp0h->GetMaximum();
        float lowest=temp0h->GetMinimum();
        if(lowest>-0.5) cout<<"tdc="<<itdc<<" chan="<<ich<<" inl lowest="<<lowest<<endl;
        if(highest<0.5) cout<<"tdc="<<itdc<<" chan="<<ich<<" inl highest="<<highest<<endl;

        inl_max1h->Fill(highest);
        inl_min1h->Fill(lowest);

      }
    }
    ntdig++;
    hfile->Close();

  }  // end wile alltdig boards

  // write everything to root file

  cout<<"Processed "<<ntdig<<" TDIG boards"<<endl;

  sprintf(tmpchr,"./inlallsummary.root");
  cout<<"Write to root file :"<<tmpchr<<endl;
  TFile* fFile=new TFile(tmpchr,"recreate");
  inl_max1h->Write();
  inl_min1h->Write();

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

