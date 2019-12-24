#include "dataLoad.h"
#include <qm.h>
#include <map>
#include <set>
#include <algorithm>

std::vector<std::string> inpNames={"asel","!fdc","!rom","!ras2","!cas2","!cas0","wr","fres"};
std::vector<std::string> outNames={"tr_dir","!tr_ce","ff_cp","!ff_oe","cdcas0","o6","o7","o8"};
std::vector<std::string> enames;

std::map<std::string,uint8_t> inpIndex,outIndex;


static inline bool getAsel(uint8_t v){
  return v&1;
}

static inline bool getRas2(uint8_t v){
  return v&(1<<3);
}

static inline bool getCas2(uint8_t v){
  return v&(1<<4);
}

static inline bool  getWr(uint8_t v){
  return v&(1<<6);
}




static inline bool getO6(uint8_t v){
  return v&(1<<5);
}

static inline bool getO8(uint8_t v){
  return v&(1<<7);
}

std::string inpDesc(uint8_t inp){
  std::string s;
  for (int i=0;i<8;i++){
    if (inp& (1<<i)){
      s+=inpNames[i]+" ";
    }
  }
  return s;
}



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




std::vector<uint8_t> findInfluences(State* data,size_t n_inputs){


  std::vector<uint8_t> influences(n_inputs,0);
  
  for (size_t i=0;i<n_inputs;i++){
    //std::cout<<"Examine input "<<i<<std::endl;
    for (size_t upperBits=0;upperBits<static_cast<size_t>(1<<(n_inputs-i-1));upperBits++){    
      for (size_t lowerBits=0;lowerBits<static_cast<size_t>(1<<i);lowerBits++){

        size_t laddr=(upperBits<<(i+1))|lowerBits;
        size_t haddr=laddr|(1<<i);
        influences[i]|=(data[laddr].v^data[haddr].v)&data[laddr].k&data[haddr].k; 
        /*std::cout<<"checking "<< bitset<4>(laddr) <<"(="<<std::hex<<(uint)data[laddr]<<std::dec<<")"
                 <<"vs"<<bitset<4>(haddr)<<"(="<<std::hex<<(uint)data[haddr]<<std::dec<<")"
                 <<"xor"<<std::hex<<(uint)(data[laddr]^data[haddr])<<" infl " <<(uint)influences[i]<<dec
                 <<endl;*/        
      }
    }    
  }
  

  for (size_t i=0;i<n_inputs;i++){
    std::cout<<"Input "<<std::dec<<i<<" influences "<<std::hex<<(uint)influences[i]<<std::dec<<std::endl;
  }
  return influences;

}


void findPureD(){
  uint8_t bad=0x00;

  State f[256];

  for (State& s:f){
    s.k=0;
  }
  
  for (const Data& d:data){
    bad|=f[d.inp].k&(f[d.inp].v^d.out);
    f[d.inp].k=0xFF;
    f[d.inp].v=d.out;
  }

  cout<<"=purely D="<<hex<<(int)(0xFF&~bad)<<endl;
  auto infl=findInfluences(f,8);

  for (int o=0;o<8;o++){
    uint8_t mask=(1<<o);
    if (mask&~bad){
      cout<<"output "<<outNames[o]<<" depends only on ";
      for (int i=0;i<8;i++){
        if (mask&infl[i]){
          cout<<inpNames[i]<<" ";
        }
      }
      cout<<endl;
    }
  }
  

}

static const string valName="01";
static const string edgeName="fr";


void findForcers(){
  uint8_t f[8][2][2];


  for (int i=0;i<8;i++){
    for (int ih=0;ih<2;ih++){
      for (int oh=0;oh<2;oh++){
        f[i][ih][oh]=0;
      }
    }
  }

  for (const Data& d:data){    
    for (int i=0;i<8;i++){
      uint8_t mask=1<<i;
      uint8_t ih=(d.inp&mask)?1:0;
      f[i][ih][0]|=~d.out;
      f[i][ih][1]|=d.out;     

    }//for i
  }//for d

  std::cout<<"=Input levels to output levels="<<endl;
  
  for (uint8_t i=0;i<8;i++){
    cout<<inpNames[i]<<" ";

    for (uint8_t o=0;o<8;o++){
      uint8_t omask=1<<o;


      for (uint8_t ih=0;ih<2;ih++){
        bool can0=f[i][ih][0]&omask;
        bool can1=f[i][ih][1]&omask;
        if (can0 ^ can1){
          cout<<valName[ih]<<"->"<<outNames[o]<<"-"<<valName[can1]<<" ";
        }        
        
      }//for jh     
    }//for o
    cout<<endl;
  }//for i


  cout<<"=Forcers for each output="<<endl;


  for (uint8_t o=0;o<8;o++){
    uint8_t omask=1<<o;
    cout<<outNames[o]<<" ";

    for (uint8_t i=0;i<8;i++){


      for (uint8_t ih=0;ih<2;ih++){
        bool can0=f[i][ih][0]&omask;
        bool can1=f[i][ih][1]&omask;
        if (can0 ^ can1){         
          cout<<inpNames[i]<<"-"<<valName[ih]<<"->"<<valName[can1]<<" ";
        }        
        
      }//for jh     
    }//for i
    cout<<endl;
  }//for o

  
}




void findForcerLatchers(){
  uint8_t f[8][2][2];

  for (int i=0;i<8;i++){
    for (int ih=0;ih<2;ih++){
      for (int oh=0;oh<2;oh++){
        f[i][ih][oh]=0;
      }
    }
  }
  
  bool set=false;
  uint8_t lout;
  //uint8_t prevOut=0;
  for (const Data& d:data){    

    if (d.edge){
      lout=d.out;
      set=true;
    }

    if (set){
      for (int i=0;i<8;i++){
        uint8_t mask=1<<i;
        uint8_t ih=(lout&mask)?1:0;
        f[lout][ih][0]|=~d.out;
        f[lout][ih][1]|=d.out;     

      }//for i
    }
    //prevOut=d.out;
  }//for d

  std::cout<<"=Latched levels to output levels="<<endl;
  
  for (uint8_t i=0;i<8;i++){
    cout<<"L"<<outNames[i]<<" ";
    for (uint8_t ih=0;ih<2;ih++){

      for (uint8_t o=0;o<8;o++){
        uint8_t omask=1<<o;
        bool can0=f[i][ih][0]&omask;
        bool can1=f[i][ih][1]&omask;
        if (can0 ^ can1){
          cout<<valName[ih]<<"->"<<outNames[o]<<"-"<<valName[can1]<<" ";
        }        
        
      }//for o     
    }//for ih
    cout<<endl;
  }//for i
 
}


void findChanges(){
  std::map<uint8_t,uint8_t> ich;
  for (uint8_t i=0;i<8;i++){
    ich[1<<i]=i;
  }


  bool tr[8][8];

  for (int i=0;i<8;i++){
    for (int j=0;j<8;j++){
      tr[i][j]=false;
    }
  }


  uint8_t prevInp=data[0].inp;
  uint8_t prevOut=data[0].out;

  for (const Data& d:data){

    auto it=ich.find(d.inp^prevInp);
    if (it==ich.end()){
      cout<<"no tr"<<hex<<(int)prevInp<<"->"<<(int)d.inp<<endl;
    }else{
      for (int o=0;o<8;++o){
        if ((d.out^prevOut)&(1<<o)){
          if (!tr[it->second][o]){
            std::cout<<"inp "<<hex<<(int)prevInp<<"->"<<(int)d.inp
                     <<" out "<<(int)prevOut<<"->"<<(int)d.out
                     <<" tr["<<(int)it->second<<"]"<<"["<<(int)o<<"]=true"<<endl;
          }
          tr[it->second][o]=true;
        }//if
      }//for o
      
    }//found
   
    
    prevInp=d.inp;
    prevOut=d.out;
  }//for d

  for (int i=0;i<8;i++){
    for (int o=0;o<8;o++){
      if (tr[i][o]){
        cout<<inpNames[i]<<"->"<<outNames[o]<<endl;
      }
    }
  }

  

}




void findTransitions(){

  uint8_t tr[8][2][2][2];

  for (int i=0;i<8;i++){    
    for (int j=0;j<2;j++){
      for (int k=0;k<2;k++){
        for (int a=0;a<2;a++){
          tr[i][j][k][a]=0;
        }
      }
    }
  }

  
  uint8_t prevInp=data[0].inp;
  uint8_t prevOut=data[0].out;
  
  for (const Data& d:data){
    uint8_t inpf=~d.inp&prevInp;
    uint8_t inpr=d.inp&~prevInp;

    uint8_t outf=~d.out&prevOut;
    uint8_t outr=d.out&~prevOut;


    uint8_t noutf= d.out;
    uint8_t noutr=~d.out;


    if ((1&(prevInp^d.inp))&&(prevOut^d.out)) {
      uint8_t m=~0x44;
      if ((prevOut^d.out)&m)  {

      std::cout<<"inp "<<hex<<(int)prevInp<<"->"<<(int)d.inp
               <<" out "<<(int)prevOut<<"->"<<(int)d.out
        /*<<" inpf "<<(int)inpf
          <<" inpr "<<(int)inpr      
          <<" outf "<<(int)outf
          <<" outr "<<(int)outr
        */
               <<" diff "<<(int)(prevOut^d.out)
               <<endl;
      }
    }



    
    
    for (int i=0;i<8;i++){
      uint8_t mask=1<<i;      
      if (inpf&mask){       
        tr[i][0][0][0]|=outf;
        tr[i][0][1][0]|=outr;
        tr[i][0][0][1]|=noutf;
        tr[i][0][1][1]|=noutr;        
      }
      if (inpr&mask){        
        
        tr[i][1][0][0]|=outf;
        tr[i][1][1][0]|=outr;
        tr[i][1][0][1]|=noutf;
        tr[i][1][1][1]|=noutr;


        
      }      
    }
    prevInp=d.inp;
    prevOut=d.out;
  }//d:data


  cout<<"=inp to out="<<endl;

  for (int i=0;i<8;i++){
    cout<<inpNames[i]<<" ";
    for (int o=0;o<8;o++){
      uint8_t mask=1<<o;
      
      for (int ie=0;ie<2;++ie){      
        for (int oe=0;oe<2;++oe){      
          if (tr[i][ie][oe][0]&mask){
            string sometimes=(tr[i][ie][oe][1]&mask)?"*":"";
            cout<<edgeName[ie]<<"->"<<outNames[o]<<"-"<<edgeName[oe]<<sometimes<<" ";
          }//if            
        }//for oe      
      }//for ie
    }//for o
    cout<<endl;
  
  }//for i

  cout<<"=out influenced by="<<endl;

  
  for (int o=0;o<8;o++){
    uint8_t mask=1<<o;
    cout<<outNames[o]<<" ";
    for (int i=0;i<8;i++){            
      for (int ie=0;ie<2;++ie){      
        for (int oe=0;oe<2;++oe){      
          if (tr[i][ie][oe][0]&mask){
            string sometimes=(tr[i][ie][oe][1]&mask)?"*":"";
            cout<<inpNames[i]<<"-"<<edgeName[ie]<<"->"<<edgeName[oe]<<sometimes<<" ";
          }//if            
        }//for oe      
      }//for ie
    }//for o
    cout<<endl;
  
  }//for i
  
  
}//findTransitions



struct F{

  std::vector<BoolState> f;

  F(){}
  
  F(size_t nbits):
    f(1<<nbits)
  {    
    reset();
  }

  void reset(){
  for (BoolState& s:f){
      s.k=false;
    }
  }

  
  bool check(size_t idx,bool val){
    //cout<<"checking"<<idx<<"size"<<f.size()<<endl;
    if (f[idx].k && (f[idx].v^val)){
      return false;
    }
    /*if (!f[idx].k){
      cout<<"learnt "<<hex<<idx<<"="<<val<<endl;
      }*/
    f[idx].k=true;
    f[idx].v=val;
    return true;
  }

};




struct Func{

  std::vector<State> f;
  
  Func(size_t nbits):
    f(1<<nbits)
  {    
    for (State& s:f){
      s.k=0;
    }
  }

  bool check(size_t idx,uint8_t val){
    //cout<<"checking"<<idx<<"size"<<f.size()<<endl;
    if (f[idx].k & (f[idx].v^val)){
      return false;
    }
    /*if (!f[idx].k){
      cout<<"learnt "<<hex<<idx<<"="<<val<<endl;
      }*/
    f[idx].k=0xFF;
    f[idx].v=val;
    return true;
  }

};



void printExtendedInput(const std::vector<bool>& influencer){
  for (int i=0;i<8;i++){
    if (influencer[i]){
      cout<<inpNames[i]<<" ";
    }
  }

  for (int i=0;i<8;i++){
    if (influencer[i+8]){
      cout<<"L"+outNames[i]<<" ";
    }
  }
}


std::vector<bool> findInfluences(F& f,size_t n_inputs){
  std::vector<bool> influencer(n_inputs,false);
  
  for (size_t i=0;i<n_inputs;i++){
    //std::cout<<"Examine input "<<i<<std::endl;
    for (size_t upperBits=0;upperBits<static_cast<size_t>(1<<(n_inputs-i-1));upperBits++){    
      for (size_t lowerBits=0;lowerBits<static_cast<size_t>(1<<i);lowerBits++){

        size_t laddr=(upperBits<<(i+1))|lowerBits;
        size_t haddr=laddr|(1<<i);
        influencer[i]= influencer[i] || ( f.f[laddr].k&&f.f[haddr].k&&(f.f[laddr].v^f.f[haddr].v)); 
        /*std::cout<<"checking "<< bitset<4>(laddr) <<"(="<<std::hex<<(uint)data[laddr]<<std::dec<<")"
                 <<"vs"<<bitset<4>(haddr)<<"(="<<std::hex<<(uint)data[haddr]<<std::dec<<")"
                 <<"xor"<<std::hex<<(uint)(data[laddr]^data[haddr])<<" infl " <<(uint)influences[i]<<dec
                 <<endl;*/
      }
    }    
  }
  

  cout<<"influenced by (and maybe more) ";
  printExtendedInput(influencer);
  cout<<endl;

  return influencer;
};



bool isDeterministic(F& f,uint16_t mask,uint8_t outmask){
  uint8_t prevInp=data[0].inp;
  uint8_t prevOut=data[0].out;

  for (const Data& d:data){
    //if ((d.inp & wrMask)==0){
      if ((d.inp&1)&&(~prevInp&1)){
        uint16_t idx=(prevOut<<8)| d.inp;
        if(!f.check(idx&mask,d.out&outmask)){
          return false;
       }
      }
      //}

    prevInp=d.inp;
    prevOut=d.out;
  }
  return true;
}

std::vector<bool> findSimplifycation(F& f,uint8_t outmask){

  uint16_t mask=0xFFFF;
  for (int i=15;i>=0;i--){
    uint16_t newmask=mask&~(1<<i);
    if (isDeterministic(f,newmask,outmask)){
      mask=newmask;
    }        
  }
  std::vector<bool> vmask(16);
  for (size_t i=0;i<16;i++){
    vmask[i]=mask& (1<<i);
  }

  cout<<"Described by ";
  printExtendedInput(vmask);
  cout<<endl;


  return vmask;
}
  



void getExpression(F& f,std::vector<bool> selected,bool neg){
  
  std::vector<int> realIdx;
  std::vector<string> realNames;
  for (size_t i=0;i<selected.size();++i){
    if (selected[i]){
      realIdx.push_back(i);
      realNames.push_back(enames[i]);
      //cout<<"using name"<<realNames[realNames.size()-1]<<endl;
    }
  }

  size_t n=realIdx.size();

  vector<Implicant> il,dc;
  vector<char> buffer(n+1);
  buffer[n]=0;
  for (int i=0;i<(1<<n);i++){
    int idx=0;   
    for (size_t b=0;b<n;b++){
      if (i & (1<<b)){
        idx|=(1<<realIdx[b]);
        buffer[b]='1';
      }else{
        buffer[b]='0';
      }
    }   
    if (!f.f[idx].k){
      dc.push_back(Implicant(string(buffer.data())));
    }else if (f.f[idx].v ^ neg){
      il.push_back(Implicant(string(buffer.data())));
    }
    
  }

  auto sol=makeQM(il,dc);
  cout<<getVerilogExpression(sol,realNames)<<endl;
  


};


void analysisO7(){
  uint8_t o7mask=1<<outIndex["o7"];
  F f(16);
  cout<<"mask"<<hex<<(int)o7mask<<endl;
  uint8_t prevInp=data[0].inp;
  uint8_t prevOut=data[0].out;
  
  for (const Data& d:data){
    //if ((d.inp & wrMask)==0){
    if ((d.inp&1)&&(~prevInp&1)){
        if(!f.check((prevOut<<8)| d.inp,d.out&o7mask)){
          cout<<hex<<(int)d.inp<<"not deterministic previously"<<f.f[d.inp].v<<" now "<<(int)d.out<<endl;
return;
        }
        //}
    }
        
    prevInp=d.inp;
    prevOut=d.out;
  }
  vector<bool> infl=findInfluences(f,16);
  vector<bool> symp=findSimplifycation(f,o7mask);
  getExpression(f,symp,false);
  
}



/*void o7dld(){
  F f(24);
  uint8_t o7mask=1<<outIndex["o7"];
  uint8_t prevInp=data[0].inp;
  uint8_t prevOut=data[0].out;
  uint8_t latchedInp=0,latchedOut=0;
  bool known=false;
  for (size_t i=1;i<data.size();i++){
    const Data& d=data[i];
    //if ((d.inp & wrMask)==0){
    if ((d.inp&1)&&(~prevInp&1)){
      latchedInp=d.inp;
      latchedOut=d.out;
      known=true;
    }
    size_t idx=(latchedOut<<16)|(prevOut<<8)| d.inp;
    //std::cout<<"indexing"<<idx<<endl;
    if(known & !f.check(idx,d.out&o7mask)){
        cout<<hex<<(int)idx<<"not deterministic previously"<<f.f[d.inp].v<<" now "<<(int)d.out<<endl;
        return;
    }
        
    prevInp=d.inp;
    prevOut=d.out;
  }
  }*/


void findUnseen(){

  
  bool seen[8][2][8][2];

  for (size_t i=0;i<8;i++){
    for (size_t iv=0;iv<2;iv++){
      for (size_t o=0;o<8;o++){
        for (size_t ov=0;ov<2;ov++){
          seen[i][iv][o][ov]=false;
        }
      }
    }
  }


  uint8_t prevOut=data[0].out;
  
  for (size_t i=1;i<data.size();i++){
    const Data& d=data[i];

    for (int i=0;i<8;i++){
      for (int o=0;o<8;o++){
        seen[i][(d.inp&(1<<i))?1:0][o][(prevOut&(1<<o))?1:0]=true;
      }
    }



    prevOut=d.out;

  }

  std::cout<<"=never seen begin="<<endl;
  for (size_t i=0;i<8;i++){
    for (size_t et=0;et<2;et++){
      for (size_t o=0;o<8;o++){
        for (size_t ov=0;ov<2;ov++){
          if (!seen[i][et][o][ov]){
            cout<<"never seen "<<inpNames[i]<<"-"<<edgeName[et]<<" when "<<outNames[o]<<"="<<valName[ov]<<endl;
          }
        }
      }
    }
  }
  std::cout<<"=never seen end="<<endl;
}

struct FlipIndex{

  std::map<uint8_t,uint8_t> idx;
  
  FlipIndex(){
    for (uint8_t i=0;i<8;i++){
      idx[1<<i]=i;
    }
  }
};

FlipIndex flip;



template<typename T>
bool isSubSequence(const T& str1,const  T& str2,size_t m, size_t n) 
{ 
    // Base Cases 
    if (m == 0) return true; 
    if (n == 0) return false; 
  
    // If last characters of two strings are matching 
    if (str1[m-1] == str2[n-1]) 
        return isSubSequence(str1, str2, m-1, n-1); 
  
    // If last characters are not matching 
    return isSubSequence(str1, str2, m, n-1); 
} 


void printOutputEffect(uint8_t prev,uint8_t next){
  for (uint8_t b=0;b<8;b++){
    uint8_t mask=1<<b;
    if ((prev^next)&mask){
      cout<<outNames[b]<<"-"<<edgeName[next&mask?1:0]<<" ";
    }
  }
}

class TrSeq{
public:
  std::vector<std::pair<uint8_t,bool> > changes;
  size_t b,e;
  
  TrSeq(size_t begin,size_t end):
    b(begin),e(end)
  {
    for (size_t i=begin+1;i<=end;i++){
      uint8_t x=data[i].inp^data[i-1].inp;
      bool s=data[i].inp&x;
      changes.push_back(make_pair(x,s));
    }        
  }
  
  bool contains(const TrSeq& sub){
    return isSubSequence(sub.changes,changes,sub.changes.size(),changes.size());

      //std::search(changes.begin(), changes.end(), sub.changes.begin(), sub.changes.end()) != changes.end();
  }


  void print(){
    cout<<"Sequence ";
    for (auto& c:changes){
      cout<<inpNames[flip.idx[c.first]]<<"-"<<edgeName[c.second]<<" ";
    }
    cout<<endl;    
    for (size_t i=b;i<=e;i++){
      cout<<hex<<(int)data[i].inp<<" "<<(int)data[i].out<<endl;
    }
        
    cout<<"Effects"<<endl;
    for (size_t i=b;i<e;i++){
      printOutputEffect(data[i].out,data[i+1].out);
      cout<<endl;
    }
    cout<<"Net effect ";
    printOutputEffect(data[b].out,data[e].out);
    cout<<endl;
    
    

  }
  
};


void findStateChanges(){
  map<uint8_t,size_t> pendingSeq;

  vector<pair<size_t,size_t> > seqs;

  //cout<<"looking for sequences"<<endl;
  
  for (size_t i=0;i<data.size();++i){
    map<uint8_t,size_t>::iterator  it;
    if ((it=pendingSeq.find(data[i].inp))!=pendingSeq.end()){
      if (data[it->second].out!=data[i].out){
        seqs.emplace_back(pair<size_t,size_t>(it->second,i));
        pendingSeq.clear();
      }
    }else{
      pendingSeq[data[i].inp]=i;
    }
  }

  //cout<<"found "<<dec<<seqs.size()<<" sequences. Sorting."<<endl;
  
  sort(seqs.begin(),seqs.end(),[](const pair<size_t,size_t>& a,const pair<size_t,size_t>& b){
                                 return a.second-a.first<b.second-b.first;
                               }
    );

  
  /*for (auto & s:seqs){
    cout<<"Sequence"<<endl;
    for (size_t i=s.first;i<=s.second;i++){
      cout<<hex<<(int)data[i].inp<<" "<<(int)data[i].out<<endl;
      }
    }*/

  //cout<<"Finding essential sequences"<<endl;
  
  vector<TrSeq> trseqs;

  size_t discarded=0;
  
  for (auto& s:seqs){
    TrSeq c(s.first,s.second);
    bool ok=true;
    for (auto& t:trseqs){
      if (c.contains(t)){
        ok=false;
        break;
      }
    }
    if (ok){
      trseqs.push_back(c);   
    }else{
      discarded++;
    }
    //cout<<"essential "<<trseqs.size()<<" discarded "<<discarded<<"\r"<<flush;
  }

  //cout<<endl;
  cout<<"=State changing transitions="<<endl;
  for (auto& e:trseqs){
      e.print();
  }

}

//map<uint8_t,uint8_t>
void findReflipChangers(){

  map<uint8_t,std::pair<uint8_t,size_t > > changers;
  
  for (size_t i=0;i<data.size()-2;i++){
    if ( (data[i].inp==data[i+2].inp) && (data[i].out!=data[i+2].out) ){
      if ((data[i].inp^data[i+1].inp)&1) continue;

      //uint8_t netFlip=data[i].out^data[i+2].out;    
      //size_t idx=((data[i].out^data[i+1].out)&netFlip)?(i+1):(i+2);
      size_t idx=i+1;
      map<uint8_t,std::pair<uint8_t,size_t> >::iterator it;
      if ( (it=changers.find(data[idx].inp))!=changers.end()){
        if (it->second.first!=data[idx].out){
          cout<<"spurious clr"<<hex<<(int)data[idx].inp<<endl;
          cout<<"First seen"<<hex<<endl;
          for (size_t pi=it->second.second;pi<=it->second.second+2;++pi){
            cout<<" "<<(int)data[pi].inp<<" "<<(int)data[pi].out<<endl;
          }
          cout<<endl;
          cout<<"Now seen"<<endl;
          for (size_t ci=i;ci<=i+2;++ci){
            cout<<" "<<(int)data[ci].inp<<" "<<(int)data[ci].out<<endl;
          }
          cout<<endl;
        }
      }else{
        changers[data[idx].inp]=make_pair(data[idx].out,i);
      }
    }
  }//for i

  /*for (auto& c:changers){
    cout<<hex<<(int)c<<endl;
    }*/
  
  std::vector<Implicant> il;
  size_t n=8;
  
  vector<char> buffer(n+1);
  buffer[n]=0;
  for (auto& c:changers){
    for (size_t b=0;b<n;b++){
      if (c.first & (1<<b)){
        buffer[n-1-b]='1';
      }else{
        buffer[n-1-b]='0';
      }
    }
    il.push_back(Implicant(string(buffer.data())));
  }
  auto sol=makeQM(il,vector<Implicant>());

  cout<<"=state changer expression="<<endl;
  cout<<getVerilogExpression(sol,inpNames)<<endl;
  //return changers;
}


  /*void checkClr(){

  Func f(17);

  bool isClr[256];
  {
    set<uint8_t> changers=findReflipChangers();
    for (int i=0;i<256;i++){
      isClr[i]=!(i&1) && (changers.find(i)!=changers.end());
    }
    
  }
  

  bool stateKnown=false;
  uint16_t state=0;
  
  for(const Data& d:data){
    if (d.edge){
      state=d.out;
      stateKnown=true;
    }
    if (isClr[d.inp]){
      state=0x100;
      stateKnown=true;
    }
    if (stateKnown&& !f.check( (state<<8)|d.inp,d.out)){
      cout<<"non deterministic for state "<<hex<<state<<endl;
      return;
    }   
  }
  cout<<"Deterministic"<<endl;  
}
  */




static inline bool isDiff(const State& a,const State& b){
  //if (a.k&b.k){
  //cout<<"something to check"<<endl;
  //}
  return (a.v^b.v)&a.k&b.k;
}

bool isResetCompatible(Func& f,uint8_t state){
  for (size_t i=0;i<0xFFFF;i++){
    if (isDiff(f.f[(0x100<<16)|i],f.f[(state<<16)|i])) return false;
  }
  return true;
}


std::vector<F> splitFunc(const Func& f){
  std::vector<F> fs(8);
  for (size_t b=0;b<8;b++){
    uint8_t mask=1<<b;
    fs[b].f.resize(f.f.size());
    for (size_t i=0;i<f.f.size();i++){
      fs[b].f[i].v=f.f[i].v&mask;
      fs[b].f[i].k=f.f[i].k&mask;
    }
  }
  return fs;
}

bool squashF(F& f,size_t mask,F& newf){
  newf.f.resize(f.f.size());
  for (size_t i=0;i<f.f.size();i++){
    if (f.f[i].k)
      if (!newf.check(i&mask,f.f[i].v)) return false;
  }
  return true;
}


void checkSR(){

  uint8_t prevOut=data[0].out;
  uint16_t state=0;
  bool stateKnown=false;

  Func f(25);
  size_t checked=0;
  for (size_t i=1;i<data.size();++i){
    prevOut=data[i-1].out;
    const Data& d=data[i];
    if (d.edge){
      state=d.out;
      stateKnown=true;
    }
    if (d.inp&(1<<6)){
      state=0x100;
      stateKnown=true;
    }
    if (stateKnown){
      uint8_t stableOut=prevOut;
      uint32_t idx=(state==0x100?(1<<24):0) |(stableOut<<16)|((state&0xFF)<<8)|d.inp;
      if (!f.check(idx,d.out)){
        cout<<"non deterministic"<<endl;
        return;
      }else{
        checked++;
      }
    }
   
    
  }
  cout<<checked<<" deterministic"<<endl;

  for (int i=0;i<0x100;i++){
    if (!isResetCompatible(f,i)){
      cout<<"state "<<hex<<i<<"is not reset compatible"<<endl;
    }
  }

  std::vector<F> funcs=splitFunc(f);   
  for (uint8_t o=0;o<8;++o){
    size_t mask=0x1FFFFFF;
    for (int b=24;b>=0;--b){
      size_t newmask=mask&~(1<<b);
      F newf;
      if (squashF(funcs[o],newmask,newf)){
        mask=newmask;
        funcs[o]=newf;
        //cout<<"no input "<<b<<" for output "<<(int)o<<endl;
      }else{
        //cout<<"input "<<enames[b]<<" necessary for "<<(int)o<<endl;
      }
      
    }
    std::vector<bool> selected;
    for (int b=0;b<=24;b++){
      selected.push_back(mask&(1<<b));
    }
    cout<<outNames[o]<<"=";
    getExpression(funcs[o],selected,false);
    cout<<endl;
    
  }

 

  
  
}



bool verbose=false;

bool directCheck(int bit,uint32_t mask){

  uint8_t prevOut;
  uint16_t state=0;
  bool stateKnown=false;

  F f(25);
  size_t checked=0;
  uint32_t outMask=1<<bit;  
  for (size_t i=1;i<data.size();++i){
    prevOut=data[i-1].out;
    const Data& d=data[i];
    if (d.edge){
      state=d.out;
      stateKnown=true;
    }
    if (d.inp&(1<<6)){
      state=0x100;
      stateKnown=true;
    }
    if (stateKnown){
      uint8_t stableOut=prevOut;
      uint32_t idx=(state==0x100?(1<<24):0) |(stableOut<<16)|((state&0xFF)<<8)|d.inp;
      idx=idx&mask;
      if (verbose){
        cout<<"idx"<<idx<<endl;
      }
      if (!f.check(idx,d.out&outMask)){
        cout<<"non deterministic"<<endl;
        return false;
      }else{
        checked++;
      }
    }
    
    
  }
  cout<<checked<<" deterministic"<<endl;
  return true;
}



void findMask(int bit){
  uint32_t mask=0x1FFFFFF;
  if (!directCheck(bit,mask)){
    cout<<"NON DETERMINISTIC"<<endl;
  }
  for (int i=24;i>=0;i--){
    uint32_t newmask=mask&(~(1<<i));    
    if (directCheck(bit,newmask)){
      mask=newmask;
    }
  }
  cout<<"MASK is"<<hex<<mask<<endl;
  for (int i=0;i<24;i++){
    if (mask&(1<<i)) cout<<enames[i]<<" ";
  }
  cout<<endl;
  //verbose=true;
  //directCheck(bit,mask);
}


void checkStay(){
  for (int o=0;o<8;o++){
    cout<<"Output "<<outNames[o]<<endl;
    set<uint32_t> h,s,c;
    uint16_t state=0;
    bool stateKnown=false;
    for (size_t i=1;i<data.size();++i){
      //uint8_t prevInp=data[i-1].inp;
      uint8_t prevOut=data[i-1].out;
      uint8_t inp=data[i].inp;
      uint8_t out=data[i].out;

      if (data[i].edge){
        state=out;
        stateKnown=true;
      }
      if (inp&(1<<6)){
        state=0x100;
        stateKnown=true;
      }

      if (stateKnown){      
        uint8_t mask=1<<o;
        uint32_t es=(state<<8)|inp;
        if ((out^prevOut)&mask){
          if (out&mask){
            s.insert(es);
          }else{
            c.insert(es);
          }        
        }else{
          h.insert(es);
        }
      }
      
    }

    for (size_t i=0;i<0xFFFF;i++){
      cout<<hex<<(int)i<<" ";
      if (c.find(i)!=c.end()){
        cout<<"c";
      }
      if (s.find(i)!=s.end()){
        cout<<"s";
      }
      if (h.find(i)!=h.end()){
        cout<<"h";
      }
      cout<<endl;
    }
    
  }
  

}




//maybe cas2 latched on the neg edge of ras2?
void tryTable8(){
  F f(4);
  //uint16_t state=0x00;
  uint8_t stateKnown=false;
  for (size_t i=1;i<data.size();++i){
    const Data& d=data[i];    
    uint8_t prevOut=data[i-1].out;
    if (d.edge){
      //state=d.out;
        stateKnown=true;
        //cout<<"edge"<<endl;
      }
    if (getWr(d.inp)){
      //state=0x100;
        stateKnown=true;
    }

    //if (stateKnown){
    //cout<<hex<<"inp="<<inpDesc(d.inp)<<" state="<<(int)state<<endl;
    size_t idx=  (getO8(prevOut)<<3) | (getO6(prevOut)<<2) |(getCas2(d.inp)<<1) | getRas2(d.inp)  ;
    //cout<<"idx"<<idx<<"val"<<getO8(d.out)<<endl;
    if (!f.check(idx,getO8(d.out))){
        cout<<"Non deterministic"<<endl;
        return;
    }
    //}
  }
  cout<<"OK"<<endl;
  uint8_t conf=0;
  for(BoolState &s:f.f){    
    cout<<s<<endl;
    conf++;
  }
}



bool verify(){


  bool o8=data[0].out&(1<<7);

  for (size_t i=0;i<data.size();i++){
    const Data& d=data[i];
    bool ras2=d.inp&(1<<3);
    bool cas2=d.inp&(1<<4);
    bool mo8=d.out&(1<<7);

    
    bool po8=ras2||(cas2&&o8);
    o8=po8;
    
    if (po8!=mo8){
      cerr<<"BAD PREDICTION"<<endl;
      return false;
    }
    
  }

  return true;
}


int main(int argc,char** argv){

  for (uint8_t i=0;i<inpNames.size();++i){
    inpIndex[inpNames[i]]=i;
    enames.push_back(inpNames[i]);
  }

  for (uint8_t i=0;i<outNames.size();++i){
    outIndex[outNames[i]]=i;
    enames.push_back("Q"+outNames[i]);
  }


  for (uint8_t i=0;i<outNames.size();++i){
    enames.push_back("D"+outNames[i]);
  }

  enames.push_back("@");


  loadIoFile(data,argv[1]);
  
  //checkSR();
  tryTable8();
  //findMask(7);
  //verify();
  //checkStay();

  //findStateChanges();
  //findReflipChangers();
  //checkClr();
  //analysisO7();
  //o7dld();
  //findUnseen();
  //loadTripletFile(data,argv[1]);
  /*findPureD();
  findForcers();
  findForcerLatchers();
  findChanges();
  findTransitions();*/
  
  
  /*for (int i=0;i<256;i++){
    f[i].resize(1<<(8*DEPTH));
    }*/



  
  /*for (int mask=0;mask<0xFFFF;mask++){
    if (testClr(mask)){
      cout<<"MASK is "<<mask<<endl;
      break;
    }
    }*/
  
  //testClr(0xFFFF);
  
  
  /*size_t unknown=0;
  
  for (int i=0;i<256;i++){
    for (int j=0;j<(1<<(8*DEPTH));j++){
      if (f[i][j].k==0){
        unknown++;
      }
    }
  }

  cout<<unknown<<" unknowns "<<endl;
  */
}
