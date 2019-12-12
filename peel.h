#ifndef _PEEL_H_
#define _PEEL_H_

struct BoolState{
  bool v,k;
};

struct State{
  uint8_t v;//value
  uint8_t k;//known
};


struct Data{
  uint8_t inp,out;
  bool edge;
};




#endif
