
#define _FILE_OFFSET_BITS  64

#include <iostream>
#include <string>
#include <sstream>
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

int main(int argc, char *argv[])
{

  // need this line to make "gif" files online....do not run code in batch mode.
  //TApplication* theApp=new TApplication("app",&argc ,argv);
  TApplication* theApp=new TApplication("app",0 ,0);
  //
  // time statistic --- start
  TBenchmark* gBenchmark = new TBenchmark();
  //start time of this program
  gBenchmark->Start("main");

  string processmode="8";
  int processboard=-1;
  // process commandline
  if(argc<2) {
    cout<<"Usage: inl_analysis --help to get help"<<endl;
    return -1;
  }
  string datafilename="unknown";
  if (argc>=2){
    for (int i=1;i<(argc);i++){
      if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")){
        cout<<"Usage: ----------->"<<endl;
        cout<<argv[0]<<" -f datafilename "<<endl;
        cout<<argv[0]<<" -mode 1, or 8 "<<endl;
        cout<<argv[0]<<" -board 0,1,4,5 or -1 for all "<<endl;
        return 0;
      }
      if (!strcmp(argv[i],"-f"))   {datafilename=argv[++i];}
      if (!strcmp(argv[i],"-mode"))   {processmode=argv[++i];}
      if (!strcmp(argv[i],"-board"))   {processboard=atoi(argv[++i]);}
    }
  }

  string ComFileName=datafilename;
  int dotpos=ComFileName.find(".dat");  
  if(dotpos>0)ComFileName=ComFileName.erase(dotpos,ComFileName.length());  
  cout<<"inl_analysis: "<<ComFileName<<endl;


  int activeboard[8]={0,-1,-1,-1,-1,-1,-1,-1};
  string tdigname1[8];
  string tdigname[8];

  
  // File name configuration: tdig-pos0-pos1-pos4-pos5.
  //  actually, it is tdig-pos4-pos5-pos0-pos1.dat
  // TCPU: J4: should be 4, 5, but we connected as: 0,1, 5/29/2008, will keep this configuration in calibration.
  // TCPU: J5: should be 0, 1, but we connected as: 4,5, 5/29/2008, will keep this configuration in calibration.
  // should reverse ??  confirm?
  // should be:
  //tdigname[0].assign(tempstr,5,3);
  //tdigname[1].assign(tempstr,9,3);
  //tdigname[4].assign(tempstr,13,3);
  //tdigname[5].assign(tempstr,17,3);
  // but we connected them like this:
  //string tempstr=ComFileName;
  //tdigname[4].assign(tempstr,5,3);
  //tdigname[5].assign(tempstr,9,3);
  //tdigname[0].assign(tempstr,13,3);
  //tdigname[1].assign(tempstr,17,3);
  //
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
 
  char tmpchr[200];
  char filename[200];
  sprintf(filename,"%s",ComFileName.c_str());

  float inlcor[1024];
  //
  TH1F* lsb2048h[8][3][8];
  TH1F* dnl2048h[8][3][8];
  TH1F* dnl1024h[8][3][8];
  TH1F* inl2048h[8][3][8];
  TH1F* inl1024h[8][3][8];

  TH2F* checkchannel2h[8];

  for(int iboard=0;iboard<8;iboard++){
    //if(activeboard[iboard]<0) continue;
    for(int itdc=0;itdc<3;itdc++){
      for(int ich=0;ich<8;ich++){
        sprintf(tmpchr,"%s_tdc%dch%d_lsb2048h",tdigname1[iboard].c_str(),itdc,ich);
        lsb2048h[iboard][itdc][ich] = new TH1F(tmpchr,tmpchr,2048,-0.5,2047.5);
        sprintf(tmpchr,"%s_tdc%dch%d_dnl2048h",tdigname1[iboard].c_str(),itdc,ich);
        dnl2048h[iboard][itdc][ich] = new TH1F(tmpchr,tmpchr,2048,-0.5,2047.5);
        sprintf(tmpchr,"%s_tdc%dch%d_dnl1024h",tdigname1[iboard].c_str(),itdc,ich);
        dnl1024h[iboard][itdc][ich] = new TH1F(tmpchr,tmpchr,1024,-0.5,1023.5);
        sprintf(tmpchr,"%s_tdc%dch%d_inl2048h",tdigname1[iboard].c_str(),itdc,ich);
        inl2048h[iboard][itdc][ich] = new TH1F(tmpchr,tmpchr,2048,-0.5,2047.5);
        sprintf(tmpchr,"%s_tdc%dch%d_inl1024h",tdigname1[iboard].c_str(),itdc,ich);
        inl1024h[iboard][itdc][ich] = new TH1F(tmpchr,tmpchr,1024,-0.5,1023.5);
      }
    }
    sprintf(tmpchr,"%s_hit_map",tdigname1[iboard].c_str());
    checkchannel2h[iboard]=new TH2F(tmpchr,tmpchr,3,-0.5,2.5,8,-0.5,7.5);
  }

  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
   
    //create some directory:
    sprintf(tmpchr,"./tempana/%s",tdigname[iboard].c_str());CreateDir(tmpchr);
    sprintf(tmpchr,"./tempana/%s/inl",tdigname[iboard].c_str());CreateDir(tmpchr);
    sprintf(tmpchr,"./tempana/%s/root",tdigname[iboard].c_str());CreateDir(tmpchr);
    sprintf(tmpchr,"./tempana/%s/gif",tdigname[iboard].c_str());CreateDir(tmpchr);
  }
  sprintf(tmpchr,"../rawdata/%s.dat",filename);
  FILE* inputfile=fopen(tmpchr,"rb");
  if(!inputfile) {cout<<"Can not open "<<tmpchr<<endl;return -1;};  
  cout<<"Process Data File:"<<tmpchr<<" "<<endl;


  unsigned int tempv;
  int nwords=0;
  int numret=0;
  int tdcid, tdcchan, tdigboardid,edgeid;
  int lsb;
  int lastevents=0;
  int goodevents=0;
  unsigned int itime;
  int wrongedge=0;
  int halftrayid=-1;

  while (numret=fread(&nwords,4,1,inputfile)==1) {
    //if(goodevents>10000) continue;
    //cout<<"-----------------------------------------------------------------------------"<<endl;
    if(goodevents%1000000==1&&goodevents>lastevents){lastevents=goodevents;cout<<" processed events "<<goodevents<<endl;}
    //cout<<hex<<"0x"<<nwords<<endl;
    // nwords saves the total data words in a events.

    for(int iword=0;iword<nwords;iword++){

      fread(&tempv,4,1,inputfile);
      //if(nwords != 0x66) continue;
      //cout<<hex<<"0x"<<tempv<<endl;

      if( (tempv&0xF0000000)>>28 == 0xE) continue;
      if((tempv>>16) ==0xa000) continue;
      if(tempv    ==0xdeadface) {halftrayid = 1; 
         continue;}
      if( (tempv&0xF0000000)>>28 == 0xC) {halftrayid = tempv&0x01;continue;}
      if( (tempv&0xF0000000)>>28 == 0x2) {continue;}

      edgeid =int( (tempv & 0xf0000000)>>28 );
      if((edgeid != 4)){if(wrongedge<50)cout<<"ERROR!!Wrong edge="<<edgeid<<hex<<"0x"<<tempv<<endl;wrongedge++;continue;} 
     
      tdcid = (tempv & 0x0F000000)>>24;  
      tdigboardid = tdcid/4 ;
      tdcid = tdcid % 4;
      if(tdcid >=3) {cout<<"TDC ID =="<<tdcid<<" check your configuration !!"<<endl;return -1;}
      tdcchan = (tempv&0x00E00000)>>21;
      if(tdcchan<0 || tdcchan >8){cout<<"TDCid="<<tdcid<<" TDCchan="<<tdcchan<<" ERROR!!"<<endl;return -1;}

      itime=((tempv&0x7ffff)<<2)+((tempv>>19)&0x00000003);
      lsb=((tempv&0x1ff)<<2)+((tempv>>19)&0x00000003);

      //if(goodevents<50)cout<<"value:0x"<<hex<<tempv<<dec<<" TDIG="<<tdigboardid<<" edge="<<edgeid<<" tdcid="<<tdcid<<" tdcchan="<<tdcchan<<" itime="<<itime<<endl; 
      checkchannel2h[tdigboardid]->Fill(tdcid,tdcchan);
    // fill histogram:
    if(processmode=="8")lsb2048h[tdigboardid][tdcid][tdcchan]->Fill(lsb);
    if(processmode=="1"){for(int ich=0;ich<8;ich++){lsb2048h[tdigboardid][tdcid][ich]->Fill(lsb);}}
 
   }

    goodevents++;
    //
  }
  fclose(inputfile);
  cout<<"Good events = "<<goodevents<<" wrong edge words ="<<wrongedge<<endl;

  // now make INL table.....
 
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;
 
    float lsbdata[2048];      
    float dnl2048[2048];
    float inl2048[2048];
    for(int i=0;i<2048;i++){lsbdata[i]=0;dnl2048[i]=0;inl2048[i]=0;}

   for(int itdc=0;itdc<3;itdc++){
      for(int ich=0;ich<8;ich++){
        //----------------------------------------------------------------
        float hihit=-1;float lowhit=99999999;
        float hidnl=-20000;float lowdnl=20000;
        float TotHit=0.;float TotBin=0;
        for(int ibin=0;ibin<lsb2048h[iboard][itdc][ich]->GetNbinsX();ibin++){
          Stat_t count=lsb2048h[iboard][itdc][ich]->GetBinContent(ibin+1);
          lsbdata[ibin]=count;
          TotHit=TotHit+count;
          TotBin++;
        }

        float interdnl=0;
        double expectedhit=double(TotHit)/double(TotBin);
        for(int ibin=0;ibin<TotBin;ibin++){
          dnl2048[ibin]=(lsbdata[ibin] - expectedhit )/expectedhit;
          if(dnl2048[ibin]>hidnl) hidnl=dnl2048[ibin];
          if(dnl2048[ibin]<lowdnl)lowdnl=dnl2048[ibin];
          if(lsbdata[ibin]>hihit) hihit=lsbdata[ibin];
          if(lsbdata[ibin]<lowhit)lowhit=lsbdata[ibin];
          interdnl=interdnl+dnl2048[ibin];
          inl2048[ibin]=interdnl;
          dnl2048h[iboard][itdc][ich]->SetBinContent(ibin,dnl2048[ibin]);
          inl2048h[iboard][itdc][ich]->SetBinContent(ibin,interdnl);
	  // corrected lsb plot       
        }

        // 1024 inltable ..................
        TotHit=0.;TotBin=0.;
        for(int ibin=0;ibin<1024;ibin++){
          TotHit=TotHit+lsbdata[ibin];
          TotBin++;
        }

        expectedhit=double(TotHit)/double(TotBin);
        interdnl=0.;
        for(int i=0;i<1024;i++){inlcor[i]=0;}
        for(int ibin=0;ibin<TotBin;ibin++){
          float dnl=(lsbdata[ibin] - expectedhit )/expectedhit;
          interdnl=interdnl+dnl;
          int tempinl=int(interdnl*10000.);
          inlcor[ibin]=tempinl/10000.;
          dnl1024h[iboard][itdc][ich]->SetBinContent(ibin,dnl);
          inl1024h[iboard][itdc][ich]->SetBinContent(ibin,interdnl);
        }

        // write out INL table
        sprintf(tmpchr,"./tempana/%s/inl/%s.tdc%d.ch%d.inl",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc,ich);
        if(processmode=="1")sprintf(tmpchr,"./tempana/%s/inl/%s.tdc%d.inl",tdigname[iboard].c_str(),tdigname[iboard].c_str(),itdc);
        ofstream inltablefile(tmpchr);
        cout<<"Write out inltable(1024) to:"<<tmpchr<<endl;
        int startbin=0;
        for(int ibin=startbin;ibin<startbin+1024;ibin++){
          inltablefile<<setw(5)<<ibin+0.5-startbin<<" "<<setw(10)<<inlcor[ibin]<<endl;
        }
        inltablefile.close();
      }
    }
    // write everything to root file
    sprintf(tmpchr,"./tempana/%s/root/%s.inl.root",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    cout<<"Write to root file :"<<tmpchr<<endl;
    TFile* fFile=new TFile(tmpchr,"recreate");
    checkchannel2h[iboard]->Write();
    for(int itdc=0;itdc<3;itdc++){
      for(int ich=0;ich<8;ich++){
        lsb2048h[iboard][itdc][ich]->Write();
        dnl2048h[iboard][itdc][ich]->Write();dnl1024h[iboard][itdc][ich]->Write();
        inl2048h[iboard][itdc][ich]->Write();inl1024h[iboard][itdc][ich]->Write();
      }
    }
    fFile->Close();
  }

  //----------------------------------------------------
  //--------------- plot the INL plots to gif file.
  // copyt from plot8inl.C
  // 
      
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
  //gStyle->SetStatFont(42);
  //gStyle->SetTitleFont(42);
  gStyle->SetTitleOffset(1.25,"Y");
  gStyle->SetTitleOffset(1.,"X");
  gStyle->SetTitleSize(0.06);
  gStyle->SetOptFile(1);

  // Set Line Widths
  gStyle->SetFrameLineWidth(2);
  gStyle->SetFuncWidth(2);
  //gStyle->SetHistLineWidth(2);

  // Set margins -- I like to shift the plot a little up and to the
  // right to make more room for axis labels
  gStyle->SetPadTopMargin(0.08);
  gStyle->SetPadBottomMargin(0.12);
  gStyle->SetPadLeftMargin(0.12);
  gStyle->SetPadRightMargin(0.08);
  //
  // Set Data/Stat/... and other options
  // Set tick marks and turn off grids
  gStyle->SetNdivisions(510,"xyz");
  //gStyle->SetPadTickX(1);
  //gStyle->SetPadTickY(1);
  //gStyle->SetTickLength(0.02,"xyz");
  gStyle->SetPadGridX(0);
  gStyle->SetPadGridY(1);
  // Adjust size and placement of axis labels
  gStyle->SetLabelSize(0.06,"x");
  gStyle->SetLabelSize(0.06,"y");
  gStyle->SetLabelOffset(0.005,"x");
  gStyle->SetLabelOffset(0.01,"y");
  // Set paper size for life in the US
  gStyle->SetPaperSize(TStyle::kUSLetter);

  gStyle->SetPalette(1);
  //
  gStyle->SetOptStat(0);
  gStyle->SetStatFont(30);
  gStyle->SetStatTextColor(4);
  gStyle->SetStatColor(0);
  gStyle->SetStatX(0.95); gStyle->SetStatY(0.92);
  gStyle->SetStatW(0.42); gStyle->SetStatH(0.20);

  gStyle->SetTitleX(0.25);gStyle->SetTitleY(0.985);
  gStyle->SetTitleW(0.7);gStyle->SetTitleH(0.065);
  gStyle->SetTitleColor(0);gStyle->SetTitleTextColor(2);
  gStyle->SetOptTitle(0);

  gStyle->SetPaintTextFormat("4.0f");

  gROOT->ForceStyle();
  //=============================================
  TCanvas* c1=new TCanvas("c1","c1",0,0,750,650);
  c1->SetFillColor(0);c1->Divide(1,3,0,0);
  TCanvas* c2=new TCanvas("c2","c2",0,0,750,650);
  c2->SetFillColor(0);

  cout<<" here "<<endl;
  
  for(int iboard=0;iboard<8;iboard++){
    if(activeboard[iboard]<0) continue;

    sprintf(tmpchr,"./tempana/%s/root/%s.inl.root",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    TFile* hfile=new TFile(tmpchr,"READ");
    cout<<"open "<<tmpchr<<endl;
    // TGaxis* A=new TGaxis(); A->SetMaxDigits(2);
    //Global title
    //sprintf(tmpchr,"%s INL ",filename);
    //TPaveLabel* gtitle1=new TPaveLabel(0.3,0.96,0.7,0.999,tmpchr);
    //gtitle1->SetFillColor(0);gtitle1->SetTextColor(4);
    //gtitle1->Draw();
    TPad* pad[3];
    for(int itdc=0;itdc<3;itdc++){
      c1->cd(itdc+1);
      pad[itdc]=(TPad*) gPad;
 
      pad[itdc]->SetTopMargin(0.05);
      if(itdc>0)pad[itdc]->SetTopMargin(0);
      pad[itdc]->SetBottomMargin(0.12);
      if(itdc<2)pad[itdc]->SetBottomMargin(0.);
      pad[itdc]->SetTickx();
      //gPad->SetTicky();
      sprintf(tmpchr,"%s_tdc%dch0_inl1024h",tdigname1[iboard].c_str(),itdc);
      TH1F* temph=(TH1F*) hfile->Get(tmpchr);

      float hmax=temph->GetMaximum();
      float hmin=temph->GetMinimum();
      temph->SetMaximum(hmax+10);temph->SetMinimum(hmin-4);
      temph->Draw();
    
      TLegend* leg1=new TLegend(0.12,0.72,0.18,0.95);leg1->SetFillColor(0);leg1->SetBorderSize(1);
      TLegend* leg2=new TLegend(0.18,0.72,0.24,0.95);leg2->SetFillColor(0);leg2->SetBorderSize(1);
      sprintf(tmpchr,"ch%d",0);leg1->AddEntry(temph,tmpchr,"lp");

      TLegend* leg3=new TLegend(0.25,0.745,0.65,0.945);leg3->SetFillColor(0);leg3->SetBorderSize(0);
      sprintf(tmpchr,"%s TDC%d INL",tdigname[iboard].c_str(),itdc);
      leg3->AddEntry(temph,tmpchr,"");
    
      for(int ich=1;ich<8;ich++){
        sprintf(tmpchr,"%s_tdc%dch%d_inl1024h",tdigname1[iboard].c_str(),itdc,ich);
        temph=(TH1F*) hfile->Get(tmpchr);
        temph->SetLineColor(ich+1);
        sprintf(tmpchr,"ch%d",ich);
        if(ich<4)leg1->AddEntry(temph,tmpchr,"lp");  
        if(ich>=4)leg2->AddEntry(temph,tmpchr,"lp");  
        temph->Draw("same");
      }

      TLatex *   tex = new TLatex(-54.,-3.5,"INL corr (bin)");
      tex->SetTextAngle(90);
      tex->SetTextSize(0.08);
      tex->SetLineWidth(2);
      tex->Draw();
      if(itdc==2) {
        tex = new TLatex(910.707,hmin-7.5,"Bins");
        tex->SetTextSize(0.095);
        tex->SetLineWidth(2);
        tex->Draw();
      }
      if(processmode=="8"){leg1->Draw();leg2->Draw();}
      leg3->Draw();
    }
    //c1->Modified();
    //gtitle1->Draw();
    sprintf(tmpchr,"./tempana/%s/gif/%s-inl.gif",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    if(processmode=="1")sprintf(tmpchr,"./tempana/%s/gif/%s-inl1.gif",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    c1->Print(tmpchr); 

    c2->cd();
    checkchannel2h[iboard]->SetMarkerSize(2);    
    checkchannel2h[iboard]->Draw("text box col");    
    sprintf(tmpchr,"./tempana/%s/gif/%s-checkchannel.gif",tdigname[iboard].c_str(),tdigname[iboard].c_str());
    c2->Print(tmpchr); 

    //
    /*
    if (gROOT->IsBatch())  { 
      sprintf(tmpchr,"pstopnm -ppm -xborder 0 -yborder 0 -portrait ../%s/gif/%s-inl.eps",filename,filename);
      cout<<tmpchr<<endl;
      gSystem->Exec(tmpchr);
      sprintf(tmpchr,"ppmtogif ../%s/gif/%s-inl.eps001.ppm > ../%s/gif/%s-inl.gif",filename,filename,filename,filename);
      cout<<tmpchr<<endl;
      gSystem->Exec(tmpchr);
      sprintf(tmpchr,"rm -f ../%s/gif/%s-inl.eps001.ppm",filename,filename);
      cout<<tmpchr<<endl;   
      gSystem->Exec(tmpchr);
    }
    */ 
    // -- 
    // theApp->Run();  
  }  // end loop boards
  /*
  //=================================================================================
  //make run script of copy files:
  sprintf(tmpchr,"./%s-filescopy.csh",datafilename.c_str());
  ofstream tocopyfiles(tmpchr);
  cout<<"Write out shell script: "<<tmpchr<<endl;
  for(int iboard=7;iboard>=0;iboard--){
    if(activeboard[iboard]<0) continue;
    sprintf(tmpchr,"echo copy files to webpage! ==============  %s  =================",tdigname[iboard].c_str());
    tocopyfiles<<tmpchr<<endl;
    sprintf(tmpchr,"cd ~/production-test/result/%s/inl",tdigname[iboard].c_str());
    tocopyfiles<<tmpchr<<endl;
    sprintf(tmpchr,"tar -cf %s.inl.tar *.inl",tdigname[iboard].c_str());
    tocopyfiles<<tmpchr<<endl;
    sprintf(tmpchr,"cd ~/production-test/result/");
    tocopyfiles<<tmpchr<<endl;
    sprintf(tmpchr,"scp -q -r %s liuj@pcs9.rice.edu:./public_html/production-test/",tdigname[iboard].c_str());
    tocopyfiles<<tmpchr<<endl;
  }
  tocopyfiles.close();
  */
  //
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
