#include <fstream>
#include <iostream>
#include "dataLoad.h"
#include "maximcrc.h"
using namespace std;


bool loadTripletFile(std::vector<Data>& data,const char* name){
  ifstream file(name, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    size_t size = file.tellg();
    uint8_t* buffer = new uint8_t [size];
    file.seekg (0, ios::beg);
    file.read (reinterpret_cast<char*>(buffer), size);

    data.resize(size/3);
    size_t k=0;
    uint8_t prevInp=0;
    for (size_t i=0;i<size;i+=3){
      data[k].inp=buffer[i];
      data[k].out=buffer[i+1];
      data[k].edge=(buffer[i] & 1) && !(prevInp & 1);
      prevInp=buffer[i];
      k++;
    }

    file.close();
    delete[] buffer;    
    return true;
  }else{
    return false;
  }
}



bool loadTransitionsFile(std::vector<Data>& data,const char* name){
  ifstream file(name, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    size_t size = file.tellg();
    uint8_t* buffer = new uint8_t [size];
    file.seekg (0, ios::beg);
    file.read (reinterpret_cast<char*>(buffer), size);

    data.resize(size/3);
    size_t k=0;
    uint8_t crc=0;
    uint8_t prevInp=0;
    for (size_t i=0;i<size;i+=3){
      crc=maximcrc_pushChunk(crc,buffer+i,2);
      if (crc!=buffer[i+2]){
        std::cerr<<"CRC mismatch, truncating at "<<k<<std::endl;
        data.resize(k);
        break;
      }
      data[k].inp=buffer[i];
      data[k].out=buffer[i+1];
      data[k].edge=(buffer[i] & 1) && !(prevInp & 1);
      prevInp=buffer[i];
    }

    file.close();
    delete[] buffer;    
    return true;
  }else{
    return false;
  }
}
