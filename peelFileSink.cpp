#include "peelFileSink.h"
#include <iostream>
#include <iomanip>

using namespace std;


PeelFileSink::PeelFileSink():
  validCnt(0){
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
  }
}

void PeelFileSink::update(const Peel& p,size_t batchInvalid){
  m.lock();
  validCnt++;
  invalidCnt+=batchInvalid;
  cout<<validCnt<<" valid configurations "<<invalidCnt<<" invalid configurations"<<endl;
  f<<","<<endl;
  f.flush();
  m.unlock();
}


void PeelFileSink::update(size_t batchInvalid){
  m.lock();
  invalidCnt+=batchInvalid;
  cout<<validCnt<<" valid configurations "<<(invalidCnt/1000000)<<"M invalid configurations"<<endl;
  m.unlock();
}

static inline void hexOut(std::ostream &os,const char* name,uint8_t val){
  os<<"\""<<name<<"\":\""<<static_cast<uint16_t>(val)<<"\",";
}


std::ostream & operator<<(std::ostream &os, const Peel& p){

  os<<hex<<setfill('0') << setw(2);
  hexOut(os,"outd",p.outd);
  hexOut(os,"fbd",p.fbd);
  hexOut(os,"outneg",p.outneg);
  
  hexOut(os,"clrMaskp",p.clrMaskp);
  hexOut(os,"clrMaskn",p.clrMaskn);
  hexOut(os,"setMaskp",p.setMaskp);
  hexOut(os,"setMaskn",p.setMaskn);

  os<<"\"f\":\"";
  for (uint16_t i=0;i<0x100;i++){    
    for (uint16_t j=0;i<0x100;j++){
      os<<static_cast<uint16_t>(p.f[i][j].v);
      os<<static_cast<uint16_t>(p.f[i][j].k);
    }
  }
  return os<<"\"]";  
}


std::istream & operator>>(std::istream &is, Peel& p){
  throw "unimplented yet";
  return is;
}

