#ifndef _IOS_H_
#define _IOS_H_

static std::vector<std::string> inpNames={"asel","fdc","rom","ras2","cas2","cas0","wr","fres"};
static std::vector<std::string> outNames={"tr_dir","tr_ce","ff_cp","ff_oe","cdcas0","o6","o7","o8"};


enum {ASEL,FDC,ROM,RAS2,CAS2,CAS0,WR,FRES};
enum {TR_DIR,TR_CE,FF_CP,FF_OE,CDCAS0,O6,O7,O8};

  
static inline bool getAsel(uint8_t v){
  return v&1;
}

static inline bool getFdc(uint8_t v){
  return v&(1<<1);
}

static inline bool getRom(uint8_t v){
  return v&(1<<2);
}

static inline bool getRas2(uint8_t v){
  return v&(1<<3);
}

static inline bool getCas2(uint8_t v){
  return v&(1<<4);
}

static inline bool getCas0(uint8_t v){
  return v&(1<<5);
}

static inline bool  getWr(uint8_t v){
  return v&(1<<6);
}


static inline bool getTrdir(uint8_t v){
  return v&1;
}


static inline bool getTrce(uint8_t v){
  return v&(1<<1);
}



static inline bool getFfcp(uint8_t v){
  return v&(1<<2);
}

static inline bool getFfoe(uint8_t v){
  return v&(1<<3);
}

static inline bool getCdcas0(uint8_t v){
  return v&(1<<4);
}


static inline bool getO6(uint8_t v){
  return v&(1<<5);
}

static inline bool getO7(uint8_t v){
  return v&(1<<6);
}

static inline bool getO8(uint8_t v){
  return v&(1<<7);
}





inline static std::string inpDesc(uint8_t inp){
  std::string s;
  for (int i=0;i<8;i++){
    if (inp& (1<<i)){
      s+=inpNames[i]+" ";
    }
  }
  return s;
}

inline static std::string outDesc(uint8_t out,const std::string& prefix=""){
  std::string s;
  for (int i=0;i<8;i++){
    if (out& (1<<i)){
      s+=prefix+outNames[i]+" ";
    }
  }
  return s;
}



#endif
