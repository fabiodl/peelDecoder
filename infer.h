#ifndef _INFER_H_
#define _INFER_H_

#include <stdint.h>
#include <vector>
#include <fstream>

struct State{
  uint8_t v;//value
  uint8_t k;//known
};


struct BoolState{
  bool v,k;
};

class AndBlock{
public:
  uint32_t mask;
  BoolState seenVal[256][256]; 
  void reset();
  bool check(BoolState& s,uint8_t inp,State fb);
};



struct Data{
  uint8_t inp,out;
  bool edge;
};

class PeelBinarySink; //fw declaration

class PeelInfer{

public:
  uint8_t outd;
  uint8_t fbd;
  uint8_t outneg;
  void reset();
  bool check(uint8_t inp,uint8_t out,bool edge);
  bool check(const std::vector<Data>& data);
private:  
  friend std::ostream & operator<<(std::ostream &os, const PeelInfer& p);
  friend std::istream & operator>>(std::istream &os, PeelInfer& p);
  friend PeelBinarySink;
  State f[256][256];  
  State q;
  AndBlock clr,set;
};
  

#endif
