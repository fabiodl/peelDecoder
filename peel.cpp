#include "peel.h"


static char symbols[2][2]={ {'u','u'},{'0','1'}};

std::ostream& operator<<(std::ostream& o,const BoolState& s){
  return o<<symbols[s.k][s.v];
}
std::ostream& operator<<(std::ostream& o,const State& s){
   for (int b=7;b>=0;b--){
      uint8_t m=1<<b;
      o<< symbols[s.k&m?1:0][s.v&m?1:0];
    }
   return o;
}
