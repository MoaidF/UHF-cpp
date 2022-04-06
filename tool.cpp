#include "platformOpts.h"
#include "tool.h"

#define EBV_MAX						5



u16_t cal_crc16_ext(u16_t initval ,u8_t *ptr,  u16_t len)
{
	u16_t crc;
	u16_t i,j;
	u8_t val;
	crc=0;
	crc= initval ;
	for(i=0;i<len;i++)
	{
		val =ptr[i];
		crc^=val ;
		for(j=0;j<8;j++)
		{
			if(crc&0x0001)
				crc=(crc>>1)^0x8408;
			else
				crc=(crc>>1);
		}
	}
	return(crc);
}




int EBV_Set(u32_t dw,u8_t* pbuf)
{
	u8_t val;

	val= dw & 0x7f ;
	if(dw > 0x7f) {
		val |=0x80 ;
	}
	pbuf[0] = val;
	if((val & 0x80) == 0) return 1;

	dw = dw >> 7 ;
	val =dw & 0x7f ;
	if(dw > 0x7f) {
		val |=0x80 ;
	}
	pbuf[1] = val;
	if((val & 0x80) == 0) return 2;

	dw = dw >> 7 ;
	val = dw & 0x7f ;
	if(dw > 0x7f) {
		val |=0x80 ;
	}
	pbuf[2] = val;
	if((val & 0x80) == 0) return 3;

	
	dw = dw >> 7 ;
	val= dw & 0x7f ;
	if(dw > 0x7f ) {
		val |=0x80 ;
	}
	pbuf[3] = val;
	if((val & 0x80) == 0) return 4;


	dw = dw >> 7 ;
	val = dw & 0x0f ;
	pbuf[4] = val;
	return 5;
}

int EBV_GetSize(u32_t dw)
{
	int n ;

	u8_t val;
	bool next;

	n =1 ;
	next =false ;
	val= dw & 0x7f ;
	if(dw > 0x7f) {
		val |=0x80 ;
		next=true ;
	}
	if(next == false) return n;

	n =2;
	next =false ;
	dw = dw >> 7 ;
	val =dw & 0x7f ;
	if(dw > 0x7f) {
		val |=0x80 ;
	}
	if(next == false) return n;

	n = 3;
	next =false ;
	dw = dw >> 7 ;
	val = dw & 0x7f ;
	if(dw > 0x7f) {
		val |=0x80 ;
	}
	if(next == false) return n;

	
	n = 4;
	dw = dw >> 7 ;
	val= dw & 0x7f ;
	if(dw > 0x7f ) {
		val |=0x80 ;
	}
	if(next == false) return n;

	n = 5;
	dw = dw >> 7 ;
	val = dw & 0x0f ;
	
	return  n ;
}


int EBV_Get(u8_t buf[],u16_t leftSize,u32_t *pval)
{
	u8_t EBV[EBV_MAX] ;
	u8_t n ;
	n=0 ;
	memset(EBV,0,EBV_MAX) ;
	if(leftSize < 1) {
		return 0 ;
	}
	EBV[0] = buf[0] ;
	leftSize-- ;
	n++ ;
	if((EBV[0] & 0x80) == 0) {
		goto finish ;
	}
	if(leftSize < 1) {
		return 0 ;
	}
	EBV[1] = buf[1] ;
	leftSize-- ;
	n++ ;
	if((EBV[1] & 0x80) == 0) {
		goto finish ;
	}
	if(leftSize < 1) {
		return 0 ;
	}
	EBV[2] = buf[2] ;
	leftSize-- ;
	n++ ;
	if((EBV[2] & 0x80) == 0) {
		goto finish ;
	}
	if(leftSize < 1) {
		return 0 ;
	}
	EBV[3] = buf[3] ;
	leftSize-- ;
	n++ ;
	if((EBV[3] & 0x80) == 0) {
		goto finish ;
	}
	if(leftSize < 1) {
		return 0 ;
	}
	EBV[4] = buf[4] ;
	n++ ;
finish:
	if(pval) *pval =(EBV[0] & 0x7f) | ((EBV[1] & 0x7f) << 7  & 0x3f80) | ( (EBV[2] & 0x7f) << 14 & 0x1FC000 ) | ( EBV[3] & 0x7f << 21  & 0xFE00000) |(EBV[4] & 0x0f << 28  & 0xf0000000) ; 
	return n;
}