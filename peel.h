#ifndef _PEEL_H_
#define _PEEL_H_

#include <stdint.h>
#include <stdbool.h>
#include <iostream>

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


std::ostream& operator<<(std::ostream&,const BoolState&);
std::ostream& operator<<(std::ostream&,const State&);

bool isSubset(State subset,State superset);
bool isIncluded(uint8_t v,State vset);


#endif
