#pragma once
#include <list>

using namespace std;
class CMalieScene
{
public:
	CMalieScene();
	~CMalieScene();
	bool Set(wstring &single);
	wstring Get(size_t index);
private:
	list<wstring> db;
};

