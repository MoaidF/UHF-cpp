#ifndef __TOOL_H__
#define __TOOL_H__


//#define u16_t unsigned short 
//#define u8_t unsigned char 
//#define u32_t unsigned int 
#include "ORFIDDevDLL.h"
#include "platformOpts.h"


u16_t cal_crc16_ext(u16_t initval ,u8_t *ptr,  u16_t len) ;
int EBV_Set(u32_t dw,u8_t* pbuf) ;
int EBV_GetSize(u32_t dw) ;
int EBV_Get(u8_t buf[],u16_t leftSize,u32_t *pval) ;



#endif
