#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "ios.h"
#include <iomanip>


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
EdgeDelay wrSetup,wrHold;

Delay wrNeighSetup[8],wrNeighHold[8];

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

      for (j=0;j<i && j<0x1000 && getWr(inp[i])==getWr(inp[i-j]);j++);
      if (j!=i){
        wrSetup.push(j,getWr(inp[i]));
      }

      for (j=0; j<0x1000 && getWr(inp[i])==getWr(inp[i+j]);j++);
      if (i+j!=inp.size()){
        wrHold.push(j,getWr(inp[i]));
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

    if (getWr(inp[i])!=getWr(inp[i-1])){

      for (int s=0;s<8;s++){
        size_t j;
        for (j=0;j<i && j<0x1000 && (((inp[i]^inp[i-j])&(1<<s))==0); j++);
        if (i!=j){
          wrNeighSetup[s].push(j-1);
        }

        for (j=0;j+i<inp.size() && j<0x1000 && (((inp[i]^inp[i+j])&(1<<s))==0); j++);
        if (i+j!=inp.size()){
          wrNeighHold[s].push(j-1);
        }
        
      }

    }

    
  }  
}


void printStats(){
  cout
    <<"ras2->asel setup"<<ras2Asel<<endl
    <<"cas2->asel setup"<<cas2Asel<<endl
    <<"cas0->asel setup"<<cas0Asel<<endl
    <<"fdc->asel setup"<<fdcAsel<<endl
    <<"rom->asel setup"<<romAsel<<endl
    <<"wr->asel setup"<<wrSetup<<endl
    <<"wr->asel hold"<<wrHold<<endl
    <<"asel->q07 "<<aselQo7<<endl
    <<"inp->cdCas0 "<<cas0Cd<<endl
    <<"inp(ras2)->o6 "<<ras2O6<< endl
    <<"inp->Do8"<<inpDo8<<endl
    <<"inp->trDir"<<inpTrdir<<endl
    <<"inp->ffOe"<<inpFfoe<<endl
    <<"ffoe->trce"<<ffoeTrce<<endl
    <<"inp->ffCp"<<inpFfoe<<endl
    ;

  for (size_t i=0;i<8;i++){
    cout<<inpNames[i]<<" setup "<<wrNeighSetup[i]<<" hold "<<wrNeighHold[i]<<endl;
  }

}


bool isSingleBit(uint8_t v){
  for (uint8_t i=0;i<8;i++){
    if (v==(1<<i)) return true;
  }
  return false;
}


bool getBit(uint8_t v,uint8_t sel){
  return v&(1<<sel);
}

typedef bool (*binaryFunction)(uint8_t ins,uint8_t out);




static const std::string  levels="01";
void printSituation(int i){

  size_t namelen=0;
  for (size_t k=0;k<8;k++){
    namelen=max(namelen,inpNames[k].length());
    namelen=max(namelen,outNames[k].length());
  }

  for (size_t ink=0;ink<8;ink++){
    cout<<std::setw(namelen)<<inpNames[ink]<<std::setw(1)<<" ";
    for (int k=-10;k<=10;k++){
      cout<<levels[getBit(inp[i+k],ink)];    
    }
    cout<<endl;        
  }
  cout<<endl;
  for (size_t ink=0;ink<8;ink++){
    cout<<std::setw(namelen)<<outNames[ink]<<std::setw(1)<<" ";
    for (int k=-10;k<=10;k++){
      cout<<levels[getBit(out[i+k],ink)];    
    }
    cout<<endl;        
  }

  

  
}


class LogicFunction{

  uint8_t insense;
  uint8_t outsense;
  binaryFunction f;
  uint8_t outsel;
  
public:  
  LogicFunction(uint8_t _insense,
                uint8_t _outsense,
                binaryFunction _f,
                uint8_t _outsel):
    insense(_insense),outsense(_outsense),f(_f),outsel(_outsel){
  }
    
    
    

  
  void getDelay(size_t skip=20){
    size_t cntNorm=0;
    for (size_t i=1+skip;i<inp.size();i++){

      bool oldv=f(inp[i-1],out[i-1]);
      bool newv=f(inp[i],out[i]);
    
      if (oldv!=newv){
        bool singleChange=true;
        for (int k=-6;k<7;k++){
          if (k==0){
            continue;
          }
          if (
              ((inp[i+k]^inp[i+k-1])&insense)||
              ((out[i+k]^out[i+k-1])&outsense)
              ){
            singleChange=false;
            break;
          }//if change
        }//for k
        if (singleChange){        
          int k;
          for (k=0;k<100;k++){
            if (getBit(out[i+k],outsel)==newv){
              break;
            }
          }
          if (k>6){
            cout<<outNames[outsel]<<" long delay"<<k<<endl;
          }else if (k<3){
            cout<<outNames[outsel]<<" short delay"<<k<<endl;
            printSituation(i+k);
          }else{
            cntNorm++;            
          }
        }      
      }   
    }
    cout<<outNames[outsel]<<" "<<cntNorm<<" norm delays"<<endl;
  }
};


bool f8(uint8_t ins,uint8_t outs){
  bool Do8=
    (getCas2(ins) && getRas2(ins)) ||
    (!getCas2(ins) && getO8(outs))  ||
    (!getO6(outs) && getO8(outs));
  return Do8;
}

bool fTrDir(uint8_t ins,uint8_t outs){
  bool Dtr_dir=
    getCas0(ins) ||
    (getRom(ins) && getFdc(ins) && getO8(outs));
  return !Dtr_dir;
}


bool fffoe(uint8_t ins,uint8_t outs){
  bool Dff_oe=
    getCas0(ins) ||
    getO8(outs) ||
    (getO6(outs) && getCas2(ins)) ||
    (getCas2(ins) && !getO7(outs)) ||
    (getFfoe(outs) && getRas2(ins) && getO7(outs)) ||
    (getFfoe(outs) && !getO6(outs) && getO7(outs));
  return Dff_oe;
}


bool ftrce(uint8_t ins,uint8_t outs){
  return !getFfoe(outs);
}

bool fffcp(uint8_t ins,uint8_t outs){
  bool Dff_cp=
    getCas2(ins) ||
    getCas0(ins) ||
    getO7(outs) ||
    getO8(outs);
  return Dff_cp;
}


static inline uint8_t _BV(uint8_t v){
  return 1<<v;
}

LogicFunction lf8(_BV(CAS2)|_BV(RAS2),
                  _BV(O6)|_BV(O8),
                  f8,O8
                  );


LogicFunction lftrdir(_BV(CAS0)|_BV(ROM)|_BV(FDC),
                      _BV(O8),
                      fTrDir,TR_DIR
                      );

LogicFunction lfffoe(_BV(CAS0)|_BV(CAS2)|_BV(RAS2),
                     _BV(O6)|_BV(O7)|_BV(O8),
                     fffoe,FF_OE
                     );


LogicFunction lftrce(0,
                     _BV(TR_CE),
                     ftrce,TR_CE
                     );


LogicFunction lfffcp(_BV(CAS2)|_BV(CAS0),
                     _BV(O7)|_BV(O8),
                     fffcp,FF_CP
                     );


bool physicalVerify(int skip=20){


  bool o6=getO6(out[skip]);
  bool Qo7=!getO7(out[skip]);
  bool o8=getO8(out[skip]);
  bool tr_ce=getTrce(out[skip]);
  bool ff_oe=getFfoe(out[skip]);

  
  for (size_t i=1+skip;i<inp.size();i++){
    bool fdc=getFdc(inp[i]);
    bool cas2=getCas2(inp[i]);
    bool ras2=getRas2(inp[i]);
    bool cas0=getCas0(inp[i]);
    bool rom=getRom(inp[i]);
    bool wr=getWr(inp[i]);

    bool Dtr_ce,Dtr_dir,Dff_oe,Do6,Do8,Dff_cp;
    bool ff_cp,tr_dir,cdcas0,o7;

    
    if (getAsel(inp[i])&&!getAsel(inp[i-1])){
      Qo7=
        (Qo7 && fdc && rom && ras2) ||
        (Qo7 && fdc && rom && cas2 && cas0) ||
        (!ras2 && cas0 && cas2);
    }
    if (wr){
      Qo7=false;
    }
    
    Do8=
      (cas2 && ras2) ||
      (!cas2 && o8)  ||
      (!o6 && o8);
         
    Dtr_dir=
      cas0 ||
      (rom && fdc && Do8);
      
    Dff_oe=
      cas0 ||
      Do8 ||
      (o6 && cas2) ||
      (cas2 && Qo7) ||
      (ff_oe && ras2 && !Qo7) ||
      (ff_oe && !o6 && !Qo7);

    //Dtr_ce=!Dff_oe;
    Dtr_ce=
      (!Do8 && !cas0 && tr_ce &&  !o6 && !Qo7) ||
      (!Do8 && !cas0 && tr_ce && !cas2) ||
      (!Do8 && !cas0 && !ras2 && o6 && !cas2) ||
      (!Do8 && !cas0 && !cas2 && Qo7);
    

    
    Dff_cp= cas2 || cas0 || !Qo7 ||Do8;
    
    Do6=ras2;



    if ( (!Dtr_dir != tr_dir) && cas0!=getCas0(inp[i-1]) ){
      bool noChange=true;
      for (size_t k=-6;k<12;k++){
        if (
            (getRom(inp[i+k]) != rom)||
            (getFdc(inp[i+k]) != fdc)||
            (getO8(inp[i+k]) != Do8)
            ){
          noChange=false;
          break;
        }        
      }
      if (noChange){
        size_t k;
        for (k=1;k<100;k++){
          if (getTrdir(out[i+k])==!Dtr_dir){
            break;
          }
        }
        std::cout<<"Delay "<<k<<std::endl;
      }
      
    }





    
    tr_dir=!Dtr_dir;
    tr_ce=Dtr_ce;
    ff_cp=Dff_cp;
    ff_oe=Dff_oe;
    cdcas0=!cas0;
    o6=Do6;
    o7=!Qo7;
    o8=Do8;
   
    uint8_t o=(o8<<7)|(o7<<6)|(o6<<5)|(cdcas0<<4)|(ff_oe<<3)|(ff_cp<<2)|(tr_ce<<1)|tr_dir;
    
    size_t same=0;

    for (size_t k=1;k<7;k++){
      if (inp[i-k]==inp[i]){
        same++;
      }
    }
    
    if (o!=out[i] && same==6){
      cerr<<"Mismatch @ "<<i<<" t="<<i/(200E6)<<" for "<<outDesc(o^out[i])
          <<": prediction "<<outDesc(o)<<" actual "<<outDesc(out[i])<<endl;
   
      return false;
    }       

    
  }
  cout<<"Physical verify complete"<<endl;
  return true;
  
}



int main(int argc,char** argv){

  for (int i=1;i<argc;i++){
    loadConv(argv[i],10);
    cout<<argv[i]<<" size "<<inp.size()<<endl;
    //checkDelay();
    //physicalVerify();
    lf8.getDelay();
    lftrdir.getDelay();
    lfffoe.getDelay();
    lftrce.getDelay();
    lfffoe.getDelay();
  }
  //printStats();
}
