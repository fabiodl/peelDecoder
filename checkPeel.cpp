#include <vector>
#include <iostream>

#include "infer.h"
#include "peelBinarySink.h"
#include "dataLoad.h"

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

PeelBinarySink validSink;

void testConf(PeelInfer& peel,uint32_t conf){
  peel.reset();
  peel.outd=conf&0xFF;
  peel.fbd=(conf>>8)&0xFF;
  peel.outneg=(conf>>16)&0xFF;
  
  if (peel.check(data)){
    validSink.addValid(peel);
  }else{
    validSink.addInvalid();
  }
}





int main(int argc,char** argv)
{

  if (argc<3){
    cout<<"Usage "<<argv[0]<<" dumpfile outfile"<<endl;
    return 0;
  }

  if (!loadTransitionsFile(data,argv[1])){   
      cerr<<"Unable to open input file "<<argv[1]<<endl;
      return -1;
  }
  
  if (!validSink.open(argv[2])){
    cerr<<"Unable to open output file "<<argv[2]<<endl;
    return -1;  
  }  
  cout<<"Possible confs are "<<0x1000000<<std::endl;

  
  PeelInfer peel;
#ifdef USE_OMP    
#pragma omp parallel for private(peel)
#endif
  for (uint32_t conf=0;conf<0x1000000;conf++){
    testConf(peel,conf);       
  }
 
  
}
