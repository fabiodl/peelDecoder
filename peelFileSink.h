#ifndef _PEEL_FILE_SINK_
#define _PEEL_FILE_SINK_

#include <fstream>
#include <mutex>
#include "simul.h"

class PeelFileSink{
public:
  PeelFileSink();
  bool open(const char* filename);
  ~PeelFileSink();
  void update(const Peel& p,size_t batchInvalid);
  void update(size_t batchInvalid);
private:
  std::ofstream f;
  std::mutex m;
  size_t validCnt,invalidCnt;  
};

std::ostream & operator<<(std::ostream &os, const Peel& p);
std::istream & operator>>(std::istream &is, Peel& p);


#endif
