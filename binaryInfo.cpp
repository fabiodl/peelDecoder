#include <fstream>
#include <iostream>
#include <map>
#include <set>

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

  
int main(int argc,char** argv){
  
  info(argv[1],3*freq);
  
}
