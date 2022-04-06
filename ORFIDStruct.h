#ifndef __RFIDSTRUCT_H__
#define __RFIDSTRUCT_H__




typedef void (*TAG_REPORT_HANDLER)(unsigned char ant,unsigned char aip,unsigned char tag,unsigned char uidlen,unsigned char uid[]);

/*
*RD5100F communication handle
*/

/*
* air protocol selection 
*/
#define AIP_SEL_ISO15693					0x01
#define AIP_SEL_ISO14443A					0x02

#define err_t int
#define BOOLEAN bool

#ifndef PACK_STRUCT_FIELD
#define PACK_STRUCT_FIELD(x) x  
#endif
#ifndef PACK_STRUCT_STRUCT
#define PACK_STRUCT_STRUCT 
#endif
#ifndef PACK_STRUCT_BEGIN
#define PACK_STRUCT_BEGIN  	__packed
#endif
#ifndef PACK_STRUCT_END
#define PACK_STRUCT_END 
#endif

typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   long    u32_t;
typedef signed     long    s32_t;

typedef float 			   f32_t;   //float

/*
* ISO15693 inventory parameter
*/

struct ISO15693InvenParam 
{
	u8_t AFI ;  
};

/*
* rfid air protocol interface type
*/
#define RFID_AIP_UNKNOWN								0	
#define RFID_AIP_ISO15693								1
#define RFID_AIP_ISO14443A								2
#define RFID_AIP_ISO14443B								3
#define RFID_AIP_FELICA									4
#define RFID_APL_ICODEEPCUID_ID							5
#define RFID_APL_I18000P3M3_ID							6
#define RFID_APL_I18092MPI_ID							7
#define RFID_APL_I18092MPI_212_ID						8
#define RFID_APL_I18092MPI_424_ID						9
#define RFID_APL_I18092MPT_ID							10
#define RFID_APL_FELICA_424_ID							11
#define RFID_APL_ICODESLI_ID							12

#ifndef NULL
#define NULL					0
#endif
#ifndef FALSE
#define FALSE					0 
#endif
#ifndef TRUE
#define TRUE					1
#endif
#include "string.h"

//typedef INT32U  mem_size_t;
#ifndef MEM_PTR_T
#define MEM_PTR_T

typedef unsigned int	INT32U;

typedef INT32U mem_ptr_t;
#endif
/** Calculate memory size for an aligned buffer - returns the next highest
 * multiple of MEM_ALIGNMENT (e.g. LWIP_MEM_ALIGN_SIZE(3) and
 * LWIP_MEM_ALIGN_SIZE(4) will both yield 4 for MEM_ALIGNMENT == 4).
 */
#ifndef MEM_ALIGNMENT
#define MEM_ALIGNMENT					4
#endif
#ifndef MEM_ALIGN_SIZE
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT-1))
#endif
/** Align a memory pointer to the alignment defined by MEM_ALIGNMENT
 * so that ADDR % MEM_ALIGNMENT == 0
 */

#ifndef DLL
#define DLL



#ifndef MEM_ALIGN
#define MEM_ALIGN(addr) ((void *)(((mem_ptr_t)(addr) + MEM_ALIGNMENT - 1) & ~(mem_ptr_t)(MEM_ALIGNMENT-1)))
#endif


/**
 * MEMCPY: override this if you have a faster implementation at hand than the
 * one included in your C library
 */
#ifndef MEMCPY
#define MEMCPY(dst,src,len)             memcpy(dst,src,len)
#endif

#define ___INLINE						__inline
	
typedef struct __CIR_BUFFER {  
  u32_t size;
  s32_t readPos;
  s32_t writePos;
  u32_t overflow;
  u32_t cursor ;
  u8_t *Buffer;
} CIR_BUFFER;
	
typedef struct
{
	u8_t *sndBuf;
	u8_t *rcvBuf;
	u16_t sndLen;
	u16_t rcvLen;
	u16_t rcvPopLeft;
	u16_t rcvPopIdx;
	u32_t timeout;
	CIR_BUFFER cbParser;
	u8_t NS;
	u8_t NR;
	u8_t DAD;

	u8_t hTag; // connected tag handle

} RL8000_COMM;
#endif
#endif