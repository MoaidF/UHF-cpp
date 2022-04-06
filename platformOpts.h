#ifndef __PROJECT_OPTS_H__
#define __PROJECT_OPTS_H__




//#define INT32U unsigned int


#ifdef WIN32

		
/*
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

*/
		
#else
	   #include "stdbool.h"
		
	



#endif 







































#endif


