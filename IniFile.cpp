#include "IniFile.h"
#include "DataCode.h"
#include "General.h"

#pragma pack (push, 8)
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iconv.h>
#pragma pack(pop)

typedef std::vector<std::string>			Buffer;

#pragma pack (push, 1)


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// 裁剪左边的指定字符
static void _vTrimLeftString(const char p_cCode, std::string& p_sText)
{
	std::string l_sText = p_sText;
	p_sText.clear();

	unsigned long l_nCount = l_sText.size();
	const char * l_pcTextPtr = l_sText.data();

	if(0 == l_nCount)
		return;

	unsigned long l_nPos = 0;

	while(l_nPos < l_nCount)
	{
		// 检查到不是指定去除的字符就退出
		if(p_cCode != l_pcTextPtr[l_nPos])
			break;

		l_nPos ++;
	}

	// 调整字符
	p_sText.append(l_sText.data()+l_nPos, l_nCount-l_nPos);
}

// 裁剪右边的指定字符
static void _vTrimRighString(const char p_cCode, std::string& p_sText)
{
	std::string l_sText = p_sText;
	p_sText.clear();

	unsigned long l_nCount = l_sText.size();
	const char * l_pcTextPtr = l_sText.data();

	if(0 == l_nCount)
		return;

	while(l_nCount > 0)
	{
		// 检查到不是指定去除的字符就退出
		if(p_cCode != l_pcTextPtr[l_nCount-1])
			break;

		l_nCount --;
	}

	// 调整字符
	p_sText.append(l_sText.data(), l_nCount);
}

// 字符串比较，忽略大小进行比较
static bool _bStringCompareNoCase(const char * p_pcString1, const char * p_pcString2)
{
	if(NULL == p_pcString1 || NULL == p_pcString2)
		return false;

	// 将数据复制到string变量
	std::string l_sString1 = p_pcString1;
	std::string l_sString2 = p_pcString2;

	// 转换成小写
	std::transform(l_sString1.begin(), l_sString1.end(), l_sString1.begin(), ::tolower);
	std::transform(l_sString2.begin(), l_sString2.end(), l_sString2.begin(), ::tolower);

	// 比较字符串是否相等
	if(l_sString1 == l_sString2)
		return true;

	return false;
}

// 计算校验和
static unsigned char _ucCheckSum(const void * p_pvBuffer, uint32_t p_uSize)
{
	unsigned char l_nSumVal = 0;

	if(p_uSize == 0)
		return l_nSumVal;

	// 进行校验和计算
	for(uint32_t i = 0; i < p_uSize; i++)
		l_nSumVal += ((unsigned char *)p_pvBuffer)[i];

	return l_nSumVal;
}

// 字符串转二进制
static void _vStringToBinary(std::string& p_sBuffer)
{
	std::string l_sBuffer;

	// 如果不足两个字符到倍数，则在前面增加一个0字符
	if(p_sBuffer.size()%2 == 0)
		l_sBuffer = p_sBuffer;
	else
		l_sBuffer = "0" + p_sBuffer;

	p_sBuffer.clear();

	// 转换操作
	for(size_t i = 0; i < l_sBuffer.size(); i+=2)
	{
		std::string l_sTemp = l_sBuffer.substr(i, 2);
		unsigned char l_nTemp = strtoul(l_sTemp.c_str(), NULL, 16);
		p_sBuffer.append((char *)&l_nTemp, 1);
	}
}

// 二进制转换字符串
static void _vBinaryToString(std::string& p_sBuffer)
{
	std::string l_sBuffer = p_sBuffer;
	p_sBuffer.clear();

	size_t l_nSize = l_sBuffer.size();

	// 没有需要转换到数据
	if(0 == l_nSize)
		return;

	char l_acTemp[5];
	unsigned char * l_pcPtr = (unsigned char *)l_sBuffer.data();

	// 转换操作
	for(size_t i = 0; i < l_nSize; i++, l_pcPtr++)
	{
		snprintf(l_acTemp, 5, "%02X", *l_pcPtr);
		p_sBuffer += l_acTemp;
	}
}

// INI文件的编码转换
static bool _bDataCodeConverter(const char * p_pcFromCode, const char * p_pcToCode, std::string& p_sDataBuffer)
{
	// 检查参数是否合法
	if(NULL == p_pcToCode || NULL == p_pcFromCode)
		return false;

	// 构建编码转换对象
	iconv_t l_poIconv = iconv_open(p_pcToCode, p_pcFromCode);

	if(NULL == l_poIconv)
		return false;

	size_t l_nInSize = p_sDataBuffer.size();
	size_t l_nOutSize = p_sDataBuffer.size() * 3;

	char * l_pcOutBuf = new char[l_nOutSize];
	memset(l_pcOutBuf, 0, l_nOutSize);

	char * l_pcInPtr = (char *)p_sDataBuffer.data();
	char * l_pcOutPtr = l_pcOutBuf;

	int l_nRet = iconv(l_poIconv, &l_pcInPtr, &l_nInSize, &l_pcOutPtr, &l_nOutSize);

	// 关闭编码转换对象
	iconv_close(l_poIconv);

	if(-1 != l_nRet)
	{
		int l_nZeroCount = 0;

		for(size_t i = 0; i < l_nOutSize; i++)
		{
			if(0x00 == l_pcOutBuf[i])
				l_nZeroCount ++;
			else
				l_nZeroCount = 0;

			if(l_nZeroCount > 4)
			{
				l_nOutSize = i - 4;
				break;
			}
		}

		p_sDataBuffer.clear();
		p_sDataBuffer.append(l_pcOutBuf, l_nOutSize);
	}

	delete[] l_pcOutBuf;

	return (-1 != l_nRet ? true : false);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// 打开文件，并返回文件描述符
static int _nOpenFile(const char * p_pcFileName, bool create)
{
	if(NULL == p_pcFileName)
		return -1;

	// 如果create为假时，认为只读，否则为读写
	int flags = O_RDONLY;

	if(true == create)
		flags = O_RDWR | O_CREAT;

	int l_iHandle;

	// 加入错误码判断
	_OSCall_Try{
		l_iHandle = open(p_pcFileName, flags, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		_OSCall_Break(-1 != l_iHandle);
	}_OSCall_Finally();

	return l_iHandle;
}

// 根据文件描述符关闭文件
static int _nCloseFile(int p_nFileDescriptor)
{
	if(p_nFileDescriptor < 0)
		return -1;

	int l_nRet;

	// 加入错误码判断
	_OSCall_Try{
		l_nRet = close(p_nFileDescriptor);
		_OSCall_Break(0 == l_nRet);
	}_OSCall_Finally();

	return l_nRet;
}

// 根据文件描述符锁定文件，用于互斥操作等待
static int _nLockFile(int p_nFileDescriptor)
{
	struct flock l_sFileLock;

	l_sFileLock.l_type = F_WRLCK;
	l_sFileLock.l_start = 0;
	l_sFileLock.l_whence = SEEK_SET;
	l_sFileLock.l_len = 0;

	int l_nRet;

	// 加入错误码判断
	_OSCall_Try{
		l_nRet=fcntl(p_nFileDescriptor, F_SETLKW, &l_sFileLock);
		_OSCall_Break(-1 !=  l_nRet);
	}_OSCall_Finally();

	return l_nRet;
}

// 根据文件描述符解锁文件，将权限释放给其实使用者
static int _nUnlockFile(int p_nFileDescriptor)
{
	struct flock l_sFileLock;

	l_sFileLock.l_type = F_UNLCK;
	l_sFileLock.l_start = 0;
	l_sFileLock.l_whence = SEEK_SET;
	l_sFileLock.l_len = 0;

	int l_nRet;

	// 加入错误码判断
	_OSCall_Try{
		l_nRet=fcntl(p_nFileDescriptor, F_SETLK, &l_sFileLock);
		_OSCall_Break(-1 !=  l_nRet);
	}_OSCall_Finally();

	return l_nRet;
}

// 根据文件描述符获取文件长度
static size_t _nGetFileSize(int p_nFileDescriptor)
{
	struct stat l_sFileStat;

	if(-1 == fstat(p_nFileDescriptor, &l_sFileStat))
		return 0;

	return l_sFileStat.st_size;
}

// 从文件中获取文件数据，Buffer每项数据就是一行以回车分隔符结尾的数据
static int _nReadData(int p_nFileDescriptor, Buffer& p_oBuffer, DataCode& p_DataCodeRet)
{
	p_oBuffer.clear();

	// 获取文件长度
	size_t l_nFileSize = _nGetFileSize(p_nFileDescriptor);

	if(0 == l_nFileSize)
		return 0;

	// 移动文件指针到文件头
	lseek(p_nFileDescriptor, 0L, SEEK_SET);

	// 分配空间
	unsigned char * l_pcBuf = new unsigned char[l_nFileSize];

	if(NULL == l_pcBuf)
		return 0;

	// 批量读取文件数据
	size_t l_nReadRet;

	// 加入错误码判断
	_OSCall_Try{
		l_nReadRet = read(p_nFileDescriptor, l_pcBuf, l_nFileSize);
		_OSCall_Break(-1 != l_nReadRet);
	}_OSCall_Finally();

	// 读取文件数据长度不正确
	if(l_nReadRet != l_nFileSize)
	{
		delete[] l_pcBuf;
		return 0;
	}

	// 获取文件数据编码
	p_DataCodeRet = ParseDataCode(l_pcBuf, l_nFileSize);

	unsigned int l_nBufPos = 0;

	// 如果是Unicode编码，先转换为UTF-8编码，再往下分析
	if(FC_Little_endian_UTF16_Unicode == p_DataCodeRet ||
		FC_Big_endian_UTF16_Unicode == p_DataCodeRet)
	{
		std::string l_sTempBuffer;
		l_sTempBuffer.append((char *)l_pcBuf, l_nFileSize);

		if(FC_Little_endian_UTF16_Unicode == p_DataCodeRet)
		{
			_bDataCodeConverter("UCS-2LE", "UTF-8", l_sTempBuffer);
		}
		else if(FC_Big_endian_UTF16_Unicode == p_DataCodeRet)
		{
			_bDataCodeConverter("UCS-2BE", "UTF-8", l_sTempBuffer);
		}

		memset(l_pcBuf, 0, l_nFileSize);
		memcpy(l_pcBuf, l_sTempBuffer.data(), l_sTempBuffer.size());

		// 如果有UTF-8 BOM头，则跳过
		if(0xEF == l_pcBuf[0] && 0xBB == l_pcBuf[1] && 0xBF == l_pcBuf[2])
			l_nBufPos = 3;
	}
	else if(FC_UTF8_Unicode_withBOM == p_DataCodeRet) // 如果是UTF-8，跳过3个字节
	{
		l_nBufPos = 3;
	}

	std::string l_sLineText;

	// 分隔每行数据到Buffer缓冲
	while(l_nReadRet > 0)
	{
		l_sLineText.append((char *)&l_pcBuf[l_nBufPos], 1);

		l_nReadRet --;

		if('\n' == l_pcBuf[l_nBufPos++] || 0 == l_nReadRet)
		{
			p_oBuffer.push_back(l_sLineText);
			l_sLineText.clear();
		}
	}

	delete[] l_pcBuf;

	return p_oBuffer.size();
}

static int _nWriteData(int p_nFileDescriptor, const Buffer& p_oBuffer, const DataCode& p_DataCodeDest)
{
	int ret = 0;

	std::string l_sWriteBuffer;

	// 如果原始文件是 UTF-8 Unicode (with BOM)，则需要加上BOM头
	if(FC_UTF8_Unicode_withBOM == p_DataCodeDest)
		l_sWriteBuffer.append("\xEF\xBB\xBF", 3);

	for(Buffer::const_iterator iter = p_oBuffer.begin(); iter != p_oBuffer.end(); iter++)
		l_sWriteBuffer += (*iter).c_str();

	size_t l_nSizeBuffer = l_sWriteBuffer.size();

	if(0 == l_nSizeBuffer)
		return 0;

	// 如果原始文件是Unicode编码，则需要转换成Unicode编码
	if(FC_Little_endian_UTF16_Unicode == p_DataCodeDest)
		_bDataCodeConverter("UTF-8", "UCS-2LE", l_sWriteBuffer);
	else if(FC_Big_endian_UTF16_Unicode == p_DataCodeDest)
		_bDataCodeConverter("UTF-8", "UCS-2LB", l_sWriteBuffer);

	// 将文件长度重置为0
	ret = ftruncate(p_nFileDescriptor, 0);

	// 移动文件指针到文件头
	ret = lseek(p_nFileDescriptor, 0L, SEEK_SET);

	// 将数据写入文件
	size_t l_nWriteRet;

	// 加入错误码判断
	_OSCall_Try{
		l_nWriteRet = write(p_nFileDescriptor, l_sWriteBuffer.data(), l_sWriteBuffer.size());
		_OSCall_Break(-1 != l_nWriteRet);
	}_OSCall_Finally();

	return l_nWriteRet;
}

// 文件自动操作类，在具体功能函数里，用于调用函数时自动打开文件和锁定文件，退出函数时自动解锁文件和关闭文件
class OAutoFileOperate
{
public:
	OAutoFileOperate(const char * p_pcFileName, bool p_bCreate = false)
	: m_nFileDescriptor(-1)
	{
		// 打开文件
		m_nFileDescriptor = _nOpenFile(p_pcFileName, p_bCreate);

		// 如果打开成功则锁定文件
		if(-1 != m_nFileDescriptor)
		{
			_nLockFile(m_nFileDescriptor);
		}
	}

	virtual ~OAutoFileOperate()
	{
		// 如果已经打开文件，则先解锁然后再关闭文件
		if(-1 != m_nFileDescriptor)
		{
			// 解锁文件
			_nUnlockFile(m_nFileDescriptor);

			// 关闭文件
			_nCloseFile(m_nFileDescriptor);

			m_nFileDescriptor = -1;
		}
	}

	// 返回文件描述符
	int nGetFileDescriptor()
	{
		return m_nFileDescriptor;
	}

private:
	int m_nFileDescriptor;
};


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// 解析Section名称，如果解析到就返回true，并将数据赋值到参数p_sSectionName
static bool _bParseSection(const std::string& p_sText, std::string& p_sSectionName)
{
	p_sSectionName.clear();

	const char * l_pcBufPtr = p_sText.data();

	// 判断首字符是否符合Section名称到规则
	if('[' != *l_pcBufPtr)
		return false;

	size_t l_nSize = p_sText.size()-1;
	unsigned int l_nBufPos = 1;

	while(l_nSize > 0)
	{
		// 结束位置有可能是字符“]”，有可能是回车符
		if(']' == l_pcBufPtr[l_nBufPos] ||
		   '\r' == l_pcBufPtr[l_nBufPos] ||
		   '\n' == l_pcBufPtr[l_nBufPos])
			break;

		p_sSectionName.append((char *)&l_pcBufPtr[l_nBufPos], 1);

		l_nSize --;
		l_nBufPos ++;
	}

	// 如果获取到Section名称则返回true，否则返回false
	return (p_sSectionName.size() > 0 ? true : false);
}

// 判断是否Section的标识
static bool _bCheckSectionFlag(const std::string& p_sText)
{
	const char * l_pcBufPtr = p_sText.data();

	if('[' == *l_pcBufPtr)
		return true;

	return false;
}

// 判断是否Section下到键项
static bool _bIsSectionItem(const std::string& p_sText)
{
	const char * l_pcBufPtr = p_sText.data();

	if('[' == *l_pcBufPtr)
		return false;

	return true;
}

// 解析以\0分隔的字符串，用于WritePrivateProfileSection函数中的p_pcString参数解析，返回列表数
static int _nParseString(const char *	p_pcString, std::vector<std::string>& p_vecString)
{
	// 即使是\0为分隔，是有字符串长度
	if(0 == strlen(p_pcString))
		return 0;

	p_vecString.clear();

	// 编译地址指针
	const char * l_pString = p_pcString;

	// \0的计数
	int l_nNullCount = 0;

	std::string l_sItem;

	do
	{
		char l_chCode = *l_pString;

		l_pString ++;

		if('\0' == l_chCode)
		{
			// 遇到一个\0字符
			l_nNullCount ++;

			// 如果缓存有值就需要添加
			if(l_sItem.size() > 0)
			{
				p_vecString.push_back(l_sItem);
				l_sItem = "";
			}
		}
		else
		{
			// 有可能是一个只带一个\0结束的字符串
			if(l_nNullCount > 0)
			{
				// 如果找不到字符串里带=符号的，或者长度大于256的都认为是结束
				if(NULL == strstr(l_pString, "=") || 256 < strlen(l_pString))
				{
					break;
				}
			}

			// 复位\0的计算
			l_nNullCount = 0;
			l_sItem.append(&l_chCode, 1);
		}

	} while(l_nNullCount < 2); // 检查到有两个\0就代表结束

	return p_vecString.size();
}

// 解析Section下到键项，如果解析到就返回true，并将数据赋值到参数p_sKeyName和p_sValue
static bool _bParseSectionItem(const std::string& p_sText, std::string& p_sKeyName, std::string& p_sValue)
{
	const char * l_pcBufPtr = p_sText.data();

	while(' ' ==  *l_pcBufPtr || '\t' == *l_pcBufPtr)
		l_pcBufPtr++;

	if('[' == *l_pcBufPtr ||
		';' == *l_pcBufPtr ||
		'/' == *l_pcBufPtr ||
		 '\\' == *l_pcBufPtr ||
		 '\r' == *l_pcBufPtr ||
		 '\n' == *l_pcBufPtr)
			return false;

	size_t l_nPos = p_sText.find('=', 0);

	if(std::string::npos == l_nPos)
		return false;

	p_sKeyName = p_sText.substr(0, l_nPos);
	p_sValue = p_sText.substr(l_nPos+1, p_sText.length()-l_nPos-1);

	// 去除两边空格
	_vTrimLeftString(' ', p_sKeyName);
	_vTrimRighString(' ', p_sKeyName);

	//去除\r\n字符
	_vTrimRighString('\n', p_sValue);
	_vTrimRighString('\r', p_sValue);

	// 去除两边空格
	_vTrimLeftString(' ', p_sValue);
	_vTrimRighString(' ', p_sValue);

	// 去除两边TAB
	_vTrimLeftString('\t', p_sValue);
	_vTrimRighString('\t', p_sValue);

	// 去除两边引号
	_vTrimLeftString('\"', p_sValue);
	_vTrimRighString('\"', p_sValue);

	return true;
}

// 更新Section下到键项，将指定到值更新到键项里
static bool _bUpdateItemValue(std::string& p_sText, const std::string& p_sValue)
{
	size_t l_nPos = p_sText.find('=', 0);

	if(std::string::npos == l_nPos)
		return false;

	std::string l_sText;

	l_sText = p_sText.substr(0, l_nPos+1);
	l_sText += p_sValue;
	l_sText += "\r\n";

	p_sText = l_sText;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t GetPrivateProfileSectionNames(
	/*Out*/		char *  		p_pcReturnBuffer,
	/*In*/		uint32_t   		p_nSize,
	/*In*/		const char * 	p_pcFileName
)
{
	if(NULL == p_pcReturnBuffer || 0 == p_nSize)
		return 0;

	// 文件自动操作
	OAutoFileOperate l_oAutoFile(p_pcFileName);

	// 文件打开失败
	if(-1 == l_oAutoFile.nGetFileDescriptor())
		return 0;

	// 从文件中读取数据
	Buffer l_oBuffer;
	DataCode l_DataCodeRet;
	int l_nCount = _nReadData(l_oAutoFile.nGetFileDescriptor(), l_oBuffer, l_DataCodeRet);

	if(0 == l_nCount)
		return 0;

	uint32_t l_nRetSize = 0;
	uint32_t l_nResidueSize = p_nSize;
	char * l_pcBufferPtr = p_pcReturnBuffer;

	// 获取Section名称列表，以\0为分隔，\0\0为结尾
	for(Buffer::const_iterator iter = l_oBuffer.begin(); iter != l_oBuffer.end(); iter++)
	{
		std::string l_sSectionName;

		// 判断是否Section名称
		if(_bParseSection((*iter), l_sSectionName))
		{
			// 修正复制长度，以防越界
			size_t l_nCopySize = l_sSectionName.size();

			if(l_nCopySize > l_nResidueSize)
				l_nCopySize = l_nResidueSize;

			// 复制数据
			memcpy(l_pcBufferPtr, l_sSectionName.data(), sizeof(char)*l_nCopySize);

			l_pcBufferPtr += l_nCopySize;
			l_nResidueSize -= l_nCopySize;
			l_nRetSize += l_nCopySize;

			// 增加间隔符 \0
			if(l_nResidueSize > 0)
			{
				*l_pcBufferPtr = '\0';

				l_pcBufferPtr ++;
				l_nResidueSize --;
				l_nRetSize ++;
			}
		}
	}

	// 增加结尾符 \0
	if(l_nResidueSize > 0)
	{
		*l_pcBufferPtr = '\0';

		l_pcBufferPtr ++;
		l_nResidueSize --;
		l_nRetSize ++;
	}

	return l_nRetSize;
}

uint32_t GetPrivateProfileSection(
	/*In*/		const char * 	p_pcAppName,
	/*Out*/		char *  		p_pcReturnedString,
	/*In*/		uint32_t  		p_nSize,
	/*In*/		const char * 	p_pcFileName
)
{
	if(NULL == p_pcAppName || NULL == p_pcReturnedString || 0 == p_nSize)
		return 0;

	// 文件自动操作
	OAutoFileOperate l_oAutoFile(p_pcFileName);

	// 文件打开失败
	if(-1 == l_oAutoFile.nGetFileDescriptor())
		return 0;

	// 从文件中读取数据
	Buffer l_oBuffer;
	DataCode l_DataCodeRet;
	int l_nCount = _nReadData(l_oAutoFile.nGetFileDescriptor(), l_oBuffer, l_DataCodeRet);

	if(0 == l_nCount)
		return 0;

	uint32_t l_nRetSize = 0;
	uint32_t l_nResidueSize = p_nSize;
	char * l_pcBufferPtr = p_pcReturnedString;

	for(Buffer::const_iterator iter = l_oBuffer.begin(); iter != l_oBuffer.end(); iter++)
	{
		std::string l_sSectionName;

		// 解析Section名称
		if(_bParseSection((*iter), l_sSectionName))
		{
			// 进行不区分大小写地比较Section名称
			if(_bStringCompareNoCase(p_pcAppName, l_sSectionName.c_str()))
			{
				for(Buffer::const_iterator iter_sub = ++iter; iter_sub != l_oBuffer.end(); iter_sub++)
				{
					std::string l_sSectionItem = (*iter_sub);

					// 判断是否属于Section的子项
					if(!_bIsSectionItem(l_sSectionItem))
						break;

					// 去除右边字符到回车符
					_vTrimRighString('\n', l_sSectionItem);
					_vTrimRighString('\r', l_sSectionItem);

					// 修正复制长度，以防越界
					size_t l_nCopySize = l_sSectionItem.size();

					if(l_nCopySize > l_nResidueSize)
						l_nCopySize = l_nResidueSize;

					// 复制数据
					memcpy(l_pcBufferPtr, l_sSectionItem.data(), sizeof(char)*l_nCopySize);

					l_pcBufferPtr += l_nCopySize;
					l_nResidueSize -= l_nCopySize;
					l_nRetSize += l_nCopySize;

					// 增加间隔符 \0
					if(l_nResidueSize > 0)
					{
						*l_pcBufferPtr = '\0';

						l_pcBufferPtr ++;
						l_nResidueSize --;
						l_nRetSize ++;
					}
				}

				// 增加结尾符 \0
				if(l_nResidueSize > 0)
				{
					*l_pcBufferPtr = '\0';

					l_pcBufferPtr ++;
					l_nResidueSize --;
					l_nRetSize ++;
				}

				break;
			}
		}
	}

	return l_nRetSize;
}

bool _GetPrivateProfileItemValue(const Buffer& p_oBuffer, const char * p_pcAppName, const char * p_pcKeyName, std::string& p_sValue)
{
	for(Buffer::const_iterator iter = p_oBuffer.begin(); iter != p_oBuffer.end(); iter++)
	{
		std::string l_sSectionName;

		// 解析Section名称
		if(_bParseSection((*iter), l_sSectionName))
		{
			// 进行不区分大小写地比较Section名称
			if(_bStringCompareNoCase(p_pcAppName, l_sSectionName.c_str()))
			{
				for(Buffer::const_iterator iter_sub = ++iter; iter_sub != p_oBuffer.end(); iter_sub++)
				{
					std::string l_sSectionItem = (*iter_sub);

					// 判断是否属于Section的子项
					if(!_bIsSectionItem(l_sSectionItem))
						break;

					std::string l_sKeyName;
					std::string l_sValue;

					// 解析键名称和键值数据
					if(_bParseSectionItem(l_sSectionItem, l_sKeyName, l_sValue))
					{
						// 比较键名称是否存在
						if(_bStringCompareNoCase(l_sKeyName.c_str(), p_pcKeyName))
						{
							// 复制键值数据
							p_sValue = l_sValue;

							// 跳出查找循环
							return true;
						}
					}
				}

				break;
			}
		}
	}

	return false;
}

uint32_t GetPrivateProfileString(
	/*In*/		const char * 	p_pcAppName,
	/*In*/		const char * 	p_pcKeyName,
	/*In*/		const char * 	p_pcDefault,
	/*Out*/		char *  		p_pcReturnedString,
	/*In*/		uint32_t  		p_nSize,
	/*In*/		const char * 	p_pcFileName
)
{
	if(NULL == p_pcAppName || NULL == p_pcKeyName || NULL == p_pcReturnedString || 0 == p_nSize)
		return 0;

	// 文件自动操作
	OAutoFileOperate l_oAutoFile(p_pcFileName);

	// 文件打开失败
	if(-1 == l_oAutoFile.nGetFileDescriptor())
		return 0;

	// 从文件中读取数据
	Buffer l_oBuffer;
	DataCode l_DataCodeRet;
	int l_nCount = _nReadData(l_oAutoFile.nGetFileDescriptor(), l_oBuffer, l_DataCodeRet);

	if(0 == l_nCount)
		return 0;

	uint32_t l_nRetSize = 0;

	// 如果存在默认值，先设置默认值
	if(p_pcDefault)
	{
		// 复制缓冲长度预处理，以防复制过程溢出
		size_t l_nDefSize = strlen(p_pcDefault);

		if(l_nDefSize > p_nSize)
			l_nDefSize = p_nSize;

		// 复制前进行初始化缓冲
		memset(p_pcReturnedString, 0, p_nSize);

		// 复制默认值到缓存中
		memcpy(p_pcReturnedString, p_pcDefault, l_nDefSize);

		// 设置默认值的长度
		l_nRetSize = l_nDefSize;
	}

	std::string l_sValue;

	// 查找键项，如果存在则复制键值数据
	if(_GetPrivateProfileItemValue(l_oBuffer, p_pcAppName, p_pcKeyName, l_sValue))
	{
		// 初始化返回缓冲
		memset(p_pcReturnedString, 0, p_nSize);

		// 复制缓冲长度预处理，以防复制过程溢出
		size_t l_nCopySize = l_sValue.size();

		if(l_nCopySize > p_nSize)
		l_nCopySize = p_nSize;

		// 复制键值数据
		memcpy(p_pcReturnedString, l_sValue.data(), l_nCopySize);

		// 返回数据长度
		l_nRetSize = l_nCopySize;
	}

	return l_nRetSize;
}

uint32_t GetPrivateProfileInt(
	/*In*/		const char *	p_pcAppName,
	/*In*/		const char *	p_pcKeyName,
	/*In*/		int32_t			p_nDefault,
	/*In*/		const char * 	p_pcFileName
)
{
	if(NULL == p_pcAppName || NULL == p_pcKeyName)
		return 0;

	uint32_t l_nRetValue = p_nDefault;

	// 文件自动操作
	OAutoFileOperate l_oAutoFile(p_pcFileName);

	// 文件打开失败
	if(-1 == l_oAutoFile.nGetFileDescriptor())
		return l_nRetValue;

	// 从文件中读取数据
	Buffer l_oBuffer;
	DataCode l_DataCodeRet;
	int l_nCount = _nReadData(l_oAutoFile.nGetFileDescriptor(), l_oBuffer, l_DataCodeRet);

	if(0 == l_nCount)
		return l_nRetValue;

	std::string l_sValue;

	// 查找键项，如果存在则复制键值数据
	if(_GetPrivateProfileItemValue(l_oBuffer, p_pcAppName, p_pcKeyName, l_sValue))
	{
		// 如果没有值，则使用默认值
		if(l_sValue.length() == 0)
			return l_nRetValue;

		l_nRetValue = strtol(l_sValue.c_str(), NULL, 10);
	}

	return l_nRetValue;
}

bool GetPrivateProfileStruct(
	/*In*/		const char *	p_pcSection,
	/*In*/		const char *	p_pcszKey,
	/*Out*/		const void *	p_pcStruct,
	/*In*/		uint32_t		p_uiSizeStruct,
	/*In*/		const char *	p_pcFileName
)
{
	if(NULL == p_pcSection || NULL == p_pcszKey || NULL == p_pcStruct || 0 == p_uiSizeStruct)
		return false;

	// 文件自动操作
	OAutoFileOperate l_oAutoFile(p_pcFileName);

	// 文件打开失败
	if(-1 == l_oAutoFile.nGetFileDescriptor())
		return false;

	// 从文件中读取数据
	Buffer l_oBuffer;
	DataCode l_DataCodeRet;
	int l_nCount = _nReadData(l_oAutoFile.nGetFileDescriptor(), l_oBuffer, l_DataCodeRet);

	if(0 == l_nCount)
		return false;

	std::string l_sValue;

	// 查找键项，如果存在则复制键值数据
	if(!_GetPrivateProfileItemValue(l_oBuffer, p_pcSection, p_pcszKey, l_sValue))
		return false;

	// 将字符串转成二进制
	_vStringToBinary(l_sValue);

	// 复制缓冲长度预处理，以防复制过程溢出
	uint32_t l_nCopySize = l_sValue.size()-1;

	if(l_nCopySize > p_uiSizeStruct)
		l_nCopySize = p_uiSizeStruct;

	// 复制转换后到到数组缓存
	memcpy((char *)p_pcStruct, l_sValue.data(), l_nCopySize);

	// 计算校验和
	unsigned char l_nCheckSum = _ucCheckSum(l_sValue.data(), l_sValue.size()-1);

	// 比较校验和是否正确，如果正确返回true，否则返回false
	if(l_nCheckSum != (unsigned char)*(l_sValue.data()+l_sValue.size()-1))
		return false;

	return true;
}

bool WritePrivateProfileSection(
	/*In*/		const char *	p_pcAppName,
	/*In*/		const char *	p_pcString,
	/*In*/		const char *	p_pcFileName
)
{
	if(NULL == p_pcAppName)
		return false;

	// 文件自动操作
	OAutoFileOperate l_oAutoFile(p_pcFileName, true);

	// 文件打开失败
	if(-1 == l_oAutoFile.nGetFileDescriptor())
		return false;

	// 从文件中读取数据
	Buffer l_oReadBuffer;
	DataCode l_DataCodeRet;
	int l_nCount = _nReadData(l_oAutoFile.nGetFileDescriptor(), l_oReadBuffer, l_DataCodeRet);

	// section找到与否标志
	bool l_bFoundSection = false;

	//  写入缓冲
	Buffer l_oWriteBuffer;

	if (l_nCount > 0)
	{
		// 查找循环
		for (Buffer::iterator iter = l_oReadBuffer.begin(); iter != l_oReadBuffer.end(); iter++)
		{
			if (iter >= l_oReadBuffer.end())
				break;

			// 优先添加
			l_oWriteBuffer.push_back((*iter).c_str());

			std::string l_sSectionName;

			// 解析Section名称
			if (_bParseSection((*iter), l_sSectionName))
			{
				// 进行不区分大小写地比较Section名称
				if (_bStringCompareNoCase(p_pcAppName, l_sSectionName.c_str()))
				{
					l_bFoundSection = true;

					if (NULL == p_pcString) // 此操作为删除整块Section段
					{
						// 删除刚刚添加到Section头
						l_oWriteBuffer.erase(l_oWriteBuffer.end() - 1);
					}
					else // 此操作为初始化整块Section段到子项
					{
						// 解析\0分隔的字符串
						std::vector<std::string> l_vecString;
						int l_nStrings = _nParseString(p_pcString, l_vecString);

						if(l_nStrings > 0)
						{
							std::string l_sString;
							l_sString = "";

							// 将字符串列表格式化为\r\n形式的字符串
							for(std::vector<std::string>::iterator val_iter = l_vecString.begin(); val_iter != l_vecString.end(); val_iter++)
							{
								l_sString += (*val_iter);
								l_sString += "\r\n";
							}

							l_oWriteBuffer.push_back(l_sString);
						}
					}

					iter++;

					// 清除Section下的所有子项
					for (; iter != l_oReadBuffer.end(); iter++)
					{
						std::string l_sSectionItem = (*iter);

						// 判断是否属于Section的子项
						if (!_bIsSectionItem(l_sSectionItem))
							break;
					}

					iter--;
				}
			}
		}
	}

	// 如果找不到则在文件末尾写入新的Section项
	if(false == l_bFoundSection)
	{
		if(NULL != p_pcString)
		{
			std::string l_sSection;

			l_sSection += "[";
			l_sSection += p_pcAppName;
			l_sSection += "]";
			l_sSection += "\r\n";

			// 解析\0分隔的字符串
			std::vector<std::string> l_vecString;
			int l_nStrings = _nParseString(p_pcString, l_vecString);

			if (l_nStrings > 0)
			{
				// 将字符串列表格式化为\r\n形式的字符串
				for (std::vector<std::string>::iterator val_iter = l_vecString.begin(); val_iter != l_vecString.end(); val_iter++)
				{
					l_sSection += (*val_iter);
					l_sSection += "\r\n";
				}
			}

			l_oWriteBuffer.push_back(l_sSection);
		}
	}

	// 将缓冲写入文件
	int l_nWirteRet = _nWriteData(l_oAutoFile.nGetFileDescriptor(), l_oWriteBuffer, l_DataCodeRet);

	if(0 == l_nWirteRet)
		return false;

	return true;
}

bool _bWritePrivateProfileValue(const Buffer& p_oReadBuffer, Buffer& p_oWriteBuffer,
		const char * p_pcAppName, const char * p_pcKeyName, const char *p_pcString)
{
	p_oWriteBuffer.clear();

	/*
	 *  l_bFoundSection : 用于标识section头有没有找到
	 *  l_bFoundSectionItem ： 用于标识section的项有没有找到
	 *  此两个变量是用于标记本次函数调用时的查找结果标记，不是每次循环的查找标记
	 */
	bool l_bFoundSection = false;
	bool l_bFoundSectionItem = false;

	// 在缓冲的末尾
	bool l_bBufferLast = false;

	// 查找Section循环
	for(Buffer::const_iterator iter = p_oReadBuffer.begin(); iter != p_oReadBuffer.end(); iter++)
	{
		//  防止越界
		if(iter >= p_oReadBuffer.end())
			break;

		p_oWriteBuffer.push_back((*iter));

		// 如果已经找到了需要更新的项，就不需要再找了
		if(l_bFoundSectionItem)
			continue;

		std::string l_sSectionName;

		// 解析Section名称
		if(!_bParseSection((*iter), l_sSectionName))
			continue;

		// 进行不区分大小写地比较Section名称
		if(!_bStringCompareNoCase(p_pcAppName, l_sSectionName.c_str()))
			continue;

		l_bFoundSection = true;

		// 可能已经是最后一行了
		if((iter+1) == p_oReadBuffer.end())
		{
			// 删除刚刚添加的Section头
			if(NULL == p_pcKeyName)
				p_oWriteBuffer.erase(p_oWriteBuffer.end()-1);

			// 到达缓冲的末尾
			l_bBufferLast = true;

			break;
		}

		// 找到Section名称， 指针后移一行
		iter ++;

		// 此操作为删除整块Section段
		if(NULL == p_pcKeyName)
		{
			// 删除刚刚添加的Section头
			p_oWriteBuffer.erase(p_oWriteBuffer.end()-1);

			// 清除Section下的所有子项
			do
			{
				// 缓冲末尾
				if((iter+1) == p_oReadBuffer.end())
				{
					// 到达缓冲的末尾
					l_bBufferLast = true;

					break;
				}

				std::string l_sSectionItem = (*iter);

				// 遇到Section标识
				if(_bCheckSectionFlag(l_sSectionItem))
				{
					iter --;
					break;
				}

				iter ++;
			}while(iter != p_oReadBuffer.end());

			continue;
		} // if(NULL == p_pcKeyName)

		// Section头已经找到，进入查找键项子循环
		do
		{
			std::string l_sSectionItem = (*iter);

			p_oWriteBuffer.push_back(l_sSectionItem);

			// 遇到Section标记退出循环
			if(_bCheckSectionFlag(l_sSectionItem))
				break;

			std::string l_sKeyName;
			std::string l_sValue;

			// 解析键名称和键值数据
			if(_bParseSectionItem(l_sSectionItem, l_sKeyName, l_sValue))
			{
				// 找到键名称的处理
				if(_bStringCompareNoCase(p_pcKeyName, l_sKeyName.c_str()))
				{
					// 找到键项，将标识置为true
					l_bFoundSectionItem = true;

					// 删除刚刚添加的键项
					p_oWriteBuffer.erase(p_oWriteBuffer.end()-1);

					//  插入新的键项
					if(NULL != p_pcString)
					{
						// 更新键项到的键值
						_bUpdateItemValue(l_sSectionItem, p_pcString);

						// 重新添加到缓冲中
						p_oWriteBuffer.push_back(l_sSectionItem);
					}

					// 删除或更新键项，退出查找键项循环
					break;
				}

				if((iter+1) == p_oReadBuffer.end())
				{
					// 到达缓冲的末尾
					l_bBufferLast = true;

					break;
				}
			}

			iter++;
		} while(iter != p_oReadBuffer.end());

		// 找到Section标识，找不到键项
		if(false == l_bFoundSectionItem)
		{
			l_bFoundSectionItem = true;

			// 创建一个键项
            if(NULL != p_pcString)
            {
                std::string l_sSectionItemNew = p_pcKeyName;
                l_sSectionItemNew += "=";
                l_sSectionItemNew += p_pcString;
                l_sSectionItemNew += "\r\n";

                if(l_bBufferLast) // 直接添加
                    p_oWriteBuffer.push_back(l_sSectionItemNew);
                else // 插入键项
                    p_oWriteBuffer.insert(p_oWriteBuffer.end()-1, l_sSectionItemNew);
            }
		}
	}

	// 如果找不到键项，新建一个Section块并且加入键项
	if(false == l_bFoundSectionItem)
	{
		if (NULL != p_pcKeyName && NULL != p_pcString)
		{
			// 如果找不到section头才添加，有可能在末尾行找到section头，但没有子项
			if (false == l_bFoundSection)
			{
				std::string l_sSectionName = "[";
				l_sSectionName += p_pcAppName;
				l_sSectionName += "]\r\n";

				p_oWriteBuffer.push_back(l_sSectionName);
			}

			std::string l_sSectionItem;

			// 有键值才会添加但当前section中
			if (strlen(p_pcKeyName) > 0)
			{
				l_sSectionItem = p_pcKeyName;
				l_sSectionItem += "=";

				// 添加键值数据
				if (strlen(p_pcString) > 0)
				{
					l_sSectionItem += p_pcString;
				}

				l_sSectionItem += "\r\n";

				// 加入缓冲中
				p_oWriteBuffer.push_back(l_sSectionItem);
			}
		}
	}

	return true;
}

bool WritePrivateProfileString(
	/*In*/	const char *	p_pcAppName,
	/*In*/	const char *	p_pcKeyName,
	/*In*/	const char *	p_pcString,
	/*In*/	const char *	p_pcFileName
)
{
	if(NULL == p_pcAppName)
		return false;

	OAutoFileOperate l_oAutoFile(p_pcFileName, true);

	if(-1 == l_oAutoFile.nGetFileDescriptor())
		return false;

	// 从文件中读取数据
	Buffer l_oReadBuffer;
	DataCode l_DataCodeRet;
	int l_nCount = _nReadData(l_oAutoFile.nGetFileDescriptor(), l_oReadBuffer, l_DataCodeRet);

	Buffer l_oWriteBuffer;

	if(0 <= l_nCount)
	{
		// 根据参数更新配置项
		if(!_bWritePrivateProfileValue(l_oReadBuffer, l_oWriteBuffer, p_pcAppName, p_pcKeyName, p_pcString))
			return false;
	}

	// 将缓冲写入文件
	int l_nWirteRet = _nWriteData(l_oAutoFile.nGetFileDescriptor(), l_oWriteBuffer, l_DataCodeRet);

	if(0 == l_nWirteRet)
		return false;

	return true;
}

bool WritePrivateProfileStruct(
	/*In*/	const char *	p_pcSection,
	/*In*/	const char *	p_pcKey,
	/*In*/	const void *	p_pvStruct,
	/*In*/	uint32_t		p_uiSizeStruct,
	/*In*/	const char *	p_pcFileName
)
{
	if(NULL == p_pcSection)
		return false;

	OAutoFileOperate l_oAutoFile(p_pcFileName, true);

	if(-1 == l_oAutoFile.nGetFileDescriptor())
		return false;

	// 从文件中读取数据
	Buffer l_oReadBuffer;
	DataCode l_DataCodeRet;
	int l_nCount = _nReadData(l_oAutoFile.nGetFileDescriptor(), l_oReadBuffer, l_DataCodeRet);

	char * l_pcString = NULL;
	Buffer l_oWriteBuffer;

	if (0 <= l_nCount)
	{
		if (0 == p_uiSizeStruct)
		{
			// 此处是键值数据置空
			if (NULL != p_pvStruct)
			{
				l_pcString = new char[3];
				strcpy(l_pcString, "00");
			}
		}
		else
		{
			// 计算校验和
			unsigned char l_nCheckSub = _ucCheckSum((char *) p_pvStruct, p_uiSizeStruct);

			// 有键值数据时，先添加到缓冲
			std::string l_sBuffer;
			l_sBuffer.append((char *) p_pvStruct, p_uiSizeStruct);
			l_sBuffer.append((char *) &l_nCheckSub, 1);

			// 将二进制转换为字符串
			_vBinaryToString(l_sBuffer);

			//复制数据
			l_pcString = new char[l_sBuffer.length() + 1];
			strcpy(l_pcString, l_sBuffer.c_str());
		}

		// 更新键项
		if (!_bWritePrivateProfileValue(l_oReadBuffer, l_oWriteBuffer, p_pcSection, p_pcKey, l_pcString))
		{
			if (l_pcString)
				delete[] l_pcString;

			return false;
		}
	}

	// 将缓冲写入文件
	int l_nWirteRet = _nWriteData(l_oAutoFile.nGetFileDescriptor(), l_oWriteBuffer, l_DataCodeRet);

	if(0 == l_nWirteRet)
	{
		if(l_pcString)
			delete[] l_pcString;

		return false;
	}

	if(l_pcString)
		delete[] l_pcString;

	return true;
}

#pragma pack (pop)

