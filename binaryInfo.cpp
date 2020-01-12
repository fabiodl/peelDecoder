#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

static const int CHANNELS=8;
static size_t freq=32000000;
static float tstep=1.0/freq;

using namespace std;


bool info(const char* name,size_t unstableT=0){
  ifstream file(name, ios::in|ios::binary);


  size_t prevt[CHANNELS];
  bool validPrevt[CHANNELS];
  for (auto& b:validPrevt){
    b=false;
  }

  size_t minIdt[CHANNELS];

  for (auto& m:minIdt){
    m=0xFFFF;
  }

  uint8_t buffer[2];
  file.read((char*)buffer,2);
  uint16_t state=buffer[0]|(buffer[1]<<8);
  size_t t=0;
  map<uint16_t,std::set<uint16_t> >  trans;

  while (file.read((char*)buffer,2)){
    t++;
    uint16_t newstate=buffer[0]|(buffer[1]<<8);
    uint16_t diff=state^newstate;
    for (int i=0;i<CHANNELS;i++){
      if (diff&(1<<i)){
        if (validPrevt[i]){
          
          if (t>unstableT){
            size_t dt=t-prevt[i];
            if (dt<minIdt[i]){
              cout<<"chan "<<i<<"down to"<<dt<<" @ "<<t<<"="<<(t*tstep)<<endl;
            }
            minIdt[i]=min(minIdt[i],dt);
          }
        }else{
          cout<<"chan"<<i<<"first transition @ "<<t<<"="<<(t*tstep)<<endl;
        }
        prevt[i]=t;
        validPrevt[i]=true;        
      }
    }    
    trans[state].insert(newstate);
    state=newstate;    
  }

  cout<<dec<<"min IDT "<<endl;
  for (size_t i=0;i<CHANNELS;i++){
    cout<<"chan "<<i<<":"<<dec<<minIdt[i]<<endl;
  }

  for (auto& t: trans){
    cout<<hex<<t.first<<"->";
    for (auto& v:t.second) cout<<v<<" ";
    cout<<endl;
  }

  return true;
  
}




static inline bool getAsel(uint8_t v){
  return v&1;
}

static inline bool getFdc(uint8_t v){
  return v&(1<<1);
}

static inline bool getRom(uint8_t v){
  return v&(1<<2);
}

static inline bool getRas2(uint8_t v){
  return v&(1<<3);
}

static inline bool getCas2(uint8_t v){
  return v&(1<<4);
}

static inline bool getCas0(uint8_t v){
  return v&(1<<5);
}

static inline bool  getWr(uint8_t v){
  return v&(1<<6);
}


static inline bool getTrdir(uint8_t v){
  return v&1;
}


static inline bool getTrce(uint8_t v){
  return v&(1<<1);
}



static inline bool getFfcp(uint8_t v){
  return v&(1<<2);
}

static inline bool getFfoe(uint8_t v){
  return v&(1<<3);
}

static inline bool getCdcas0(uint8_t v){
  return v&(1<<4);
}


static inline bool getO6(uint8_t v){
  return v&(1<<5);
}

static inline bool getO7(uint8_t v){
  return v&(1<<6);
}

static inline bool getO8(uint8_t v){
  return v&(1<<7);
}


std::vector<uint8_t> inp;
std::vector<uint8_t> out;

bool loadConv(const char* name,size_t unstableT=0){
  ifstream file(name, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    size_t size = file.tellg();
    inp.resize(size/4-unstableT);
    out.resize(size/4-unstableT);
    file.seekg (0, ios::beg);

    char zeros[4];

    for (size_t i=0;i<unstableT;i++){
      file.read(zeros,4);
    }
    
    for (size_t i=0;i<size-unstableT;i++){
      file.read(reinterpret_cast<char*>(inp.data()+i),1);
      file.read(reinterpret_cast<char*>(out.data()+i),1);
      file.read(zeros,2);
    }            
    return true;
  }
  
  return false;
}



struct Delay{
  size_t mind,maxd;

  Delay():
    mind(0xFFF),maxd(0){}


  void push(size_t d){
    mind=min(d,mind);
    maxd=max(d,maxd);
  }

};


std::ostream& operator<<(std::ostream& o,const Delay& d){
  return o<<d.mind<<"~"<<d.maxd;
}


struct EdgeDelay{
  Delay up,down;
  void push(size_t d,bool v){
    if (v){
      up.push(d);
    }else{
      down.push(d);
    }
  }
  
};

std::ostream& operator<<(std::ostream& o,const EdgeDelay& d){
  return o<<"up "<<d.up<<" down "<<d.down;
}


void checkDelay(){

  EdgeDelay cas0,ras2,ffoe;
  
  for (size_t i=1;i<inp.size();i++){
    
    if  (getCas0(inp[i])!=getCas0(inp[i-1])){
      size_t j;
      for (j=0;getCdcas0(out[i+j])==getCas0(inp[i]);j++);      
      //cout<<"j"<<j<<endl;
      cas0.push(j,getCas0(inp[i]));
    }

    if  (getRas2(inp[i])!=getRas2(inp[i-1])){
      size_t j;
      for (j=0;getO6(out[i+j])!=getRas2(inp[i]);j++);            
      ras2.push(j,getRas2(inp[i]));
    }

    if (getFfoe(out[i])!=getFfoe(out[i-1])){
      size_t j;
      for (j=0;getTrce(out[i+j])==getFfoe(out[i]);j++);            
      ffoe.push(j,getFfoe(out[i]));
    }

    
  }
  cout<<"cas0 "<<cas0<<endl
      <<"ras2 "<<ras2<< endl
      <<"ffoe "<<ffoe<<endl;

    ;
  
}


int main(int argc,char** argv){
  loadConv(argv[1],10);
  cout<<"size"<<inp.size()<<endl;
  checkDelay();
  
}
