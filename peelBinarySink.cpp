#include "peelBinarySink.h"
#include <iostream>
#include <iomanip>

using namespace std;

PeelBinarySink::PeelBinarySink():
  validCnt(0),invalidCnt(0){
}

bool PeelBinarySink::open(const char* filename)
{
  f.open(filename,ios::binary);
  return f.is_open();
}


PeelBinarySink::~PeelBinarySink(){
  if (f.is_open()){
    f.close();
  }  
}


void PeelBinarySink::printProgress(){
  size_t p=(100*(validCnt+invalidCnt))>>24;
  cout<<"tested "<<p<<"% :"<<validCnt <<" valid configurations "<<invalidCnt<<" invalid configurations ("<<setprecision(2)<<(100.0*validCnt/(validCnt+invalidCnt))  <<"%)          \r"<<flush;
}

void PeelBinarySink::addValid(const PeelInfer& p){
  m.lock();
  validCnt++;  
  write(p);
  printProgress();
  f.flush();
  m.unlock();
}

void PeelBinarySink::addInvalid(){
  m.lock();
  invalidCnt++;
  m.unlock();
}



static inline void write(ofstream& f,const AndBlock& ab){
  f.write(reinterpret_cast<const char*>(&ab.mask),sizeof(ab.mask));
  f.write(reinterpret_cast<const char*>(&ab.seenVal),sizeof(BoolState)*256*256);  
}


void PeelBinarySink::write(const PeelInfer& p){
  f.write(reinterpret_cast<const char*>(&p.outd),1);
  f.write(reinterpret_cast<const char*>(&p.fbd),1);
  f.write(reinterpret_cast<const char*>(&p.outneg),1);
  f.write(reinterpret_cast<const char*>(&p.f),246*256*sizeof(State));
  f.write(reinterpret_cast<const char*>(&p.q),sizeof(State));
  ::write(f,p.clr);
  ::write(f,p.set);
}
