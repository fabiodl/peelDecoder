#ifndef _INFER_H_
#define _INFER_H_

#include "peel.h"

#include <stdint.h>
#include <vector>
#include <fstream>



class AndBlock{
public:
  uint32_t mask;
  BoolState seenVal[256][256]; 
  void reset();
  bool check(BoolState& s,uint8_t inp,State fb,uint8_t fbd);
  BoolState predict(uint8_t inp,State fb,uint8_t fbd);

  bool getDeterministic(uint32_t &,uint8_t fbd );
};

class PeelBinarySink; //fw declaration
class PeelBinarySource;

class DeterministicPeel{
public:
  uint8_t outd;
  uint8_t fbd;
  uint8_t outneg;
  uint8_t predict(uint8_t inp,bool edge);
  uint8_t q,fb;
  uint8_t f[256][256];
  uint32_t clrMask,setMask;
};


class PeelInfer{

public:
  uint8_t outd;
  uint8_t fbd;
  uint8_t outneg;
  void reset();
  bool check(uint8_t inp,uint8_t out,bool edge);//returns true on success
  bool check(const std::vector<Data>& data);//returns true on success
  void forgetState();
  State predict(uint8_t inp,bool edge);

  DeterministicPeel getDeterministic();

  size_t fUnknownCount();
  
private:  
  friend std::ostream & operator<<(std::ostream &os, const PeelInfer& p);
  friend std::istream & operator>>(std::istream &os, PeelInfer& p);
  friend PeelBinarySink;
  friend PeelBinarySource;

  State f[256][256];  
  State q,fb;
  AndBlock clr,set;
};
  

#endif
