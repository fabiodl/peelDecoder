#ifndef _PEEL_FILE_SINK_
#define _PEEL_FILE_SINK_

#include <fstream>
#include <mutex>
#include "infer.h"

class PeelFileSink{
public:
  PeelFileSink();
  bool open(const char* filename);
  ~PeelFileSink();
  void update(const PeelInfer& p,size_t batchInvalid);
  void update(size_t batchInvalid);
  void addValid(const PeelInfer& p);
  void addInvalid();
private:
  std::ofstream f;
  std::mutex m;
  size_t validCnt,invalidCnt;  
  void printProgress();
};

std::ostream & operator<<(std::ostream &os, const PeelInfer& p);
std::istream & operator>>(std::istream &is, PeelInfer& p);


#endif
