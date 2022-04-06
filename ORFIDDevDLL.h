#ifndef __RFIDDEVDLL_H__
#define __RFIDDEVDLL_H__


#include "ORFIDStruct.h"

#define ERR_OK					0
#define ERR_UNKNOWN				1
#define ERR_COMM_IO				2
#define ERR_TIMEOUT				3
#define ERR_MSGSIZE				4
#define ERR_MSGDATA				5
#define ERR_SYNCLOST			6
#define ERR_ARG					7
#define ERR_TAGACCESS			8

extern "C"
{
	int RL8000_WinTimerOpen();
	
	int RL8000_WinOpenCom(char *comName);

	/*
	* create RL8000 handle
	*/
	//RL8000_COMM *RL8000_CommOpen(void) ;
	
	/*
	* create RL8000 handle  with 485 bus Addr
	*/
	
	//RL8000_COMM *RL8000_CommOpenWithBusAddr(u8_t busAddr);
	
	err_t RL8000_WinTagInventory(u8_t busAddr, char *tagUid, int &nlen);
	/*
	* connect device 
	*/
	//err_t RL8000_Connect(RL8000_COMM *h) ;
	
	void RL8000_WinKillTimer();

	void RL8000_WinClosePort();
	
	/*
	* get device info 
	*/
	//err_t RL8000_CMD_GetDeviceInfo(RL8000_COMM *h ,u8_t *devInf,u16_t *nSize) ;
	/*
	*reset device 
	*/
	err_t RL8000_CMD_Reset(RL8000_COMM *h ) ;
	/*
	*tag inventory 
	*/
	//err_t RL8000_CMD_TagInventory(RL8000_COMM *h ,BOOLEAN continueInven,u8_t AIPSelFlag ,struct ISO15693InvenParam *pISO15693InvenParm,TAG_REPORT_HANDLER tag_report_handler,u16_t *tagGetted) ;
	/*
	*disconnect from tag 
	*/
	err_t RL8000_CMD_TagDisconnect(RL8000_COMM *h) ;
}

#endif