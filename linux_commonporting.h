#ifndef LINUX_COMMONPORTING_H_
#define LINUX_COMMONPORTING_H_

#pragma pack (push, 8)
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#pragma pack(pop)

#ifndef __packed
#define __packed        __attribute__ ((__packed__))
#endif
/********************************************************************************************************/
/*	类型定义区																						    */
/********************************************************************************************************/
#ifndef CONST
#define CONST 					const
#endif

typedef int				BOOL;
typedef unsigned char 	BYTE;
typedef BYTE			byte;
typedef char 			CHAR;
typedef unsigned short 	WORD;
typedef unsigned int 	DWORD;
typedef int 			INT;
typedef unsigned int 	UINT;
typedef long 			LONG;
typedef long long       LONGLONG;
typedef unsigned long 	ULONG;
typedef wchar_t 		WCHAR;
typedef double          DOUBLE;
#define VOID 			void
typedef VOID			*PVOID;
typedef VOID            *LPVOID;
typedef size_t 			SIZE_T;
typedef BOOL			*LPBOOL;
typedef BYTE            *LPBYTE;
typedef CHAR 			*LPSTR;
typedef const CHAR 	 	*LPCSTR;
typedef WCHAR 			*LPWSTR;
typedef const WCHAR 	*LPCWSTR;
#ifdef UNICODE
typedef WCHAR           TCHAR;
#else
typedef CHAR            TCHAR;
#endif
typedef TCHAR           *LPTSTR;
typedef const TCHAR     *LPCTSTR;
typedef LPVOID			HANDLE;
typedef HANDLE          *PHANDLE;
typedef LPVOID			HMODULE;
typedef DWORD			DWORD_PTR;		// 32 位机器
typedef DWORD			ULONG_PTR;		// 32 位机器
typedef int 			CLSID;
typedef int 			errno_t;
typedef int			 	HRESULT;
typedef int             LRESULT;
typedef unsigned int    WPARAM;
typedef long            LPARAM;
typedef DWORD           *PDWORD;
typedef unsigned long long ULONGLONG;
typedef HANDLE          HWND;
typedef CONST void		  *LPCVOID;
typedef WORD FILEOP_FLAGS;
typedef unsigned char 	UCHAR;
typedef UCHAR 	*PUCHAR;
typedef CHAR 	*PCHAR;
typedef unsigned short 	USHORT;

#define FO_MOVE 1
#define FO_COPY 2
#define FO_DELETE   3
#define FO_RENAME   4
#define FOF_MULTIDESTFILES  1
#define FOF_CONFIRMMOUSE    2
#define FOF_SILENT  4
#define FOF_RENAMEONCOLLISION   8
#define FOF_NOCONFIRMATION  16
#define FOF_WANTMAPPINGHANDLE   32
#define FOF_ALLOWUNDO   64
#define FOF_FILESONLY   128
#define FOF_SIMPLEPROGRESS  256
#define FOF_NOCONFIRMMKDIR  512
#define FOF_NOERRORUI       1024




#pragma  pack(push,1)
typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef struct _LUID {
    DWORD LowPart;
    LONG  HighPart;
} LUID, *PLUID;

typedef struct _LUID_AND_ATTRIBUTES {
    LUID  Luid;
    DWORD Attributes;
} LUID_AND_ATTRIBUTES, *PLUID_AND_ATTRIBUTES;

#define ANYSIZE_ARRAY   1

typedef struct _TOKEN_PRIVILEGES {
    DWORD               PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[ANYSIZE_ARRAY];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;


typedef struct _SHFILEOPSTRUCT {
  HWND         hwnd;
  UINT         wFunc;
  LPCTSTR      pFrom;
  LPCTSTR      pTo;
  FILEOP_FLAGS fFlags;
  BOOL         fAnyOperationsAborted;
  LPVOID       hNameMappings;
  LPCTSTR      lpszProgressTitle;
} SHFILEOPSTRUCT, *LPSHFILEOPSTRUCT;

/*
 * BMP 信息首部
 */
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}  BITMAPINFOHEADER, *PBITMAPINFOHEADER;

/*
 * RGB 结构
 */
typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
}  RGBQUAD;

/*
 * BMP 文件首部
 */
typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;


typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _EXCEPTION_POINTERS {
    INT    NOTHING_MAKE_IT_HAPPY;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS, *LPEXCEPTION_POINTERS;

typedef struct tagMODULEENTRY32 {
    INT    NOTHING_MAKE_IT_HAPPY;
} MODULEENTRY32, *PMODULEENTRY32;

typedef struct _GUID {
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} GUID;

typedef struct tagPOINT {
  LONG x;
  LONG y;
} POINT, *PPOINT;

typedef struct tagMSG {
  HWND   hwnd;
  UINT   message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD  time;
  POINT  pt;
} MSG, *PMSG, *LPMSG;

#pragma  pack(pop)

/********************************************************************************************************/
/*	标识定义区																					        */
/********************************************************************************************************/
#ifndef LONG_STR_BUF_SIZE
#define LONG_STR_BUF_SIZE        4096
#endif

#ifndef MINI_STR_BUF_SIZE
#define MINI_STR_BUF_SIZE        128
#endif

#ifndef MIDDLE_STR_BUF_SIZE
#define MIDDLE_STR_BUF_SIZE      1024
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE        ((HANDLE)NULL)
#endif

#ifndef nullptr
#define nullptr				NULL
#endif

#ifndef FALSE
#define FALSE 				0
#endif

#ifndef TRUE
#define TRUE  				1
#endif

#ifndef FAIL
#define FAIL				1
#endif

#ifndef SUCCESS
#define SUCCESS				0
#endif

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS       0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef _In_
#define _In_				IN
#endif

#ifndef _Out_
#define _Out_				OUT
#endif

#ifndef _Inout_
#define _Inout_
#endif

#ifndef _Out_opt_
#define _Out_opt_
#endif

#ifndef _In_opt_
#define _In_opt_
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef __stdcall
#define __stdcall
#endif

#ifndef __cdecl
#define __cdecl
#endif

#ifndef WINAPI
#define WINAPI
#endif

#ifndef FAR
#define FAR
#endif

#ifndef MAX_PATH
#define MAX_PATH			PATH_MAX
#endif

// DllMain 函数用到
#define DLL_PROCESS_ATTACH 	1
#define DLL_THREAD_ATTACH	2
#define DLL_THREAD_DETACH	3
#define DLL_PROCESS_DETACH	4

#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY		ENOMEM
#endif

#ifndef E_FAIL
#define E_FAIL				EFAULT
#endif

// ???
#define UISCRName_DEVCAPSETTING "UISCRName_DEVCAPSETTING"

#ifndef AFX_EXT_CLASS
#define AFX_EXT_CLASS
#endif
/********************************************************************************************************/
/*	宏定义区																						    */
/********************************************************************************************************/
#define __list_entry(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))

#ifdef UNICODE
#define _T(x)				L##x
#define __T(x)              L##x
#else
#define _T(x)				x
#define __T(x)				x
#endif

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#define _countof(x)			(sizeof(x) / sizeof(x[0]))
#ifndef __declspec
#define __declspec(x)
#endif
#ifndef _declspec
#define _declspec(x) __declspec(x)
#endif
#define DECLARE_HANDLE(x)	\
	typedef HANDLE x;

// CodePage 参数
#define CP_ACP				0xA
#define CP_UTF8				0x8

// timeout_ms 参数, 永不超时
#define INFINITE 				-1
#define MIL                     1000000L
#define BIL                     1000000000L

// WaitForMultipleObjects WaitForSingleObject 返回值
#define WAIT_OBJECT_0 			0
#define WAIT_FAILED 			96
#define WAIT_ABANDONED_0 		128

// initflag 参数
#define CREATE_BREAKAWAY_FROM_JOB   0x01000000
#define CREATE_DEFAULT_ERROR_MODE   0x04000000
#define CREATE_NEW_CONSOLE          0x00000010
#define CREATE_NEW_PROCESS_GROUP    0x00000200
#define CREATE_NO_WINDOW            0x08000000
#define CREATE_PROTECTED_PROCESS    0x00040000
#define CREATE_PRESERVE_CODE_AUTHZ_LEVEL    0x02000000
#define CREATE_SEPARATE_WOW_VDM     0x00000800
#define CREATE_SHARED_WOW_VDM       0x00001000
#define CREATE_SUSPENDED            0x00000004
#define CREATE_UNICODE_ENVIRONMENT  0x00000400

// Msg 参数
#define WM_QUIT                     0x0012
#define WM_USER                     0x0400

#define TOKEN_ADJUST_PRIVILEGES 0x00
#define TOKEN_QUERY             0x01

#define EWX_SHUTDOWN            0x00000001
#define EWX_REBOOT              0x00000002
#define EWX_FORCE               0x00000004

#endif /* LINUX_COMMONPORTING_H_ */
