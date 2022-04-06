#ifndef INIFILE_H_
#define INIFILE_H_

#pragma pack (push, 8)
#include <stdint.h>
#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack (push, 1)

/*
 *  Remarks :
 *  	为了保持与原始的API接口的输入输出规则，如果文件编码是Unicode编码，
 *  	程序内部会使用UTF-8编码进行处理，在写文件时会将UTF-8编码转换成原来
 *  	的Unicode编码，其他编码不作转换处理。
 *
 */



/*
 * Function ： 从INI获取所有sections的名称
 *
 * Parameter ：
 *		Out	p_pcReturnBuffer	数据接收缓冲，使用\0间隔一个或多个字符串，并以两个\0结束
 *		In	p_nSize				缓冲区的大小
 *		In	p_pcFileName		INI文件路径
 *
 * Return ： uint32_t			返回p_pcReturnBuffer缓冲的数据接收长度，包含\0分隔符及两个\0结束符
 *
 */

uint32_t GetPrivateProfileSectionNames(
	/*Out*/		char *  		p_pcReturnBuffer,
	/*In*/		uint32_t   		p_nSize,
	/*In*/		const char * 	p_pcFileName
);

/*
 * Function ： 获取指定到section块的所有子项，包括键名称和键值数据
 *
 * Parameter ：
 *		In	p_pcAppName			section的名称
 *		Out	p_pcReturnedString	数据接收缓冲，使用\0间隔一个或多个字符串，并以两个\0结束
 *		In	p_nSize				缓冲区的大小
 *		In	p_pcFileName		INI文件路径
 *
 * Return ： uint32_t			返回p_pcReturnedString缓冲的数据接收长度，包含\0分隔符及两个\0结束符
 *
 */
uint32_t GetPrivateProfileSection(
	/*In*/		const char * 	p_pcAppName,
	/*Out*/		char *  		p_pcReturnedString,
	/*In*/		uint32_t  		p_nSize,
	/*In*/		const char * 	p_pcFileName
);

/*
 * Function ： 根据section名称和键名称获取键值数据
 *
 * Parameter ：
 *		In	p_pcAppName			section的名称
 *		In	p_pcKeyName			键名称
 *		In	p_pcDefault			默认键值数据
 *		Out	p_pcReturnedString	接收键值数据缓冲
 *		In	p_nSize				缓冲区的大小
 *		In	p_pcFileName		INI文件路径
 *
 * Return ： uint32_t			返回p_pcReturnedString以\0结束的字符串长度
 *
 */
uint32_t GetPrivateProfileString(
	/*In*/		const char * 	p_pcAppName,
	/*In*/		const char * 	p_pcKeyName,
	/*In*/		const char * 	p_pcDefault,
	/*Out*/		char *  		p_pcReturnedString,
	/*In*/		uint32_t  		p_nSize,
	/*In*/		const char * 	p_pcFileName
);

/*
 * Function ： 根据section名称和键名称获取键值的整型数值
 *
 * Parameter ：
 *		In	p_pcAppName			section的名称
 *		In	p_pcKeyName			键名称
 *		In	p_pcDefault			默认的整型键值
 *		In	p_pcFileName		INI文件路径
 *
 * Return ： uint32_t			返回整型键值
 *
 */
uint32_t GetPrivateProfileInt(
	/*In*/		const char *	p_pcAppName,
	/*In*/		const char *	p_pcKeyName,
	/*In*/		int32_t			p_nDefault,
	/*In*/		const char * 	p_pcFileName
);

/*
 * Function ： 根据section名称和键名称获取结构体类型的键值数据
 *
 * Parameter ：
 *		In	p_pcSection			section的名称
 *		In	p_pcszKey			键名称
 *		Out	p_pcStruct			结构体接收缓冲
 *		In	p_uiSizeStruct		结构体大小
 *		In	p_pcFileName		INI文件路径
 *
 * Return ： bool				获取成功返回true，否则返回false
 *
 */
bool GetPrivateProfileStruct(
	/*In*/		const char *	p_pcSection,
	/*In*/		const char *	p_pcszKey,
	/*Out*/		const void *	p_pcStruct,
	/*In*/		uint32_t		p_uiSizeStruct,
	/*In*/		const char *	p_pcFileName
);

/*
 * Function ： 更新或删除section块
 *
 * Parameter ：
 *		In	p_pcAppName			section的名称
 *		In	p_pcString			用于替换的键项列表，可以为“”，格式：key=value，以\0分隔，以\0\0结束，如果为NULL，则删除存在的section块
 *		In	p_pcFileName		INI文件路径
 *
 * Return ： bool				执行成功返回true，否则返回false
 *
 */
bool WritePrivateProfileSection(
	/*In*/		const char *	p_pcAppName,
	/*In*/		const char *	p_pcString,
	/*In*/		const char *	p_pcFileName
);

/*
 * Function ： 更新section块中的字符串键项或删除整个section块
 *
 * Parameter ：
 *		In	p_pcAppName			section的名称
 *		In	p_pcKeyName			键名称，如果为NULL，则删除整块存在的section块
 *		In	p_pcString			用于更新的键值数据，如果为“”，则清空键值数据，如果为NULL，则删除当前键项
 *		In	p_pcFileName		INI文件路径
 *
 * Return ： bool				执行成功返回true，否则返回false
 *
 */
bool WritePrivateProfileString(
	/*In*/	const char *	p_pcAppName,
	/*In*/	const char *	p_pcKeyName,
	/*In*/	const char *	p_pcString,
	/*In*/	const char *	p_pcFileName
);

/*
 * Function ： 更新section块中的结构体键项或删除整个section块
 *
 * Parameter ：
 *		In	p_pcSection			section的名称
 *		In	p_pcKey				键名称，如果为NULL，则删除整块存在的section块
 *		In	p_pvStruct			用于更新的结构体数据，如果为“”，则清空键值数据，如果为NULL，则删除当前键项
 *		In	p_uiSizeStruct		结构体大小，如果为0时，根据p_pvStruct参数清空键值数据或删除键项
 *		In	p_pcFileName		INI文件路径
 *
 * Return ： bool				执行成功返回true，否则返回false
 *
 */
bool WritePrivateProfileStruct(
	/*In*/	const char *	p_pcSection,
	/*In*/	const char *	p_pcKey,
	/*In*/	const void *	p_pvStruct,
	/*In*/	uint32_t		p_uiSizeStruct,
	/*In*/	const char *	p_pcFileName
);

#pragma pack (pop)

#ifdef __cplusplus
}
#endif

#endif /* INIFILE_H_ */
