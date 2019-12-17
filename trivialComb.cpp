#include "dataLoad.h"

using namespace std;


static inline bool merge(State& s,uint8_t out){
  if (s.k && s.v!=out) return false;
  s.k=0xFF;
  s.v=out;
  return true;
}


int main(int argc,char** argv){
  
  State f[256][256];

  for (int i=0;i<256;i++){
    for (int j=0;j<256;j++){
      f[i][j].k=0;
    }
  }
  
  
  vector<Data> data;  
  loadTripletFile(data,argv[1]);
  State q;
  q.k=0;
  q.v=0;
  
  for (const Data& d:data){
    if (d.edge){
      q.k=0xFF;
      q.v=d.inp;
      cout<<"edge"<<endl;
    }
    if (q.k){
      if (!merge(f[d.inp][q.v],d.out)){
        cerr<<"Mismatch"<<endl;
        break;
      }
    }    
  }

  size_t unknown=0;
  
  for (int i=0;i<256;i++){
    for (int j=0;j<256;j++){
      if (f[i][j].k==0){
        unknown++;
      }
    }
  }

  cout<<unknown<<" unknowns "<<endl;
  
}
