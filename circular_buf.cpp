#include "platformOpts.h"
#include "circular_buf.h"


#define CIRBUF_INTSAVE_DATA
#define CIRBUF_ENTER_CRITICAL()
#define CIRBUF_LEAVE_CRITICAL()


static s32_t cirbuf_offset(CIR_BUFFER *circular_buf ,u32_t start,u32_t end) 
{
    int offset ;
	offset=end-start;
	if(offset<0)  offset=circular_buf->size+offset ;

	return offset ;
}


s32_t cirbuf_init(CIR_BUFFER *circular_buf, u32_t size, u8_t *ptr) 
{

	if(circular_buf == NULL || ptr == NULL || size == 0)
	     return -1;
	circular_buf->Buffer =ptr;
	circular_buf->size = size;
	circular_buf->readPos = 0;
	circular_buf->writePos = 0;
	circular_buf->overflow = 0 ;

	return  0 ;
}


/*
*  ²»¶ÔÒç³öÅÐ¶Ï
*/
s32_t cirbuf_push(CIR_BUFFER *circular_buf, u8_t *buffer, u32_t size )
{
	s32_t nextWritePos;
	s32_t itmp ;
	CIRBUF_INTSAVE_DATA
	CIRBUF_ENTER_CRITICAL() 

	if(circular_buf == NULL) {
	 	return 0;
	}
	nextWritePos =circular_buf->writePos + size ;
	if(nextWritePos <= circular_buf->size) 
	{
		memcpy(circular_buf->Buffer +circular_buf->writePos,buffer,size) ; 
	}
	else {
	    itmp =(circular_buf->size -circular_buf->writePos) ;
	    memcpy(circular_buf->Buffer +circular_buf->writePos,buffer,itmp) ;
		memcpy(circular_buf->Buffer,buffer+ itmp, nextWritePos - circular_buf->size) ;
	}
	if(	nextWritePos >=circular_buf->size) {
	 	circular_buf->writePos =nextWritePos -circular_buf->size   ;
	} 
	else {
		circular_buf->writePos = nextWritePos;
	}
	CIRBUF_LEAVE_CRITICAL()
		
	return size;
}


//u32_t cirbuf_get_available_size( CIR_BUFFER *circular_buf ) 
u32_t cirbuf_get_free_size( CIR_BUFFER *circular_buf )  
{
  //u32_t retval;
  if(!circular_buf) {
   	return  0 ;
  }
  return (circular_buf->size - cirbuf_get_size (circular_buf)) ;

}


/*
* to return data in bytes have push into the buffer 
*/
u32_t cirbuf_get_size(CIR_BUFFER *circular_buf) 
{
  s32_t pushBytes ;
  pushBytes = 0;
  if(!circular_buf) {
   	  return 0 ;
  }
  if(circular_buf->writePos >= circular_buf->readPos) {
   	 pushBytes =circular_buf->writePos - circular_buf->readPos ;
  }
  else {
	 pushBytes =circular_buf->size  - circular_buf->readPos; 
	 pushBytes = pushBytes + circular_buf->writePos;
  }
  return pushBytes;
}


/*
*  return bytes pop
*/
s32_t cirbuf_pop(CIR_BUFFER *circular_buf, u8_t *buffer, u32_t size)
{
	u32_t newPos ;
	s32_t bytesToPop;
	s32_t itmp;
	CIRBUF_INTSAVE_DATA
	CIRBUF_ENTER_CRITICAL() 


	if(!circular_buf) {
	 	return  0 ;
	}
	bytesToPop = cirbuf_get_size (circular_buf);
	if(	bytesToPop ==0) {
		return  0 ;
	}
	if(bytesToPop > size) {
	    bytesToPop = size ;
	}
	newPos = circular_buf->readPos  + bytesToPop ;
	itmp = circular_buf->size -circular_buf->readPos ; 
	if(newPos > circular_buf->size )
	{
		if(buffer) memcpy(buffer , circular_buf->Buffer + circular_buf->readPos ,itmp) ; 
		if(buffer) memcpy(buffer + itmp ,circular_buf->Buffer,(bytesToPop- itmp)) ; 
	}
	else {
	    if(buffer) memcpy(buffer , circular_buf->Buffer + circular_buf->readPos ,bytesToPop) ;
	}
	if(newPos >=circular_buf->size ){
	    circular_buf->readPos=newPos - circular_buf->size  ;
	}
	else {
		circular_buf->readPos = newPos;
	}

	CIRBUF_LEAVE_CRITICAL()
	return bytesToPop;
}

s32_t cirbuf_flush( CIR_BUFFER *circular_buf )
{
  s32_t retval;
  retval = 0 ;
  if ( circular_buf !=NULL )
  {
    circular_buf->writePos = 0;
    circular_buf->readPos = 0;
	circular_buf->overflow = 0 ;
    //memset( circular_buf->Buffer, 0x0, circular_buf->size );
  }
  else
  {
    retval = -1;
  }
  return retval;
}




/**********************************Cursor operation functions*************************************/

/*
*  Cursor exist in the the range of data pushed ,but not in the total buffer.
*  the range is from read position to (write position -1)
*/
// return 1:ok,0:fail;
s32_t  cirbuf_cursor_move_first(CIR_BUFFER *circular_buf) 
{
	if(	circular_buf->readPos == circular_buf->writePos ) {//The buffer is empty
	 	return 0 ;
	} 
	circular_buf->cursor=circular_buf->readPos ;
	return 1 ;
}



// return 1:ok,0:fail;
s32_t cirbuf_cursor_move_next(CIR_BUFFER *circular_buf,int offset) 
{
	int newpos ;
	int result ;
	u32_t writePos ;
	result =1;
	writePos = circular_buf->writePos ;
	if(	circular_buf->readPos  == writePos) {
		return 0 ;
	}
	newpos = circular_buf->cursor + offset ;
	if( circular_buf->readPos <writePos ) {
	    /* 
		*  0*************R___________________W******************10
		*
		*/
	 	if((newpos >=writePos) || (newpos < circular_buf->readPos)){
			//newpos =circular_buf->writePos-1; //set to end 
			result = 0 ;
		} 
	}
	else {
	    /*  0______cursor2_____W**************************R_______cursor1________10 */
		if((circular_buf->cursor >= circular_buf->readPos) && (circular_buf->cursor < circular_buf->size)) {
	   		if(	newpos >=circular_buf->size) {
				newpos =newpos -circular_buf->size ;
				if(newpos >=writePos) {
					result = 0 ;
				}
			} 
		}
		else {
			if(newpos >= writePos) {
				result =0 ;
			}
		}
	}
	if(result) { 
	    circular_buf->cursor = newpos ;
	}
	return result;
}

/*
*  return bytes read
*/
s32_t cirbuf_cursor_read_data(CIR_BUFFER *circular_buf,int bytesToRead,u8_t *buffer)
{
//    s32_t iret;
//    s32_t backup;
//	backup = circular_buf->readPos;
//	circular_buf->readPos = circular_buf->cursor ;
//	iret = cirbuf_pop(circular_buf , buffer , bytesToRead) ;
//	circular_buf->readPos = backup ;
	u32_t newPos ;
	s32_t bytesToPop;
	s32_t itmp;
	CIRBUF_INTSAVE_DATA
	CIRBUF_ENTER_CRITICAL() 


	if(!circular_buf) {
	 	return  0 ;
	}
	if(bytesToRead==1) {
		buffer[0] =circular_buf->Buffer[circular_buf->cursor] ; 
		return 1;	
	}
	bytesToPop = cirbuf_cursor_get_left (circular_buf);
	if(bytesToPop > bytesToRead) {
	    bytesToPop = bytesToRead ;
	}
	newPos = circular_buf->cursor  + bytesToPop ;
	itmp = circular_buf->size -circular_buf->cursor ; 
	if(newPos > circular_buf->size )
	{
		if(buffer) memcpy(buffer , circular_buf->Buffer + circular_buf->cursor ,itmp) ; 
		if(buffer) memcpy(buffer + itmp ,circular_buf->Buffer,(bytesToPop- itmp)) ; 
	}
	else {
	    if(buffer) memcpy(buffer , circular_buf->Buffer + circular_buf->cursor ,bytesToPop) ;
	}
	CIRBUF_LEAVE_CRITICAL()
	return bytesToPop;		
}



s32_t cirbuf_cursor_get_left(CIR_BUFFER *circular_buf)
{
    int left ;
	u32_t writePos ;
	left =0 ;
	writePos=circular_buf->writePos ; 
    if ( (circular_buf->cursor <=writePos ))
    {
       left = ( writePos ) - circular_buf->cursor;
    }
	else 
    {
       left = (circular_buf->size - circular_buf->cursor) + writePos ;
    }

   return left;
}


//// return 1:ok,0:fail;
//s32_t cirbuf_cursor_move_next(CIR_BUFFER *circular_buf,int offset) 
//{
//	int newpos ;
//	int result ;
//	result =1;
//	newpos = circular_buf->cursor + offset ;
//	if( circular_buf->readPos < circular_buf->writePos) {
//	    /* 
//		*  0*************R___________________W******************10
//		*
//		*/
//	 	if(newpos >=circular_buf->writePos){
//			//newpos =circular_buf->writePos-1; //set to end 
//			result = 0 ;
//		} 
//	}
//	else {
//	    /*  0______cursor2_____W**************************R_______cursor1________10 */
//	    if(circular_buf->cursor >=circular_buf->readPos ) {
//		    if(newpos >= circular_buf->size) {
//			   newpos= newpos-circular_buf->size ;
//			   if(newpos>=circular_buf->writePos) {
//				  //newpos =circular_buf->writePos -1 ;//set to end  
//				  //if(newpos < 0) {
//				  //   newpos = circular_buf->size -1;
//				  //}
//				  result = 0 ;
//			   }
//			}
//		} 
//		else {
//		    if(newpos >= circular_buf->writePos) {
//				//newpos =circular_buf->writePos -1 ; //set to end 
//				//if(newpos < 0) {
//				//    newpos = circular_buf->size -1;
//				//}
//				result = 0 ;
//			}
//		} 
//	}
//	if(result) { 
//	    circular_buf->cursor = newpos ;
//	}
//	return result;
//}





s32_t cirbuf_pop_to_cursor(CIR_BUFFER *circular_buf)
{
    int nlen;
	nlen=cirbuf_offset(circular_buf , circular_buf->readPos ,circular_buf->cursor )  ;
	return cirbuf_pop(circular_buf , NULL,nlen) ;
} ;








#define CIRBUF_PEEK				0
#if CIRBUF_PEEK
u32_t cirbuf_peek(CIR_BUFFER *circular_buf, u8_t *buffer, u32_t size) 
{
	INT8S retval;
	u32_t i;
	u32_t readPosition;
	readPosition = circular_buf->readPos;
	retval= 0 ;
	if ( circular_buf != NULL )	
	{		 
		i =cirbuf_get_size (circular_buf);
		if(size > i ) size =  i ;
		for (i=0; i < size; i++ )
		{
			buffer[i] = circular_buf->Buffer [ readPosition ];
			readPosition ++ ;
			if ( readPosition == circular_buf->size )
			{
				readPosition = 0;
			}
		}
		retval = size;			
	}
	else
	{
		retval = 0;
	}
	return retval;
}
#endif


/*
*	the function flush the circular buffer and write data to buffer from head
*	return the len push to the buffer 
*/
#define CIRBUF_HEAD_PUSH		0
#if CIRBUF_HEAD_PUSH
s32_t  cirbuf_head_push(CIR_BUFFER *circular_buf,u8_t *buffer ,u32_t len)
{
	cirbuf_flush(circular_buf);
	if(len > circular_buf->size) len  = circular_buf->size ;
	/* batch copy */
	memcpy(circular_buf->Buffer,buffer,len) ;
	if(len >= circular_buf->size)
	{
		circular_buf->writePos = 0;
		circular_buf->overflow = 1;
	}
	else
	{
		circular_buf->writePos = len;
	}
	
	return len ;
}
#endif
