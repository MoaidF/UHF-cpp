#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#include "ORFIDDevDLL.h"





s32_t cirbuf_offset(CIR_BUFFER *circular_buf ,u32_t start,u32_t end)  ;

s32_t cirbuf_init(CIR_BUFFER *circular_buf, u32_t size, u8_t *ptr)  ;
u32_t cirbuf_get_free_size( CIR_BUFFER *circular_buf )   ;
u32_t cirbuf_get_size(CIR_BUFFER *circular_buf)  ;
s32_t cirbuf_push(CIR_BUFFER *circular_buf, u8_t *buffer, u32_t size )   ;
u32_t cirbuf_peek(CIR_BUFFER *circular_buf, u8_t *buffer, u32_t size)  ;
s32_t cirbuf_pop(CIR_BUFFER *circular_buf, u8_t *buffer, u32_t size)  ;
s32_t cirbuf_flush( CIR_BUFFER *circular_buf ) ;

s32_t cirbuf_pop_to_cursor(CIR_BUFFER *circular_buf) ;

s32_t  cirbuf_cursor_move_first(CIR_BUFFER *circular_buf)  ;
s32_t cirbuf_cursor_move_next(CIR_BUFFER *circular_buf,int offset)  ;
s32_t cirbuf_cursor_read_data(CIR_BUFFER *circular_buf,int bytesToRead,u8_t *buffer);
s32_t cirbuf_cursor_get_left(CIR_BUFFER *circular_buf) ;

s32_t cirbuf_push_pbuf(CIR_BUFFER *circular_buf, struct pbuf *pDatas, u32_t size) ;

#endif
