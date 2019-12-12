#ifndef _FIXEDSR_H__
#define _FIXEDSR_H__

#include <stdint.h>
#include <vector>
#include <fstream>

struct State{
  uint8_t v;//value
  uint8_t k;//known
};

struct Data{
  uint8_t inp,out;
  bool edge;
};



class Peel{

public:
  uint8_t outd;
  uint8_t fbd;
  uint8_t outneg;
  uint8_t clrMaskp,clrMaskn,setMaskp,setMaskn;  
  void reset();
  bool check(uint8_t inp,uint8_t out,bool edge);
  bool check(const std::vector<Data>& data);
private:  
  friend std::ostream & operator<<(std::ostream &os, const Peel& p);
  friend std::istream & operator>>(std::istream &os, Peel& p);
  State f[256][256];  
  State q;
};
  

#endif
