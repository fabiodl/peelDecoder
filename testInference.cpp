#include "infer.h"
#include <iostream>
#include <iomanip>

using namespace std;





void testAlld(){
  PeelInfer peel;
  peel.outd=0xFF;
  peel.fbd=0xFF;
  peel.outneg=0x00;
  peel.reset();
  
  for (int i=0;i<256;i++){
    if (!peel.check(i,i,false)){
      cout<<"Check failed"<<endl;
    }
  }
  peel.forgetState();

  for (int i=0;i<10;i++){
    cout<<hex<<setfill('0')<<setw(2)<<" i="<<i<<endl;
    State o=peel.predict(i,false);
    cout<<" o="<<o<<endl;
  }  
}


void testoutq(){
  PeelInfer peel;
  peel.outd=0x00;
  peel.fbd=0xFF;
  peel.outneg=0x00;
  peel.reset();
  
  for (int i=0;i<256;i++){
    if (!peel.check(i,i-1,false)){
      cout<<"Check failed"<<endl;
    }
    if (!peel.check(i,i,true)){
      cout<<"Check failed"<<endl;
    }    
  }

  peel.forgetState();

  for (int i=0;i<10;i++){
    cout<<hex<<setfill('0')<<setw(2)<<" i="<<i<<endl;
    State o=peel.predict(i,false);
    cout<<" o="<<o<<endl;
    o=peel.predict(i,true);
    cout<<" o="<<o<<endl;
  }  
  
  
}


int main(){

  testoutq();

  
  

}
