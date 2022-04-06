
#ifndef _INI_OFILE_H_V001
#define _INI_OFILE_H_V001

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

//#include "typedef.h"
#include "linux_commonporting.h"
#define  MAX_CFG_LENGTH	256
enum CFG_DATA_TYPE
{
	TYPE_SECTION = 1,
	TYPE_KEY,
	TYPE_VALUE,
	TYPE_COMMENT,
	TYPE_OTHERS,
};

struct tGrgCfgData
{
	CFG_DATA_TYPE eType;
	char acData[MAX_CFG_LENGTH];
	int iValidLength;

	char acSection[MAX_CFG_LENGTH];
	char acSectionComment[MAX_CFG_LENGTH];
	char acKeyName[MAX_CFG_LENGTH];
	char acValue[MAX_CFG_LENGTH];
	char acComment[MAX_CFG_LENGTH];
	int  iValue;
};

// typedef list<tGrgCfgData> MYLIST;

class OIniFile
{
public:
	OIniFile();
	virtual ~OIniFile();

	int iLoadFile(char *p_pcFileName);

	BOOL bIsOpenOK();

	int iGetIntValue(const char *p_pcSection, const char *p_pcKeyName, int& p_iKeyValue);
	int iGetStringValue(const char *p_pcSection, const char *p_pcKeyName, char *p_pcKeyValue, UINT& p_uiLength);

    int iSetIntValue(const char *p_pcSection, const char *p_pcKeyName, long p_lKeyValue);
    int iSetStringValue(const char *p_pcSection, const char *p_pcKeyName, char *p_pcKeyValue, UINT p_uiLength);

	BOOL bFindSections(char *p_pcSection);

	int iDeleteSection(const char *p_pcSection);
    int iDeleteKey(const char* p_pcSection, const char* p_pcKeyName);

private:
    BOOL m_bOpenOK;
	char	m_acFileName[MAX_PATH];
};

#endif // _INI_OFILE_H_V001
