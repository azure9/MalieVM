#pragma once
#pragma warning(disable:4091)
#include <vector>
#include <string>
#include <windows.h>
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////
//						Define Macros Here
//////////////////////////////////////////////////////////////////////////////////////
#ifdef kDays
#define DbgInfoA(x) OutputDebugStringA(x)
#define DbgInfoW(x) OutputDebugStringW(x)
#else
#define DbgInfoA(x) ;
#define DbgInfoW(x) ;
#endif // kDays

#define GetOffset(x) PDWORD((DWORD)rva2raw(pNtH->FileHeader.NumberOfSections,IMAGE_FIRST_SECTION(pNtH),x)+(DWORD)pSarcheck->pBuffer)
#define THM_UPDATE_STATUS WM_USER+0x426
typedef struct blob_data 
{
	const unsigned char *data;
	size_t len;
	blob_data(const unsigned char *data,size_t len)
		:data(data),len(len){};
};

typedef struct retFileData
{
	string name;
	DWORD len;
	blob_data data;
	retFileData(string name,DWORD len,blob_data data)
		:name(name),len(len),data(data){};
};

typedef struct ver_pair 
{
	DWORD timestamp;
	string descr;
};

typedef struct nonstd_string
{
	DWORD len;
	wstring str;
	nonstd_string(DWORD len,wstring str)
		:len(len),str(str){};
};
const int TOUHOU_CHS = 1;
const int DWM_COMPETABLE = 2;

typedef  void (*dispatchMsg)(int percent);


DWORD rva2raw(WORD nSections, PIMAGE_SECTION_HEADER pSectionHeader, DWORD rva);
string stringf(CHAR* format,...);