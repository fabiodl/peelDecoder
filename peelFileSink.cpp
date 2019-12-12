#include "peelFileSink.h"
#include <iostream>
#include <iomanip>

using namespace std;


PeelFileSink::PeelFileSink():
  validCnt(0),invalidCnt(0){
}

bool PeelFileSink::open(const char* filename)
{
  f.open(filename);
  if (f.is_open()){
    f<<"[";
    return true;
  }
  return false;
}


PeelFileSink::~PeelFileSink(){
  if (f.is_open()){
    f<<"{}]";
    f.close();
  }
}

void PeelFileSink::update(const PeelInfer& p,size_t batchInvalid){
  m.lock();
  validCnt++;
  invalidCnt+=batchInvalid;
  cout<<validCnt<<" valid configurations "<<invalidCnt<<" invalid configurations"<<endl;
  f<<p<<","<<endl;
  f.flush();
  m.unlock();
}


void PeelFileSink::update(size_t batchInvalid){
  m.lock();
  invalidCnt+=batchInvalid;
  cout<<validCnt<<" valid configurations "<<(invalidCnt/1000000)<<"M invalid configurations"<<endl;
  m.unlock();
}



void PeelFileSink::printProgress(){
  size_t p=(100*(validCnt+invalidCnt))>>24;
  cout<<p<<"% :"<<validCnt <<" valid configurations "<<invalidCnt<<" invalid configurations\r"<<flush;
}


void PeelFileSink::addValid(const PeelInfer& p){
  m.lock();
  validCnt++;  
  f<<p<<","<<endl;
  printProgress();
  f.flush();
  m.unlock();
}

void PeelFileSink::addInvalid(){
  m.lock();
  invalidCnt++;
  m.unlock();
}




static inline void key(std::ostream &os,const char* name){
  os<<"\""<<name<<"\":";
}


static inline void hexOut(std::ostream &os,const char* name,uint8_t val){
  key(os,name);
  os<<"\""<<static_cast<uint16_t>(val)<<"\",";
}


static inline std::ostream& operator<<(std::ostream &os, const BoolState& bs){
    return os<<(bs.k? (bs.v?'1':'0'):'u');
  }

static inline std::ostream& operator<<(std::ostream &os, const AndBlock& ab){
  os<<"{";
  key(os,"mask");
  os<<"\""<<setw(8)<<ab.mask<<"\","<<setw(2);
  key(os,"seen");
  os<<"\"";
  for (size_t i=0;i<256;i++){
    for (size_t j=0;j<256;j++){
      os<<ab.seenVal[i][j];
    }
  }
  return os<<"\"}";    
}


std::ostream & operator<<(std::ostream &os, const PeelInfer& p){

  os<<hex<<setfill('0') << setw(2);
  hexOut(os,"outd",p.outd);
  hexOut(os,"fbd",p.fbd);
  hexOut(os,"outneg",p.outneg);
  key(os,"clr");
  os<<p.clr<<",";
  key(os,"set");
  os<<p.set<<",";  
  os<<"\"f\":\"";
  for (uint16_t i=0;i<256;i++){    
    for (uint16_t j=0;j<256;j++){
      os<<static_cast<uint16_t>(p.f[i][j].v);
      os<<static_cast<uint16_t>(p.f[i][j].k);
    }
  }
  return os<<"\"]";  
}


std::istream & operator>>(std::istream &is, PeelInfer& p){
  throw "unimplented";
  return is;
}

