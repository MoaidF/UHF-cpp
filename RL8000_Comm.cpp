
#include "platformOpts.h"
#include "rl8000_comm.h"
#include "tool.h"
#include "rfid_tag_cmd_coding.h"
#include"OIniFile.h"



#include "platformMem.h"
#include "platformUart.h"
#include "platformTimer.h"

#include <stdio.h>


#include <sys/time.h>

#pragma pack (1)


#define MEMP_TYPE_RL8000					2

/* 
* frame header except SOF
*/

struct frameHeader
{
	PACK_STRUCT_FIELD(u8_t sof) ;     //start byte of frame
	PACK_STRUCT_FIELD(u8_t len );     //length of the frame except SOF
	PACK_STRUCT_FIELD(u8_t rid );     //node address ,0xff is broadcast address
	PACK_STRUCT_FIELD(u8_t pcb );     //protocol control byte
}PACK_STRUCT_STRUCT;
#define SIZEOF_FRAMEHEADER()			sizeof(struct frameHeader) 


/*
* I-block respond header 
*/

struct IBlockRespHeader 
{
	PACK_STRUCT_FIELD(u8_t cmd) ;
	PACK_STRUCT_FIELD(u16_t sta);
}PACK_STRUCT_STRUCT;

#define SIZEOF_IBLOCKRESPHEADER() sizeof(struct IBlockRespHeader )
/*
* get device information respond 
*/

struct respDevInf
{
	PACK_STRUCT_FIELD(u8_t prodID[3]) ;   //product type
	PACK_STRUCT_FIELD(u8_t firmVer[2]) ;  //firmware version
	PACK_STRUCT_FIELD(u8_t serial[5]) ;   //serial number
	PACK_STRUCT_FIELD(u8_t hardVer[2]) ;  //hardware version
}PACK_STRUCT_STRUCT;


#define SIZEOF_RESPDEVINF()				sizeof(struct respDevInf)



/*
*inventory respond header
*/

struct respInventoryHeader
{
	PACK_STRUCT_FIELD(u8_t stopTriggerType) ;
	PACK_STRUCT_FIELD(u16_t totalNum) ;
	PACK_STRUCT_FIELD(u8_t transferNum) ;

}PACK_STRUCT_STRUCT;


#define SIZEOF_RESPINVENTORYHEADER()			sizeof(struct respInventoryHeader)



/*
*get tag buffer respond header
*/

struct respGetTagBufferHeader
{
	PACK_STRUCT_FIELD(u16_t leftNum) ;
	PACK_STRUCT_FIELD(u8_t transferNum) ;

}PACK_STRUCT_STRUCT;


#define SIZEOF_RESPGETTAGBUFFERHEADER()			sizeof(struct respGetTagBufferHeader)


/*
* tag access respond command result header
*/

struct respTagAccessCmdHeader
{
	PACK_STRUCT_FIELD(u16_t cmd) ;  //access code
	PACK_STRUCT_FIELD(u8_t sta) ;   //access status ,0 is ok ,not 0 is fail

}PACK_STRUCT_STRUCT;


#define SIZEOF_RESPTAGACCESSCMDHEADER()			sizeof(struct respTagAccessCmdHeader)







#ifdef WIN32
#pragma pack (0)
#endif





static void  CBCS_MOVE_NEXT(CIR_BUFFER *pcb)
{
	pcb->cursor++ ; 
	if(pcb->cursor >=(pcb)->size) 
		pcb->cursor =0 ;
}
static BOOLEAN  CBCS_IS_END(CIR_BUFFER *pcb)
{
		if(pcb->cursor == pcb->writePos) 
			return TRUE;
	  else 
			return FALSE ;
}
static int CBCS_GET_LEN(CIR_BUFFER *pcb)
{
    int left ;
	  left =0 ;
    if ( (pcb->cursor <=pcb->writePos ))
    {
       left = ( pcb->writePos ) - pcb->cursor;
    }
	  else 
    {
       left = (pcb->size -pcb->cursor) + pcb->writePos ;
    }

   return left;
	
}
static int CBCS_READ(CIR_BUFFER *pcb ,u8_t *pMEM,int bytesToRead)
{
	int bc;
	int i ;

	i =0 ;
	bc =pcb->cursor;
	while(bytesToRead >0 && (pcb->cursor != pcb->writePos )) {
			pMEM[i]= pcb->Buffer[pcb->cursor] ;
		  i++ ;
		  bytesToRead -- ;
		  CBCS_MOVE_NEXT(pcb) ;
	}
	pcb->cursor = bc;
	return  i;
}


static BOOLEAN ParseIncomeData(RL8000_COMM *h )
{
	int bcs ;
	int implePos;
	int newPos;
	u8_t sof ,frmlen;  
	u16_t cksum1,cksum2;
	CIR_BUFFER cb ;
	u8_t miniFrmLen;
	BOOLEAN blRet;
	
	cb.readPos  = h->cbParser.readPos  ;
	cb.writePos = h->cbParser.writePos ;
	cb.Buffer = h->cbParser.Buffer ;
	cb.size =h->cbParser.size ;

	
  if(cb.readPos == cb.writePos) {
			return FALSE ;
	}
	blRet =FALSE ;
    cb.cursor = cb.readPos ;
	implePos =-1;
	while(cb.cursor != cb.writePos) {
		bcs =cb.cursor  ;
		sof = cb.Buffer[cb.cursor] ;
		if(( sof!= T1_SOF)) {
			 goto label_next_byte;
		}
		//printf("\r\nT1_SOF ok\r\n");
		
		CBCS_MOVE_NEXT(&cb) ;
		if(CBCS_IS_END(&cb)) {  
			  if(implePos == -1) implePos = bcs ;
			  break;
		}
		frmlen = cb.Buffer[cb.cursor] & 0xff  ;
		miniFrmLen =T1_MINI_FRAME_SIZE ;
		
		//printf("\r\frmlen %d ok\r\n",frmlen);
		
		if(frmlen < (miniFrmLen-1)) {
			 continue;
		} 
		if(frmlen > CBCS_GET_LEN(&cb)){
			  if(implePos ==-1) implePos = bcs ;
			  continue;
		}
		
		h->rcvBuf[0] = sof ;
		CBCS_READ(&cb,h->rcvBuf+1,frmlen ) ;
		cksum1 = cal_crc16_ext(0xffff,h->rcvBuf+1,frmlen-2) ;
		cksum2 = (h->rcvBuf[frmlen+1-2] & 0xff ) | (h->rcvBuf[frmlen+1-1] << 8 & 0xff00) ; 
		
	
		if(cksum1 != cksum2) {
			//printf("\r\ncrc error\r\n");
			  continue ;
		}
		//printf("\r\ncrc ok\r\n");
		
		
		h->rcvLen = frmlen +1 -2 ; //delete 2 bytes checksum
		blRet =TRUE ;
		newPos = cb.cursor  + frmlen ;
		if(newPos >=cb.size) {
			//memset(pcb.Buffer +pcb.readPos,0x00  ,(pcb.size -pcb.readPos  )	) ;
			//memset(pcb.Buffer ,0x00  ,(newPos - pcb.size )	) ;
			h->cbParser.readPos=newPos -cb.size ;  
		} 
		else {
			//memset(pcb.Buffer +pcb.readPos,0x00  ,(newPos -pcb.readPos  )	) ;
			h->cbParser.readPos=h->cbParser.writePos ; //newPos;   
		}	
		
		
		
		goto exit_func;
		
label_next_byte:
		CBCS_MOVE_NEXT(&cb) ;
	}
	if(implePos==-1) {

		h->cbParser.readPos=cb.writePos;
	}
	else {
		h->cbParser.readPos=implePos;
	}
exit_func:
	return blRet ;
}







RL8000_COMM *RL8000_CommOpen(void)
{
	u8_t *p ;
	RL8000_COMM * h ;
	//h =(RL8000_COMM *)SYSMEM_MALLOC(MEM_TYPE_RL8000,sizeof(RL8000_COMM)) ;
	h =(RL8000_COMM *)malloc(sizeof(RL8000_COMM)) ;
	if(!h) {
		return NULL ;
	}
	memset(h,0,sizeof(RL8000_COMM)) ;

	h->sndBuf =(u8_t *)malloc(256) ;
	if(!h->sndBuf) {
		SYSMEM_FREE(0,h) ;
		return NULL ;
	}
	h->rcvBuf = (u8_t *)malloc(256) ;
	if(!h->rcvBuf) {
		SYSMEM_FREE(0,h->sndBuf ) ;
		SYSMEM_FREE(0,h) ;
		return NULL ;
	}
	h->NR =0 ;
	h->NS =0 ;
	h->DAD =0xff ;
	p= (u8_t *)malloc(256) ;
	cirbuf_init(&h->cbParser ,256,p) ;
	return h ;
}


RL8000_COMM *RL8000_CommOpenWithBusAddr(u8_t busAddr)
{
	u8_t *p ;
	RL8000_COMM * h ;
	//h =(RL8000_COMM *)SYSMEM_MALLOC(MEM_TYPE_RL8000,sizeof(RL8000_COMM)) ;
	h =(RL8000_COMM *)malloc(sizeof(RL8000_COMM)) ;
	if(!h) {
		return NULL ;
	}
	memset(h,0,sizeof(RL8000_COMM)) ;

	h->sndBuf =(u8_t *)malloc(256) ;
	if(!h->sndBuf) {
		SYSMEM_FREE(0,h) ;
		return NULL ;
	}
	h->rcvBuf = (u8_t *)malloc(256) ;
	if(!h->rcvBuf) {
		SYSMEM_FREE(0,h->sndBuf ) ;
		SYSMEM_FREE(0,h) ;
		return NULL ;
	}
	h->NR =0 ;
	h->NS =0 ;
	h->DAD =busAddr ;
	p= (u8_t *)malloc(256) ;
	cirbuf_init(&h->cbParser ,256,p) ;
	return h ;
}



#define INIT_SEND(h) {h->sndLen =0;cirbuf_flush((&h->cbParser) ); UART_PURGECOMM(); }
#define PUSH_SEND_BYTE(h,val)  {h->sndBuf[h->sndLen]=val ;h->sndLen++ ;}
#define PUSH_SEND_WORD(h,val)  {h->sndBuf[h->sndLen]=val & 0xff ;h->sndBuf[h->sndLen+1]=val >> 8 & 0xff;h->sndLen +=2 ;} 
#define PUSH_SEND_DWORD(h,val) {h->sndBuf[h->sndLen]=val & 0xff ;h->sndBuf[h->sndLen+1]=val >> 8 & 0xff;h->sndBuf[h->sndLen+2]=val >> 16 & 0xff;h->sndBuf[h->sndLen+3]=val >> 24 & 0xff;h->sndLen +=4 ;}
#define PUSH_SEND_PTR(h,buf,nSize)  {memcpy(h->sndBuf + h->sndLen ,buf,nSize ) ; h->sndLen+=nSize ;}
___INLINE void PUSH_SEND_EBV_LENGTH(RL8000_COMM *h,u32_t val)
{
	int n ;
	n=EBV_Set(val,h->sndBuf+h->sndLen) ;
	h->sndLen+=n ;
}
#define OFFSET_HEADER(h) h->sndLen  = SIZEOF_FRAMEHEADER() 

#define INIT_RCV_POP(h) {h->rcvPopLeft =h->rcvLen;h->rcvPopIdx =0 ;} 
#define GET_POP_CUR(h)  (h->rcvBuf + h->rcvPopIdx)       //get pointer of current popped position
___INLINE int  RCV_POP_PTR(RL8000_COMM *h,u8_t *buf ,u16_t nPop)
{
	if(nPop >h->rcvPopLeft) nPop = h->rcvPopLeft ;
	if(buf)memcpy(buf,h->rcvBuf + h->rcvPopIdx,nPop) ;
	h->rcvPopIdx +=nPop ;
	h->rcvPopLeft -=nPop ;
	return nPop ;
}
___INLINE BOOLEAN RCV_POP_EBV_LENGTH(RL8000_COMM *h,u32_t *pLen)
{
	u32_t val ;
	int nb;
	nb = EBV_Get(GET_POP_CUR(h),h->rcvPopLeft,&val)  ;
	if(nb ==0) {
		return FALSE ;
	}
	h->rcvPopIdx +=nb ;
	h->rcvPopLeft -=nb ;
	if(pLen) *pLen =val ;
	return TRUE ;
}



extern "C" err_t CommTransceive(RL8000_COMM *h,u8_t pcb)
{
	err_t iret;
	u16_t nRW ;
	u16_t chksum;
	u8_t tmpbuf[32] ;
	BOOLEAN isFree;
	

	/* set header */
	h->sndBuf[0] = T1_SOF ;
	h->sndBuf[1] = h->sndLen-1 + 2 ;
	h->sndBuf[2] = h->DAD ;
	h->sndBuf[3] =pcb;

	/* push checksum */
	chksum =cal_crc16_ext(0xffff,h->sndBuf +1,h->sndLen-1 ) ;
	PUSH_SEND_BYTE(h,chksum & 0xff );
	PUSH_SEND_BYTE(h,chksum >> 8 & 0xff);

	


	nRW = UART_WRITE(h->sndBuf,h->sndLen) ;//
	if(nRW != h->sndLen ) {
		iret = -ERR_COMM_IO;
		goto exit_fail ;
	}
	TIMER_TRANSC_START(h->timeout);
	while(1) {
		isFree = TIMER_TRANSC_IS_FREE() ;
		if(isFree) {
			iret =-ERR_TIMEOUT ;
			goto exit_fail;
		}
		nRW= UART_READ(tmpbuf,sizeof(tmpbuf)) ;//shou
		if(nRW){
			cirbuf_push(&h->cbParser,tmpbuf,nRW) ;
		}
		if(ParseIncomeData(h)) {
			/* new frame  income */
			
			
		
			
			iret = ERR_OK ;
			break;
		}
		
	}
exit_fail:
	return iret;
}

/*
* transceive i-block  ;
*  parameter respLen: IN:buffer size ,OUT: bytes written .
*/
 err_t IBlock_Transceive(RL8000_COMM * h,u8_t cmd)
{


	err_t iret;
	u8_t pcb;
	struct frameHeader *pHdr;
	struct IBlockRespHeader  *pIBRespHdr;
	u16_t ncopy;

	pcb = T1_I_PCB(); 
	T1_I_SET_NS(pcb,h->NS) ; /* set send number */
	//if(chaining ){
	//	T1_I_SET_CHAIN(pcb) ;
	//}

	h->timeout =1000 ;
	h->rcvLen =0 ;
	iret = CommTransceive(h,pcb) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	T1_I_INC_SN(h->NS) ;  //next send number
	INIT_RCV_POP(h) ;
	if(h->rcvPopLeft < SIZEOF_FRAMEHEADER()) {
		iret = -ERR_MSGSIZE ;
		goto exit_fail ;
	}
	pHdr =(struct frameHeader *)GET_POP_CUR(h) ;

	if(T1_IS_I_BLOCK(pHdr->pcb)==FALSE )
	{
		if(T1_IS_S_BLOCK(pHdr->pcb) )
		{
			/* FIXME:process s-block income ,Maybe it is WXT */
			iret=-ERR_MSGDATA ;
			goto exit_fail; 

		}
		else if(T1_IS_R_BLOCK(pHdr->pcb))
		{
			/* FIXME:process r-block income */
			iret=-ERR_MSGDATA ;
			goto exit_fail; 
		}
		else {
			iret=-ERR_MSGDATA ;
			goto exit_fail; 
		}
	}
	/* process i-block income */
	if(h->NR != T1_I_GET_NS(pHdr->pcb)) {
		/* FIXME: protocol synchronize lost,call error handler */
		iret =-ERR_SYNCLOST ;
		goto exit_fail ;
	}
	/* Set next receive number */
	h->NR ^= 1  ;
	RCV_POP_PTR(h,NULL,SIZEOF_FRAMEHEADER()) ;
	pIBRespHdr =(struct IBlockRespHeader *)GET_POP_CUR(h) ;
	if(h->rcvPopLeft < SIZEOF_IBLOCKRESPHEADER() ) {
		iret = -ERR_MSGSIZE ;
		goto exit_fail ;
	}
	if(pIBRespHdr->cmd != cmd) {
		iret =-ERR_MSGDATA ;
		goto exit_fail ;
	}
	if(pIBRespHdr->sta != 0) {
		iret =pIBRespHdr->sta ;
		goto exit_fail ;
	}
	RCV_POP_PTR(h,NULL,SIZEOF_IBLOCKRESPHEADER()) ;
	iret = 0 ;

exit_fail:
	return iret;
}


err_t SBlock_Resync(RL8000_COMM * h)
{
	u8_t pcb;
	
	err_t iret;
	struct frameHeader *pHdr;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	pcb = T1_S_PCB() ;
	T1_S_SET_CMD(pcb,T1_S_RESYNC) ;

	h->timeout =3000 ;
	h->rcvLen =0 ;
	iret = CommTransceive(h ,pcb) ;
	if(iret != 0) {
		goto exit_fail;
	}
	INIT_RCV_POP(h) ;
	if(h->rcvPopLeft < SIZEOF_FRAMEHEADER()) {
		iret = -ERR_MSGSIZE ;
		goto exit_fail ;
	}
	pHdr =(struct frameHeader *)GET_POP_CUR(h) ;
	if(!T1_S_IS_RESPONSE(pHdr->pcb)){
		iret =-ERR_MSGDATA ;
		goto exit_fail ;
	}
	if(T1_S_TYPE(pHdr->pcb)!=T1_S_RESYNC) {
		iret =-ERR_MSGDATA ;
		goto exit_fail ;
	}
	h->NR =0 ;
	h->NS =0 ;
	iret =ERR_OK;
exit_fail:
	return iret ;
}

err_t RL8000_Connect(RL8000_COMM *h)
{
	return SBlock_Resync(h) ;
}





/*********************************  control commands **********************************/


extern "C" err_t RL8000_CMD_ReadCfgBlock()
{
	return -1;
}


extern "C" err_t RL8000_CMD_WriteCfgBlock() 
{
	return -1;
}


extern "C" err_t RL8000_CMD_LoadFactoryDefault()
{
	return -1;
}



#define RL8000_CMD_GETDEVINFO 0x04
#define RL8000_CMD_RESET      0x05
#define	RL8000_CMD_RFSWITCH	 0x06  
#define RL8000_CMD_GETTAGREPORT 0x35
#define RL8000_CMD_TAGINVENTORY   0x31
#define RL8000_CMD_TAGCONNECT     0x32
#define	RL8000_CMD_TAGACCESS	0x34
#define RL8000_CMD_RFRESET     0x0a
#define RL8000_CMD_TAGDISCONN  0x33
#define RL8000_CMD_ISO14443p4_TRANSCEIVE  0x36


/*
* get device information parameters:
* h: RL8000 handle ,created by RL8000_CommOpen
* devInf: bytes buffer to store device infor
* nSize: Input size of the buffer devInf, Output bytes written into buffer devInf
*/
err_t RL8000_CMD_GetDeviceInfo(RL8000_COMM *h ,u8_t *devInf,u16_t *nSize)
{
	err_t iret;
	struct respDevInf *pDevInf;
	int ncopy ;
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_GETDEVINFO) ;
	iret = IBlock_Transceive(h,RL8000_CMD_GETDEVINFO) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	pDevInf=(struct respDevInf *)GET_POP_CUR(h) ;
	if(h->rcvPopLeft < SIZEOF_RESPDEVINF() ) {
		iret = -ERR_MSGSIZE ;
		goto exit_fail ;
	}
	ncopy = SIZEOF_RESPDEVINF() ;
	if(ncopy > *nSize) ncopy = *nSize ;
	memcpy(devInf ,pDevInf,ncopy) ;
	*nSize =ncopy ;
	iret = 0;
exit_fail:
	return iret;
}

extern "C" err_t RL8000_CMD_Reset(RL8000_COMM *h )
{
	err_t iret;

	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_RESET) ;
	PUSH_SEND_BYTE(h,0x00) ;
	iret = IBlock_Transceive(h,RL8000_CMD_RESET) ;
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;
}

err_t RL8000_CMD_SetGPO()
{
	return -1;
}


/*
* RF switch parameters:
* h: RL8000 handle ,created by RL8000_CommOpen
* on: TRUE-switch on ,FALSE-switch off
*/
extern "C" err_t RL8000_CMD_RFSwitch(RL8000_COMM *h,BOOLEAN on)
{
	err_t iret;
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_RFSWITCH) ;
	if(on) {
		PUSH_SEND_BYTE(h,0x01) ;
	}
	else {
		PUSH_SEND_BYTE(h,0x00) ;
	}
	iret = IBlock_Transceive(h,RL8000_CMD_RFSWITCH) ;
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;

}


extern "C" err_t RL8000_CMD_RFReset(RL8000_COMM *h)
{
	err_t iret;
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_RFRESET) ;
	iret = IBlock_Transceive(h,RL8000_CMD_RFRESET) ;
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;
}


extern "C" err_t RL8000_CMD_GetRandomNumber()
{
	return -1;
}

extern "C" err_t RL8000_CMD_DownloadKey()
{
	return -1;
}



/**************************** RFID Tag commands ********************************************/
/*
* inventory flag bits
*/
#define INVEN_FLAG_BIT_ANT_PRESENT				0x01					//antenna field present or not  ,RL8000 is single antenna ,so this bit should not be set
#define INVEN_FLAG_BIT_AIP_PRESENT				0x02					//air protocol interface field or not
#define INVEN_FLAG_BIT_STOPTRIGGER_PRESENT		0x04					//stop trigger field or not
#define INVEN_FLAG_BIT_CONTINUEINVEN			0x08					//continue inventory indicator bit 





/*
* inventory report field flag
*/
#define INVEN_REPORT_FLAG_BIT_ANT					0x01				//Antenna field is present or not
#define INVEN_REPORT_FLAG_BIT_AIP					0x02				//Air protocol interface type ID is present or not
#define INVEN_REPORT_FLAG_BIT_TAG					0x04				//Tag type ID is present or not
#define INVEN_REPORT_FLAG_BIT_UID					0x08				//Tag serial number is present or not




extern "C" err_t RL8000_CMD_GetTagBuffer(RL8000_COMM *h,u16_t *left,u8_t *getted,TAG_REPORT_HANDLER tag_report_handler)
{
	err_t iret;
	u8_t repFlag,repAnt,repAIP,repTAG,repUIDLen;
	u8_t uid[12] ;
	int i ;
	struct respGetTagBufferHeader hdr;
	
	
		
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_GETTAGREPORT) ;
	PUSH_SEND_BYTE(h,0x01) ;

	iret = IBlock_Transceive(h,RL8000_CMD_GETTAGREPORT) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	if(h->rcvPopLeft < SIZEOF_RESPGETTAGBUFFERHEADER()) {
		iret =-ERR_MSGSIZE ;
		goto exit_fail ;
	}
	RCV_POP_PTR(h,((u8_t *)&hdr),SIZEOF_RESPGETTAGBUFFERHEADER());
	/* start parse transferred tag reports */
	for(i=0;i<hdr.transferNum;i++ ) {
		if(h->rcvPopLeft<=0) break;
		RCV_POP_PTR(h,&repFlag,1) ;
		if((repFlag &INVEN_REPORT_FLAG_BIT_ANT) >0) {
			if(h->rcvPopLeft<=0) break;
			RCV_POP_PTR(h,&repAnt,1) ;
		}
		if((repFlag & INVEN_REPORT_FLAG_BIT_AIP)>0) {
			if(h->rcvPopLeft<=0) break;
			RCV_POP_PTR(h,&repAIP,1) ;
		}
		if((repFlag & INVEN_REPORT_FLAG_BIT_TAG)> 0) {
			if(h->rcvPopLeft<=0) break;
			RCV_POP_PTR(h,&repTAG,1) ;
		}
		if((repFlag & INVEN_REPORT_FLAG_BIT_UID) >0) {
			if(h->rcvPopLeft<=0) break;
			RCV_POP_PTR(h,&repUIDLen,1) ;
			if(h->rcvPopLeft < repUIDLen) break;
			if(repUIDLen > sizeof(uid)) repUIDLen =sizeof(uid) ;
			RCV_POP_PTR(h,uid,repUIDLen) ;
		}
		tag_report_handler(repAnt ,repAIP ,repTAG ,repUIDLen , uid  );
	}
	if(i != hdr.transferNum) {
		iret=-ERR_MSGSIZE ;
		goto exit_fail ;
	}
	if(getted) *getted =hdr.transferNum ;
	if(left) *left =hdr.leftNum ;

	iret = 0;
exit_fail:
	return iret;
}


err_t RL8000_CMD_TagInventory(RL8000_COMM *h ,BOOLEAN continueInven,u8_t AIPSelFlag ,struct ISO15693InvenParam *pISO15693InvenParm,TAG_REPORT_HANDLER tag_report_handler,u16_t *tagGetted)
{
	err_t iret;
	u8_t flag;
	u8_t AIPCnt;
	u8_t repFlag,repAnt,repAIP,repTAG,repUIDLen;
	u8_t uid[12] ;
	u8_t tmpGet;
	u16_t left;
	u16_t totalGetted;
	int idx;
	int i ;
	struct respInventoryHeader invenHdr;
	
	
	totalGetted =0 ;
	if(AIPSelFlag == 0) {
		/* please tell which air protocol interface to select for inventory */
		return -ERR_ARG ;
	}
		
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_TAGINVENTORY) ;
	flag =INVEN_FLAG_BIT_AIP_PRESENT ;
	if(continueInven) {
		flag |=INVEN_FLAG_BIT_CONTINUEINVEN ;	
	}
	PUSH_SEND_BYTE(h,flag) ;
	/*
	*  air protocol interface field
	*/
	AIPCnt =0 ;
	if((AIPSelFlag & AIP_SEL_ISO15693)>0) {
		AIPCnt++ ;
	}
	if((AIPSelFlag & AIP_SEL_ISO14443A)>0) {
		AIPCnt++ ;
	}
	PUSH_SEND_BYTE(h,AIPCnt) ;
	if((AIPSelFlag & AIP_SEL_ISO15693) >0) {
		/* Add ISO15693 air protocol interface inventory parameter*/
		PUSH_SEND_BYTE(h,RFID_AIP_ISO15693) ;
		PUSH_SEND_BYTE(h,0x00) ;//Antenna 
		PUSH_SEND_EBV_LENGTH(h,2)  ;  //Param length
		PUSH_SEND_BYTE(h,pISO15693InvenParm->AFI) ;
		PUSH_SEND_BYTE(h,0x01) ;//slot number
	}
	if((AIPSelFlag & AIP_SEL_ISO14443A) >0) {
		/* Add ISO14443a air protocol interface inventory parameter */
		PUSH_SEND_BYTE(h,RFID_AIP_ISO14443A) ;
		PUSH_SEND_BYTE(h,0x00) ;//Antenna 
		PUSH_SEND_EBV_LENGTH(h,0x00)  ;  //Param is empty
	}
	/*
	* stop trigger
	*/
	//if((flag & INVEN_FLAG_BIT_STOPTRIGGER_PRESENT) >0) {
	//	PUSH_SEND_BYTE(h,pInvenStopTriggerParm->type) ;
	//	PUSH_SEND_DWORD(h,pInvenStopTriggerParm->timeout) ;
	//	PUSH_SEND_WORD(h,pInvenStopTriggerParm->setVal) ;
	//}

	iret = IBlock_Transceive(h,RL8000_CMD_TAGINVENTORY) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	if(h->rcvPopLeft < SIZEOF_RESPINVENTORYHEADER()) {
		iret =-ERR_MSGSIZE ;
		goto exit_fail ;
	}
	RCV_POP_PTR(h,((u8_t *)&invenHdr),SIZEOF_RESPINVENTORYHEADER());
	/* start parse transferred tag reports */
	for(i=0;i<invenHdr.transferNum;i++ ) {
		if(h->rcvPopLeft<=0) break;
		RCV_POP_PTR(h,&repFlag,1) ;
		if((repFlag &INVEN_REPORT_FLAG_BIT_ANT) >0) {
			if(h->rcvPopLeft<=0) break;
			RCV_POP_PTR(h,&repAnt,1) ;
		}
		if((repFlag & INVEN_REPORT_FLAG_BIT_AIP)>0) {
			if(h->rcvPopLeft<=0) break;
			RCV_POP_PTR(h,&repAIP,1) ;
		}
		if((repFlag & INVEN_REPORT_FLAG_BIT_TAG)> 0) {
			if(h->rcvPopLeft<=0) break;
			RCV_POP_PTR(h,&repTAG,1) ;
		}
		if((repFlag & INVEN_REPORT_FLAG_BIT_UID) >0) {
			if(h->rcvPopLeft<=0) break;
			RCV_POP_PTR(h,&repUIDLen,1) ;
			if(h->rcvPopLeft < repUIDLen) break;
			if(repUIDLen > sizeof(uid)) repUIDLen =sizeof(uid) ;
			RCV_POP_PTR(h,uid,repUIDLen) ;
		}
		tag_report_handler(repAnt ,repAIP ,repTAG ,repUIDLen , uid  );
	}
	if(i != invenHdr.transferNum) {
		iret=-ERR_MSGSIZE ;
		goto exit_fail ;
	}
	totalGetted =invenHdr.transferNum ;
	if(invenHdr.totalNum > invenHdr.transferNum) {
		/* more tag report still keeping in the buffer */
		do {
			left = tmpGet =0 ;
			iret =RL8000_CMD_GetTagBuffer(h,&left ,&tmpGet,tag_report_handler) ;
			if(iret != 0) {
				goto exit_fail ;
			}
			totalGetted +=tmpGet ;
		}while(left > 0) ;

	}
	if(tagGetted) *tagGetted= totalGetted ;
	iret = 0;
exit_fail:
	return iret;
}




extern "C" err_t RL8000_CMD_TagDisconnect(RL8000_COMM *h)
{
	err_t iret;
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_TAGDISCONN) ;
	PUSH_SEND_BYTE(h,h->hTag) ;
	iret = IBlock_Transceive(h,RL8000_CMD_TAGDISCONN) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	h->hTag =0 ;
	iret = 0;
exit_fail:
	return iret;
}





/****************** iso15693 tag access API ******************************************/
extern "C" err_t ISO15693_TagConnect(RL8000_COMM *h ,u8_t tagType,u8_t addrMode,u8_t uid[])
{
	err_t iret;
	u8_t respTagHandle;
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_TAGCONNECT) ;
	/* connect header */
	PUSH_SEND_BYTE(h,0x00) ; //Antenna
	PUSH_SEND_BYTE(h,RFID_AIP_ISO15693);
	PUSH_SEND_BYTE(h,tagType);
	PUSH_SEND_BYTE(h,9) ;
	/* iso15693 connect parameter */
	PUSH_SEND_BYTE(h,addrMode) ; 
	PUSH_SEND_PTR(h,uid,8) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGCONNECT) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	if(h->rcvPopLeft <  1) {
		iret = -ERR_MSGSIZE ;
		goto exit_fail ;
	}
	RCV_POP_PTR(h,&respTagHandle,1) ;
	h->hTag = respTagHandle ;
	iret = 0;
exit_fail:
	return iret;
}




#define PUSH_SEND_TAG_ACCESS_HEAD(h,cmd ,parmLen)   \
{  \
PUSH_SEND_BYTE(h,RL8000_CMD_TAGACCESS) ;  \
PUSH_SEND_BYTE(h,h->hTag) ; \
PUSH_SEND_EBV_LENGTH(h,2 + parmLen) ;   \
PUSH_SEND_WORD(h,cmd) ;  \
}

___INLINE err_t PARSE_TAG_ACCESS_RESULT_HEAD(RL8000_COMM *h,u16_t accCode )
{
	u32_t ebvLen;
	err_t iret;
	struct respTagAccessCmdHeader hdr;

	if(RCV_POP_EBV_LENGTH(h,&ebvLen)==FALSE) {
		iret =-ERR_MSGSIZE;
		goto exit_fail ;
	}
	if((ebvLen < SIZEOF_RESPTAGACCESSCMDHEADER() ) || ( h->rcvPopLeft < ebvLen)) {
		iret =-ERR_MSGSIZE;
		goto exit_fail ;
	}
	RCV_POP_PTR(h,(u8_t *)&hdr,SIZEOF_RESPTAGACCESSCMDHEADER()) ;
	if(hdr.cmd != accCode) {
		iret =-ERR_MSGDATA ;
		goto exit_fail ;
	}
	/*
	* sta =1 means access ok 
	* sta =0 means access fail
	*/
	if(hdr.sta == 0) {
		/* access tag fail */
		iret = -ERR_TAGACCESS ;
		goto exit_fail ;
	}
	iret =0 ;
exit_fail:
	return iret;
}

extern "C" err_t ISO15693_Reset(RL8000_COMM *h )
{
	err_t iret;

	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_RESET,0) ;
	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_RESET);

exit_fail:
	return iret;
}



/*
*read single block function:
*nSize: input->size of the buffer 'pBlkBuf' ,output->bytes written into buffer 'pBlkBuf'
*/
extern "C" err_t ISO15693_ReadSingleBlock(RL8000_COMM *h,BOOLEAN readSecByte,u16_t blkAddr,u8_t *pBlkBuf,u16_t *nSize )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_READ_SINGLE_BLK,3) ;
	/* access parameters */
	if(readSecByte) {
		PUSH_SEND_BYTE(h,0x01) ;
	}
	else {
		PUSH_SEND_BYTE(h,0x00) ;
	}
	PUSH_SEND_WORD(h,blkAddr) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_READ_SINGLE_BLK);
	if(iret != 0) {
		goto exit_fail ;
	}
	ncopy =h->rcvPopLeft ;
	if(ncopy > *nSize ) ncopy = *nSize ;
	RCV_POP_PTR(h,pBlkBuf,ncopy) ;
	*nSize =ncopy ;
	iret = 0;
exit_fail:
	return iret;

}


extern "C" err_t ISO15693_WriteSingleBlock(RL8000_COMM *h,u16_t blkAddr,u8_t *blockData,u16_t nSize )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_WRITE_SINGLE_BLK,(2+nSize)) ;
	/* access parameters */
	PUSH_SEND_WORD(h,blkAddr) ;
	PUSH_SEND_PTR(h,blockData,nSize) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_WRITE_SINGLE_BLK);
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;

}




/*
*read single block function:
*nSize: input->size of the buffer 'pBlkBuf' ,output->bytes written into buffer 'pBlkBuf'
*/
extern "C" err_t ISO15693_ReadMultipleBlocks(RL8000_COMM *h,BOOLEAN readSecByte,u16_t blkAddr,u16_t blkNum,u8_t *pBlkBuf,u16_t *nSize )
{
	
	//short unsigned int vdlld=0xffff;
	
	
	err_t iret;
	u16_t nBlkRead ;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_READ_MULTI_BLKS,5) ;
	/* access parameters */
	if(readSecByte) {
		PUSH_SEND_BYTE(h,0x01) ;
	}
	else {
		PUSH_SEND_BYTE(h,0x00) ;
	}
	PUSH_SEND_WORD(h,blkAddr) ;
	PUSH_SEND_WORD(h,blkNum) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_READ_MULTI_BLKS);
	if(iret != 0) {
		goto exit_fail ;
	}
	if(h->rcvPopLeft < 2) {
		iret =-ERR_MSGSIZE;
		goto exit_fail ;
	}
	RCV_POP_PTR(h,(u8_t*)&nBlkRead,2) ;
	
	ncopy =h->rcvPopLeft ;
	if(ncopy > *nSize ) ncopy = *nSize ;
	RCV_POP_PTR(h,pBlkBuf,ncopy) ;
	*nSize =ncopy ;
	iret = 0;
exit_fail:
	return iret;

}




extern "C" err_t ISO15693_WriteMultipleBlocks(RL8000_COMM *h,u16_t blkAddr,u16_t blkNum,u8_t *blockData,u16_t nSize )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_WRITE_MULTI_BLKS,(4+nSize)) ;
	/* access parameters */
	PUSH_SEND_WORD(h,blkAddr) ;
	PUSH_SEND_WORD(h,blkNum) ;
	PUSH_SEND_PTR(h,blockData,nSize) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_WRITE_MULTI_BLKS);
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;

}



extern "C" err_t ISO15693_LockMultipleBlocks(RL8000_COMM *h,u16_t blkAddr,u16_t blkNum )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_LOCK_MULTI_BLKS,4) ;
	/* access parameters */
	PUSH_SEND_WORD(h,blkAddr) ;
	PUSH_SEND_WORD(h,blkNum) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_LOCK_MULTI_BLKS);
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;

}


extern "C" err_t ISO15693_WriteAFI(RL8000_COMM *h,u8_t AFI )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_WRITE_AFI,1) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,AFI);

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_WRITE_AFI);
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;

}


extern "C" err_t ISO15693_LockAFI(RL8000_COMM *h )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_LOCK_AFI,0) ;
	/* access parameters */


	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_LOCK_AFI);
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;

}



extern "C" err_t ISO15693_WriteDSFID(RL8000_COMM *h,u8_t DSFID )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_WRITE_DSFID,1) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,DSFID);

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_WRITE_DSFID);
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;

}

extern "C" err_t ISO15693_LockDSFID(RL8000_COMM *h )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_LOCK_DSFID,0) ;
	/* access parameters */


	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_LOCK_DSFID);
	if(iret != 0) {
		goto exit_fail ;
	}

	iret = 0;
exit_fail:
	return iret;

}


/*
*get tag system information:
*nSize: input->size of the buffer 'infBuf' ,output->bytes written into buffer 'infBuf'
*/
extern "C" err_t ISO15693_GetTagSysInfor(RL8000_COMM *h , u8_t *infBuf,u16_t *nSize)
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_GET_SYSINFO,0) ;
	/* access parameters */

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_GET_SYSINFO);
	if(iret != 0) {
		goto exit_fail ;
	}
	ncopy =h->rcvPopLeft ;
	if(ncopy > *nSize ) ncopy = *nSize ;
	RCV_POP_PTR(h,infBuf,ncopy) ;
	*nSize =ncopy ;
	iret = 0;
exit_fail:
	return iret;
}



/*
*get tag system information:
*nSize: input->size of the buffer 'staBuf' ,output->bytes written into buffer 'staBuf'
*/
extern "C" err_t ISO15693_GetBlockSecuritySta(RL8000_COMM *h ,u16_t blkAddr ,u16_t blkNum, u8_t *staBuf,u16_t *nSize)
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,ISO15693_ACC_GET_MULTI_BLKS_SS,4) ;
	/* access parameters */
	PUSH_SEND_WORD(h,blkAddr) ;
	PUSH_SEND_WORD(h,blkNum) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,ISO15693_ACC_GET_MULTI_BLKS_SS);
	if(iret != 0) {
		goto exit_fail ;
	}
	ncopy =h->rcvPopLeft ;
	if(ncopy > *nSize ) ncopy = *nSize ;
	RCV_POP_PTR(h,staBuf,ncopy) ;
	*nSize = ncopy;
	iret = 0;
exit_fail:
	return iret;
}

/***********************ISO14443A tag access API *********************************************/
extern "C"  err_t ISO14443A_TagConnect(RL8000_COMM *h ,u8_t tagType,u8_t uid[] ,u8_t uidLen)
{
	err_t iret;
	u8_t respTagHandle;
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_TAGCONNECT) ;
	/* connect header */
	PUSH_SEND_BYTE(h,0x00) ; //Antenna
	PUSH_SEND_BYTE(h,RFID_AIP_ISO14443A);
	PUSH_SEND_BYTE(h,tagType);
	PUSH_SEND_BYTE(h,uidLen) ;
	/* iso15693 connect parameter */
	PUSH_SEND_PTR(h,uid,uidLen) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGCONNECT) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	if(h->rcvPopLeft <  1) {
		iret = -ERR_MSGSIZE ;
		goto exit_fail ;
	}
	RCV_POP_PTR(h,&respTagHandle,1) ;
	h->hTag = respTagHandle ;

	iret = 0;
exit_fail:
	return iret;
}


#define MIFARE_CLASSIC_BLOCK_LENGTH					16


extern "C" err_t MifareS50_Authenticate(RL8000_COMM *h,u8_t blkAddr ,u8_t authType,u8_t keyIdx,u8_t *pKey)
{
	err_t iret;
	int ncopy ;
	int prmLen;

	prmLen = 3 ;
	if(keyIdx ==0) {
		prmLen +=6;
	}
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFCL_ACC_AUTHEN,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,blkAddr) ;
	PUSH_SEND_BYTE(h,authType) ;
	PUSH_SEND_BYTE(h,keyIdx) ;
	if(keyIdx ==0) {
		PUSH_SEND_PTR(h,pKey,6) ;
	}

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFCL_ACC_AUTHEN);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;
}

/*
*read blocks
*nSize: input->size of the buffer 'blkBuf' ,output->bytes written into buffer 'blkBuf'
*/
extern "C" err_t MifareS50_Read(RL8000_COMM *h,u8_t blkAddr ,u8_t blkNum ,BOOLEAN authBeforeRead ,u8_t authType,u8_t keyIdx,u8_t *pKey ,u8_t *blkBuf,u16_t *nSize)
{
	err_t iret;
	int ncopy ;
	int prmLen;

	prmLen = 3 ;
	if(authBeforeRead) {
		prmLen +=2;
		if(keyIdx ==0) {
			prmLen +=6;
		}
	}
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFCL_ACC_READ,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,blkAddr) ;
	PUSH_SEND_BYTE(h,blkNum) ;
	if(authBeforeRead) {
		PUSH_SEND_BYTE(h,0x01) ;
	}
	else {
		PUSH_SEND_BYTE(h,0x00) ;
	}
	if(authBeforeRead ) {
		PUSH_SEND_BYTE(h,authType) ;
		PUSH_SEND_BYTE(h,keyIdx) ;
		if(keyIdx ==0) {
			PUSH_SEND_PTR(h,pKey,6) ;
		}
	}


	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFCL_ACC_READ);
	if(iret != 0) {
		goto exit_fail ;
	}
	ncopy =h->rcvPopLeft ;
	if(ncopy > *nSize ) ncopy = *nSize ;
	RCV_POP_PTR(h,blkBuf,ncopy) ;
	*nSize = ncopy ;
	iret = 0;
exit_fail:
	return iret;
}

extern "C" err_t MifareS50_Write(RL8000_COMM *h,u8_t blkAddr ,u8_t blkNum  ,u8_t *blkData, BOOLEAN authBeforeWrite ,u8_t authType,u8_t keyIdx,u8_t *pKey)
{
	err_t iret;
	int prmLen;

	prmLen = 3+ blkNum * MIFARE_CLASSIC_BLOCK_LENGTH;
	if(authBeforeWrite) {
		prmLen +=2;
		if(keyIdx ==0) {
			prmLen +=6;
		}
	}
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFCL_ACC_WRITE,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,blkAddr) ;
	PUSH_SEND_BYTE(h,blkNum) ;
	if(authBeforeWrite) {
		PUSH_SEND_BYTE(h,0x01) ;
	}
	else {
		PUSH_SEND_BYTE(h,0x00) ;
	}
	if(authBeforeWrite ) {
		PUSH_SEND_BYTE(h,authType) ;
		PUSH_SEND_BYTE(h,keyIdx) ;
		if(keyIdx ==0) {
			PUSH_SEND_PTR(h,pKey,6) ;
		}
	}
	PUSH_SEND_PTR(h,blkData,blkNum * MIFARE_CLASSIC_BLOCK_LENGTH) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFCL_ACC_WRITE);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;
}

extern "C" err_t MifareS50_Increment(RL8000_COMM *h,u8_t blkAddr ,u32_t increValue,BOOLEAN autoTransfer)
{
	err_t iret;
	int prmLen;

	prmLen = 6;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFCL_ACC_INCREMENT,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,blkAddr) ;
	PUSH_SEND_DWORD(h,increValue) ;
	if(autoTransfer) {
		PUSH_SEND_BYTE(h,0x01) ;
	}
	else {
		PUSH_SEND_BYTE(h,0x00) ;
	}

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFCL_ACC_INCREMENT);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;
}

extern "C" err_t MifareS50_Decrement(RL8000_COMM *h,u8_t blkAddr ,u32_t decreValue,BOOLEAN autoTransfer)
{
	err_t iret;
	int prmLen;

	prmLen = 6;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFCL_ACC_DECREMENT,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,blkAddr) ;
	PUSH_SEND_DWORD(h,decreValue) ;
	if(autoTransfer) {
		PUSH_SEND_BYTE(h,0x01) ;
	}
	else {
		PUSH_SEND_BYTE(h,0x00) ;
	}

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFCL_ACC_DECREMENT);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;
}

extern "C" err_t MifareS50_Restore(RL8000_COMM *h,u8_t blkAddr)
{
	err_t iret;
	int prmLen;

	prmLen = 6;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFCL_ACC_RESTORE,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,blkAddr) ;
	PUSH_SEND_DWORD(h,0x00) ;
	PUSH_SEND_BYTE(h,0x00) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFCL_ACC_RESTORE);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;
}


extern "C" err_t MifareS50_Transfer(RL8000_COMM *h,u8_t blkAddr)
{
	err_t iret;
	int prmLen;

	prmLen = 1;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFCL_ACC_TRANSFER,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,blkAddr) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFCL_ACC_TRANSFER);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;

}



#define MIFARE_ULTRALIGHT_PAGE_LENGTH					4


/*
* authentication is only supported by mifare ultralight C 
*/
extern "C" err_t MFUltralight_Authenticate(RL8000_COMM *h,u8_t keyIdx ,u8_t *pKey)
{
	err_t iret;
	int ncopy ;
	int prmLen;

	prmLen =1 ;
	if(keyIdx ==0) {
		prmLen +=16;
	}
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFULTRALIGHT_ACC_AUTHENTICATE,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,keyIdx) ;
	if(keyIdx ==0) {
		PUSH_SEND_PTR(h,pKey,16) ;
	}

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFULTRALIGHT_ACC_AUTHENTICATE);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;
}

extern "C" err_t MFUltralight_ReadPages(RL8000_COMM *h,u16_t startPage ,u16_t pagesToRead,u8_t *blkBuf,u16_t *nSize)
{
	err_t iret;
	int ncopy ;
	int prmLen;

	prmLen =4 ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFULTRALIGHT_ACC_READ_MULTIPLE_PAGES,prmLen) ;
	/* access parameters */
	PUSH_SEND_WORD(h,startPage) ;
	PUSH_SEND_WORD(h,pagesToRead) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFULTRALIGHT_ACC_READ_MULTIPLE_PAGES);
	if(iret != 0) {
		goto exit_fail ;
	}
	ncopy =h->rcvPopLeft ;
	if(ncopy > *nSize ) ncopy = *nSize ;
	RCV_POP_PTR(h,blkBuf,ncopy) ;
	*nSize =ncopy ;
	iret = 0;
exit_fail:
	return iret;
}

extern "C" err_t MFUltralight_WritePages(RL8000_COMM *h,u16_t startPage,u16_t pagesToWrite,u8_t *pPageData)
{
	err_t iret;
	int prmLen;

	prmLen =4 + pagesToWrite * MIFARE_ULTRALIGHT_PAGE_LENGTH  ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFULTRALIGHT_ACC_WRITE_MULTIPLE_PAGES,prmLen) ;
	/* access parameters */
	PUSH_SEND_WORD(h,startPage) ;
	PUSH_SEND_WORD(h,pagesToWrite) ;
	PUSH_SEND_PTR(h,pPageData,pagesToWrite * MIFARE_ULTRALIGHT_PAGE_LENGTH) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFULTRALIGHT_ACC_WRITE_MULTIPLE_PAGES);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;
}

/*
* authentication is only supported by mifare ultralight C 
*/
extern "C" err_t MFUltralight_UpdatePassword(RL8000_COMM *h,u8_t *newPwd)
{
	err_t iret;
	int prmLen;

	prmLen =16 ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFULTRALIGHT_ACC_UPDATE_PWD,prmLen) ;
	/* access parameters */
	PUSH_SEND_PTR(h,newPwd,16) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFULTRALIGHT_ACC_UPDATE_PWD);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;

}

/*
* authentication is only supported by mifare ultralight C 
*/
extern "C" err_t MFUltralight_UpdateAuthCfg(RL8000_COMM *h,u8_t cfg0,u8_t cfg1)
{
	err_t iret;
	int prmLen;

	prmLen =2 ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	/* push access header */
	PUSH_SEND_TAG_ACCESS_HEAD(h,MFULTRALIGHT_ACC_UPDATE_AUTHCFG,prmLen) ;
	/* access parameters */
	PUSH_SEND_BYTE(h,cfg0) ;
	PUSH_SEND_BYTE(h,cfg1) ;

	iret = IBlock_Transceive(h,RL8000_CMD_TAGACCESS) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = PARSE_TAG_ACCESS_RESULT_HEAD(h,MFULTRALIGHT_ACC_UPDATE_AUTHCFG);
	if(iret != 0) {
		goto exit_fail ;
	}
	iret = 0;
exit_fail:
	return iret;

}





/*
* ISO14443A-4 transceive function
* apduRespLen: input:the max bytes can write to apduResp buffer ;Output:bytes written to apduResp buffer
*/
extern "C" err_t ISO14443p4_Transceive(RL8000_COMM *h ,u32_t apduReqLen ,u8_t *apduReq ,u32_t *apduRespLen ,u8_t *apduResp   )
{
	err_t iret;
	int ncopy ;
	
	INIT_SEND(h) ;
	OFFSET_HEADER(h) ;  //offset header struct 
	PUSH_SEND_BYTE(h,RL8000_CMD_ISO14443p4_TRANSCEIVE) ;  
	PUSH_SEND_BYTE(h,h->hTag) ; 
	PUSH_SEND_EBV_LENGTH(h,apduReqLen) ;   
	PUSH_SEND_PTR(h,apduReq , apduReqLen) ;

	iret = IBlock_Transceive(h,RL8000_CMD_ISO14443p4_TRANSCEIVE) ;
	if(iret != 0) {
		goto exit_fail ;
	}
	ncopy = h->rcvPopLeft ;
	if(ncopy > *apduRespLen) ncopy = *apduRespLen ;
	RCV_POP_PTR(h,apduResp,ncopy);
	 *apduRespLen =ncopy ;
	iret = 0;
exit_fail:
	return iret;
}


#define MAX_UIDS			100
#define MAX_UID_LEN			16

//ragma pack(push, 4)
struct tagreport
{
	u8_t tag;
	u8_t uidLen;
	u8_t uid[16];
	u8_t user[255];
	u8_t userlen;
} ;
//ragma pack(pop)

struct tagreport tagReportSet[MAX_UIDS] ; 
u16_t tagReportAdded ;

void tag_report_handler(u8_t ant,u8_t aip,u8_t tag,u8_t uidlen,u8_t uid[])
{
	if(aip == RFID_AIP_ISO15693) {
		if(uidlen == 9) {
			if(tagReportAdded >=MAX_UIDS) {
				return;
			}
			tagReportSet[tagReportAdded].tag =tag ;
			memcpy(tagReportSet[tagReportAdded].uid,uid,uidlen-1) ;
			tagReportSet[tagReportAdded].uidLen =uidlen-1 ;
			tagReportAdded++ ;
		
		}
		
	}
			
}
	
void BytesReverseTest(u8_t *buf,int len ) 
{
	int i;
	u8_t tmp ;
	for(i = 0;i< len/ 2 ;i++ ) {
		tmp = buf[i] ;
		buf[i] = buf[len-1 -i] ;
		buf[len-1 -i] = tmp ;
	}
}



extern "C" err_t RL8000_WinTagInventory(u8_t busAddr, char *tagUid, int &nlen)
{
	RL8000_COMM * hr ;
	u16_t tagGetted;
	struct ISO15693InvenParam ISO15693InvenParm ;
	

	OIniFile Ifile;
	Ifile.iLoadFile("RfAddr.ini");
	BOOL retOpen = Ifile.bIsOpenOK();
	if(retOpen)
	{
		int tt = 5;
		char addr[2] = {0};

		sprintf(addr,"%d",busAddr);
		int val = 0;
		int ret = Ifile.iGetIntValue("RFIDADDR", addr, val);
		printf("----------->ret = %d, %d\n", ret,val);
		hr = RL8000_CommOpenWithBusAddr((u8_t)val);
	}
	else
		hr = RL8000_CommOpenWithBusAddr(busAddr);
	
	
	if(hr == NULL)
	{
		
		nlen = 0;
		return -1;
	}
	
	err_t ret = RL8000_Connect(hr);
	if(ret != 0)
	{
		nlen=0;
		return ret;
	}
	
	tagReportAdded = 0;
	ISO15693InvenParm.AFI =0 ;//write
	tagGetted =0 ;
	memset(tagReportSet, 0, sizeof(struct tagreport)*MAX_UIDS);
	int iret =RL8000_CMD_TagInventory(hr,false,(AIP_SEL_ISO15693 | AIP_SEL_ISO14443A),&ISO15693InvenParm,tag_report_handler,&tagGetted);
	if(iret != 0)
	{
		nlen=0;
		return iret;
	}
	
	int nCount = 0;
	if(nlen < 1024)
	{
		nlen = 0;
		return -1;
	}
	
	int nSumlen = nlen;
	nlen=0;
	printf("================>tagGetted = %d\n",tagGetted);
	for(int i=0;i<tagGetted;i++ ) 
	{		

		/*
		u8_t uidreverse[8]={0};
		memcpy(uidreverse,tagReportSet[i].uid,tagReportSet[i].uidLen);
		BytesReverseTest(uidreverse,8);
		*/
						
		iret =ISO15693_TagConnect(hr,7,ISO15693_ADDR_MODE_ADDRESS,tagReportSet[i].uid) ;
		if(iret != 0)
		{
			nlen=0;
			return iret;
		}
	
		if(iret==0) 
		{
			/* reset command test */
			iret =ISO15693_Reset(hr) ;
			if(iret == 0) {
				printf("ISO15693 reset ok \n .....") ;
			}
			else {
				printf("ISO15693 reset fail \n .....") ;
			}

			/* read single block command test */
			
				
					
			/* read multiple blocks command test */
			tagReportSet[i].userlen=0;
				

			u16_t nSize =8*5;
			u8_t t_userbuff[8*5]={0};
			
			iret =ISO15693_ReadMultipleBlocks(hr,true,0,8,t_userbuff,&nSize) ;
			printf("==============>ISO15693_ReadMultipleBlocks_nSize = %d\n",nSize);
			if(iret ==0) 
			{
				printf("read multiple blocks ok \n") ;
				tagReportSet[i].userlen=nSize;
				
				
				for(int j=0;j<8;j++)
				{
					memcpy(tagReportSet[i].user+j*4,&t_userbuff[j*5+1],4);
				}
				
				tagReportSet[i].userlen=8*4;
			}
			else
			{
				printf("read multiple blocks fail \n") ;
				tagReportSet[i].userlen=0;
			}
				
				
				
			RL8000_CMD_TagDisconnect(hr) ;
		}

		char strbuf[64] ;
		memcpy(tagUid+sizeof(struct tagreport)*i, (char *)&tagReportSet[i], sizeof(struct tagreport));
		nlen += sizeof(struct tagreport);
		if(nlen > nSumlen)
			break;
		
	}
	
	
	delete hr;
	hr = NULL;
	
	return iret;
}