#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
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


void hexPrint(uint8_t* buffer,size_t n){
  cout<<setfill('0')<<setw(2)<<hex;
  for (size_t i=0;i<n;i++){
    cout<<(int)buffer[i]<<" ";
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
    uint8_t buffer[14];
    size_t k=0;    
    for (size_t chunk=0;chunk<size;chunk++){
      file.read(reinterpret_cast<char*>(buffer),14);
      
      for (size_t i=0;i<9;i++){
        data[k+i].out=buffer[1+i];
      }
      data[k+0].inp=buffer[0];      
      for (size_t i=0;i<8;i++){
        //std::cout<<"F"<<(int)flip(buffer+10,i)<<endl;
        data[k+1+i].inp=data[k+i].inp^(1<<flip(buffer+10,i));
      }
      
      uint8_t crc=0;

      for (int i=0;i<9;i++){
        crc=maximcrc_push(crc,data[k+i].inp);
        crc=maximcrc_push(crc,data[k+i].out);        
        data[k+i].edge=(buffer[i] & 1) && !(prevInp & 1);
        prevInp=data[k+i].inp;
      }
      
      if (crc!=buffer[13]){
        hexPrint((uint8_t*)buffer,14);
        cout<<endl<<"IOS"<<endl;
        for (int i=0;i<9;i++){
          hexPrint(&data[k+i].inp,1);
          hexPrint(&data[k+i].out,1);
          cout<<endl;
        }
        cout<<"crc"<<(int)crc<<endl;
        std::cerr<<"CRC mismatch, truncating at "<<k<<std::endl;
        data.resize(k);
        //char skip;
        //file.read(&skip,1);
        break;
      }
      k+=9;
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


bool loadIoFile(std::vector<Data>& data,const char* name){
  ifstream f(name, ios::binary|ios::ate);
  if (f.is_open()){
    ifstream file(name, ios::in|ios::binary|ios::ate);
    size_t size = file.tellg();
    if (size%2){
      std::cerr<<"File is truncated"<<std::endl;
    }
    file.seekg (0, ios::beg);    
    size=size/2;
    data.resize(size);

    uint8_t prevInp=0;
    for (size_t i=0;i<size;i++){
      Data& d=data[i];
      uint8_t buff[2];
      file.read((char*)buff,2);
      d.inp=buff[0];
      d.out=buff[1];
      data[i].edge=(d.inp & 1) && !(prevInp & 1);
      prevInp=d.inp;
    }
   return true;
   
 }
 return false;
}

static constexpr int CHANNELS=16;
static constexpr int HEX_PER_LINE=128;


/*inline static bool  readChunk(ifstream& f,std::vector<std::vector<uint8_t>>& hexStream){
  char semicolon;
  int channel;
  std::string line;
  for (int ch=0;ch<CHANNELS;ch++){
    if (!std:getline(f,line)) return false;
    std::istringstream iss(line);
    iss>>channel>>semicolon;
    if (channel!=ch){
      cerr<<"Unexpected channel"<<channel<<endl;
      return false;
    }
    uint16_t v;
    for (int j=0;j<HEX_PER_LINE;j++){
      iss>>hex>>v;
      hexStream[ch][j]=v;
    }      
  }
  return true;
}




bool loadSigrokFile(std::vector<Data>& data,const char* name){
  ifstream f(name);
  std::string line;
  for (size_t i=0;i<2;i++){
  std:getline(infile,line); //throw away header
  }

  std::vector<std::vector<uint8_t>> hexStream(CHANNELS);
  for (int i=0;i<CHANNELS;i++){
    hexStream[i].resize(HEX_PER_LINE);
  }
  
  uint16_t state;

  readChunk(f,hexStream);
  

}

*/

bool loadVcdFile(std::vector<Data>& data,const char* name){
  ifstream f(name);

  std::map<char,uint8_t> chanId;
  string cmd;
  string line;
  do{
    getline(f,line);
    std::istringstream iss(line);
    iss>>cmd;
    //    cout<<"CMD {"<<cmd<<"}"<<endl;
    if (!iss) return false;

    if (cmd=="$var"){
      string wire;
      int n;
      char id;
      iss>>wire>>n>>id;
      uint16_t chan=chanId.size();
      chanId[id]=chan;
    }
  }while (cmd!="$enddefinitions");
  cout<<chanId.size()<<" channel definitions read"<<endl;
  for (auto& it:chanId){
    cout<<it.first<<":"<<(int)it.second<<endl;
  }
  
  uint16_t state=0;
  size_t prevT=0;
  int toSkip=2;
  size_t minIdt,minOdt;
  minIdt=minOdt=0xFFFF;
  size_t maxIdt,maxOdt;
  maxIdt=maxOdt=0;
  while(getline(f,line)){
    std::istringstream iss(line);
    char sharp;
    size_t t;
    iss>>sharp>>t;    
    uint16_t newstate=state;
    //cout<<"line"<<line<<endl;
    while(iss){
      char val;
      char chan;
      iss>>val>>chan;
      uint16_t mask=1<<chanId[chan];
      if (val=='1'){
        newstate|=mask;
      }else if (val=='0'){
        newstate&=~mask;
      }else{
        cerr<<"Unexpected value "<<val<<endl;
        return false;
      }
    }
    //cout<<hex<<"state"<<newstate<<endl;
    size_t dt=t-prevT;
    if ((state^newstate)&0xFF){
      if (!toSkip){
        if (dt<630){
          cout<<"Warning: Input changed after "<<(t-prevT)<<"@ t="<<t<<endl;
        }/*else if (dt>150){
          cout<<"Warning: Input changed after "<<(t-prevT)<<"@ t="<<t<<endl;
          }*/else{
          minIdt=min(minIdt,dt);
          maxIdt=max(maxIdt,dt);
        }
        Data d;
        d.inp=state&0xFF;
        d.out=(state>>8)&0xFF;
        d.edge=!data.size()?false: ((data[data.size()-1].inp^d.inp)&1);
        data.push_back(d);
      }else{
        toSkip--;
      }
      prevT=t;
      dt=0;
    }
    if (minIdt<630){
      cout<<"BAD minIDt @"<<t<<endl;
    }
    if ((state^newstate)&0xFF00){      
      /*if (dt>2){
        cout<<"Warning: Output changed after "<<(t-prevT)<<" dt "<<dt<<"@ t="<<t<<endl;
        }*/
      minOdt=min(minOdt,dt);
      maxOdt=max(maxOdt,dt);
    }
    state=newstate;
  }
  
  cout<<dec<<"min IDT "<<minIdt<<" max IDT "<<maxIdt<<endl;
  cout<<dec<<"min ODT "<<minOdt<<" max ODT "<<maxOdt<<endl;
  return true;
}
