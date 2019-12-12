#include "maximcrc.h"


static const unsigned char L[16]={0x00,0x5e,0xbc,0xe2,
      0x61,0x3f,0xdd,0x83,
      0xc2,0x9c,0x7e,0x20,
      0xa3,0xfd,0x1f,0x41};
static const unsigned char H[16]={0x00,0x9d,0x23,0xbe,
      0x46,0xdb,0x65,0xf8,
      0x8c,0x11,0xaf,0x32,
      0xca,0x57,0xe9,0x74};


uint8_t maximcrc_push(uint8_t crc, uint8_t crc_data)
{
  uint8_t i;
  i = (crc_data ^ crc) ;
  crc = L[i&0x0F]^H[i>>4];
  return crc;
}



uint8_t maximcrc_pushChunk(uint8_t crc,uint8_t* data,uint8_t n){
  uint8_t idx;
  for (int i=0;i<n;i++){
    idx=( (*data) ^ crc);
    crc = L[idx&0x0F]^H[idx>>4];
    ++data;
  }
  return crc;
}
