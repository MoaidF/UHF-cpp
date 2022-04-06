#ifndef DATACODE_H_
#define DATACODE_H_

#pragma pack (push, 8)
#include <stdint.h>
#include <sys/types.h>
#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack (push, 1)

typedef enum _eDataCode
{
	FC_Unknown = 0,
	FC_ASCII,
	FC_UTF8_Unicode_withBOM,
	FC_UTF8_Unicode,
	FC_Little_endian_UTF16_Unicode,
	FC_Big_endian_UTF16_Unicode,
	FC_ISO_8859,
	FC_Non_ISO_extended_ASCII,
	FC_EBCDIC,
	FC_International_EBCDIC
} DataCode;

/*
 * Function ： 获取数据的编码
 *
 * Parameter ：
 *		In	p_pcBuffer		数据缓存地址
 *		In	p_nBytes		数据缓存长度
 *
 * Return ： DataCode		数据编码
 *
 */

DataCode ParseDataCode(const unsigned char * p_pcBuffer, size_t p_nBytes);

#pragma pack (pop)

#ifdef __cplusplus
}
#endif

#endif /* DATACODE_H_ */
