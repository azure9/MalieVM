// MalieVM.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <locale.h>
#include <conio.h>
#include "MalieExec.h"
#include "Malie_VMParse.h"
#include "MalieCHS.h"

int _tmain(int argc, _TCHAR* argv[])
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(L"*.txt", &FindFileData);
	vector<wstring> v;

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			v.push_back(FindFileData.cFileName);
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
	CMalieCHS db(v);
	MalieExec exec("exec.dat");
	setlocale(LC_ALL, "Japanese");
	exec.RebuildVMBinary(db, "exec.dat", "exec_chs.dat");
	binfstream exec_chs("exec_chs.dat");
	auto p = new unsigned char[exec_chs.GetFileSize()];
	exec_chs.read(p, exec_chs.GetFileSize());
	if (HANDLE hRes = BeginUpdateResource(L"L:\\Software\\deCrypt\\KDays\\MalieSystem\\沉默的法则\\Fixed_chs.exe", FALSE))
	{
		UpdateResource(hRes, L"EXEC", L"EXEC", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			p, exec_chs.GetFileSize()); //buffer[i].length()+1
		EndUpdateResource(hRes, FALSE);
		delete p;
	}
	exit(0);
	//	MalieExec exec("D:\\Software\\deCrypt\\KDays\\MalieSystem\\Dies irae  -Amantes amentes-\\exec.dat");
	exec.ExportStrByCode();
	_getch();
	// 	CMalie_VMParse vm(&exec);//0xF8Aexec.GetFuncOffset(L"_ms_message")
	// 	vm.diasm(exec.GetFuncOffset(L"maliescenario"),0x100000);
	// 	freopen("out.txt","wt,ccs=UNICODE",stdout);
	// 	exec.ExportStrByCode();
	//	printf("0x%X",exec.GetFuncOffset(L"system_onInit"));
	return 0;
}