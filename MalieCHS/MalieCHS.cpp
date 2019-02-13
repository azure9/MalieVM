// MalieCHS.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MojiCHS.h"
#pragma comment(lib,"detours.lib")
CMojiCHS *moji;
WCHAR buf[0x1000];
int off = 0;
#pragma comment(linker, "/EXPORT:DirectInput8Create=DINPUT8.DirectInput8Create")
unsigned int __fastcall PushSTR(unsigned int VM_SP, unsigned char* VM_DATA);
typedef decltype(&PushSTR) __pfnPushSTR;
extern __pfnPushSTR pfnPushSTR;
string stringf(CHAR* format, ...)
{
	va_list vArgList;
	va_start(vArgList, format);
	basic_string<CHAR> str(_vsnprintf(NULL, 0, format, vArgList), 0);
	_vsnprintf((CHAR *)str.c_str(), str.size(), format, vArgList);
	va_end(vArgList);
	return (str);
}

unsigned int __fastcall GetTranslate(unsigned int VM_SP, unsigned char* VM_DATA, unsigned int off)
{
	auto jis = wstring((WCHAR *)(VM_DATA + off));
	OutputDebugStringW(jis.c_str());
	auto chs = moji->GetString(jis);
	if (chs != L"")
	{
		memcpy(buf, chs.c_str(), (chs.size() + 1)*sizeof(WCHAR));
		off = (unsigned char *)buf - VM_DATA;
	}
	return off;
}

unsigned int __fastcall HookPushSTR(unsigned int VM_SP, unsigned char* VM_DATA)
{
	__asm mov off,eax
	GetTranslate(VM_SP, VM_DATA, off);
	return pfnPushSTR(VM_SP,VM_DATA);
}