// provide a number of special definitions for use with WinWpi

#ifndef OS2_INCLUDED
#define OS2_INCLUDED

#include <time.h>

#define __WINWPI__

typedef void VOID;
typedef char CHAR;
typedef char* PSZ;
typedef const char* PCSZ;
typedef unsigned char BYTE;
typedef unsigned char* PBYTE;
typedef int BOOL;
typedef int* PBOOL;
typedef signed long LONG;
typedef signed long* PLONG;
typedef unsigned long ULONG;
typedef unsigned long* PULONG;
typedef signed short SHORT;
typedef unsigned short USHORT;
typedef signed long APIRET;

#define FALSE 0
#define TRUE 1

#define XWPENTRY
#define _System
#define _Export

#define alloca _alloca

#define NO_ERROR                   0
#define ERROR_NOT_ENOUGH_MEMORY   -1
#define ERROR_INVALID_PARAMETER   -1

#include <direct.h>

#endif
