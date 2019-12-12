#ifndef _DATALOAD_H_
#define _DATALOAD_H_

#include <vector>
#include "peel.h"

bool loadTripletFile(std::vector<Data>& data,const char* name);
bool loadTransitionsFile(std::vector<Data>& data,const char* name);

#endif
