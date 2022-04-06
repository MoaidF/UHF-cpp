#ifndef __RD5100F_COMM_H__
#define __RD5100F_COMM_H__


#include "circular_buf.h"
#include "ORFIDDevDLL.h"


#define  T1_SOF						    0xFA
#define  T1_MINI_FRAME_SIZE				6	        /* 包含SOF*/
#define  T1_MAX_INF_SIZE		       (0xff - (T1_MINI_FRAME_SIZE-1)) /* 字节长度只占一个字节*/
#define  T1_NRTX						3
#define  T1_APDU_OFFSET					4


/* T=1 protocol constants */
#define T1_I_BLOCK		0x00		
#define T1_R_BLOCK		0x80
#define T1_S_BLOCK		0xC0

#define T1_MORE_BLOCKS		0x20
#define T1_NAD(nad)  		(nad & 0x07)   
#define T1_set_NAD(sn,dn)	(((sn) & 0x07 ) | ((dn) << 4 & 0x70 ))
#define T1_NAD_get_sn(nad)  ((nad) & 0x07 ) 
#define T1_NAD_get_dn(nad)	((nad)>> 4 & 0x07 ) 
/* S block stuff */
#define T1_IS_S_BLOCK(pcb ) (((pcb) & 0xc0)== 0xc0 ?1:0 )
#define T1_S_IS_RESPONSE(pcb)	((pcb) & T1_S_RESPONSE)
#define T1_S_TYPE(pcb)		((pcb) & 0x0F)
#define T1_S_RESPONSE		0x20
#define T1_S_RESYNC		0x00
#define T1_S_IFS		0x01
#define T1_S_ABORT		0x02
#define T1_S_WTX		0x03
#define T1_S_RESYNC_PCB()  (T1_S_BLOCK | T1_S_RESYNC )
#define T1_S_RESYNC_RESP_PCB()  (T1_S_BLOCK | T1_S_RESPONSE | T1_S_RESYNC )
#define T1_S_IFS_PCB()  (T1_S_BLOCK | T1_S_IFS  )
#define T1_S_IFS_RESP_PCB() (T1_S_BLOCK | T1_S_RESPONSE | T1_S_IFS  )
#define T1_S_WTX_RESP_PCB()  (T1_S_BLOCK | T1_S_RESPONSE | T1_S_WTX)
#define T1_S_ABORT_RESP_PCB() (T1_S_BLOCK | T1_S_RESPONSE | T1_S_ABORT) 
#define T1_S_SET_CMD(pcb,cmd)   (pcb) =(pcb) | (cmd & 0x3f) 
#define T1_S_PCB()	(T1_S_BLOCK)

/* R block stuff */
#define T1_IS_R_BLOCK(pcb)   (((pcb)  & 0x80 ) == 0x80 ?1:0)
#define T1_IS_ERROR(pcb)	((pcb) & 0x0F)
#define T1_EDC_ERROR		0x01
#define T1_OTHER_ERROR		0x02
#define T1_R_SEQ_SHIFT		4
#define T1_R_SEQ			0x10
#define T1_R_GET_NR(pcb)	((pcb & 0x10) >> T1_R_SEQ_SHIFT ) 
#define T1_R_SET_NR(pcb,nr)	 (pcb) = ((nr)==0)? (pcb) & (~ T1_R_SEQ)  : (((pcb) | ( 0x01 << T1_R_SEQ_SHIFT & T1_R_SEQ )) ) 
#define T1_R_GET_ERR(pcb)	(pcb & 0x0f)
#define T1_R_SET_ERR(pcb,err)  (pcb) = (pcb) | (err & 0x0f) 
#define T1_R_PCB()	(T1_R_BLOCK)
/* I block stuff */
#define T1_IS_I_BLOCK(pcb)  (((pcb) & 0x80) == 0? 1: 0 )
#define T1_I_NS				0x40
#define T1_I_CHAIN			0x20 
#define T1_I_PCB()	(T1_I_BLOCK )
#define T1_I_SET_NS(pcb,ns)   pcb = (ns==0)?  pcb & (~(ns << 6 & 0x40) ) : pcb | (ns << 6 & 0x40)
#define T1_I_SET_CHAIN(pcb)  pcb = pcb | T1_I_CHAIN
#define T1_I_IS_CHAIN(pcb)  (pcb & T1_I_CHAIN)
#define T1_I_GET_NS(pcb)	( (pcb & T1_I_NS ) >> 6) 
#define T1_I_INC_SN(sn)  (sn)= (sn) ^ 0x01 

/* Prolog fields index */
#define PFI_NAD 0
#define PFI_PCB 1
#define PFI_LEN 2




/* RD5100F commands */
#define RD5100F_CMD_READSYSCFG				0x01			//Read configuration block 
#define RD5100F_CMD_WRITESYSCFG				0x02			//Write configuration block
#define RD5100F_CMD_LOADFACTORYDEF			0x03			//Load factory default 
#define RD5100F_CMD_GETDEVINFO				0x04			//Get device information
#define RD5100F_CMD_RESET					0x05			//Reset
#define RD5100F_CMD_SETOUTPUT				0x06			//Set output port 
#define RD5100F_CMD_GETINPUT					0x07			//Get input port
#define RD5100F_CMD_RFSWITCH					0x08			//RF Switch
#define RD5100F_CMD_GETSTA					0x09			//Getdevice status
#define RD5100F_CMD_RFRESET					0x0A			//RF reset
#define RD5100F_CMD_GETRANDOM				0x0B			//Get random
#define RD5100F_CMD_DOWNLOADKEY				0x0C			//Download　key
#define RD5100F_CMD_UPDATESERIAL				0x0D			//Update serial number

// RFID command 
#define RD5100F_CMD_TAGINVENTORY				0x31			//Tag inventory
#define RD5100F_CMD_TAGCONNECT				0x32			//Tag connect
#define RD5100F_CMD_TAGDISCONN				0x33			//Tag disconnect
#define RD5100F_CMD_TAGACCESS				0x34			//Tag access
#define RD5100F_CMD_GETTAGREPORT				0x35            //Get left buffer tag reports after inventory command
#define RD5100F_CMD_ISO14443p4_TRANSCEIVE	0x36			//ISO14443-4 transceive

/*
* rfid air protocol interface type
*/
#define RFID_AIP_UNKNOWN								0	
#define RFID_AIP_ISO15693								1
#define RFID_AIP_ISO14443A								2
#define RFID_AIP_ISO14443B								3
#define RFID_AIP_FELICA									4
#define RFID_APL_ICODEEPCUID_ID							5
#define RFID_APL_I18000P3M3_ID							6
#define RFID_APL_I18092MPI_ID							7
#define RFID_APL_I18092MPI_212_ID						8
#define RFID_APL_I18092MPI_424_ID						9
#define RFID_APL_I18092MPT_ID							10
#define RFID_APL_FELICA_424_ID							11
#define RFID_APL_ICODESLI_ID							12


enum
{
   RFID_ISO14443A_PICC_ULTRALIGHT_ID = 1,
   RFID_ISO14443A_PICC_MIFARE_S50 = 2,
   RFID_ISO14443A_PICC_MIFARE_S70 = 3,
   RFID_ISO14443A_PICC_DESFIRE= 4, //Desfire or desfire EV1 
   RFID_ISO14443A_PICC_NTAG21X=5,  //NTAG21x
} ;




#define ERR_OK					0
#define ERR_UNKNOWN				1
#define ERR_COMM_IO				2
#define ERR_TIMEOUT				3
#define ERR_MSGSIZE				4
#define ERR_MSGDATA				5
#define ERR_SYNCLOST			6
#define ERR_ARG					7
#define ERR_TAGACCESS			8



/*
*RD5100F communication handle 
*/
//typedef struct 
//{
//	u8_t *sndBuf ;
//	u8_t *rcvBuf ;
//	u16_t sndLen ;
//	u16_t rcvLen;
//	u16_t rcvPopLeft ;
//	u16_t rcvPopIdx;
//	u32_t timeout;
//	CIR_BUFFER cbParser ;
//	u8_t NS ;    
//	u8_t NR ;
//	u8_t DAD;
//
//	u8_t hTag ; // connected tag handle
//
//} RD5100F_COMM ;




/*
* Inventory stop trigger parameter
*/
struct InvenStopTriggerParam
{
	u8_t type ;
	u32_t timeout;
	u16_t setVal ;
} ;






/*
* stop trigger type
*/
#define ST_TYPE_Tms					0
#define ST_TYPE_N_Attempt			1
#define ST_TYPE_N_TagFound			2
#define ST_TYPE_TIMEOUT				3


typedef void (*TAG_REPORT_HANDLER)(u8_t ant,u8_t aip,u8_t tag,u8_t uidlen,u8_t uid[]);


extern "C"
{

	/*
	*reset device 
	*/
	err_t RL8000_CMD_Reset(RL8000_COMM *h ) ;

	/*
	*disconnect from tag 
	*/
	err_t RL8000_CMD_TagDisconnect(RL8000_COMM *h) ;
	
	err_t RL8000_WinTagInventory(u8_t busAddr, char *tagUid, int &nlen);
	
}

	/*
	* create RL8000 handle
	*/
	RL8000_COMM *RL8000_CommOpen(void) ;
	
	/*
	* create RL8000 handle  with 485 bus Addr
	*/
	
	RL8000_COMM *RL8000_CommOpenWithBusAddr(u8_t busAddr);
	
	
	/*
	* connect device 
	*/
	err_t RL8000_Connect(RL8000_COMM *h) ;
	
	/*
	*tag inventory 
	*/
	err_t RL8000_CMD_TagInventory(RL8000_COMM *h ,BOOLEAN continueInven,u8_t AIPSelFlag ,struct ISO15693InvenParam *pISO15693InvenParm,TAG_REPORT_HANDLER tag_report_handler,u16_t *tagGetted) ;



	
	/*
	* get device info 
	*/
	err_t RL8000_CMD_GetDeviceInfo(RL8000_COMM *h ,u8_t *devInf,u16_t *nSize) ;

#define ISO15693_ADDR_MODE_NONE			0
#define ISO15693_ADDR_MODE_ADDRESS		1
#define ISO15693_ADDR_MODE_SELECT		2

extern "C"
{
	err_t ISO15693_TagConnect(RL8000_COMM *h ,u8_t tagType,u8_t addrMode,u8_t uid[]) ;
	err_t ISO15693_Reset(RL8000_COMM *h ) ;
	err_t ISO15693_ReadSingleBlock(RL8000_COMM *h,BOOLEAN readSecByte,u16_t blkAddr,u8_t *pBlkBuf,u16_t *nSize ) ;
	err_t ISO15693_WriteSingleBlock(RL8000_COMM *h,u16_t blkAddr,u8_t *blockData,u16_t nSize ) ;
	err_t ISO15693_ReadMultipleBlocks(RL8000_COMM *h,BOOLEAN readSecByte,u16_t blkAddr,u16_t blkNum,u8_t *pBlkBuf,u16_t *nSize ) ;
	err_t ISO15693_WriteMultipleBlocks(RL8000_COMM *h,u16_t blkAddr,u16_t blkNum,u8_t *blockData,u16_t nSize ) ;
	err_t ISO15693_LockMultipleBlocks(RL8000_COMM *h,u16_t blkAddr,u16_t blkNum ) ;
	err_t ISO15693_WriteAFI(RL8000_COMM *h,u8_t AFI ) ;
	err_t ISO15693_LockAFI(RL8000_COMM *h ) ;
	err_t ISO15693_WriteDSFID(RL8000_COMM *h,u8_t DSFID ) ;
	err_t ISO15693_LockDSFID(RL8000_COMM *h ) ;
	err_t ISO15693_GetTagSysInfor(RL8000_COMM *h , u8_t *infBuf,u16_t *nSize) ;
	err_t ISO15693_GetBlockSecuritySta(RL8000_COMM *h ,u16_t blkAddr ,u16_t blkNum, u8_t *staBuf,u16_t *nSize) ;
	err_t ISO14443A_TagConnect(RL8000_COMM *h ,u8_t tagType,u8_t uid[] ,u8_t uidLen) ;
}







#define MIFARE_CLASSIC_AUTH_KEY_A					0x60
#define MIFARE_CLASSIC_AUTH_KEY_B					0x61

extern "C"
{
	err_t MifareS50_Authenticate(RL8000_COMM *h,u8_t blkAddr ,u8_t authType,u8_t keyIdx,u8_t *pKey) ;
	err_t MifareS50_Read(RL8000_COMM *h,u8_t blkAddr ,u8_t blkNum ,BOOLEAN authBeforeRead ,u8_t authType,u8_t keyIdx,u8_t *pKey ,u8_t *blkBuf,u16_t *nSize) ;
	err_t MifareS50_Write(RL8000_COMM *h,u8_t blkAddr ,u8_t blkNum  ,u8_t *blkData, BOOLEAN authBeforeWrite ,u8_t authType,u8_t keyIdx,u8_t *pKey) ;
	err_t MifareS50_Increment(RL8000_COMM *h,u8_t blkAddr ,u32_t increValue,BOOLEAN autoTransfer) ;
	err_t MifareS50_Decrement(RL8000_COMM *h,u8_t blkAddr ,u32_t decreValue,BOOLEAN autoTransfer) ;
	err_t MifareS50_Restore(RL8000_COMM *h,u8_t blkAddr) ;
	err_t MifareS50_Transfer(RL8000_COMM *h,u8_t blkAddr) ;
	err_t MFUltralight_Authenticate(RL8000_COMM *h,u8_t keyIdx ,u8_t *pKey) ;
	err_t MFUltralight_ReadPages(RL8000_COMM *h,u16_t startPage ,u16_t pagesToRead,u8_t *blkBuf,u16_t *nSize) ;
	err_t MFUltralight_WritePages(RL8000_COMM *h,u16_t startPage,u16_t pagesToWrite,u8_t *pPageData) ;
	err_t MFUltralight_UpdatePassword(RL8000_COMM *h,u8_t *newPwd) ;
	err_t MFUltralight_UpdateAuthCfg(RL8000_COMM *h,u8_t cfg0,u8_t cfg1) ;


	err_t ISO14443p4_Transceive(RL8000_COMM *h ,u32_t apduReqLen ,u8_t *apduReq ,u32_t *apduRespLen ,u8_t *apduResp   ) ;
}








#endif

