
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "ios.h"
#include <iomanip>
#include <list>

using namespace std;

static const int CHANNELS=16;


double mean(const std::vector<size_t>& v){
  double m=0;
  for (auto& e:v){
    m+=e;
  }
  if (v.size()){
    m=m/v.size();
  }
  return m;
}



struct FwState{

  std::vector<bool> in,fb;
  size_t t;
  FwState(const std::vector<bool>& inps,const std::vector<bool>& outs,size_t t);
};


FwState::FwState(const std::vector<bool>& inps,const std::vector<bool>& outs,size_t _t):
  in(inps),
  fb(outs),
  t(_t)
{
}

static const int DELAY=25;


std::string desc(const std::vector<bool>& bv,const std::vector<string>& names){
  std::stringstream ss;
  for (size_t i=0;i<bv.size();i++){
    if (bv[i]) ss<<" "<<names[i];
  }
  return ss.str();
}


void push(const std::vector<bool>& inps,const std::vector<bool>& outs,size_t t){
  static std::list<FwState> s;
  auto it=s.end();
  for (--it;it!=s.begin()&&it->t>t-DELAY;--it){
  }
  if (it!=s.end()&&it->t<=t-DELAY){
    //std::cout<<desc(it->in,inpNames)<<","<<desc(it->fb,outNames)<<"->"<<desc(outs,outNames)<<endl;
  }else{
    //cout<<"no history"<<endl;
  }
  s.push_back(FwState(inps,outs,t));
  
}




static const size_t NOTIME=(size_t)-1;

static const int CMPCHAN=4;
static const int CMPOFFSET=8;


/*static const int CMPCHAN=8;
static const int CMPOFFSET=0;
*/

std::vector<std::vector<size_t> > delay(CMPCHAN*2);


class ChanData{
public:
  std::vector<bool> v;
  ChanData();
  ChanData(const std::vector<bool>& _v);
};

ChanData::ChanData():v(CHANNELS){}
ChanData::ChanData(const std::vector<bool>& _v):v(_v){}

std::vector<ChanData> channels;

void pushChannel(const std::vector<bool> & value,size_t t){
  size_t target=t/5;  
  for (size_t i=channels.size();i<target;i++){
    channels.push_back(channels.back());
  }
  channels.push_back(ChanData(value));
}


static const size_t MINDIFF=5;
void checkChannels(const char* name, const std::vector<std::string>& cmpNames){

  for (size_t t=0;t<channels.size();++t){
    for (size_t i=0;i<CMPCHAN;i++){
      uint16_t me=CMPOFFSET+i;
      uint16_t other=CMPOFFSET+((i+CMPCHAN)%(CMPCHAN*2));
      size_t diffs=0;
      for (size_t dt=0;dt<MINDIFF&&t+dt<channels.size() ;dt++){
        if (channels[t+dt].v[me]!=channels[t+dt].v[other]){
          diffs++;
        }
      }
      if (diffs==MINDIFF){
        cout<<"difference for chan "<<cmpNames[i]<<" in "<<name<<" at "<< std::fixed << std::setprecision(9)<<5E-9*t<<endl;
      }

    }
  }

};



bool load(const char* name,const std::vector<std::string> & cmpNames){
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

  /*cout<<chanId.size()<<" channel definitions read"<<endl;
  for (auto& it:chanId){
    cout<<it.first<<":"<<(int)it.second<<endl;
    }*/
  
  std::vector<bool> state(CHANNELS),newstate;

  std::vector<bool> changed(CHANNELS);
  std::vector<size_t> lastchange(CHANNELS);
  std::fill(lastchange.begin(),lastchange.end(),NOTIME);
  while(getline(f,line)){
    std::istringstream iss(line);
    char sharp;
    size_t t;
    iss>>sharp>>t;    
    newstate=state;
    std::fill(changed.begin(), changed.end(), false);
    while(iss){
      char val;
      char chan;
      iss>>val>>chan;
      if (!iss) break;
      uint16_t ch=chanId[chan];
      if (val=='1'){
        newstate[ch]=true;
      }else if (val=='0'){
        newstate[ch]=false;
      }else{
        cerr<<"Unexpected value "<<val<<endl;
        return false;
      }
      //cout<<"channel "<<ch<<" changed to "<<newstate[ch]<<endl;
      changed[ch]=true;
    }//while iss

    for (uint16_t i=0;i<CMPCHAN*2;i++){
      uint16_t me=CMPOFFSET+i;
      uint16_t other=CMPOFFSET+((i+CMPCHAN)%(CMPCHAN*2));
      if (changed[me]&&!changed[other]&&lastchange[other]!=NOTIME&& newstate[me]==newstate[other]){
        size_t dt=t-lastchange[other];
        delay[i].push_back(dt);
        /*if (dt>25 && t-lastchange[me]>5){
          cout<<"spike lenght"<<t-lastchange[me]<<me<<endl;
          cout<<"big delay "<<dt<<" at "<<name<<" t"<<t<<" for "<<cmpNames[i%CMPCHAN]<<" "<<std::fixed << std::setprecision(9)<<t/1E9<<endl;
          }*/
      }
    }
    
    for (size_t ch=0;ch<CHANNELS;ch++){
      if (changed[ch]){
        lastchange[ch]=t;
      }
    }
    
    /*std::vector<bool> ins(8);
    std::vector<bool> outs(8);
    std::copy(newstate.begin(),newstate.begin()+8,ins.begin());
    std::copy(newstate.begin()+8,newstate.begin()+16,outs.begin());
    push(ins,outs,t);*/

    pushChannel(newstate,t);
    
    state=newstate;
    
  }//while getline
  return true;
}


void printDelay(const std::vector<std::string> & cmpNames){
  for (size_t i=0;i<CMPCHAN;i++){    
    cout<<cmpNames[i]<<" "<<mean(delay[i])<<"["<<delay[i].size()<<"]"<<" vs "<< mean(delay[i+CMPCHAN])<<"["<<delay[i+CMPCHAN].size()<<"]"<<endl;
  }
}



int main(int argc,char** argv){

  std::vector<std::string>  cmpNames={"cdcas0","o6","o7","o8"};
  //const std::vector<std::string> &cmpNames=outNames;
  for (int i=1;i<argc;i++){
    channels.clear();
    load(argv[i],cmpNames);
    checkChannels(argv[i],cmpNames);
  }
  printDelay(cmpNames);
}
