#include "MojiCHS.h"
#include "rapidjson\document.h"

using namespace rapidjson;
int IsSimplifiedCH(unsigned short &dch)
{
	unsigned char highbyte;
	unsigned char lowbyte;

	highbyte = dch & 0x00ff;
	lowbyte  = dch / 0x0100;

	if (highbyte==0 || lowbyte==0)
		return 0;

	//    区名    码位范围   码位数  字符数 字符类型
	// 双字节2区 B0A1—F7FE   6768    6763    汉字
	if ((highbyte >= 0xb0 && highbyte <= 0xf7) && (lowbyte  >= 0xa1 && lowbyte  <= 0xfe))
		return 1;

	return 0;
}

int IsGraphicCH(const unsigned short dch)
{
	int nRet;
	unsigned char highbyte;
	unsigned char lowbyte;

	highbyte = dch & 0x00ff;
	lowbyte  = dch / 0x0100;

	//    区名    码位范围   码位数  字符数 字符类型
	// 双字节1区 A1A1—A9FE    846     718  图形符号
	if (
		(highbyte >= 0xa1 && highbyte <= 0xa3) &&
		(lowbyte  >= 0xa1 && lowbyte  <= 0xfe)
		)
		nRet = 1;
	else
		nRet = 0;

	return nRet;
}
wstring GBK2UNI(const char* pGBK, long nBytesCount)
{
	DWORD dwMinSize = MultiByteToWideChar(936, 0, pGBK, nBytesCount, NULL, 0);
	wstring strUNI(dwMinSize,0);
	if (MultiByteToWideChar(936, 0, (LPCSTR)pGBK, nBytesCount, (WCHAR*)strUNI.c_str(), dwMinSize) > 0)
	{
		return (strUNI);
	}
	return (strUNI);
}

string UNI2GBK(const WCHAR* pUNI, long nBytesCount)
{
	DWORD dwMinSize = WideCharToMultiByte(936, 0, pUNI, nBytesCount, NULL, 0,0,0);
	string strGBK(dwMinSize,0);
	
	if (WideCharToMultiByte(936, 0, (LPCWSTR)pUNI, nBytesCount, (char *)strGBK.c_str(), dwMinSize,0,0) > 0)
	{
		return (strGBK);
	}
	return (strGBK);
}

string UNI2SJIS(const WCHAR* pUNI, long nBytesCount)
{
	DWORD dwMinSize = WideCharToMultiByte(932, 0, pUNI, nBytesCount, NULL, 0,0,0);
	string strSJIS(dwMinSize,0);
	
	if (WideCharToMultiByte(932, 0, (LPCWSTR)pUNI, nBytesCount, (char *)strSJIS.c_str(), dwMinSize,0,0) > 0)
	{
		return (strSJIS);
	}
	return (strSJIS);
}

wstring SJIS2UNI(const char* pSJIS, long nBytesCount)
{
	DWORD dwMinSize = MultiByteToWideChar(932, 0, pSJIS, nBytesCount, NULL, 0);
	wstring strUNI(dwMinSize,0);
	if (MultiByteToWideChar(932, 0, (LPCSTR)pSJIS, nBytesCount, (WCHAR*)strUNI.c_str(), dwMinSize) > 0)
	{
		return (strUNI);
	}
	return (strUNI);
}

wstring UTF82UNI(const char* pUTF8, long nBytesCount)
{
	DWORD dwMinSize = MultiByteToWideChar(CP_UTF8, 0, pUTF8, nBytesCount, NULL, 0);
	wstring strUNI(dwMinSize, 0);
	if (MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, nBytesCount, (WCHAR*)strUNI.c_str(), dwMinSize) > 0)
	{
		return (strUNI);
	}
	return (strUNI);
}

inline int getLine(WCHAR *&pStart,WCHAR *&pEnd)
{
	if (*pStart)
	{
		pEnd = pStart;
		for (;*pEnd!=L'\n';pEnd++);
		return 1;
	}
	return 0;
}

CMojiCHS::CMojiCHS(void)
	:langType(0)
{
	UINT nLanID = GetSystemDefaultLangID();   
	WORD PriLan = PRIMARYLANGID(nLanID);   
	WORD SubLan = SUBLANGID(nLanID);
	if (PriLan == LANG_CHINESE)  
	{  
		if (SubLan == SUBLANG_CHINESE_SIMPLIFIED)   
			langType = 1;   //系统是简体中文   
		else if (SubLan == SUBLANG_CHINESE_TRADITIONAL)   
			langType = 2;   //系统是繁体中文   
	}
}

int CMojiCHS::ParseDict(void *pContent)
{
	Document d;
	d.Parse((char *)pContent);
	for (Value::ConstMemberIterator it = d.MemberBegin();
		it != d.MemberEnd(); ++it)
	{
		const char *pName = it->name.GetString();
		const char *pValue = it->value.GetString();
		dict_jis.push_back(UTF82UNI(pName,strlen(pName)));
		dict_chs.push_back(UTF82UNI(pValue,strlen(pValue)));
	}
	return d.Size();
}


CMojiCHS::~CMojiCHS(void)
{
}


wstring CMojiCHS::GetString(wstring & ori)
{
	vector<wstring>::iterator it = find(dict_jis.begin(),dict_jis.end(),ori);
	if (it==dict_jis.end())
	{
		return L"";
	}
	return dict_chs[it-dict_jis.begin()];
}

wstring CMojiCHS::GetSrcString(size_t idx)
{
	if (idx<dict_jis.size())
	{
		return dict_jis[idx];
	}
	return L"";
}

wstring CMojiCHS::GetDstString(size_t idx)
{
	if (idx<dict_jis.size())
	{
		if (idx<dict_chs.size())
		{
			return dict_chs[idx];
		}
		return dict_jis[idx];
	}
	return L"";
}
