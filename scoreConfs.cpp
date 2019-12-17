#include "peelBinarySink.h"
#include <algorithm>
#include <iomanip>

using namespace std;


int main(int argc,char** argv){
  if (argc<2){
    cout<<"Usage "<<argc<<" "<<endl;
    return 0;
  }

  PeelBinarySource src;
  if (!src.open(argv[1])){
    cerr<<"Unable to open "<<argv[1]<<endl;
    return -1;
  }

  std::vector<size_t> u=src.unknownCount();
  size_t idx=std::distance(u.begin(), std::min_element(u.begin(), u.end()));
  PeelInfer best=src.read(idx);

  cout<<"File has "<<src.size()<<" confs"<<endl;

  cout<<"best is ";
  cout<<setw(2)<<setfill('0')<<hex
      <<"outd="<<(int)best.outd
      <<" fbd="<<(int)best.fbd
      <<" outneg="<<(int)best.outneg
      <<" unknowns="<<best.fUnknownCount()      
      <<endl;    
}
