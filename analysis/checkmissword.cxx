#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <TBenchmark.h>     // ROOT head file for benchmarking applications

using namespace std;

int main(int argc, char *argv[])
{

  // time statistic --- start
  TBenchmark* gBenchmark = new TBenchmark();
  //start time of this program
  gBenchmark->Start("main");

  // process commandline
  if(argc<2) {
    cout<<"Usage: checkmissword --help to get help"<<endl;
    return -1;
  }
  string datafilename="unknown";
  int totchan=8;
  int dotdc=0;
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

  char tmpchr[200];
  char filename[200];
  sprintf(filename,"%s",datafilename.c_str());
 
  sprintf(tmpchr,"%s",filename);
  FILE* inputfile=fopen(tmpchr,"r");
  if(!inputfile) {cout<<"Can not open "<<tmpchr<<endl;return -1;};  
  cout<<"Process Data File:"<<tmpchr<<" total chan="<<totchan<<endl;

  int numret=0;
  int tdcid, tdcchan, tdig;

  unsigned int tempv[totchan],value;
  int edgeid;
  int misswordevents=0;
  int goodevents=0;
  int channelnum[totchan];
  int lastevents=0;
  while ((numret=fscanf(inputfile,"%x",&tempv[0]))==1) {
    //cout<<"-----------------------------------------------------------------------------"<<endl;
    if(goodevents%500000==1&&goodevents>lastevents) {lastevents=goodevents;cout<<" processed events "<<goodevents<<endl;}
    if(tempv[0]>>16==0xe000) continue;
    edgeid =int( (tempv[0] & 0xf0000000)>>28 );if((edgeid != 4) && (edgeid != 5)) {continue;} 

    tdcid = (tempv[0] & 0x0F000000)>>24;  tdcid = tdcid % 8;
    if(tdcid !=dotdc) continue;

    for(int i=0;i<totchan;i++){channelnum[i]=-1;}

    tdcchan = (tempv[0]&0x00E00000)>>21;
    channelnum[tdcchan]=tdcchan;
    
    for(int i=1;i<totchan;i++){
      tempv[i]=0;
      numret=fscanf(inputfile,"%x",&value);if(numret!=1) continue;
      if(value>>16==0xe000) continue;
      edgeid =int( (value & 0xf0000000)>>28 );if((edgeid != 4) && (edgeid != 5)) {continue;} 
      tdcid = (value & 0x0F000000)>>24;  tdcid = tdcid % 8;
      if(tdcid !=dotdc) continue;
      tempv[i]=value;
      tdcchan = (value&0x00E00000)>>21;
      channelnum[tdcchan]=tdcchan;
    }

    int totalfiredchan=0; int misschan[8]={-1,-1,-1,-1,-1,-1,-1,-1};
    for(int i=0;i<totchan;i++){if(channelnum[i]>-1)totalfiredchan++;if(channelnum[i]<0)misschan[i]=i;} 
   
    if(totalfiredchan !=totchan){
       misswordevents++;
       if(misswordevents<50)cout<<" Missed words --------total hit chan= "<<totalfiredchan<<endl;
       for(int i=0;i<totchan;i++){
         if(tempv[i]==0) continue;
         if(channelnum[i]<0) continue;
         tdcid = (tempv[i] & 0x0F000000)>>24;  tdcid = tdcid % 8;
         tdig = tdcid/4;
         tdcchan = (tempv[i]&0x00E00000)>>21;
         if(misswordevents<50)cout<<"value:0x"<<hex<<tempv[i]<<dec<<" TDIG="<<tdig<<" edge="<<edgeid<<" tdcid="<<tdcid<<" tdcchan="<<tdcchan<<endl;
       }
       continue;
    }

    for(int i=0;i<totchan;i++){channelnum[i]=-1;} 
    goodevents++;

    for(int i=0;i<totchan;i++){
      tdcid = (tempv[i] & 0x0F000000)>>24;  tdcid = tdcid % 8;
      tdig = tdcid/4;
      //tdcid = tdcid % 4;
      tdcchan = (tempv[i]&0x00E00000)>>21;
      unsigned int itime=((tempv[i]&0x7ffff)<<2)+((tempv[i]>>19)&0x00000003);
      if(goodevents<10)cout<<"value:0x"<<hex<<tempv[i]<<dec<<" TDIG="<<tdig<<" edge="<<edgeid<<" tdcid="<<tdcid<<" tdcchan="<<tdcchan<<" itime="<<itime<<endl; 
    }
    for(int i=0;i<totchan;i++){tempv[i]=0;}
  }

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

  return 0;
}
