// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <WinUser.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#include <iostream>
#include <sstream>
#include <psapi.h>
#include <shellapi.h>
#include <UIAutomation.h>
#include <atlbase.h>
#include <comdef.h>
#include <memory>
#include <map>
#include <vector>
#include <queue>
#include <utility>
#include <thread>
#include <future>
#include <stdlib.h>


#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _DEBUG
    #ifndef DBG_NEW
        #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
        #define new DBG_NEW
    #endif
#endif  // _DEBUG

