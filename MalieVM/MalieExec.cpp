#include "stdafx.h"
#include "MalieExec.h"
#include "Malie_VMParse.h"
#include "common.h"


MalieExec::MalieExec(char *lpFileName)
{
	execstream bin(lpFileName);
	DWORD dwCnt = bin.readdw();
	for (; dwCnt; --dwCnt)//skip var
	{
		DWORD szLen = bin.readdw();
		bin.seek(szLen & 0x7FFFFFFF, FILE_CURRENT);
		GetNext(&bin);
		bin.seek(sizeof(DWORD) * 4, FILE_CURRENT);
	}
	//至于这里为啥要跳过一个DW我也忘了、最初的解析里面没有写
	//  [7/20/2013 Azure]
	bin.seek(sizeof(DWORD), FILE_CURRENT);//skip 0x3130 dwCnt = 0x5F4

	//////////////////////////////////////////////////////////////////////////
	// Function parse block
	dwCnt = bin.readdw();
	//	freopen("functionList.txt","wt,ccs=UNICODE",stdout);
	for (; dwCnt; --dwCnt)//skip function
	{
		DWORD szLen; char temp[1000]; DWORD ch[3];
		VM_FUNCTION func;
		szLen = bin.readdw();
		bin.read(temp, (szLen & 0x7FFFFFFF));
		bin.read(ch, 3 * sizeof(DWORD));
		func.dwID = ch[0];
		func.wstrName = (WCHAR *)temp;
		func.dwVMCodeOffset = ch[2];
		func.dwReserved0 = ch[1];
		funcList[func.wstrName] = func;
		vecFuncList.push_back(func);
//		wprintf(L"%d,%ls,%d,0x%.4X\n",ch[0],temp,ch[1],ch[2]);
	}

	fprintf(stderr, "VM_FUNCTION:%d\n", funcList.size());

	//////////////////////////////////////////////////////////////////////////
	// Label parse block
	dwCnt = bin.readdw();
	for (; dwCnt; --dwCnt)//skip label
	{
		DWORD szLen; char temp[1000]; DWORD ch;
		MALIE_LABEL label;
		szLen = bin.readdw();
		bin.read(temp, (szLen & 0x7FFFFFFF));
		ch = bin.readdw();
		label.wstrName = (WCHAR *)temp;
		label.dwVMCodeOffset = ch;
		labelList[label.wstrName] = label;
	}

	fprintf(stderr, "LABEL:%d\n", labelList.size());

	//////////////////////////////////////////////////////////////////////////
	// VM_DATA : just read to new area

	szVM_DATA = bin.readdw();
	//dump original scene
	pVM_DATA = new unsigned char[szVM_DATA];
	bin.read(pVM_DATA, szVM_DATA);

	fprintf(stderr, "VM_DATA size: %8X\n", szVM_DATA);

	//////////////////////////////////////////////////////////////////////////
	// VM_CODE : just read to new area
	szVM_CODE = bin.readdw();
	pVM_CODE = new unsigned char[szVM_CODE];
	bin.read(pVM_CODE, szVM_CODE);

	fprintf(stderr, "VM_CODE size: %8X\n", szVM_CODE);

//	fprintf(stderr,"system_onInit:0x%X\n",func_List.find(L"system_onInit")->second.dwVMCodeOffset);
	//////////////////////////////////////////////////////////////////////////
	// strTable

	offStrTable = bin.seek(0, FILE_CURRENT);
	DWORD unkSize = bin.readdw();
	if (unkSize * 8 > bin.GetFileSize() - bin.seek(0, FILE_CURRENT))
	{
		szStrTable = unkSize;//ziped crypted
		pStrTable = new unsigned char[unkSize];
		bin.read(pStrTable, szStrTable);

		cntStrIndex = bin.readdw();
		vStrIndex.reserve(cntStrIndex / 2 + 1);
		DWORD offset = bin.readdw();
		for (size_t idx = 1; idx < cntStrIndex; ++idx)
		{
			DWORD tmpOffset = bin.readdw();
			vStrIndex.push_back(STRING_INFO(offset, tmpOffset - offset));
			offset = tmpOffset;
		}
	}
	else
	{
		cntStrIndex = unkSize;//normal
		vStrIndex.resize(cntStrIndex);
		bin.read(&vStrIndex[0], cntStrIndex*sizeof(STRING_INFO));

		szStrTable = bin.readdw();
		pStrTable = new unsigned char[szStrTable];
		bin.read(pStrTable, szStrTable);
	}
}


MalieExec::~MalieExec(void)
{
	delete pVM_DATA;
	delete pVM_CODE;
}


DWORD MalieExec::GetFuncOffset(wstring funcName)
{
	map<wstring, VM_FUNCTION>::iterator it = funcList.find(funcName);
	if (it != funcList.end())
	{
		return it->second.dwVMCodeOffset;
	}
	return 0;
}


DWORD MalieExec::GetFuncOffset(size_t funcId)
{
	if (funcId < vecFuncList.size())
	{
		return move(vecFuncList[funcId].dwVMCodeOffset);
	}
	return 0;
}


wstring MalieExec::GetFuncName(size_t funcId)
{
	if (funcId < vecFuncList.size())
	{
		return move(vecFuncList[funcId].wstrName);
	}
	return wstring();
}


int MalieExec::GetFuncId(wstring funcName)
{
	map<wstring, VM_FUNCTION>::iterator it = funcList.find(funcName);
	if (it != funcList.end())
	{
		return it->second.dwID;
	}
	return -1;
}


wstring MalieExec::ParseString(DWORD dwIndex)
{
	bool fl_rub = 0, fl_vol = 0;

	wstring strLine(
		(WCHAR *)&pStrTable[vStrIndex[dwIndex].offset],
		(WCHAR *)&pStrTable[vStrIndex[dwIndex].offset + vStrIndex[dwIndex].length]
		);

	wstring outLine(strLine.size() + 3, 0);


	size_t len = 0;
	for (size_t idx = 0; idx < strLine.size(); ++idx)
	{
		switch (strLine[idx])
		{
		case 0:
			if (fl_rub || fl_vol) fl_rub = fl_vol = 0;
			outLine[len++] = EOSTR;
			break;
		case 1:
			idx += 4;
			break;
		case 2:
			++idx;
			break;
		case 3:
			idx += 2;
			break;
		case 4:
			++idx;
			break;
		case 5:
			idx += 2;
			break;
		case 6:
			idx += 2;
			break;
		case 0xA:
			outLine[len++] = fl_rub ? STRUB : L'\n';
			break;
		case 7:
			switch (strLine[++idx])
			{
			case 0x0001://递归调用文字读取，然后继续处理（包含注释的文字）
				outLine[len++] = TO_RUB;
				fl_rub = 1;
				break;
			case 0x0004://下一句自动出来
				outLine[len++] = NXL;
				break;
			case 0x0006://代表本句结束
				outLine[len++] = TO_RTN;
				break;
			case 0x0007://递归调用文字读取然后wcslen，跳过不处理。应该是用于注释
				++idx;
				idx += wcslen(&strLine[idx]);
				break;
			case 0x0008://LoadVoice 后面是Voice名
				outLine[len++] = TO_VOL;
				fl_vol = 1;
				break;
			case 0x0009://LoadVoice结束
				outLine[len++] = EOVOL;
				break;
			default:
				outLine[len++] = UNKNOW_SIG;
				break;
			}
			break;
		default:
			outLine[len++] = strLine[idx];
		}
	}
	outLine[len++] = EOPAR;
	outLine[len++] = L'\n';
	outLine[len++] = L'\n';

	return move(outLine);
}

wstring MalieExec::ImportString(DWORD dwIndex, wstring chsLine)
{
	bool fl_rub = 0, fl_vol = 0;

	wstring strLine(
		(WCHAR *)&pStrTable[vStrIndex[dwIndex].offset],
		(WCHAR *)&pStrTable[vStrIndex[dwIndex].offset + vStrIndex[dwIndex].length]
		);

	wstring oldLine(strLine);

	vector<wstring> tokens;
	auto x = chsLine.find_first_of(L'※');
	if (x != wstring::npos)
	{
		chsLine = wstring(&chsLine[x + 1], &chsLine[chsLine.size() - 1]);
	}

	for (auto ch = wcstok(&chsLine[0], L"▲△▼▽◁◆◎★※⊙"); ch; ch = wcstok(NULL, L"▲△▼▽◁◆◎★※⊙"))
	{
		tokens.push_back(ch);
	}

	vector<wstring> old_tokens;
	WCHAR *old_ar = L"\x1\x2\x3\x4\x5\x6\x7\x8\x9\xb\xc\xd\xe\xf\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f";
	for (int i = 0; i < oldLine.size() - 1; ++i)
	{
		if (oldLine[i] == 0)
		{
			oldLine[i] = 1;
		}
		if (oldLine[i] == 7 && oldLine[i + 1] == 1)
		{
			for (auto j = i; j < oldLine.size(); ++j)
			{
				if (oldLine[j] == 0xa)
				{
					oldLine[j] = 1;
					break;
				}
			}
		}
	}
	for (auto ch = wcstok(&oldLine[0], old_ar); ch; ch = wcstok(NULL, old_ar))
	{
		old_tokens.push_back(ch);
	}
	if (tokens.size()>1)
	{
		auto it = tokens.end() - 1;
		if (*it == L" ")
		{
			tokens.erase(it);
		}

	}
	if (old_tokens.size() != tokens.size())
	{
		fprintf(stderr, "Error! Tokens mismatch! Line %d\n", dwIndex);
		getchar();
	}
	size_t len = 0;
	size_t token_i = 0;
	wstring outLine;

	outLine.resize(max(chsLine.size(), strLine.size()) + 30);

	for (size_t idx = 0; idx < strLine.size(); ++idx)
	{
		if (strLine[idx] == '\n')
		{
			outLine[len++] = '\n';
		}
		if (strLine[idx] < 0x20 && strLine[idx] != '\n')
		{
			outLine[len++] = strLine[idx];
		}
		else
		{
			memcpy(&outLine[len], &tokens[token_i][0], sizeof(WCHAR)*tokens[token_i].size());
			len += tokens[token_i].size();
			idx += old_tokens[token_i].size() - 1;
			token_i++;
		}
	}

	return wstring(&outLine[0], &outLine[len]);
}

pair<vector<STRING_INFO>, wstring> MalieExec::RebuildStringSection(CMalieCHS &db)
{
	vector<STRING_INFO> vChsIndex;
	size_t offset = 0;
	wstring buf;
	fprintf(stderr, "VM binary string counts:%d\nLoaded chs string counts:%d\n", vStrIndex.size(), db.GetSize());
	for (size_t i = 0; i < vStrIndex.size(); ++i)
	{
		//fprintf(stderr, "Cursor: %d\n", i);
		auto && jis = db.GetString(i);
		if (jis.empty())
		{
			fprintf(stderr, "Error! Empty string got from chs db. Line: %d\n", i);
		}
		auto && chs = ImportString(i, jis);
		vChsIndex.push_back(STRING_INFO(offset, chs.size() * 2));
		offset += chs.size() * 2;
		buf += chs;
	}
	return move(pair<vector<STRING_INFO>, wstring>(move(vChsIndex), move(buf)));
}

int MalieExec::RebuildVMBinary(CMalieCHS &scene, char *lpInFile, char *lpOutFile)
{
	auto && str = RebuildStringSection(scene);
	auto && x = str.first;
	auto && y = str.second;
	size_t cbStrInfo = x.size()*sizeof(STRING_INFO);
	size_t cbStrTable = y.size()*sizeof(WCHAR);
	binfstream in(lpInFile);
	size_t cbFinal = offStrTable + 8 + cbStrInfo + cbStrTable;
	unsigned char * pBuf = new unsigned char[cbFinal];
	in.read(pBuf, offStrTable);
	unsigned char * p = &pBuf[offStrTable];
	*(DWORD *)p = x.size();
	p += 4;
	memcpy(p, &x[0], cbStrInfo);
	p += cbStrInfo;
	*(DWORD *)p = cbStrTable;
	p += 4;
	memcpy(p, &y[0], cbStrTable);
	binfstream out(lpOutFile, OF_WRITE);
	out.write(pBuf, cbFinal);
	delete pBuf;
	return 0;
}

int MalieExec::ExportStrByCode(void)
{
	CMalie_VMParse vm(this);
	vector<wstring> chapterName;
	vector<DWORD> chapterIndex;
	vector<DWORD> chapterRegion;
	vector<Malie_Moji> && moji = vm.ParseScenario(chapterName, chapterIndex);

	if (!chapterName.size())
	{
		vector<DWORD>::iterator it = unique(chapterIndex.begin(), chapterIndex.end());
		chapterIndex.erase(it, chapterIndex.end());
	}

	auto exportFunc = [&](pair<DWORD, wstring>(&x), FILE *fp){
		fwprintf(fp, L"○%08d○\n%s●%08d●\n%s◇%08d◇\n\n\n",
			x.first, x.second.c_str(), x.first, x.second.c_str(), x.first);
	};

	fprintf(stderr, "\nStarting dumping text to file...\n");

	if (chapterIndex.size())
	{
		chapterRegion = chapterIndex;
		chapterRegion.erase(chapterRegion.begin());
		chapterRegion.push_back(moji.size());
		for (size_t i = 0; i < chapterIndex.size(); ++i)
		{
			wstring && name = i < chapterName.size() ?
				stringf(L"%02d %ls.txt", i, chapterName[i].c_str()) :
				stringf(L"%02d.txt", i);

			FILE *fp;
			_wfopen_s(&fp, name.c_str(), L"wt,ccs=UNICODE");

			for_each(moji.begin() + chapterIndex[i], moji.begin() + chapterRegion[i], [&](Malie_Moji x)
			{
				wstring kotoba;
				if (!x.name.empty())
				{
					kotoba = x.name + L"※";
				}
				kotoba += ParseString(x.index);

				exportFunc(pair<DWORD, wstring>(x.index, kotoba), fp);
				fflush(fp);
			});

			fclose(fp);
		}
	}
	else
	{
		FILE *fp;
		_wfopen_s(&fp, L"MalieMoji.txt", L"wt,ccs=UNICODE");

		for_each(moji.begin(), moji.end(), [&](Malie_Moji x)
		{
			wstring kotoba;
			if (!x.name.empty())
			{
				kotoba = x.name + L"※";
			}
			kotoba += ParseString(x.index);

			exportFunc(pair<DWORD, wstring>(x.index, kotoba), fp);
		});

		fclose(fp);
	}

	fprintf(stderr, "Done.\n");
	return 0;
}


unsigned char * MalieExec::GetVMCodeBase(void)
{
	return pVM_CODE;
}


unsigned char * MalieExec::GetVMDataBase(void)
{
	return pVM_DATA;
}