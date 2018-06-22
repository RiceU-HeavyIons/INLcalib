 #include <iostream>
#include <string>
#include <fstream>
#include <iomanip>

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
  char tmpchr[200];
  //-----------------
  sprintf(tmpchr,"%s",datafilename.c_str());
  FILE* inputfile=fopen(tmpchr,"r");
  if(!inputfile) {cout<<"Can not open "<<tmpchr<<endl;return -1;};  
  cout<<"Process Data File:"<<tmpchr<<" "<<endl;

  unsigned int itime,tempv;
  int numret;
  int tdig(0),edgeid,tdcid,tdcchan;
  int events=0,wrongedgeevents=0;
  while ((numret=fscanf(inputfile,"%x",&tempv))==1) {

    if(tempv>>16==0xe000) {cout<<" ----------- Find a Seperator: "<<hex<<tempv<<endl;continue;}

    edgeid =int( (tempv & 0xf0000000)>>28 );
    if((edgeid != 4) && (edgeid != 5)){cout<<"ERROR!!----------------Wrong edge="<<edgeid<<endl;wrongedgeevents++;continue;} 

    tdcid = (tempv & 0x0F000000)>>24;  
    //tdig = tdcid/3;
    //tdcid = tdcid % 3;
    tdcchan = (tempv&0x00E00000)>>21;
    itime=((tempv&0x7ffff)<<2)+((tempv>>19)&0x00000003);
    cout<<"events="<<events<<" value:0x"<<hex<<tempv<<dec<<" TDIG="<<tdig<<" edge="<<edgeid<<" tdcid="<<tdcid<<" tdcch="<<tdcchan<<" time="<<itime<<endl; 
    events++;
  }
  fclose(inputfile);
  cout<<"Processed total events = "<<events<<" , with wrong edge events="<<wrongedgeevents<<endl;

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
