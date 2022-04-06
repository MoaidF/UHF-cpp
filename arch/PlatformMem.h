#ifndef __SYSMEM_H__
#define __SYSMEM_H__


/*
*  malloc RAM ,if the platform 
*/
#define MEM_TYPE_RAM						1						
#define MEMP_TYPE_RL8000					2			


/* 
* Note: According to the different platforms to modify this memory calls
* malloc and free only  supported by WIN32 platform, if you use other platform you need 
* to modify it .

*SYSMEM_MALLOC: malloc memory 
*SYSMEM_FREE: free memory
*/


#ifdef WIN32
#define SYSMEM_MALLOC(type ,size )		malloc(size)
#define SYSMEM_FREE(type ,p)			free(p)
#endif







#endif
