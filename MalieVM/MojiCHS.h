#pragma once
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <codecvt>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <windows.h>
#include <set>
#include <map>

using namespace std;


#define IMPORT_SIGNATURE L'¡ø'
#define CHS_SIGNATURE L'¡ñ'

class CMojiCHS
{
public:
	CMojiCHS(const TCHAR *pFileName, bool bSaveIndex = false, bool bParseIndex = true);
	~CMojiCHS(void);
private:
	vector<wstring> vecString;
	vector<wstring> vecIndex;
	vector<int> vIdx;
	bool bParseIndex;
public:
	wstring GetString(int index);
	wstring GetIndexDescr(int index)
	{
		return vecIndex[index];
	}
	size_t GetDictSize(void)
	{
		return vecString.size();
	}
	map<int, wstring> GetDB()
	{
		map<int, wstring> db;
		size_t lines = vIdx.size();
		for (size_t i = 0; i < lines; ++i)
		{
			db[vIdx[i]] = vecString[i];
		}
		return move(db);
	}
};

