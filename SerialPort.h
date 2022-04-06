#ifndef _WINSERIALPORT_H
#define _WINSERIALPORT_H
//#include "communication.h"
#include "serialPortCtrl.h"
#include "ORFIDDevDLL.h"
extern "C"
{
	//int OpenCom(char* comName);
	//void ClosePort();
	int WinSerial_Write(unsigned char *data, int len);
	int WinSerial_Read(unsigned char *buff,int ByteToRecv);
	void WinSerial_PurgeComm();
}


#endif