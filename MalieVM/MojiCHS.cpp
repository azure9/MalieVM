#include "stdafx.h"
#include "MojiCHS.h"

using namespace std;
CMojiCHS::CMojiCHS(const TCHAR *pFileName, bool bSaveIndex, bool bParseIndex)
	:bParseIndex(bParseIndex)
{
	std::wifstream fin(pFileName, ios::binary);
	if (fin.fail())
	{
		return;
	}
// 	// apply BOM-sensitive UTF-16 facet
 	fin.imbue(std::locale(fin.getloc(),
 		new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
	wstring line;
	while (!getline(fin,line).eof())
	{
		line[line.size()-1]=0;
		switch (line[0])
		{
		case CHS_SIGNATURE:
			{
				if (size_t pos = line.find_last_of(CHS_SIGNATURE))
				{
					if (pos!=wstring::npos)
					{
						if (bSaveIndex|bParseIndex)
						{
							wstring strIndex(&line[1],&line[pos]);
							if (bSaveIndex)
							{
								vecIndex.push_back(move(strIndex));
							}
							else if (bParseIndex)
							{
								int i;
								wstringstream str(strIndex);
								str>>i;
								vIdx.push_back(i);
							}
						}
						getline(fin, line);
						line[line.size() - 1] = 0;
						vecString.push_back(move(line));
					}
				}
			}
			break;
		case IMPORT_SIGNATURE:
			{
				if (size_t pos = line.find_last_of(IMPORT_SIGNATURE))
				{
					if (pos++!=wstring::npos)
					{
						getline(fin, line);
						line[line.size() - 1] = 0;
						if (wcslen(&line[0]))
						{
							vecString[vecString.size() - 1] = (move(line));
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
}


CMojiCHS::~CMojiCHS(void)
{
}


wstring CMojiCHS::GetString(int index)
{
	if (bParseIndex)
	{
		if (vIdx.size()&&index<=*(vIdx.end()-1))
		{
			auto it = find(vIdx.begin(),vIdx.end(),index);
			if (it!=vIdx.end())
			{
				return vecString[it-vIdx.begin()];
			}
			return L"";
		}
		else
			return L"";
	}
	else if (index>vecString.size())
	{
		return L"";
	}
	return vecString[index];
}