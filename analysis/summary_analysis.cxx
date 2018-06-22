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
  if(argc<2) {
    cout<<"Usage: summary_analysis -h or summary_analysis --help to get help"<<endl;
    return -1;
  }
  string datafilename="unknown";
  string processmode="8";
  if (argc>1){
    for (int i=1;i<(argc);i++){
      if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")){
        cout<<"Usage: ----------->"<<endl;
        cout<<argv[0]<<" -mode 1, or 8 "<<endl;
        cout<<argv[0]<<" -f datafilename "<<endl;
        return 0;
      }
      if (!strcmp(argv[i],"-f"))   {datafilename=argv[++i];}
      if (!strcmp(argv[i],"-mode"))   {processmode=argv[++i];}
    }
  }

  char tmpchr[200];
  string ComFileName=datafilename;
  int dotpos=ComFileName.find(".dat");  
  if(dotpos>0)ComFileName=ComFileName.erase(dotpos,ComFileName.length());  
  cout<<"summary_analysis: "<<ComFileName<<endl;
  char filename[200];
  sprintf(filename,"%s",ComFileName.c_str());

  //====================================================
  int activeboard[8]={0,1,-1,-1,4,5,-1,-1};
  string tdigname1[8];
  string tdigname[8];
 
  string tempstr=ComFileName;
  // int dashpos = tempstr.find("-");
  //tempstr= tempstr.erase(0,dashpos+1); 
  
  // File name configuration: tdig-pos0-pos1-pos4-pos5. <--accurrate mdm Apr 2012
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

  //=========================================================

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

  gStyle->SetNdivisions(505,"xyz");

  gStyle->SetLabelSize(0.06,"xyz");
  gStyle->SetLabelSize(0.05,"y");
  gStyle->SetLabelSize(0.08,"x");
  gStyle->SetLabelOffset(0.01,"x");
  gStyle->SetLabelOffset(0.01,"y");
  gStyle->SetTickLength(0.09);
  gStyle->SetTitleColor(2,"xyz");
  gStyle->SetTitleOffset(1.3,"y");

  gStyle->SetTitleX(0.); gStyle->SetTitleY(1.);
  gStyle->SetTitleW(0.98); gStyle->SetTitleH(0.08);
  gStyle->SetOptTitle(0);

  gStyle->SetStatX(0.90); gStyle->SetStatY(0.90);
  gStyle->SetStatW(0.20); gStyle->SetStatH(0.15);

  gStyle->SetOptFit(11);
  gStyle->SetOptStat(0);

  gStyle->SetPadGridX(0);
  gStyle->SetPadGridY(0);

  //

	
  float maxsigma=61.;
  float minsigma=0.;
  TH2F* meanvssigma2h[8][3];
  TH2F* ndtimevssigma2h[8][3];
  TH2F* avesigmavschan2h[8][3];
  TH1F* sigma1h[8][3];
  TH1F* avechansigma1h[8][3][8];
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
    for(int itdc=0;itdc<3;itdc++){
      sprintf(tmpchr,"%s_ndtime_vs_sigma_tdc%d",tdigname1[iboard].c_str(),itdc);
      ndtimevssigma2h[iboard][itdc]=new TH2F(tmpchr,tmpchr,280,-0.5,28.5,300,minsigma,maxsigma);
      sprintf(tmpchr,"%s_mean_vs_sigma_tdc%d",tdigname1[iboard].c_str(),itdc);
      meanvssigma2h[iboard][itdc]=new TH2F(tmpchr,tmpchr,1000,0.,20000.5,50,0.,1.5*maxsigma);
      sprintf(tmpchr,"%s_Avesigma_vs_chan_tdc%d",tdigname1[iboard].c_str(),itdc);
      avesigmavschan2h[iboard][itdc]=new TH2F(tmpchr,tmpchr,80,-0.5,7.5,300,minsigma,maxsigma-0.2);

      sprintf(tmpchr,"%s_TDC%d_all_sigma",tdigname1[iboard].c_str(),itdc);
      sigma1h[iboard][itdc]=new TH1F(tmpchr,tmpchr,50,0.,1.5*maxsigma);
    }
    //
    for(int itdc=0;itdc<3;itdc++){
      for(int ich=0;ich<8;ich++){
        sprintf(tmpchr,"%s_avesigma_tdc%d_ch%d",tdigname1[iboard].c_str(),itdc,ich);
        avechansigma1h[iboard][itdc][ich]=new TH1F(tmpchr,tmpchr,50,0.,2*maxsigma);
      }
    }
  }
  //---------
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;

    //====================================================================
    TFile* hfile[3];
    for(int itdc=0;itdc<3;itdc++){
      sprintf(tmpchr,"../result/%s/root/%s.tdc%d.corr8.root",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc);
      if(processmode=="1")sprintf(tmpchr,"../result/%s/root/%s.tdc%d.corr1.root",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc);
      cout<<"Open: "<<tmpchr<<endl;
      hfile[itdc]=new TFile(tmpchr,"READ");


	
    }
    int icolor[3]={2,3,4};
    int imarker[3]={20,21,23};
    float fmarkersize=1.1;
    for(int itdc=0;itdc<3;itdc++){

	//MDM check stuff. Comment out by mid-may 2012
      //	char tmpchr2[50];
      //	sprintf(tmpchr2,"../result/mdm/%s/gif/%s.tdc%d.mdm",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc);
      //	ofstream mdmcheck;
      //	mdmcheck.open(tmpchr2);

      hfile[itdc]->cd();
      for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
          if(j==i) continue;
          sprintf(tmpchr,"%s_tdc%d_deltaT_ch%d_ch%d",tdigname1[iboard].c_str(),itdc,i,j);
          TH1F* temp0h=(TH1F*)hfile[itdc]->Get(tmpchr);
          float mean0=temp0h->GetMean();
          int  maxbin=temp0h->GetMaximumBin();
          mean0 = temp0h->GetBinCenter(maxbin);
          temp0h->SetAxisRange(mean0-6,mean0+6,"X");
          float sigma0=temp0h->GetRMS(); 
          //if(sigma0>3) continue;    // to get rid of some big RMS due to bad/dead channel 
          mean0=temp0h->GetMean();
          sigma0=temp0h->GetRMS(); 
          avechansigma1h[iboard][itdc][i]->Fill(1000.*25./1024.*sigma0);
        }
        avesigmavschan2h[iboard][itdc]->Fill(i,avechansigma1h[iboard][itdc][i]->GetMean());

	//More MDM stuff. Comment out by mid-may 2012
	//	mdmcheck << i << "     " << (avechansigma1h[iboard][itdc][i]->GetMean()) << "\n";

        avesigmavschan2h[iboard][itdc]->SetMarkerColor(icolor[itdc]);
        avesigmavschan2h[iboard][itdc]->SetMarkerStyle(imarker[itdc]);
        avesigmavschan2h[iboard][itdc]->SetMarkerSize(fmarkersize);

      }
	
	//last line of mdmcheck stuff. 
      //	mdmcheck.close();
    }
    //--------
    for(int itdc=0;itdc<3;itdc++){
      int ncomb=0;
      hfile[itdc]->cd();

      for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
          if(j<=i) continue;
          ncomb++;

          sprintf(tmpchr,"%s_tdc%d_deltaT_ch%d_ch%d",tdigname1[iboard].c_str(),itdc,i,j);
          TH1F* temp0h=(TH1F*)hfile[itdc]->Get(tmpchr);
          float mean0=temp0h->GetMean();
          int  maxbin=temp0h->GetMaximumBin();
          mean0 = temp0h->GetBinCenter(maxbin);
          temp0h->SetAxisRange(mean0-6,mean0+6,"X");
          mean0=temp0h->GetMean();
          float sigma0=temp0h->GetRMS(); 
          //if(sigma0>3) continue;    // to get rid of some big RMS due to bad/dead channel 
          sigma0=temp0h->GetRMS(); 

          meanvssigma2h[iboard][itdc]->Fill(mean0,sigma0*1000.*25./1024.);
          sigma1h[iboard][itdc]->Fill(sigma0);
          ndtimevssigma2h[iboard][itdc]->Fill(ncomb,sigma0*1000.*25./1024.);

          ndtimevssigma2h[iboard][itdc]->SetMarkerColor(icolor[itdc]);
          ndtimevssigma2h[iboard][itdc]->SetMarkerStyle(imarker[itdc]);
          ndtimevssigma2h[iboard][itdc]->SetMarkerSize(fmarkersize);
          meanvssigma2h[iboard][itdc]->SetMarkerColor(icolor[itdc]);
          meanvssigma2h[iboard][itdc]->SetMarkerStyle(imarker[itdc]);
          meanvssigma2h[iboard][itdc]->SetMarkerSize(fmarkersize);
          sigma1h[iboard][itdc]->SetLineColor(icolor[itdc]);
          sigma1h[iboard][itdc]->SetLineWidth(2);
        }
      }
    }
    // write everything to root file

    
    sprintf(tmpchr,"../result/%s/root/%s.summary.root",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    if(processmode=="1")sprintf(tmpchr,"../result/%s/root/%s.summary1.root",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    cout<<"Write to root file :"<<tmpchr<<endl;
    TFile* fFile=new TFile(tmpchr,"recreate");
    for(int itdc=0;itdc<3;itdc++){
      ndtimevssigma2h[iboard][itdc]->Write();
      meanvssigma2h[iboard][itdc]->Write(); 
      avesigmavschan2h[iboard][itdc]->Write();
      sigma1h[iboard][itdc]->Write();
    }

    fFile->Close();

    for(int itdc=0;itdc<3;itdc++){hfile[itdc]->Close();}

  }  // end loop board


  //===============================================================================
  TLatex* tex;
  TLine* line;
  TCanvas *c1 = new TCanvas("c1", "c1",0,0,750,650);
  c1->Divide(1,2,0.01,0.01);c1->SetFillColor(0);

  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
    c1->cd(1);
    gPad->SetGridy(1);
    TLegend *leg1 = new TLegend(0.11,0.70,0.23,0.90);leg1->SetBorderSize(1);leg1->SetFillColor(0);
    leg1->AddEntry(ndtimevssigma2h[iboard][0],"TDC0","lp");
    leg1->AddEntry(ndtimevssigma2h[iboard][1],"TDC1","lp");
    leg1->AddEntry(ndtimevssigma2h[iboard][2],"TDC2","lp");

    ndtimevssigma2h[iboard][0]->Draw();
    ndtimevssigma2h[iboard][1]->Draw("same");
    ndtimevssigma2h[iboard][2]->Draw("same");
    int ncomb=0;
    for(int i=0;i<8;i++){
      for(int j=0;j<8;j++){
        if(j<=i) continue;
        ncomb++;

        sprintf(tmpchr,"ch %d-%d",i,j);
        float xpos=ncomb-0.44;
        tex = new TLatex(xpos,minsigma,tmpchr);
        tex->SetTextColor(ncomb%2+7);
        tex->SetTextSize(0.044);tex->SetTextAngle(70);tex->SetLineWidth(1);
        tex->Draw();
      }
    }
 
    tex = new TLatex(-2.1,30.0,"Sigma (ps)");
    tex->SetTextAngle(90);tex->SetLineWidth(2);
    tex->Draw();
    tex = new TLatex(22,minsigma-6,"N (ch x-ch y)");
    tex->SetTextSize(0.08);tex->SetLineWidth(2);
    tex->Draw();
    //----
    sprintf(tmpchr,"%s",tdigname[iboard].c_str());
    tex = new TLatex(5.,maxsigma-5,tmpchr);
    tex->SetTextSize(0.055);
    tex->Draw();

    line=new TLine(-0.5,45,28.5,45);
    line->SetLineWidth(2);
    line->SetLineColor(6);
    line->SetLineStyle(2);
    line->Draw();

    tex = new TLatex(12.,48,"Requirement");
    tex->SetTextSize(0.065);
    tex->SetTextColor(6);
    tex->Draw();

    leg1->Draw();
    //=======================================================================
    c1->cd(2);

    gPad->SetGridy(1);
    TLegend *leg2 = new TLegend(0.11,0.70,0.23,0.90);leg2->SetBorderSize(1);leg2->SetFillColor(0);
    leg2->AddEntry(avesigmavschan2h[iboard][0],"TDC0","lp");
    leg2->AddEntry(avesigmavschan2h[iboard][1],"TDC1","lp");
    leg2->AddEntry(avesigmavschan2h[iboard][2],"TDC2","lp");

    avesigmavschan2h[iboard][0]->Draw();
    avesigmavschan2h[iboard][1]->Draw("same");
    avesigmavschan2h[iboard][2]->Draw("same");
 
    tex = new TLatex(-0.9,22,"Ave. Sigma (ps)");
    tex->SetTextAngle(90);tex->SetLineWidth(2);
    tex->Draw();
    tex = new TLatex(6.5,minsigma-6,"Chan #");
    tex->SetTextSize(0.075);tex->SetLineWidth(2);
    tex->Draw();


    line=new TLine(-0.5,45,7.5,45);
    line->SetLineWidth(2);
    line->SetLineColor(6);
    line->SetLineStyle(2);
    line->Draw();

    tex = new TLatex(3.,48,"Requirement");
    tex->SetTextSize(0.065);
    tex->SetTextColor(6);
    tex->Draw();

    leg2->Draw();

    //============================================================================
    sprintf(tmpchr,"../result/%s/gif/%s-summary.gif",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    if(processmode=="1")sprintf(tmpchr,"../result/%s/gif/%s-summary1.gif",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    c1->Print(tmpchr);
  }  // end loop boards
  //
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

