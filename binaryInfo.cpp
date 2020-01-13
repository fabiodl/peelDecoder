#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "ios.h"

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


EdgeDelay cas0Cd,ras2O6,ffoeTrce;
Delay ras2Asel,cas0Asel,cas2Asel,fdcAsel,romAsel;
Delay aselQo7;
EdgeDelay inpDo8,inpTrdir,inpFfoe,inpFfcp;


void checkDelay(){


    
  bool Qo7=getO7(out[0]);
  bool Do8=getO8(out[0]);
  bool trDir=getTrdir(out[0]);
  bool ffOe=getFfoe(out[0]);
  bool ffCp=getFfcp(out[0]);
  for (size_t i=1;i<inp.size();i++){
    bool fdc=getFdc(inp[i]);
    bool rom=getRom(inp[i]);
    bool ras2=getRas2(inp[i]);
    bool cas2=getCas2(inp[i]);
    bool cas0=getCas0(inp[i]);
    bool wr=getWr(inp[i]);

    
    if  (getCas0(inp[i])!=getCas0(inp[i-1])){
      size_t j;
      for (j=0;getCdcas0(out[i+j])==getCas0(inp[i]);j++);      
      //cout<<"j"<<j<<endl;
      cas0Cd.push(j,getCas0(inp[i]));
    }

    if  (getRas2(inp[i])!=getRas2(inp[i-1])){
      size_t j;
      for (j=0;getO6(out[i+j])!=getRas2(inp[i]);j++);            
      ras2O6.push(j,getRas2(inp[i]));
    }

    if (getFfoe(out[i])!=getFfoe(out[i-1])){
      size_t j;
      for (j=0;getTrce(out[i+j])==getFfoe(out[i]);j++);            
      ffoeTrce.push(j,getFfoe(out[i]));
    }


    if (getAsel(inp[i]) && !getAsel(inp[i-1])){
      size_t j;
      for (j=0;j<i && getRas2(inp[i])==getRas2(inp[i-j]);j++);
      if (j!=i){
        ras2Asel.push(j);
      }

      for (j=0;j<i && getCas2(inp[i])==getCas2(inp[i-j]);j++);
      if (j!=i){
        cas2Asel.push(j);
      }

      for (j=0;j<i && getCas0(inp[i])==getCas0(inp[i-j]);j++);
      if (j!=i){
        cas0Asel.push(j);
      }

      for (j=0;j<i && j<0x1000 && getFdc(inp[i])==getFdc(inp[i-j]);j++);
      if (j!=i){
        fdcAsel.push(j);
      }

      for (j=0;j<i && j<0x1000 && getRom(inp[i])==getRom(inp[i-j]);j++);
      if (j!=i){
        romAsel.push(j);
      }

      bool newO7= !(
        (!Qo7 && fdc && rom && ras2) ||
        (!Qo7 && fdc && rom && cas2 && cas0) ||
        (!ras2 && cas0 &&cas2)
                    );
      if (wr) newO7=true;
      
      if (newO7!=Qo7){
        for (j=0;getO7(out[i+j])!=newO7;j++);
        aselQo7.push(j);

      }
      Qo7=newO7;
      
    }//asel trans

    if (wr) Qo7=true;
    
    bool newDo8=
      (cas2 && ras2) ||
      (!cas2 && Do8)  ||
      (!getO6(out[i]) && Do8);
    if (newDo8!=Do8){
      size_t j;
      for (j=0;getO8(out[i+j])!=newDo8;j++);
      inpDo8.push(j,newDo8);
    }
    Do8=newDo8;

    bool o8glitch;
    {
      size_t j,k;
      for (j=1;j<3&&getO8(out[i+j])==getO8(out[i]);j++);
      for (k=1;k<3&&getO8(out[i-k])==getO8(out[i]);k++);
      o8glitch=(j+k-2)<3;      //-2 because @ the loop exit is the first bad
    }
    
    
    bool newTrdir=! (
                     cas0 ||
                     (rom&&fdc && getO8(out[i]))
                     );

    
    if (newTrdir!=trDir && !o8glitch ){
      size_t j;
      for (j=0;getTrdir(out[i+j])!=newTrdir;j++);
      inpTrdir.push(j,newTrdir);
      if (j>10){
        cout<<"bad at "<<i<<" = "<<((i+10)/200E6   ) <<" "<<inpDesc(inp[i])<<"->"
            <<newTrdir<<endl;
        
      }
    }
    trDir=newTrdir;

    if (!o8glitch){    
      bool newFfoe=
        cas0 ||
        getO8(out[i]) ||
        (getO6(out[i]) && cas2) ||
        (cas2 && !getO7(out[i])) ||
        (ffOe && ras2 && getO7(out[i])) ||
        (ffOe && !getO6(out[i]) && getO7(out[i]));

      if (newFfoe!=ffOe){
        size_t j;
        for (j=0;getFfoe(out[i+j])!=newFfoe;j++);
        inpFfoe.push(j,newFfoe);
         if (j>10){
           cout<<"bad at "<<i<<" = "<<((i+10)/200E6   ) <<" "<<inpDesc(inp[i])<<"->"
               <<newFfoe<<" trans "<<inpDesc(inp[i]^inp[i-1])<< endl;
           
         }
      }
      ffOe=newFfoe;         

      bool newFfcp= cas2 || cas0 || getO7(out[i]) || getO8(out[i]);
      
      if (newFfcp!=ffCp){
        size_t j;
        for (j=0;getFfcp(out[i+j])!=newFfcp;j++);
        inpFfcp.push(j,newFfcp);        
      }
      ffCp=newFfcp;

    }//o8glitch 
  }  
}


void printStats(){
  cout
    <<"ras2->asel setup"<<ras2Asel<<endl
    <<"cas2->asel setup"<<cas2Asel<<endl
    <<"cas0->asel setup"<<cas0Asel<<endl
    <<"fdc->asel setup"<<fdcAsel<<endl
    <<"rom->asel setup"<<romAsel<<endl
    
    <<"asel->q07 "<<aselQo7<<endl
    <<"inp->cdCas0 "<<cas0Cd<<endl
    <<"inp(ras2)->o6 "<<ras2O6<< endl
    <<"inp->Do8"<<inpDo8<<endl
    <<"inp->trDir"<<inpTrdir<<endl
    <<"inp->ffOe"<<inpFfoe<<endl
    <<"ffoe->trce"<<ffoeTrce<<endl
    <<"inp->ffCp"<<inpFfoe<<endl
    ;

}

int main(int argc,char** argv){

  for (int i=1;i<argc;i++){
    loadConv(argv[i],10);
    cout<<argv[i]<<" size "<<inp.size()<<endl;
    checkDelay();
  }
  printStats();
}
