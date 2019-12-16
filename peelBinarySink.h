#ifndef _PEEL_BINARY_SINK_
#define _PEEL_BINARY_SINK_

#include <mutex>
#include <fstream>
#include "infer.h"

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



class PeelBinarySource{
public:
  bool open(const char* filename);
  PeelInfer readNext();
  PeelInfer read(size_t idx);
  std::vector<size_t> unknownCount();
  size_t size(){return s;}
private:
  std::ifstream f;
  size_t s;
  
};

#endif
