
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ORFIDDevDLL.h"
int timerCounter =0 ;

bool runTimer=false;
void ThreadTimer(void )
{
	runTimer=true;
	while(runTimer)
	{
		usleep(20*1000);
		timerCounter+=20;
	}

}

int transcTimeout = 0;
int transcStartTime =0 ;
bool transcTimerStarted=false;

pthread_t id1;
extern "C" int RL8000_WinTimerOpen()
{
	transcTimeout = 0;
	transcStartTime =0 ;
	transcTimerStarted=false ;

	int i = 0;
	int ret = 0;
	//pthread_t id1,id2;
 
	ret = pthread_create(&id1, NULL, (void *)ThreadTimer,NULL);
    if( ret )
    {
        return -1;
    }

    return 0;
}

void WinTranscTimer_Start(int timeout)
{
	transcStartTime = timerCounter ;
	transcTimeout = timeout ;
	transcTimerStarted =true ;
}

bool WinTranscTimer_IsFree(void)   
{
	if(!transcTimerStarted) return true ;
	if(timerCounter >=(transcStartTime+transcTimeout)){
		/* free */
		return true ;
	}
	return false ;
}


void WinTranscTimer_Stop(void)
{
	transcTimerStarted =false ; 
}


extern "C" void RL8000_WinKillTimer()
{
	runTimer=false;
	pthread_join(id1,NULL);
}
