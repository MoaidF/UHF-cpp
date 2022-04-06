#include "SerialPort.h"
#include <stdio.h>
#include"OIniFile.h"
#include <time.h>
#include <sys/time.h>


CSerialPortCtrl  *pSerialPort = NULL;
extern "C" int RL8000_WinOpenCom(char *comName)
{
    pSerialPort = new CSerialPortCtrl();

	OIniFile Ifile;
	Ifile.iLoadFile("SetComm.ini");
	BOOL retOpen = Ifile.bIsOpenOK();
	int ret = 0;
	if(retOpen)
	{
		int ComBaud = 0, ByteSize = 0, Parity = 0, topBits = 0,ComID;
		Ifile.iGetIntValue("RFIDREADER", "ComBaud", ComBaud);
		Ifile.iGetIntValue("RFIDREADER", "ByteSize", ByteSize);
		Ifile.iGetIntValue("RFIDREADER", "Parity", Parity);
		Ifile.iGetIntValue("RFIDREADER", "topBits", topBits);
		Ifile.iGetIntValue("RFIDREADER", "ComID", ComID);
		
		char cCom[16] = {0};
		sprintf(cCom,"/dev/COM%d",ComID);
		
		
		ret = pSerialPort->OpenCom(cCom, ComBaud, ByteSize, Parity, topBits, 0);
		printf("----------->cCom = %s, ret =%d\n", cCom,ret);

	}
	else
	{
		ret = pSerialPort->OpenCom(comName, 38400, 8, 2, 0, 0);
	}
	
	if(ret == 1)
		ret = -1;
	return ret;
}


void BytesToHexStr11111(u8_t *bBuffer,int bLen,char *strBuf) 
{ 
     u8_t l4b,r4b;
	 char l4c,r4c;
	 int i;
	 for(i=0;i<bLen;i++)
	 {
          r4b=bBuffer[i] & 0x0f ;
          if(r4b>=0 && r4b<=9)  
			  r4c=0x30+r4b ;
		  else
			  r4c=0x41+(r4b-0x0a) ;

		  l4b=(bBuffer[i] & 0xf0) >> 4 ;
          if(l4b>=0 && l4b<=9)  
			  l4c=0x30+l4b ;
		  else
			  l4c=0x41+(l4b-0x0a) ;


		  strBuf[i*2]=l4c ;
		  strBuf[i*2+1]=r4c;

	 }

}


extern "C" int WinSerial_Write(unsigned char *data, int len)
{
	
	char strbuff[1024]={0};
	
	if(len>0)
	{
		//BytesToHexStr11111(data,len,strbuff);
		//printf("WinSerial_Write %s \r\n",strbuff);
	}

	
	struct timeval t_start,t_end;
	 

   int iret=pSerialPort->WriteData(data, len, 100, false);

	//gettimeofday(&t_start, NULL);
	//printf("\r\nWrite Start time: %ld  %ld ms\r\n", t_start.tv_sec,t_start.tv_usec/1000);


	return len;
}



extern "C" int WinSerial_Read(unsigned char *buff,int ByteToRecv)
{
	char strbuff[1024]={0};
	//printf("WinSerial_Read\r\n");
	
	//struct timeval t_start,t_end;
	//gettimeofday(&t_start, NULL);
	//printf("\r\nRead Start time: %ld %ld ms\r\n",t_start.tv_sec,t_start.tv_usec/1000);
	
    pSerialPort->ReadDataEx(buff, ByteToRecv, 1000);
	

	 
	//gettimeofday(&t_start, NULL);
	//printf("\r\nRead end time: %ld %ld ms\r\n",t_start.tv_sec,t_start.tv_usec/1000);
	
	
	if(ByteToRecv>0)
	{
		//BytesToHexStr11111(buff,ByteToRecv,strbuff);

		//printf("pSerialPort->ReadDataEx  recvdata %s \r\n",strbuff);
	}
	//printf("pSerialPort->ReadDataEx  %d \r\n",ByteToRecv);
	

	
	
	return ByteToRecv;
	
}


extern "C" void WinSerial_PurgeComm()  
{

	pSerialPort->ClearBuffer();

}


extern "C" void RL8000_WinClosePort()
{
    pSerialPort->CloseCom();
}