#include "dataLoad.h"

using namespace std;
static constexpr uint8_t DEPTH=1;


static inline bool merge(State& s,uint8_t out){
  if (s.k && s.v!=out) {
    cout<<"previously seen"<<(int)s.v<<" now seen "<<(int)out<<endl;
    return false;
  }
  s.k=0xFF;
  s.v=out;
  return true;
}


static inline bool allKnown(const State (& q) [DEPTH] ){
  for (int i=0;i<DEPTH;i++){
    if (q[i].k==0) return false;
  }
  return true;
}

static inline size_t addr(const State (& q) [DEPTH] ){
  size_t a=0;
  for (int i=0;i<DEPTH;i++){
    a|=q[i].v<<(8*i);
  }
  return a;
};


vector<Data> data;  
std::vector<std::vector<State> > f(256);
State q[DEPTH];

bool testClr(uint16_t mask){
  
  
  for (int i=0;i<256;i++){
    for (int j=0;j<(1<<(8*DEPTH));j++){
      f[i][j].k=0;
    }
  }
  
  for (uint8_t i=0;i<DEPTH;i++){
    q[i].k=0;
    q[i].v=0;
  }
  
  for (const Data& d:data){

    uint16_t einp=((~d.inp)<<8)|d.inp;
    if ((einp & mask)==mask){
      for (uint8_t i=0;i<DEPTH;i++){
        q[i].k=0xFF;
        q[i].v=0;
        cout<<"clr"<<endl;
      }
    }else{
      if (d.edge){
        
        for (uint8_t i=DEPTH-1;i>0;i--){
          q[i]=q[i-1];
        }
        q[0].k=0xFF;
        q[0].v=d.inp;        
        cout<<"edge"<<endl;
      }
    }
    
    if (allKnown(q)){
      cout<<hex<<(int)d.inp<<" "<<addr(q)<<"->"<<(int)d.out<<endl;
      if (!merge(f[d.inp][addr(q)],d.out)){
        cerr<<"Mismatch"<<endl;
        return false;
      }
    }    
  }
  
  return true;
}



int main(int argc,char** argv){
  
  

  loadTripletFile(data,argv[1]);
  
  for (int i=0;i<256;i++){
    f[i].resize(1<<(8*DEPTH));
  }

  
  /*for (int mask=0;mask<0xFFFF;mask++){
    if (testClr(mask)){
      cout<<"MASK is "<<mask<<endl;
      break;
    }
    }*/

  testClr(0xFFFF);
  
  
  size_t unknown=0;
  
  for (int i=0;i<256;i++){
    for (int j=0;j<(1<<(8*DEPTH));j++){
      if (f[i][j].k==0){
        unknown++;
      }
    }
  }

  cout<<unknown<<" unknowns "<<endl;
  
}
