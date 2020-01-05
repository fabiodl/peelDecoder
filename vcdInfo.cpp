#include <fstream>
#include <sstream>
#include <iostream>
#include <map>


using namespace std;

static const int CHANNELS=8;

bool inspectVcdFile(const char* name){
  ifstream f(name);

  std::map<char,uint8_t> chanId;
  string cmd;
  string line;
  do{
    getline(f,line);
    std::istringstream iss(line);
    iss>>cmd;
    //    cout<<"CMD {"<<cmd<<"}"<<endl;
    if (!iss) return false;

    if (cmd=="$var"){
      string wire;
      int n;
      char id;
      iss>>wire>>n>>id;
      uint16_t chan=chanId.size();
      chanId[id]=chan;
    }
  }while (cmd!="$enddefinitions");
  cout<<chanId.size()<<" channel definitions read"<<endl;
  for (auto& it:chanId){
    cout<<it.first<<":"<<(int)it.second<<endl;
  }
  
  uint16_t state=0;
  size_t prevt[CHANNELS];
  bool validPrevt[CHANNELS];
  for (auto& b:validPrevt){
    b=false;
  }

  size_t minIdt[CHANNELS];

  for (auto& m:minIdt){
    m=0xFFFF;
  }

  
  while(getline(f,line)){
    std::istringstream iss(line);
    char sharp;
    size_t t;
    iss>>sharp>>t;    
    uint16_t newstate=state;
    //cout<<"line"<<line<<endl;
    while(iss){
      char val;
      char chan;
      iss>>val>>chan;
      uint16_t mask=1<<chanId[chan];
      if (val=='1'){
        newstate|=mask;
      }else if (val=='0'){
        newstate&=~mask;
      }else{
        cerr<<"Unexpected value "<<val<<endl;
        return false;
      }
    }
    //cout<<hex<<"state"<<newstate<<endl;



    uint16_t diff=state^newstate;
    for (int i=0;i<CHANNELS;i++){
      if (diff&(1<<i)){
        if (validPrevt[i]){
          size_t dt=t-prevt[i];
          minIdt[i]=min(minIdt[i],dt);
        }
        prevt[i]=t;
        validPrevt[i]=true;        
      }
    }
    
    state=newstate;
  }
  
  cout<<dec<<"min IDT "<<endl;
  for (size_t i=0;i<CHANNELS;i++){
    cout<<"chan "<<i<<":"<<dec<<minIdt[i]<<endl;
  }
  return true;
}


int main(int argc,char** argv){
  inspectVcdFile(argv[1]);
}
