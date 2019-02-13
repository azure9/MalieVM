// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "detours.h"
#include "common.h"
#include "MojiCHS.h"
#define DbgPrint(...) OutputDebugStringA(stringf(__VA_ARGS__).c_str())
unsigned int __fastcall HookPushSTR(unsigned int VM_SP, unsigned char* VM_DATA);
unsigned int __fastcall PushSTR(unsigned int VM_SP, unsigned char* VM_DATA);
int PkgFindFile(char *pkgPath, LPCWSTR fileName, void *a3, void *a4);
typedef decltype(&PushSTR) __pfnPushSTR;
typedef decltype(&PkgFindFile) __pfnPkgFindFile;
__pfnPushSTR pfnPushSTR;
__pfnPkgFindFile pfnPkgFindFile;

extern CMojiCHS *moji;
void *FindStub(void * pBegin, size_t cbSearch, unsigned long signature)
{
	void *pEnd = (PBYTE)pBegin + cbSearch;
	for (PBYTE pbTest = (PBYTE)pBegin; pbTest < pEnd;) {
		if (*(PDWORD)pbTest == signature)
		{
			return pbTest;
		}
		BYTE rbDst[128];
		PVOID pbDstPool = (PVOID)(rbDst + sizeof(rbDst));
		LONG lExtra = 0;
		PVOID pbTarget = NULL;
		ULONG cbStep = (ULONG)((PBYTE)DetourCopyInstruction(rbDst, &pbDstPool, pbTest,
			&pbTarget, &lExtra) - pbTest);
		pbTest += cbStep;
	}

	return 0;
}
class binfstream
{
public:
	binfstream(LPCSTR lpFileName, int iReadWrite = OF_READ)
		:hFile(0), cbFile(0)
	{
		switch (iReadWrite)
		{
		case OF_READ:
			hFile = _lopen(lpFileName, iReadWrite);
			cbFile = _llseek(hFile, 0, FILE_END);
			_llseek(hFile, 0, FILE_BEGIN);
			break;
		case OF_WRITE:
			hFile = _lcreat(lpFileName, 0);
			break;
		default:
			break;
		}
	}
	~binfstream()
	{
		close();
	}

	bool close()
	{
		if (hFile)
		{
			_lclose(hFile);
			return false;
		}
		return false;
	}

	virtual size_t read(void *dest, size_t cbRead)
	{
		return _lread(hFile, dest, cbRead);
	}

	virtual size_t write(void *src, size_t cbWirte)
	{
		return _lwrite(hFile, (LPCCH)src, cbWirte);
	}

	bool eof()
	{
		return _llseek(hFile, 0, FILE_CURRENT) == cbFile;
	}

	bool is_open()
	{
		return hFile != -1;
	}

	size_t seek(LONG lOffset, int iOrigin)
	{
		return _llseek(hFile, lOffset, iOrigin);
	}

	size_t GetFileSize()
	{
		return cbFile;
	}
protected:
	size_t cbFile;
	HFILE hFile;
};

int HookPkgFindFile(char *pkgPath, LPCWSTR fileName, void *a3, void *a4)
{
	if (GetFileAttributes(fileName)!=-1)
	{
		return 0;
	}
	return pfnPkgFindFile(pkgPath, fileName, a3, a4);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		binfstream bin("trans.txt");
		char *p = new char[bin.GetFileSize()+2];
		bin.read(p, bin.GetFileSize());
		moji = new CMojiCHS();
		DbgPrint("%d strings loaded.", moji->ParseDict(p));
		delete p;
		PIMAGE_DOS_HEADER pDH;
		PIMAGE_NT_HEADERS pNtH;
		pDH = (PIMAGE_DOS_HEADER)GetModuleHandle(0);
		pNtH = (PIMAGE_NT_HEADERS32)((DWORD)pDH + pDH->e_lfanew);
		//signature 
		/*
		00535EB6    83E9 04         sub     ecx, 0x4
		00535EB9    03D0            add     edx, eax
		*/

		pfnPushSTR = (__pfnPushSTR)FindStub((unsigned char *)pDH + pNtH->OptionalHeader.BaseOfCode, pNtH->OptionalHeader.BaseOfData - pNtH->OptionalHeader.BaseOfCode, 0x0304e983);
		BYTE *pTempAddr = (BYTE *)FindStub((unsigned char *)pDH + pNtH->OptionalHeader.BaseOfCode, pNtH->OptionalHeader.BaseOfData - pNtH->OptionalHeader.BaseOfCode, 0x51535055);
		pTempAddr += 5;
		pfnPkgFindFile = (__pfnPkgFindFile)(*(PDWORD)pTempAddr +4 + pTempAddr);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DbgPrint("Hook v_pushstr at 0x%08X", pfnPushSTR);
		DbgPrint("Hook PkgFindFile at 0x%08X", pfnPkgFindFile);

		DetourAttach((PVOID *)&pfnPushSTR, HookPushSTR);
		DetourAttach((PVOID *)&pfnPkgFindFile, HookPkgFindFile);
		DetourTransactionCommit();
	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

