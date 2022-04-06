#ifndef LOG_GENERAL_H
#define LOG_GENERAL_H

#include "linux_commonporting.h"
//#include "DevDriverLog.h"

#define GRG_CFG_SECTION_NAME				"#!"

#define GRG_LOG_PATH_NAME					"DevDriverLog/"
#define GRG_LOG_FILE_NAME						"log"
#define GRG_LOG_EXT_NAME						".log"

#define GRG_LOG_DEVBASE_DEFAULT			"commlog.txt"
#define GRG_INI_LOG_CFG_FILE_NAME		"GrgLogServer.cfg"

#define GRG_MIN_LOG_COUNT					100

//配置日志路径长度用到
#define LIMIT_PATH_LEN			(PATH_MAX-25)

#pragma pack (push, 8)
#include <vector>
#include <string>
#pragma pack(pop)

#pragma pack(push,1)

#ifdef __cplusplus
extern "C" {
#endif

struct CLogData
{
	ULONG		CheckSum;
	ULONG		Size;
	ULONG		Version;
	ULONG		Level;
	ULONG		Flags;
	SYSTEMTIME	Time;
	ULONG		Length;
	CHAR*		Buffer;
};

//#define _OSCall_Try 	 	do{ int __itryRepeat = 3; do{
//#define _OSCall_Finally()  if(__itryRepeat-- <= 0) break; }while(((EINTR == errno) || (EAGAIN == errno))); }while(0)

#define _OSCall_Try 	 	do{
#define _OSCall_Finally()   usleep(1000);}while(((EINTR == errno) || (EAGAIN == errno)))
#define _OSCall_Break(cond) if(cond) break;

// 比较系统滴嗒时间,支持一次反转
#define COMPARE_TICKTIME(usStart,usEnd) 	((usStart > usEnd) ? (0xFFFFFFFFUL - usStart + usEnd) : (usEnd - usStart))

// format
ULONG FormatMsg(void* buffer, CLogData* data);

/* =========================== config file ========================= */

int GetCfgFilePath(char * pathname, unsigned int max_len);

/* =========================== date time ========================= */
// 获取当前时间
void GetLocalTime(SYSTEMTIME *localTime);

// 计算两个时间之间的相差天数
int DifferDay(SYSTEMTIME Time1, SYSTEMTIME Time2);

// 判断当前日期是否同一天
BOOL SameDay(SYSTEMTIME Time1, SYSTEMTIME Time2);

// 判断 time1 是不是 time2 以前的日期
BOOL LessDay(SYSTEMTIME Time1, SYSTEMTIME Time2);

// 转换时间
void TransitionTime(time_t second, unsigned long milliseconds, SYSTEMTIME *time);

/* =========================== system ========================= */
int DiskGetFreeSpace(LPCTSTR lpRootPathName);

typedef struct tLogFileInfo
{
	time_t modify;
	std::string filepath;

	bool operator > (const tLogFileInfo& rhs) const
	{
		return modify > rhs.modify;
	}

	bool operator < (const tLogFileInfo& rhs) const
	{
		return modify < rhs.modify;
	}
} tLogFileInfo;

// 获取指定目录下的文件列表
void GetLogFileList(LPCTSTR lpPathName,  LPCTSTR pattern,  std::vector<tLogFileInfo>& vecFiles);

// 备份日志文件
bool BackupLogFile(const char * filename);

// 删除陈旧的日志备份包
bool DeleteOutOfDateLogFile(const char * filename, unsigned long logdays);

// 获取记录日志文件的最早时间
int GetLogFileEarliestEime(const char * filename);

// 删除当前配置逻辑的所有日志
void DeleteModuleAllLogFile(const char * filename);

bool ClearEarlyLogFile(LPCTSTR lpPathName, ULONGLONG p_ulMinFreeSpace, unsigned int p_nMinLogCount);

bool is_dir(const char* path);
bool is_file(const char* path);
bool is_special_dir(const char* path);

void GetFullPath(const char* path, char* fullpath);

BOOL MakeDirectory(LPCTSTR path);
void MakePath(char* pathfile, const char* l_acDir, const char* l_acFile, const char* l_acExt);
void SplitPath(const char* pathfile, char* l_acDir, char* l_acFile, char* l_acExt);

int iGetModuleFileName(char *p_pcFileName, int p_iNameSize);

int iCreateDirectory(char *p_pcFileName);

void SetDuiteConfigDirPath(const char * p_dir); //设置Dunite配置文件路径
const char* GetDuiteConfigDirPath(void); // 获取自定义Dunite配置文件路径

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif // LOG_GENERAL_H
