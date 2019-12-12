#include "infer.h"
#include <iostream>
#include <iomanip>

using namespace std;






int main(){

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
  peel.forgetQ();

  for (int i=0;i<2;i++){
    cout<<hex<<setfill('0')<<setw(2)<<" i="<<i<<endl;
    State o=peel.predict(i,false);
    cout<<" o="<<o<<endl;
  }
  

  

}
