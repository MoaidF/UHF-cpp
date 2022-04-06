#ifndef __PLATFORM_TIMER_H__
#define __PLATFORM_TIMER_H__





/* uart functions ,according to your platform modify the Macro function below */
/*
*  TIMER_TRANSC_START:   start transceive timer
   timeout: type:u32_t ,unit: ms
*  TIMER_TRANSC_IS_FREE:  transceive timer is free ?
*  TIMER_TRANSC_STOP:   stop transceive timer
*/


#include "Timer.h"
#define TIMER_TRANSC_START(timeout)			WinTranscTimer_Start(timeout)
#define TIMER_TRANSC_IS_FREE()				WinTranscTimer_IsFree() 
#define TIMER_TRANSC_STOP()					WinTranscTimer_Stop()

















#endif

