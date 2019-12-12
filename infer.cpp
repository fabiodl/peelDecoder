#include "infer.h"
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


void AndBlock::reset(){
  mask=0xFFFFFFFF;
  memset(seenVal,0,256*256*sizeof(BoolState));
}


bool AndBlock::check(BoolState&s,uint8_t inp,State fb){
  uint32_t einp=(inp<<24)|(~inp<<16)|(fb.v<<8)|(~fb.v);
  if (s.k && s.v){
    mask&= einp | (~fb.k<<8) | (~fb.k); //if not known leave unaltered
  }
  /*the output of the AND block is 1 if all the inputs we care for are 1
    the mask is a superset of such inputs
    if all the inputs in the the superset are 1
    then we know that the output of the and shoule be 1
  */
  bool knownTrue= (mask & einp & ((fb.k<<8)|fb.k)) == mask;

  if (knownTrue){
    if (s.k && !s.v) return false;
    s.k=s.v=true;
  }
  
  if (fb.k==0xFF){
    BoolState& seen=seenVal[inp][fb.v];
    if (seen.k && s.k && seen.v != s.v) return false;
    if (seen.k){
      s=seen;
    }else{
      seen=s;
    }
  }
  return true;
}




void PeelInfer::reset(){
  q.k=0;
  memset(f,0,256*256);
  clr.reset();
  set.reset();
}


inline void alor(State& s,BoolState b){
  if (b.k){
    if (b.v){
      s.v=0xFF;
      s.k=0xFF;
    }else{
      return;
    }
  }else{
    s.k&=s.v; //it becomes unknown where it's 0
  }  
}

inline void aland(State& s,BoolState b){
  if (b.k){
    if (b.v){
      return;
    }else{
      s.v=0;
      s.k=0xFF;
    }
  }else{
    s.k&=~s.v;//it becomes unknown where it is 1
  }
}


bool PeelInfer::check(uint8_t inp,uint8_t out,bool edge){   
    
  State backq,backd,fb;
  BoolState backclr,backset;
  
  backd.v=backq.v=out^outneg;
  backd.k=outd;
  backq.k=~outd;

  if (backq.v & backq.k){ //if there's a sure 1,clr is surely false
    backclr.v=false;
    backclr.k=true;
  }else{
    backclr.k=false;
  }


  if (edge && ( (backq.v|backq.k)!=0xFF)){ //if there's a sure 0, set is surely false
    backset.v=false;
    backset.k=true;
  }else{
    backset.k=false;
  }

  fb.v=backd.v;// would be mux(fbd,backd.v,backq.v), but backd.v and backq.v are the same
  fb.k=mux(fbd,backd.k,backq.k);
  
  if (!clr.check(backclr,inp,fb)) return false;
  if (edge){
    if (!set.check(backset,inp,fb)) return false;
    alor(q,backset);
  }
  aland(q,backclr);

  if (edge && backclr.k && !backclr.v &&  backset.k && !backset.v){ //there's a bridge
    backq.k=backd.k=0xFF;      
  }

  if (mergeState(q,backq)) return false;
        
  fb.v=mux(fbd,backd.v,q.v);
  fb.k=mux(fbd,backd.k,q.k);
    
        
  if (fb.k==0xFF){
    if (mergeState(backd,f[inp][fb.v])) return false;
    f[inp][fb.v]=backd; //backd is already a merge with previous info by the previous line
  } 
  return true;
}


bool PeelInfer::check(const std::vector<Data>& data){
  for (size_t i=0;i<data.size();i++){
    if (!check(data[i].inp,data[i].out,data[i].edge)) return false;
  }
  return true;
}


