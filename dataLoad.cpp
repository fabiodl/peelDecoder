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
      k++;
    }

    file.close();
    delete[] buffer;    
    return true;
  }else{
    return false;
  }
}

static inline uint8_t flip(const uint8_t* buff,uint8_t i){
  switch(i){
  case 0:
    return buff[0]>>5;
  case 1:
    return (buff[0]>>2)&0x07;
  case 2:
    return ((buff[0]&0x03)<<1)|(buff[1]>>7);
  case 3:
    return (buff[1]>>4)&0x07;
  case 4:
    return (buff[1]>>1)&0x07;
  case 5:
    return ((buff[1]&1)<<2)|((buff[2]>>6)&0x03);
  case 6:
    return (buff[2]>>3)&0x07;
  default:
    return buff[2]&0x07;    
  }
}

bool loadFlipFile(std::vector<Data>& data,const char* name){
  ifstream file(name, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    size_t size = file.tellg();
    if (size%14){
      std::cerr<<"File is truncated"<<std::endl;
    }
    file.seekg (0, ios::beg);    
    data.resize(size/14*9);
    size=size/14;
    uint8_t prevInp=0;
    char buffer[14];
    size_t k=0;    
    for (size_t chunk=0;chunk<size;chunk++){
      file.read(buffer,14);
      
      data[k+0].inp=buffer[0];      
      for (size_t i=0;i<9;i++){
        data[k+i].inp=buffer[1+i];
      }
      data[k].out=buffer[0];
      for (size_t i=0;i<8;i++){
        data[k+1+i].out=data[k+i].out^=flip(reinterpret_cast<uint8_t*>(buffer+10),i);
      }
      
      uint8_t crc=0;

      for (int i=0;i<9;i++){
        crc=maximcrc_push(crc,data[k+i].inp);
        crc=maximcrc_push(crc,data[k+i].out);        
        data[k+i].edge=(buffer[i] & 1) && !(prevInp & 1);
        prevInp=data[k+i].inp;
      }
      
      if (crc!=buffer[k+13]){
        std::cerr<<"CRC mismatch, truncating at "<<k<<std::endl;
        data.resize(k);
        break;
      }     
      k+=14;
    }

    file.close();
    return true;
  }else{
    return false;
  }

  
}


bool saveIoFile(const std::vector<Data>& data,const char* name){
 ofstream f(name, ios::binary);
 if (f.is_open()){
   for (const Data& d:data){
     f.write(reinterpret_cast<const char*>(&d.inp),1);
     f.write(reinterpret_cast<const char*>(&d.out),1);
   }

   return true;
   
 }
 return false;
}
