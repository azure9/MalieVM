#include "stdafx.h"
#include "MalieCHS.h"


CMalieCHS::CMalieCHS(vector<wstring> & filelist)
{
	for each (auto && file in filelist)
	{
		CMojiCHS moji(file.c_str());
		auto && x = moji.GetDB();
		db.insert(x.begin(), x.end());
	}
}


CMalieCHS::~CMalieCHS()
{
}
