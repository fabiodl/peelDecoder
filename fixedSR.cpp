#include "fixedSR.h"
#include <cstring>

inline static uint8_t mux(uint8_t m,uint8_t a,uint8_t b){
  return (m&a) | ((~m) & b);
}



inline static bool mergeState(State& a,State b){ //returns true on error
  if ((a.v^b.v)&a.k&b.k) return true;
  a.v=mux(a.k,a.v,b.v);
  a.k=a.k|b.k;
  return false;
}


void Peel::reset(){
  q.k=0;
  memset(f,0,256*256);
}

bool Peel::check(uint8_t inp,uint8_t out,bool edge){   
  bool clr,set;

  clr=((inp&clrMaskp)==clrMaskp)&&(((~inp)&clrMaskn)==clrMaskn);
  set=((inp&setMaskp)==setMaskp)&&(((~inp)&setMaskn)==setMaskn);
    
  State backq,backd,fb;
    
  backd.v=backq.v=out^outneg;
  backd.k=outd;
  backq.k=~outd;

  if (!clr && edge &&!set){ //there's a bridge
    backq.k=backd.k=0xFF;      
  }


  if (clr){
    q.v=0;
    q.k=0xFF;
  }else if (edge){
    if (set){
      q.v=0xFF;
      q.k=0xFF;
    }
    else{
      q.k=0;  //no info from the past, only backq
    }
  }//else stay what it is


  if (mergeState(q,backq)) return false;

        
  fb.v=mux(fbd,backd.v,q.v);
  fb.k=mux(fbd,backd.k,q.k);
    
        
  if (fb.k==0xFF){
    if (mergeState(backd,f[inp][fb.v])) return false;
    f[inp][fb.v]=backd; //backd is already a merge with previous info by the previous line
  } 
  return true;
}


bool Peel::check(const std::vector<Data>& data){
  for (size_t i=0;i<data.size();i++){
    if (!check(data[i].inp,data[i].out,data[i].edge)) return false;
  }
  return true;
}


