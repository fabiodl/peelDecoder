#ifndef _MAXIMCRC_H_
#define _MAXIMCRC_H_
#include <stdint.h>


uint8_t maximcrc_push(uint8_t crc,uint8_t data);
uint8_t maximcrc_pushChunk(uint8_t crc,uint8_t* data,uint8_t n);

#endif
