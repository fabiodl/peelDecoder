#include <vector>
#include <iostream>
#include <iomanip>
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



bool verify(PeelInfer& p){
  p.forgetState();
  for (size_t i=0;i<data.size();++i){
    State pred=p.predict(data[i].inp,data[i].edge);
    if (!isIncluded(data[i].out,pred)){
      State real;
      real.v=data[i].out;
      real.k=0xFF;
      cerr<<endl<<"BUG, wrong prediction for index "<<dec<<i<<endl<<pred<<endl<<real<<endl;      
      cerr<<hex<<setfill('0')<<setw(2)
          <<" outd "<<(int)p.outd
          <<" fbd "<<(int)p.fbd
          <<" outneg "<<(int)p.outneg
          <<endl;
      
      return false;
    }
  }
  //cout<<endl<<"verify ok"<<endl;
  return true;
}


static constexpr uint8_t NPASSES=3;

void testConf(PeelInfer& peel,uint32_t conf){
  peel.reset();
  peel.outd=conf&0xFF;
  peel.fbd=(conf>>8)&0xFF;
  peel.outneg=(conf>>16)&0xFF;

  for (uint8_t i=0;i<NPASSES;i++){
    if (!peel.check(data)){
      validSink.addInvalid();
      if (i>1){
        std::cout<<"pass "<<(i+1)<<"reject"<<std::endl;
      }
      return;
    }
  }
  validSink.addValid(peel);  
  verify(peel);
  peel.getDeterministic();
}





int main(int argc,char** argv)
{

  if (argc<3){
    cout<<"Usage "<<argv[0]<<" dumpfile outfile"<<endl;
    return 0;
  }

  if (!loadTripletFile(data,argv[1])){   
      cerr<<"Unable to open input file "<<argv[1]<<endl;
      return -1;
  }
  
  if (!validSink.open(argv[2])){
    cerr<<"Unable to open output file "<<argv[2]<<endl;
    return -1;  
  }  
  cout<<"Possible confs are "<<0x1000000<<" test data are "<<data.size()<<std::endl;

  
  PeelInfer peel;
#ifdef USE_OMP    
#pragma omp parallel for private(peel)
#endif
  for (uint32_t conf=0;conf<0x1000000;conf++){
    testConf(peel,conf);       
  }
 
  
}
