#include "OIniFile.h"
#include "IniFile.h"

#pragma pack (push, 8)
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#pragma pack(pop)

OIniFile::OIniFile()
{
	m_bOpenOK = FALSE;
	memset(m_acFileName, 0, sizeof(m_acFileName));
}

OIniFile::~OIniFile()
{

}

int OIniFile::iLoadFile(char *p_pcFileName)
{
	if(NULL == p_pcFileName)
		return FAIL;

	if(0 > access(p_pcFileName, R_OK))
		return FAIL;

	m_bOpenOK = TRUE;
	snprintf(m_acFileName, sizeof(m_acFileName) - 1, "%s", p_pcFileName);

	return SUCCESS;
}

BOOL OIniFile::bIsOpenOK()
{
	return m_bOpenOK;
}

int OIniFile::iGetIntValue(const char *p_pcSection, const char *p_pcKeyName, int& p_iKeyValue)
{
	if(FALSE == m_bOpenOK)
		return FAIL;

	if(NULL == p_pcSection)
		return FAIL;

	if(strlen(m_acFileName) == 0)
		return FAIL;

	char l_acBuffer[256];
	memset(l_acBuffer, 0, sizeof(l_acBuffer));

	uint32_t l_nSize = GetPrivateProfileString(p_pcSection, p_pcKeyName, NULL, l_acBuffer, 256, m_acFileName);

	if(l_nSize == 0)
		return FAIL;

	p_iKeyValue = strtol(l_acBuffer, NULL, 10);

	return SUCCESS;
}

int OIniFile::iGetStringValue(const char *p_pcSection, const char *p_pcKeyName, char *p_pcKeyValue, UINT& p_uiLength)
{
	if(FALSE == m_bOpenOK)
		return FAIL;

	if(NULL == p_pcSection)
		return FAIL;

	if(strlen(m_acFileName) == 0)
		return FAIL;

	uint32_t l_nSize = p_uiLength;
	p_uiLength = 0;

	l_nSize = GetPrivateProfileString(p_pcSection, p_pcKeyName, NULL, p_pcKeyValue, l_nSize, m_acFileName);

	if(l_nSize == 0)
		return FAIL;

	p_uiLength = l_nSize;
	
	return SUCCESS;
}

int OIniFile::iSetIntValue(const char *p_pcSection, const char *p_pcKeyName, long p_lKeyValue)
{
	if(FALSE == m_bOpenOK)
		return FAIL;

	if(NULL == p_pcSection)
		return FAIL;

	if(strlen(m_acFileName) == 0)
		return FAIL;

	char l_acValue[100];
	memset(l_acValue, 0, 100);
	snprintf(l_acValue, sizeof(l_acValue) - 1, "%ld", p_lKeyValue);

	bool l_bRet = WritePrivateProfileString(p_pcSection, p_pcKeyName, l_acValue, m_acFileName);

	return (l_bRet ? SUCCESS : FAIL);
}

int OIniFile::iSetStringValue(const char *p_pcSection, const char *p_pcKeyName, char *p_pcKeyValue, UINT p_uiLength)
{
	if(FALSE == m_bOpenOK)
		return FAIL;

	if(NULL == p_pcSection)
		return FAIL;

	if(strlen(m_acFileName) == 0)
		return FAIL;

	bool l_bRet = WritePrivateProfileString(p_pcSection, p_pcKeyName, p_pcKeyValue, m_acFileName);

	return (l_bRet ? SUCCESS : FAIL);
}

BOOL OIniFile::bFindSections(char *p_pcSection)
{
	if(FALSE == m_bOpenOK)
		return FALSE;

	if(NULL == p_pcSection)
		return FALSE;

	if(strlen(m_acFileName) == 0)
		return FALSE;

	size_t l_nSize = 10000UL;

	char * l_pcBuffer = new char [l_nSize];

	l_nSize = GetPrivateProfileSectionNames(l_pcBuffer, l_nSize, m_acFileName);

	if(l_nSize > 0)
	{
		std::string l_sStr1 = p_pcSection;
		std::transform(l_sStr1.begin(), l_sStr1.end(), l_sStr1.begin(), ::tolower);

		std::string l_sStr2;

		for(size_t i = 0; i < l_nSize; i++)
		{
			if(0x00 == l_pcBuffer[i])
			{
				std::transform(l_sStr2.begin(), l_sStr2.end(), l_sStr2.begin(), ::tolower);

				if(l_sStr1 == l_sStr2)
				{
					delete[] l_pcBuffer;
					return TRUE;
				}

				l_sStr2.clear();
			}
			else
			{
				l_sStr2.append(&l_pcBuffer[i], 1);
			}
		}
	}

	delete[] l_pcBuffer;

	return FALSE;
}

int OIniFile::iDeleteSection(const char *p_pcSection)
{
	if(FALSE == m_bOpenOK)
		return FAIL;

   if(NULL == p_pcSection)
		return FAIL;

   if(strlen(m_acFileName) == 0)
		return FAIL;

   char l_acbuffer[256];

   unsigned long l_dwGeted = GetPrivateProfileSection(p_pcSection, l_acbuffer, 256*sizeof(char), m_acFileName);

   if (l_dwGeted == 0)
   {
      return SUCCESS;
   }

   bool l_bRet = WritePrivateProfileSection(p_pcSection, "", m_acFileName);

   if(!l_bRet)
       return FAIL;

   return SUCCESS;
}

int OIniFile::iDeleteKey(const char* p_pcSection, const char* p_pcKeyName)
{
	if(FALSE == m_bOpenOK)
		return FAIL;

   if(NULL == p_pcSection || NULL == p_pcKeyName)
		return FAIL;

	if(strlen(m_acFileName) == 0)
		return FAIL;

   bool l_bRet = WritePrivateProfileString(p_pcSection, p_pcKeyName, NULL, m_acFileName);

   if(!l_bRet)
      return FAIL;

   return SUCCESS;
}





















