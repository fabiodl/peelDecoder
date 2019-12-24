#include "dataLoad.h"
#include <iostream>

using namespace std;

int main(int argc,char** argv){
  if (argc<3){
    cout<<"Usage "<<argv[0]<<" infile.vcd outfile.io"<<endl;
    return 0;
  }

  std::vector<Data> data;
  if (!loadVcdFile(data,argv[1])){
    cerr<<"Error in loading "<<argv[1]<<endl;
  }
  cout<<"File contains "<<data.size()<<" data"<<endl;
  if (!saveIoFile(data,argv[2])){
    cerr<<"Error in saving "<<argv[2]<<endl;
  }
}
