#ifndef _DATALOAD_H_
#define _DATALOAD_H_

#include <vector>
#include "peel.h"

bool loadTripletFile(std::vector<Data>& data,const char* name);
bool loadTransitionsFile(std::vector<Data>& data,const char* name);
bool loadFlipFile(std::vector<std::vector<Data> >& data,const char* name);

bool saveIoFile(const std::vector<Data>& data,const char* name);
bool loadIoFile(std::vector<Data>& data,const char* name);


#endif
