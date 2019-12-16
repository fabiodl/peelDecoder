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
  f.write(reinterpret_cast<const char*>(&p.f),256*256*sizeof(State));
  f.write(reinterpret_cast<const char*>(&p.q),sizeof(State));
  ::write(f,p.clr);
  ::write(f,p.set);
}

static constexpr size_t SIZE=3+256*256*sizeof(State)+sizeof(State)+2*(sizeof(uint32_t)+sizeof(BoolState)*256*256);


bool PeelBinarySource::open(const char* filename){
  f.open(filename,ios::binary);
  if (f.is_open()){
    f.seekg (0, f.end);
    s = f.tellg();
    f.seekg (0, f.beg);
    if (s%SIZE){
      cerr<<"Warning: Truncated file"<<endl;
    }
    s=s/SIZE;
    return true;
  }
  return false;
}


static inline void read(ifstream& f,AndBlock& ab){
  f.read(reinterpret_cast< char*>(&ab.mask),sizeof(ab.mask));
  f.read(reinterpret_cast< char*>(&ab.seenVal),sizeof(BoolState)*256*256);  
}



PeelInfer PeelBinarySource::readNext(){
  PeelInfer p;
  f.read(reinterpret_cast< char*>(&p.outd),1);
  f.read(reinterpret_cast< char*>(&p.fbd),1);
  f.read(reinterpret_cast< char*>(&p.outneg),1);
  f.read(reinterpret_cast< char*>(&p.f),256*256*sizeof(State));
  f.read(reinterpret_cast< char*>(&p.q),sizeof(State));
  ::read(f,p.clr);
  ::read(f,p.set);
  return p;
}

PeelInfer PeelBinarySource::read(size_t idx){
  f.seekg(idx*SIZE,ios_base::beg);
  return readNext();
}


vector<size_t> PeelBinarySource::unknownCount(){
  vector<size_t> r(s);
  for (size_t i=0;i<s;i++){
    r[i]=readNext().fUnknownCount();
  }
  return r;
}
