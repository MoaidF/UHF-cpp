#ifndef __PROJECT_OPTS_H__
#define __PROJECT_OPTS_H__







#ifdef WIN32

		

		#ifndef PACK_STRUCT_FIELD
		#define PACK_STRUCT_FIELD(x) x  
		#endif
		#ifndef PACK_STRUCT_STRUCT
		#define PACK_STRUCT_STRUCT 
		#endif
		#ifndef PACK_STRUCT_BEGIN
		#define PACK_STRUCT_BEGIN  	
		#endif
		#ifndef PACK_STRUCT_END
		#define PACK_STRUCT_END 
		#endif


		typedef char s8_t;
		typedef short s16_t ;
		typedef int s32_t ;

		typedef unsigned char u8_t ;
		typedef unsigned short u16_t ;
		typedef unsigned int   u32_t;
		typedef unsigned char BOOLEAN;


		typedef int err_t ;

		#ifndef FALSE
				#define FALSE	  0
		#endif
		#ifndef TRUE
				#define TRUE	  1
		#endif
		#ifndef NULL
		#define NULL			0
		#endif



		#define ___INLINE				
#else
	   #include "stdbool.h"
		
	

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

#endif 







































#endif


