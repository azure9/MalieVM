#pragma once
#include "common.h"
#include <stdio.h>
#include <iostream>
#include <locale>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <windows.h>

using namespace std;

int IsSimplifiedCH(unsigned short &dch);
int IsGraphicCH(const unsigned short dch);
wstring GBK2UNI(const char* pGBK, long nBytesCount);
string UNI2GBK(const WCHAR* pUNI, long nBytesCount);
string UNI2SJIS(const WCHAR* pUNI, long nBytesCount);
wstring SJIS2UNI(const char* pSJIS, long nBytesCount);
#define JIS_SIGNATURE L'¡ñ'
#define CHS_SIGNATURE L'¡ð'


class CMojiCHS
{
public:
	CMojiCHS(void);
	~CMojiCHS(void);
private:
	int langType;
	vector<wstring> dict_chs;
	vector<wstring> dict_jis;
	int ExtraMultibytes(LPCSTR & lpStart, LPCSTR & lpEnd);
public:
	wstring GetString(wstring & ori);
	wstring GetSrcString(size_t idx);
	wstring GetDstString(size_t idx);
	int ParseDict(void *pChsBuffer);
	int Translate(LPCSTR lpString, int cbString,void * &pOut);
};

