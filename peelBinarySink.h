#ifndef _PEEL_BINARY_SINK_
#define _PEEL_BINARY_SINK_

#include <mutex>
#include <fstream>
#include "peel.h"

class PeelBinarySink{
public:
  PeelBinarySink();
  bool open(const char* filename);
  ~PeelBinarySink();
  void addValid(const PeelInfer& p);
  void addInvalid();
private:
  std::ofstream f;
  std::mutex m;
  size_t validCnt,invalidCnt;  
  void printProgress();
  void write(const PeelInfer& p);
};


#endif
