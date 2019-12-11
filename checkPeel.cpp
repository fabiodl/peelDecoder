#include "simul.h"
#include "peelFileSink.h"
#include <vector>
#include <iostream>


#define USE_OMP

#ifdef USE_OMP
#include <omp.h>
#else
size_t omp_get_thread_num(){
  return 0;
}

size_t omp_get_num_threads(){
  return 1;
}

#endif



using namespace std;

vector<Data> data;



PeelFileSink validSink;

static inline bool validConfig(uint32_t c){
  uint8_t common=((c>>8)&c&0xFF);
  return common && common!=0b11;
}


void testConf(Peel& peel,uint32_t conf){
  size_t batchInvalid=0;
  for (uint32_t cm=0;cm<0x10000;cm++){
    if (!validConfig(cm)) continue; 
    for (uint32_t sm=0;sm<0x10000;sm++){
      if (!validConfig(sm)) continue; 
      peel.reset();
      peel.outd=conf&0xFF;
      peel.fbd=(conf>>8)&0xFF;
      peel.outneg=(conf>>16)&0xFF;
      peel.clrMaskp=cm&0xFF;
      peel.clrMaskn=(cm>>8)&0xFF;
      peel.setMaskp=sm&0xFF;
      peel.setMaskn=(sm>>8)&0xFF;
      
      if (peel.check(data)){
        validSink.update(peel,batchInvalid);
        batchInvalid=0;
      }else{
        batchInvalid++;
      }
      if (batchInvalid>=10000000){
        validSink.update(batchInvalid);
        batchInvalid=0;
      }
      
    }
  }      
}




bool loadTripletFile(const char* name){
  ifstream file(name, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    size_t size = file.tellg();
    uint8_t* buffer = new uint8_t [size];
    file.seekg (0, ios::beg);
    file.read (reinterpret_cast<char*>(buffer), size);

    data.resize(size/3);
    size_t k=0;
    uint8_t prevInp=0;
    for (size_t i=0;i<size;i+=3){
      data[k].inp=buffer[i];
      data[k].out=buffer[i+1];
      data[k].edge=(buffer[i] & 1) && !(prevInp & 1);
      prevInp=buffer[i];
    }

    file.close();
    delete[] buffer;    
    return true;
  }else{
    return false;
  }
}


int main(int argc,char** argv)
{

  if (argc<3){
    cout<<"Usage "<<argv[0]<<" dumpfile outfile"<<endl;
    return 0;
  }

  if (!loadTripletFile(argv[1])){   
      cerr<<"Unable to open input file "<<argv[1]<<endl;
      return -1;
  }
  
  if (!validSink.open(argv[2])){
    cerr<<"Unable to open output file "<<argv[2]<<endl;
    return -1;  
  }

  Peel peel;
#ifdef USE_OMP    
#pragma omp parallel for private(peel)
#endif
  for (uint32_t conf=0;conf<0x1000000;conf++){
    testConf(peel,conf);       
  }
 
  
}
