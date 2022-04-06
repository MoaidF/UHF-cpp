#include<stdio.h>
#include "ORFIDDevDLL.h"
//#include "platformOpts.h"
#include <string>
#include <pthread.h>
//#include "rl8000_comm.h"
//#include "Timer.h"
//#include "SerialPort.h"


/*
* tag report handler
*/

#define MAX_UIDS			100
#define MAX_UID_LEN			16
struct tagreport
{
	u8_t tag;
	u8_t uidLen;
	u8_t uid[16] ;
	
	u8_t user[255];
	u8_t userlen;
	
} ;

/*
struct tagreport tagReportSet[MAX_UIDS] ; 
u16_t tagReportAdded ;
*/
/*
struct Tag
{
	byte uid[8];
	int uidlen;
	int busAddr;
}

map<byte,vector<Tag>> mapBusAddrToReports;
*/


//ant:天线，不必须
//aip：射频空中协议类型，不必须
//tag:标签类型，必须
//uidlen:uid长度。必须
//uid数组,必须

void BytesToHexStr(u8_t *bBuffer,int bLen,char *strBuf) 
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
void BytesReverse(u8_t *buf,int len ) 
{
	int i;
	u8_t tmp ;
	for(i = 0;i< len/ 2 ;i++ ) {
		tmp = buf[i] ;
		buf[i] = buf[len-1 -i] ;
		buf[len-1 -i] = tmp ;
	}
}

//byte currentAddr;

int main(void)
{
	
	RL8000_COMM * hr ;
	
	int i ;
	u16_t tagGetted;
	tagGetted=0;
	struct ISO15693InvenParam ISO15693InvenParm ;
	
	/*
	int size = sizeof(struct tagreport);
	printf("---------------%d\n",size);
	return 0;
	*/
	/*
	int retcom = RL8000_WinOpenCom("/dev/ttyS0");
	
	if(retcom == -1)
	{
		printf("连接串口失败\n");
		return -1;
	}
	*/
	//usleep(500 *1000);
	
	/*
	for(int i=1;i<7;i++)
	{
	
	hr = RL8000_CommOpenWithBusAddr(i) ;
	
	//currentAddr=i;
	
	RL8000_Connect(hr);
	
	unsigned char devInfBuf[32] = {0};
	u16_t	nSize = sizeof(devInfBuf) ;
	//memset(devInfBuf, 0, sizeof(devInfBuf));
	//int ret = RL8000_CMD_GetDeviceInfo(hr,devInfBuf,&nSize) ;
	printf("-----------> nSize = %d",nSize);
	
	tagReportAdded = 0;
	ISO15693InvenParm.AFI =0 ;//write
	tagGetted =0 ;
	int iret =RL8000_CMD_TagInventory(hr,false,(AIP_SEL_ISO15693 | AIP_SEL_ISO14443A),&ISO15693InvenParm,tag_report_handler,&tagGetted);
	
	

		for(i=0;i<tagReportAdded;i++ ) {
		
			if(tagReportSet[i].aip == RFID_AIP_ISO15693) {

				
				char strbuf[64] ;
				u8_t rsUID[8];
				memcpy(rsUID,tagReportSet[i].uid ,8) ;
				BytesReverse(rsUID ,8) ;
				memset(strbuf,0,sizeof(strbuf)) ;
				BytesToHexStr(rsUID,8 ,strbuf) ;
				printf("UID:%s  \n",strbuf) ;

			}
		}

		
		delete hr;
		hr = NULL;
	}
	
	*/
#if 0
	bool bIsOpenComm = false;
	while(1)
	{
		printf("\r\n1:打开串口通信； 2:获取标签信息； 3:关闭串口； 0：退出程序\n");
		printf("Please select a menu: ");
		scanf("%d",&i);
		
			
		if (i==0)
		{
			printf("Byebye.\n\n");
			exit(1);
		}
		switch(i)
		{
			case 1:
			{
				if(!bIsOpenComm)
				{
					RL8000_WinTimerOpen();
					int retcom = RL8000_WinOpenCom("/dev/ttyS0");
		
					if(retcom == -1)
					{
						printf("连接串口失败\n");
						return -1;
					}
					else
						bIsOpenComm = true;
				}

			}
			break;
			case 2:
			{
				if(bIsOpenComm)
				{
					int addr = 1;
					printf("Please input addr(1-6): ");
					scanf("%d",&addr);
					char *tag = new char[1024];
					int  nlen = 1024;
					
					int ret = RL8000_WinTagInventory(addr, tag, nlen);
					
					if(ret!=0)
						printf("bus:%d  error!!!!!!!!!!!!!!!\r\n",addr);
					
					int cout = nlen / sizeof(struct tagreport);
					printf("----------------->ret = %d, cout = %d\n", ret,cout);
					for(int i = 0; i < cout; i++)
					{
						struct tagreport tagReport;
						
						memcpy((char *)&tagReport, tag + i * sizeof(struct tagreport), sizeof(struct tagreport));
						
						char strbuf[64] ;
						char strUserbuf[64]={0} ;
						u8_t rsUID[8];
						memcpy(rsUID,tagReport.uid ,tagReport.uidLen) ;
						BytesReverse(rsUID ,tagReport.uidLen) ;
						memset(strbuf,0,sizeof(strbuf)) ;
						
						
						BytesToHexStr(rsUID,tagReport.uidLen ,strbuf) ;
						BytesToHexStr(tagReport.user,tagReport.userlen ,strUserbuf) ;
						
						printf("BudAddr:%d UID:%s  user:%s\n",addr,strbuf,strUserbuf) ;
						
						
						
					}
				}
				else
				{
					printf("请打开串口通信\n");
				}
			}
			break;
			case 3:
			{
				if(bIsOpenComm)
				{
					RL8000_WinKillTimer();	
					RL8000_WinClosePort();
					bIsOpenComm = false;
				}

			}
			break;
			default:
			break;
		}
	}
	
#else
	RL8000_WinTimerOpen();

	int retcom = RL8000_WinOpenCom("/dev/ttyUSB0");
	
	if(retcom == -1)
	{
		printf("连接串口失败\n");
		return -1;
	}
	for(int bus=1;bus<6;bus++)
	{
		
		printf("\r\n\r\nstart inventory   %d!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n",bus);
		for(int cnt=0;cnt<10;cnt++)
		{
		
			char *tag = new char[1024];
			int  nlen = 1024;
			
			int ret = RL8000_WinTagInventory(bus, tag, nlen);
			
			if(ret!=0)
				printf("bus:%d  error!!!!!!!!!!!!!!!\r\n",bus);
			
			int cout = nlen / sizeof(struct tagreport);
			printf("----------------->ret = %d, cout = %d\n", ret,cout);
			for(int i = 0; i < cout; i++)
			{
				struct tagreport tagReport;
				
				memcpy((char *)&tagReport, tag + i * sizeof(struct tagreport), sizeof(struct tagreport));
				
				char strbuf[64] ;
				char strUserbuf[255]={0} ;
				u8_t rsUID[8];
				memcpy(rsUID,tagReport.uid ,tagReport.uidLen) ;
				BytesReverse(rsUID ,tagReport.uidLen) ;
				memset(strbuf,0,sizeof(strbuf)) ;
				
				
				BytesToHexStr(rsUID,tagReport.uidLen ,strbuf) ;
				BytesToHexStr(tagReport.user,tagReport.userlen ,strUserbuf) ;
				
				printf("BudAddr:%d:%d UID:%s  user:%s     userlen:%d   \n",bus,cnt+1,strbuf,strUserbuf,tagReport.userlen) ;
				
				
				
			}
	
		}
	}
	
	
	RL8000_WinKillTimer();	
	RL8000_WinClosePort();
	
#endif
	return 0;
}