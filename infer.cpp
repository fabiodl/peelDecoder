#include "infer.h"
#include <cstring>
#include <iomanip>

using namespace std;

inline static uint8_t mux(uint8_t m,uint8_t a,uint8_t b){
  return (m&a) | ((~m) & b);
}


inline static State mux(uint8_t m,State a,State b){
  State r;
  r.v=mux(m,a.v,b.v);
  r.k=mux(m,a.k,b.k);
  return r;
}


inline static bool mergeState(State& a,State b){ //returns true on success
  if ((a.v^b.v)&a.k&b.k) return false;
  a.v=mux(a.k,a.v,b.v);
  a.k=a.k|b.k;
  return true;
}


void AndBlock::reset(){
  mask=0xFFFFFFFF;
  memset(seenVal,0,256*256*sizeof(BoolState));
}

inline static uint32_t extendInput(uint8_t inp,uint8_t fbv){
  return (inp<<24)|(~inp<<16)|(fbv<<8)|(~fbv);
}


  /*the output of the AND block is 1 if all the inputs we care for are 1
    the mask is a superset of such inputs
    if all the inputs in the the superset are 1
    then we know that the output of the AND should be 1
  */
inline static bool knownTrue(uint32_t mask,uint32_t einp,State fb){
  return (mask & einp & ((fb.k<<8)|fb.k)) == mask;
}


inline static bool getAddress(uint8_t& idx,State fb,uint8_t fbd){
  bool known=(fb.k|fbd)==0xFF;
  if (known){
    idx=fb.v&~fbd; //"canonical" address forces 0 on fbd lines
  }
  return known;
}


bool AndBlock::check(BoolState&s,uint8_t inp,State fb,uint8_t fbd){
  uint32_t einp=extendInput(inp,fb.v);
  if (s.k && s.v){
    mask&= einp | (~fb.k<<8) | (~fb.k); //if not known leave unaltered
  }

  if (knownTrue(mask,einp,fb)){
    if (s.k && !s.v) return false;
    s.k=s.v=true;
  }
  
  uint8_t idx;
  if (getAddress(idx,fb,fbd)){
    BoolState& seen=seenVal[inp][idx];
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
  memset(f,0,256*256);
  clr.reset();
  set.reset();
  forgetState();
}

void PeelInfer::forgetState(){
  q.k=0;
  fb.k=0;
}


inline static void alor(State& s,BoolState b){
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

inline static void aland(State& s,BoolState b){
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


inline static void alandnot(State& s,BoolState b){
  if (b.k){
    if (b.v){
      s.v=0;
      s.k=0xFF;
    }else{
      return;
    }
  }else{
    s.k&=~s.v;//it becomes unknown where it is 1
  }
}



inline static bool almismatch(State&a, State& b){
  return (a.v^b.v)&a.k&b.k;
}

bool PeelInfer::check(uint8_t inp,uint8_t out,bool edge){   
    
  State backq,backd,fb;
  BoolState backclr,backset;
  uint8_t idx;
  
  backd.v=backq.v=out^outneg;
  backd.k=outd;
  backq.k=~outd;

  
  if (backq.v & backq.k){ //if there's a sure 1, clr is surely false
    backclr.v=false;
    backclr.k=true;
  }else{
    backclr.k=false;
  }


  if (edge && (backq.k & ~backq.v)){ //if there's a sure 0, set is surely false
    backset.v=false;
    backset.k=true;
  }else{
    backset.k=false;
  }


  //cout<<"backq "<<backq<<" backd "<<backd<<" backclr "<<backclr<<" backset "<<backset<<endl;
  
    
  if (almismatch(backq,q)){   //q is pre-edge
    //cout<<"q changed"<<endl;
    if (edge){
      if (getAddress(idx,fb,fbd)){//fb is pre-edge
        //cout<<"address identified"<<endl;
        State fwd=f[inp][idx];
        if (almismatch(fwd,backq)){          
          if ((backq.v & backq.k)==0){
            backclr.v=true;
            backclr.k=true;
            backq.v=0;
            backq.k=0xFF;
          }else if ((backq.k & backq.v) == backq.k){
            backset.v=true;
            backset.k=true;
            backq.v=0xFF;//no possible mismatches given the if is true
            backq.k=0xFF;            
          }else{
            return false;
          }              
        }//if mismatch(fwd,backq)        
      }//if getaddress
    }else{ //not edge
      if ((backq.v & backq.k)==0){
        backclr.v=true;
        backclr.k=true;
        backq.v=0;
        backq.k=0xFF;
      }else{
        return false;
      }
    }
    
  }//mismatch backq - q


  if (edge){
    if (!set.check(backset,inp,fb,fbd)) return false;
    alor(backq,backset);
  }
  
  fb=mux(fbd,backd,backq);
  
  if (!clr.check(backclr,inp,fb,fbd)) return false;
  alandnot(backq,backclr);

 
  if (edge && backclr.k && !backclr.v &&  backset.k && !backset.v){ //there's a bridge
    backq.k=backd.k=0xFF;      
  }      

  q=backq;
  

  if (getAddress(idx,fb,fbd)){
    //cout<<"address identified for storing"<<endl;
    if (!mergeState(backd,f[inp][idx])) return false;
    f[inp][idx]=backd; //backd is already a merge with previous info by the previous line
    //cout<<"stored f"<<(int)inp<<" "<<(int)idx<<" ="<<backd<<endl;
  } 
  return true;
}


bool PeelInfer::check(const std::vector<Data>& data){
  for (size_t i=0;i<data.size();i++){
    if (!check(data[i].inp,data[i].out,data[i].edge)) return false;
  }
  return true;
}

inline static State alpick(State* f,State fb,uint8_t fbd){
  uint8_t canBe1=0x00;
  uint8_t canBe0=0x00;

  for (size_t i=0;i<255;i++){
    if (i&fbd) continue; //only allow "canonical" idx
    if (fb.k & (i^fb.v)) continue;
    //debug<<"considering fb idx"<<i<<"="<<f[i]<<"idx"<<idx<<endl;

    canBe0|=~f[i].v | ~f[i].k;
    canBe1|= f[i].v | ~f[i].k;
    if ((canBe0^canBe1) == 0) break; //we know nothing, abort
  }

  State s;
  
  s.v=canBe1;
  s.k=canBe0 ^ canBe1; //the case 0^0 is not possible
  return s;  
}


inline static BoolState alpick(BoolState* f,State fb,uint8_t fbd){
  bool canBe1=false;
  bool canBe0=false;

  for (size_t i=0;i<255;i++){
    if (i&fbd) continue; //only allow "canonical" idx
    if (fb.k & (i^fb.v)) continue;
    canBe0|=!f[i].v || !f[i].k;
    canBe1|= f[i].v || !f[i].k;
    if ((canBe0^canBe1) == 0) break; //we know nothing, abort
  }

  BoolState s;  
  s.v=canBe1;
  s.k=canBe0 ^ canBe1; //the case 0^0 is not possible
  return s;  
}




BoolState AndBlock::predict(uint8_t inp,State fb,uint8_t fbd){
  uint32_t einp=extendInput(inp,fb.v);
  if (knownTrue(mask,einp,fb)){
    BoolState r;
    r.v=r.k=true;
    return r;
  }
  return alpick(seenVal[inp],fb,fbd);  
}


State PeelInfer::predict(uint8_t inp,bool edge){
  State fb;
  fb.k=mux(fbd,0x00,q.k);
  fb.v=q.v; //for D, we do not know, so assign to q anyway

  //cout<<"fb="<<fb<<endl;

  State d=alpick(f[inp],fb,fbd);
  //cout<<" d="<<d<<endl;
  if (edge){
    q=d;
    BoolState setState=set.predict(inp,fb,fbd);
    alor(q,setState);
    //cout<<" s="<<setState<<endl;
  }  
  BoolState clrState=clr.predict(inp,fb,fbd);
  alandnot(q,clrState);
  //cout<<" c="<<clrState<<endl;
  //cout<<" q="<<q<<endl;
  State out;
  out.v=mux(outd,d.v,q.v)^outneg;
  out.k=mux(outd,d.k,q.k);
  return out;
}


bool AndBlock::getDeterministic(uint32_t& m,uint8_t fbd){
  //cout<<"getting deterministic"<<std::endl;
  for (int inp=0;inp<256;inp++){
    for (int fb=0;fb<256;fb++){
      uint8_t idx=fb&~fbd;
      if (seenVal[inp][idx].k){
        bool s=seenVal[inp][idx].v;
        bool mskRes=(extendInput(inp,fb) &  mask)==mask;
        if (s!=mskRes){
          return false;
        }
      }
    }
  }
  m=mask;
  //cout<<"got deterministic"<<hex<<mask<<dec<<endl;
  return true;
}


bool isActive(uint32_t einp,uint32_t mask){
  return (einp&mask)==mask;
}


uint8_t DeterministicPeel::predict(uint8_t inp,bool edge){  
  uint8_t idx=fb&~fbd;
  uint8_t d=f[inp][idx];
  fb=mux(fbd,d,q);
  uint8_t einp=extendInput(d,fb);
  
  if (isActive(einp,clrMask)){
    q=0;
  }else if (edge){
    if (isActive(einp,setMask)){
      q=0xFF;
    }else{
      q=d;
    }    
  }
  return mux(outd,d,q)^outneg;  
}


class OnesCount{

public:
  uint8_t table[256];


  OnesCount(){
    for (int i=0;i<256;i++){
      table[i]=0;
      for(uint8_t b=0;b<8;b++){
        if (i&(1<<b)){
          table[i]++;
        }
      }
    }
  }
  
};

OnesCount onesCount;


size_t PeelInfer::fUnknownCount(){
  size_t c=0;
  for (int inp=0;inp<256;++inp){
    for (int fb=0;fb<256;fb++){
      uint8_t idx=fb&~fbd;      
      c+=onesCount.table[0xFF&(~f[inp][idx].k)];
    }
  }
  return c;
}

DeterministicPeel PeelInfer::getDeterministic(){
  DeterministicPeel d;
  d.outd=outd;
  d.fbd=fbd;
  d.outneg=outneg;
  if (!clr.getDeterministic(d.clrMask,fbd)){
    cerr<<"Unable to get deterministic clr"<<std::endl;
  }
  
  if (!set.getDeterministic(d.setMask,fbd)){
    cerr<<"Unable to get deterministic clr"<<std::endl;
  }         

  return d;
}
