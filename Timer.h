#ifndef __WINTIMER_H__
#define __WINTIMER_H__
#include "ORFIDDevDLL.h"
//extern "C" int WinTimerOpen();
void WinTranscTimer_Start(int timeout);
bool WinTranscTimer_IsFree(void);
void WinTranscTimer_Stop(void);

#endif