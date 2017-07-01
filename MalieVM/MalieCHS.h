#pragma once
#include "MojiCHS.h"
class CMalieCHS
{
private:
	map<int, wstring> db;
public:
	CMalieCHS(vector<wstring> & filelist);
	~CMalieCHS();
	wstring GetString(int idx)
	{
		if (idx < db.size())
		{
			return db[idx];
		}
		return L"";
	}
	size_t GetSize()
	{
		return db.size();
	}
};

