// Created by Peter Denton
// v. 2.0

#include <iostream>
#include <fstream>

#include <TH1.h>
#include <TFile.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TImage.h>
#include <TStyle.h>
#include <TMath.h>

using namespace std;

int main(int argc, char *argv[]) {

char tmpchr[255],proj[255],goodevents[255],tmpchr2[255];
int eventnumber=0,offseton=0,quiet=0;
string projstr;
sprintf(goodevents,"goodevents=");

if(argc<2) {
  cout<<"Usage: cosmic_corr_analysis2 --help to get help"<<endl;
  return -1;
}

if (argc>=2){
  for (int i=1;i<(argc);i++){
    if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")){
      cout<<"Usage: ----------->"<<endl;
      cout<<argv[0]<<" -f datafilename "<<endl;
      cout<<argv[0]<<" -o -- add an offset"<<endl;
      cout<<argv[0]<<" -q -- quiet mode"<<endl;
      return 0;
    }
    if (!strcmp(argv[i],"-f"))   {projstr=argv[++i];}
    if (!strcmp(argv[i],"-o"))   {offseton=1;}
    if (!strcmp(argv[i],"-q"))   {quiet=1;}
  }
}

int dotpos=projstr.find(".dat");
if(dotpos>0)projstr=projstr.erase(dotpos,projstr.length());  
sprintf(proj,"%s",projstr.c_str());
sprintf(tmpchr,"../result/cosmics/%s/events.dat",proj);
ifstream f(tmpchr);

//initialize hists
TH1F* rfreqh = new TH1F("Rising frequency by channel and edge",proj,24,1,25);
TH1F* ffreqh = new TH1F("Falling frequency by channel and edge",proj,24,1,25);
TH1F* multipleh = new TH1F("Multiple hits",proj,7,1,8);
TH1F* toth[3][5];
TH1F* edgedroph = new TH1F("Edges dropped",proj,6,1,7);
TH1F* totsumstdh = new TH1F("TOT Summary of STDs",proj,12,1,13);

//name hists
int c=1;
for(int a=0;a<3;a++){
  for(int b=0;b<8;b++){
    sprintf(tmpchr,"%i,%i",a,b);
    rfreqh->GetXaxis()->SetBinLabel(c,tmpchr);
    sprintf(tmpchr,"%i,%i",a,b);
    ffreqh->GetXaxis()->SetBinLabel(c,tmpchr);
    c++;
  }
}

multipleh->GetXaxis()->SetBinLabel(1,"bottom");
multipleh->GetXaxis()->SetBinLabel(2,"middle");
multipleh->GetXaxis()->SetBinLabel(3,"top");
multipleh->GetXaxis()->SetBinLabel(4,"b,m");
multipleh->GetXaxis()->SetBinLabel(5,"b,t");
multipleh->GetXaxis()->SetBinLabel(6,"m,t");
multipleh->GetXaxis()->SetBinLabel(7,"all");

for(int a=0;a<5;a++){
  if(a==0) sprintf(tmpchr,"single");
  if(a==1) sprintf(tmpchr,"double: b,m");
  if(a==2) sprintf(tmpchr,"double: b,t");
  if(a==3) sprintf(tmpchr,"double: m,t");
  if(a==4) sprintf(tmpchr,"triple");
  sprintf(tmpchr2,"bottom: %s",tmpchr);
  toth[0][a] = new TH1F(tmpchr2,proj,150,0,35);
  sprintf(tmpchr2,"middle: %s",tmpchr);
  toth[1][a] = new TH1F(tmpchr2,proj,150,0,35);
  sprintf(tmpchr2,"top: %s",tmpchr);
  toth[2][a] = new TH1F(tmpchr2,proj,150,0,35);
}

edgedroph->GetXaxis()->SetBinLabel(1,"leading,b");
edgedroph->GetXaxis()->SetBinLabel(2,"falling,b");
edgedroph->GetXaxis()->SetBinLabel(3,"leading,m");
edgedroph->GetXaxis()->SetBinLabel(4,"falling,m");
edgedroph->GetXaxis()->SetBinLabel(5,"leading,t");
edgedroph->GetXaxis()->SetBinLabel(6,"falling,t");

totsumstdh->GetXaxis()->SetBinLabel(1,"bottom");
totsumstdh->GetXaxis()->SetBinLabel(2,"middle");
totsumstdh->GetXaxis()->SetBinLabel(3,"top");
totsumstdh->GetXaxis()->SetBinLabel(4,"bottom (b,m)");
totsumstdh->GetXaxis()->SetBinLabel(5,"middle (b,m)");
totsumstdh->GetXaxis()->SetBinLabel(6,"bottom (b,t)");
totsumstdh->GetXaxis()->SetBinLabel(7,"top (b,t)");
totsumstdh->GetXaxis()->SetBinLabel(8,"middle (m,t)");
totsumstdh->GetXaxis()->SetBinLabel(9,"top (m,t)");
totsumstdh->GetXaxis()->SetBinLabel(10,"bottom (all)");
totsumstdh->GetXaxis()->SetBinLabel(11,"middle (all)");
totsumstdh->GetXaxis()->SetBinLabel(12,"top (all)");

//set up vars
int leadtdc[3],leadch[3],falltdc[3],fallch[3],group[3];
leadtdc[0]=0;
leadch[0]=0;
falltdc[0]=1;
fallch[0]=7;

leadtdc[1]=0;
leadch[1]=5;
falltdc[1]=1;
fallch[1]=6;

leadtdc[2]=2;
leadch[2]=6;
falltdc[2]=1;
fallch[2]=1;

int edge,tdc,ch,time,sum,counter=0;
float itime,fallingtime[3],leadingtime[3],tmp;
float offset=9030*offseton;

//initialize vars
for(int i=0;i<3;i++){group[i]=0;fallingtime[i]=0;leadingtime[i]=0;}

//big while loop
while(1){
  f>>tmpchr;
  if(!f.good()) break;
  if(strncmp(tmpchr,goodevents,100)==0){
    f>>eventnumber;
    sum=0;
    for(int i=0;i<3;i++) {sum=sum+group[i];}
    if(sum>0){
      if(sum>1) {sum=1;}
      else {sum=0;}
      for(int i=0;i<3;i++) {sum=sum+(i+1)*group[i];}
      switch(sum){
        case 1: multipleh->Fill(1); break;
        case 2: multipleh->Fill(2); break;
        case 3: multipleh->Fill(3); break;
        case 4: multipleh->Fill(1); multipleh->Fill(2); multipleh->Fill(4); break;
        case 5: multipleh->Fill(1); multipleh->Fill(3); multipleh->Fill(5); break;
        case 6: multipleh->Fill(2); multipleh->Fill(3); multipleh->Fill(6); break;
        case 7: for(int j=1;j<8;j++) {multipleh->Fill(j);} break;
      }
      for(int i=0;i<3;i++){
        if(leadingtime[i]>0 && fallingtime[i]>0){

          tmp=fabs(fallingtime[i]-leadingtime[i]);
          if(tmp>1000000) tmp=2097152-tmp;

          switch(sum){
            case 1: toth[i][0]->Fill(tmp*25/1024-offset); break;
            case 2: toth[i][0]->Fill(tmp*25/1024-offset); break;
            case 3: toth[i][0]->Fill(tmp*25/1024-offset); break;
            case 4: if(i!=2) toth[i][1]->Fill(tmp*25/1024-offset); toth[i][0]->Fill(tmp*25/1024-offset); break;
            case 5: if(i!=1) toth[i][2]->Fill(tmp*25/1024-offset); toth[i][0]->Fill(tmp*25/1024-offset); break;
            case 6: if(i!=0) toth[i][3]->Fill(tmp*25/1024-offset); toth[i][0]->Fill(tmp*25/1024-offset); break;
            case 7: toth[i][4]->Fill(tmp*25/1024-offset); if(i!=2) toth[i][1]->Fill(tmp*25/1024-offset); if(i!=1) toth[i][2]->Fill(tmp*25/1024-offset); if(i!=0) toth[i][3]->Fill(tmp*25/1024-offset); break;
          }
        }
        if(leadingtime[i]==0 && fallingtime[i]>0) edgedroph->Fill(2*i+1);
        if(fallingtime[i]==0 && leadingtime[i]>0) edgedroph->Fill(2*i+2);
      }
    }
    for(int i=0;i<3;i++){group[i]=0;fallingtime[i]=0;leadingtime[i]=0;}
    counter++;
    if(counter%5000==0 && quiet==0) cout<<" Processed "<<counter<<" cosmics"<<endl;
    continue;
  }
  edge=atoi(tmpchr);
  f>>tdc>>ch>>time>>itime;
  if(edge==4) rfreqh->Fill(8*tdc+ch+1);
  if(edge==5) ffreqh->Fill(8*tdc+ch+1);
  for(int i=0;i<3;i++){
    if(edge==4 && tdc==leadtdc[i] && ch==leadch[i]) {
      group[i]=1;
      if(leadingtime[i]==0 || itime<leadingtime[i]) leadingtime[i]=itime;
    }
    if(edge==5 && tdc==falltdc[i] && ch==fallch[i]) {
      group[i]=1;
      if(fallingtime[i]==0 || itime<fallingtime[i]) fallingtime[i]=itime;
    }
  }
}

//fill totsum plots from toth
c=1;
for(int a=0;a<5;a++){
  for(int b=0;b<3;b++){
    if(a+b!=3){
      totsumstdh->Fill(c,toth[b][a]->GetRMS());
      c++;
    }
  }
}

//make hist gifs
if(quiet==0) cout<<" Generating gifs"<<endl;
//gStyle->SetOptTitle(0);
gStyle->SetOptStat("e");

gStyle->SetCanvasBorderMode(0);
gStyle->SetFrameBorderMode(0);
gStyle->SetPadBorderMode(0);
gStyle->SetDrawBorder(0);
gStyle->SetCanvasBorderSize(0);
gStyle->SetFrameBorderSize(0);
gStyle->SetPadBorderSize(0);
gStyle->SetTitleBorderSize(1);

TCanvas* c1 = new TCanvas("c1","c1",1000,750);
rfreqh->SetLineColor(2);
ffreqh->SetLineColor(3);
TLegend* legend = new TLegend(.8,.2,1,.3);
legend->AddEntry(rfreqh,"Leading Edge");
legend->AddEntry(ffreqh,"Falling Edge");
rfreqh->SetTitle(proj);
rfreqh->Draw();
ffreqh->Draw("same");
legend->Draw();
TImage *img = TImage::Create();
img->FromPad(c1);
sprintf(tmpchr,"../result/cosmics/%s/freq_by_channel.gif",proj);
img->WriteImage(tmpchr);gStyle->SetOptStat("ne");

multipleh->Draw();
img->FromPad(c1);
sprintf(tmpchr,"../result/cosmics/%s/multiple_hits.gif",proj);
img->WriteImage(tmpchr);

edgedroph->Draw();
img->FromPad(c1);
sprintf(tmpchr,"../result/cosmics/%s/dropped_edges.gif",proj);
img->WriteImage(tmpchr);

gStyle->SetOptStat("");
totsumstdh->SetMarkerStyle(20);
totsumstdh->GetYaxis()->SetTitle("STD");
totsumstdh->Draw("P");
img->FromPad(c1);
sprintf(tmpchr,"../result/cosmics/%s/tot_sum.gif",proj);
img->WriteImage(tmpchr);

TCanvas* c2 = new TCanvas("c2","c2",1500,2000);
gStyle->SetOptStat("nemruo");
c2->Divide(3,5);
c=1;
for(int b=0;b<5;b++){
  for(int a=0;a<3;a++){
    c2->cd(c);
    if(a+b!=3){
      toth[a][b]->GetXaxis()->SetTitle("ns");
      toth[a][b]->GetYaxis()->SetTitle("counts");
      toth[a][b]->Draw();
    }
    c++;
  }
}
c2->cd();
img->FromPad(c2);
sprintf(tmpchr,"../result/cosmics/%s/tot_chart.gif",proj);
img->WriteImage(tmpchr);

//write to file
if(quiet==0) cout<<" Writing root files"<<endl;
sprintf(tmpchr,"../result/cosmics/%s/%s.root",proj,proj);
TFile* cosmicsFile=new TFile(tmpchr,"recreate");
rfreqh->Write();
ffreqh->Write();
multipleh->Write();
for(int a=0;a<3;a++){
  for(int b=0;b<5;b++){
    if(a+b==3) continue;
    toth[a][b]->Write();
  }
}
edgedroph->Write();
totsumstdh->Write();

cosmicsFile->Close();

cout<<" Finished "<<proj<<endl;
return 0;
}
