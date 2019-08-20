#pragma once
#include<windows.h>
#include<string>
#ifdef UNICODE
typedef std::wstring string;
#define cout std::wcout
#else
typedef std::string string;
#define cout std::cout
#endif
