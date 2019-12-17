#include "dataLoad.h"
#include <iostream>



using namespace std;

int main(int argc,char** argv){
  if (argc<3){
    cout<<"Usage "<<argv[0]<<" infile outfile"<<endl;
    return 0;
  }
  
  vector<Data> data;
  loadFlipFile(data,argv[1]);
  saveIoFile(data,argv[2]);
  
}
