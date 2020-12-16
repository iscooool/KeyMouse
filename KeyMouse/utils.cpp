#include "stdafx.h"
#include "utils.h"
#include "KeyMouse.h"
#include "UIAHandler.h"

#include <mutex>
inline void throw_if_fail(HRESULT hr) {
	if (FAILED(hr))
		throw _com_error(hr);
}

//IUIAutomation *pAutomation;

HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation) {
	return CoCreateInstance(CLSID_CUIAutomation, nullptr,
		CLSCTX_INPROC_SERVER, IID_IUIAutomation,
		reinterpret_cast<void**>(ppAutomation));
}

KeyMouse::Context *GetContext(HWND hMainWnd) {
   return reinterpret_cast<KeyMouse::Context *>(GetClassLongPtr(hMainWnd, 0));
}

KeyMouse::PElementVec CachingElementsFromWindow(HWND hMainWnd, std::vector<HWND> TopWindowVec,KeyMouse::PElementVec (*lpEnumFunc)(HWND, HWND)){
	KeyMouse::Context *pCtx = GetContext(hMainWnd);
	IUIAutomation* pAutomation = pCtx->GetAutomation();

	KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
	KeyMouse::PElementVec pTempElementVec;

	
	auto& pCacheWindow = pCtx->GetCacheWindow();

	for (auto it = TopWindowVec.begin(); it != TopWindowVec.end(); ++it) {
		if (pCacheWindow->count(*it) == 0) {
			CComPtr<IUnknown> pEHTemp(new KeyMouse::CacheEventHandler(hMainWnd, *it));
			CComPtr<IUIAutomationElement> pElement;
			HRESULT hr = pAutomation->ElementFromHandle(*it, &pElement);
			hr = pAutomation->AddStructureChangedEventHandler(
				pElement,
				TreeScope_Children,
				NULL,
				(IUIAutomationStructureChangedEventHandler*)pEHTemp.p
			);

			CComPtr<IUnknown> pPropertyChangedEHTemp(new KeyMouse::PropertyChangedEventHandler(hMainWnd));
			PROPERTYID pPIDProperties[] = {
				UIA_IsOffscreenPropertyId,
				UIA_BoundingRectanglePropertyId
			};
			hr = pAutomation->AddPropertyChangedEventHandlerNativeArray(
				pElement,
				TreeScope_Subtree,
				NULL,
				(IUIAutomationPropertyChangedEventHandler*) pPropertyChangedEHTemp.p,
				pPIDProperties,
				sizeof(pPIDProperties) / sizeof(pPIDProperties[0])
			);
			auto pEventHandlers = pCtx->GetEventHandlers();
			(*pEventHandlers)[*it].push_back(pEHTemp);
			(*pEventHandlers)[*it].push_back(pPropertyChangedEHTemp);
			pCtx->SetEventHandlers(pEventHandlers);
		}
	}

	auto Profile = pCtx->GetProfile();
	if (!Profile.only_forewindow) {


#ifdef _DEBUG
        DWORD start_time = GetTickCount();
#endif
		size_t nThread = TopWindowVec.size();
		std::vector<std::future<KeyMouse::PElementVec>> Futures(nThread);
		// Task assignment for multi-threads.
		for(size_t i = 0; i < nThread; ++i) {
			HWND hTopWindow = TopWindowVec[i];
			Futures[i] = std::async(
				lpEnumFunc,
				hMainWnd,
				hTopWindow
			);
		}
		for(size_t i = 0; i < nThread; ++i) {
			auto FutureGet = Futures[i].get();
			HWND hTopWindow = TopWindowVec[i];
			// cache the elements for hTopWindow.
			(*pCacheWindow)[hTopWindow]["AllCache"] = FutureGet;

			if (FutureGet) {
			pElementVec->insert(pElementVec->end(), 
				FutureGet->begin(), FutureGet->end());
			}
		}
#ifdef _DEBUG
        DWORD end_time = GetTickCount();
        DWORD total_time = end_time - start_time;
        cout<<total_time<<std::endl;
#endif
	}
	else {
		for (HWND hTopWindow : TopWindowVec) {
			pTempElementVec = lpEnumFunc(hMainWnd, hTopWindow);
			// cache the elements for hTopWindow.
			(*pCacheWindow)[hTopWindow]["AllCache"] = pTempElementVec;
			if (pTempElementVec) {
				pElementVec->insert(pElementVec->end(),
					pTempElementVec->begin(), pTempElementVec->end());
			}
		}
	}
		
	pCtx->SetCacheExpiredState(false);
	pCtx->SetCacheExpiredStructChanged(false);
	return	pElementVec;
}

struct ParamForEnum
{
	HMONITOR *pMonitor;
	std::vector<HWND> *pTopWindowVec;
};

BOOL CALLBACK lpEnumTopWindowFunc(_In_ HWND hwnd, _In_ LPARAM lParam) {
	ParamForEnum *pParam = reinterpret_cast<ParamForEnum *>(lParam);
	HMONITOR *pTargetMonitor = pParam->pMonitor;
	std::vector<HWND> *pTopWindowVec = pParam->pTopWindowVec;

	RECT Rect;
	GetWindowRect(hwnd, &Rect);
	HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor != *pTargetMonitor)
		return TRUE;
	MONITORINFO MonitorInfo;
	MonitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor, &MonitorInfo);
	RECT Intersect;
	if (IntersectRect(&Intersect, &Rect, &MonitorInfo.rcWork)) {
		if (MonitorInfo.dwFlags != MONITORINFOF_PRIMARY) {
			WCHAR str[100];
			LPWSTR pstr = str;
			GetWindowText(hwnd, pstr, 100);
			pTopWindowVec->push_back(hwnd);
			return FALSE;
			
		}
	}

	return TRUE;
};

BOOL CALLBACK lpEnumMonitor(HMONITOR hMonitor, HDC hDC, LPRECT lpRect, LPARAM lParam) {
	ParamForEnum Param;
	Param.pMonitor = &hMonitor;
	Param.pTopWindowVec = reinterpret_cast<std::vector<HWND>*>(lParam);
	EnumWindows(lpEnumTopWindowFunc, reinterpret_cast<LPARAM>(&Param));
	return TRUE;
}

std::thread t_tagmap;
void SetTagMapThread(HWND hWnd, bool bUseCache) {
	HWND *phMainWnd = reinterpret_cast<HWND *>(GetClassLongPtr(hWnd, 0));
	KeyMouse::Context *pCtx = GetContext(*phMainWnd);
	if (pCtx == nullptr)
		return;


	IUIAutomation* pAutomation = pCtx->GetAutomation();
	pCtx->SetTransWindow(hWnd);
	KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
	KeyMouse::PElementVec pTempElementVec;

	HWND hForeWnd = GetForegroundWindow();


	std::vector<HWND> TopWindowVec;
	
	auto Profile = pCtx->GetProfile();
	if (!Profile.only_forewindow) {
		HMONITOR hMonitor = MonitorFromWindow(hForeWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO MonitorInfo;
		MonitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &MonitorInfo);

		// due to the hardness of identifing real top visible winodow on primary monitor.
		// when foreground window is not on primary monitor, we use hwnd previously stored
		// in ctx as the top visible window on primary monitor.
		if (MonitorInfo.dwFlags != MONITORINFOF_PRIMARY) {
			hForeWnd = pCtx->GetForeWindow();
		}
		else {
			pCtx->SetForeWindow(hForeWnd);
		}

		TopWindowVec.push_back(hForeWnd);
		EnumDisplayMonitors(NULL, NULL, lpEnumMonitor, reinterpret_cast<LPARAM>(&TopWindowVec));


	}
	else {
		pCtx->SetForeWindow(hForeWnd);
		TopWindowVec.push_back(hForeWnd);
	}

	DWORD LastTime = pCtx->GetLastCacheTick();
	DWORD LastInputTime = pCtx->GetLastInputTick();
    DWORD CurrentTime = GetTickCount();
	if (bUseCache && !pCtx->GetCacheExpiredState() && !pCtx->GetCacheExpiredStructChanged()) {
		for(auto it = TopWindowVec.begin(); it != TopWindowVec.end();) {
			auto& pCacheWindow = pCtx->GetCacheWindow();
			if (pCacheWindow->count(*it) > 0) {
				auto& pCache = (*pCacheWindow)[*it]["AllCache"];
				if (pCache) {
					pElementVec->insert(pElementVec->end(),
						pCache->begin(), pCache->end());
				}
				it = TopWindowVec.erase(it);
			}
			else {
				//TODO: eventhandler needs to be cleaned up.
				CComPtr<IUnknown> pEHTemp(new KeyMouse::CacheEventHandler(*phMainWnd, *it));
				CComPtr<IUIAutomationElement> pElement;
				HRESULT hr = pAutomation->ElementFromHandle(*it, &pElement);
				hr = pAutomation->AddStructureChangedEventHandler(
					pElement,
					TreeScope_Children,
					NULL,
					(IUIAutomationStructureChangedEventHandler*)pEHTemp.p
				);

				CComPtr<IUnknown> pPropertyChangedEHTemp(new KeyMouse::PropertyChangedEventHandler(*phMainWnd));
				PROPERTYID pPIDProperties[] = {
					UIA_IsOffscreenPropertyId,
					UIA_BoundingRectanglePropertyId
				};
				hr = pAutomation->AddPropertyChangedEventHandlerNativeArray(pElement,
					TreeScope_Subtree,
					NULL,
					(IUIAutomationPropertyChangedEventHandler*) pPropertyChangedEHTemp.p,
					pPIDProperties,
					sizeof(pPIDProperties) / sizeof(pPIDProperties[0])
				);
				auto pEventHandlers = pCtx->GetEventHandlers();
				(*pEventHandlers)[*it].push_back(pEHTemp);
				(*pEventHandlers)[*it].push_back(pPropertyChangedEHTemp);
				pCtx->SetEventHandlers(pEventHandlers);
				++it;
			}
		}
	}

	KeyMouse::PElementVec pElementsFromUncache;
	pElementsFromUncache = CachingElementsFromWindow(*phMainWnd, TopWindowVec, EnumConditionedElement);
	if (pElementsFromUncache) {
		pElementVec->insert(pElementVec->end(), 
			pElementsFromUncache->begin(), pElementsFromUncache->end());
	}
	
	//pElementVec = EnumTargetWindow(*phMainWnd, hForeWnd);

	// find the elements of taskbar.
	CComPtr<IUIAutomationElement> pDesktop;
	HRESULT hr = pAutomation->GetRootElement(&pDesktop);
	throw_if_fail(hr);

	IUIAutomationCondition *pCondition;
	VARIANT Val;
	Val.vt = VT_BSTR;
	Val.bstrVal = SysAllocString(TEXT("Shell_TrayWnd"));
	hr = pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId,
		Val,
		&pCondition);
	throw_if_fail(hr);

	CComPtr<IUIAutomationElement> pTaskBar;
	pDesktop->FindFirst(TreeScope_Children, pCondition, &pTaskBar);
	if (pTaskBar) {
		UIA_HWND hTaskBar;
		pTaskBar->get_CurrentNativeWindowHandle(&hTaskBar);
		if (hForeWnd != static_cast<HWND>(hTaskBar)) {
			pTempElementVec = EnumConditionedElement(*phMainWnd, static_cast<HWND>(hTaskBar));
			if (pTempElementVec) {
				pElementVec->insert(pElementVec->end(),
					pTempElementVec->begin(), pTempElementVec->end());
			}
		}
	}
	// find the elements of other windows.
	KeyMouse::PElementVec pWindowVec(new std::vector<CComPtr<IUIAutomationElement>>);
	if (Profile.enable_window_switching) {
		CComPtr<IUIAutomationCondition> pIsOffScreenCondition;
		Val.vt = VT_BOOL;
		Val.boolVal = VARIANT_FALSE;
		hr = pAutomation->CreatePropertyCondition(UIA_IsOffscreenPropertyId,
			Val,
			&pIsOffScreenCondition);
		throw_if_fail(hr);
		CComPtr<IUIAutomationCacheRequest> pCacheRequest;
		hr = pAutomation->CreateCacheRequest(&pCacheRequest);
		throw_if_fail(hr);
		hr = pCacheRequest->AddProperty(UIA_BoundingRectanglePropertyId);
		throw_if_fail(hr);
		CComPtr<IUIAutomationElementArray> pWindowElementArray;
		hr = pDesktop->FindAllBuildCache(TreeScope_Children, 
				pIsOffScreenCondition,
				pCacheRequest,
				&pWindowElementArray);
		throw_if_fail(hr);
		int nWindowNum = 0;
		if(pWindowElementArray != nullptr) {
			hr = pWindowElementArray->get_Length(&nWindowNum);
			throw_if_fail(hr);
		}

		for (int i = 0; i < nWindowNum; ++i) {
			IUIAutomationElement *pTempElement;
			pWindowElementArray->GetElement(i, &pTempElement);
			pWindowVec->push_back(pTempElement);
		}

	}

	//-------------------------------------------------------------------
	KeyMouse::PTagMap pTagMap(new std::map<string, CComPtr<IUIAutomationElement>>);
	KeyMouse::PTagMap pWindowMap(new std::map<string, CComPtr<IUIAutomationElement>>);
	KeyMouse::TagCreator TC;
	std::queue<string> TagQueue = TC.AllocTag(pElementVec->size() + pWindowVec->size());
	// the last one of the queue must be the longest one.
	pCtx->SetMaxTagLen(TagQueue.back().length());

	// Traverse the items of ElementArray and paint all hints on the screen.
	for(auto& pTempElement : *pElementVec) {
		string szTemp = TagQueue.front();
		TagQueue.pop();

		// insert the tag and Element into the keymap.
		pTagMap->insert(std::pair<string, CComPtr<IUIAutomationElement>>(
					 szTemp, pTempElement)); 
	}
	for(auto& pTempElement : *pWindowVec) {
		string szTemp = TagQueue.front();
		TagQueue.pop();

		// insert the tag and Element into the keymap.
		pWindowMap->insert(std::pair<string, CComPtr<IUIAutomationElement>>(
					 szTemp, pTempElement)); 
	}

	HDC hDC = GetDC(hWnd);
	if (pWindowMap != nullptr) {
		for (auto& item : *pWindowMap) {
			RECT Rect;
			item.second->get_CachedBoundingRectangle(&Rect);
		
			// print the hint on the screen.
			const TCHAR *psText = item.first.c_str();
			POINT point;
			point.x = (Rect.left + Rect.right) / 2;
			point.y = Rect.top;
			KeyMouse::Font font = Profile.window_tag_font;
			DrawTag(*phMainWnd, hWnd, hDC, point, psText, font);
			
		}
	}
	if (pTagMap != nullptr) {
		for (auto& item : *pTagMap) {
			RECT Rect;
			item.second->get_CachedBoundingRectangle(&Rect);
		
			// print the hint on the screen.
			const TCHAR *psText = item.first.c_str();
			POINT point;
			point.x = Rect.left;
			point.y = Rect.top;
			DrawTag(*phMainWnd, hWnd, hDC, point, psText, Profile.font);
			
		}
	}
	ReleaseDC(hWnd, hDC);
	pCtx->SetTagMap(pTagMap);
	pCtx->SetWindowMap(pWindowMap);

	VariantClear(&Val);
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);

}


void CacheThread(HWND hWnd) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);

	KeyMouse::Context *pCtx =
		reinterpret_cast<KeyMouse::Context *>(
			GetClassLongPtr(hWnd, 0)
			);
	if (pCtx == nullptr)
		return;

	bool bIsCacheExpired = pCtx->GetCacheExpiredState();
	bool bCacheExpiredStructChanged = pCtx->GetCacheExpiredStructChanged();
	LASTINPUTINFO Lii;
	Lii.cbSize = sizeof(LASTINPUTINFO);
	GetLastInputInfo(&Lii);
    DWORD current_time = GetTickCount();
	DWORD LastTime = pCtx->GetLastInputTick();
	pCtx->SetLastInputTick(Lii.dwTime);
	DWORD time = current_time - Lii.dwTime;
	if ((current_time - LastTime > 50 && bIsCacheExpired) ||
		(current_time - LastTime > 50 && bCacheExpiredStructChanged)) {
		if (pCtx->GetEnableState()) {
			KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
			KeyMouse::PElementVec pTempElementVec;

			HWND hForeWnd = GetForegroundWindow();
			std::vector<HWND> TopWindowVec;
			
			auto Profile = pCtx->GetProfile();
			if (!Profile.only_forewindow) {
				HMONITOR hMonitor = MonitorFromWindow(hForeWnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO MonitorInfo;
				MonitorInfo.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(hMonitor, &MonitorInfo);

				// due to the hardness of identifing real top visible winodow on primary monitor.
				// when foreground window is not on primary monitor, we use hwnd previously stored
				// in ctx as the top visible window on primary monitor.
				if (MonitorInfo.dwFlags != MONITORINFOF_PRIMARY) {
					hForeWnd = pCtx->GetForeWindow();
				}
				else {
					pCtx->SetForeWindow(hForeWnd);
				}

				TopWindowVec.push_back(hForeWnd);
				EnumDisplayMonitors(NULL, NULL, lpEnumMonitor, reinterpret_cast<LPARAM>(&TopWindowVec));


			}
			else {
				pCtx->SetForeWindow(hForeWnd);
				TopWindowVec.push_back(hForeWnd);
			}

			pCtx->SetLastcacheTick(current_time);
			pCtx->SetCacheExpiredState(false);
			pCtx->SetCacheExpiredStructChanged(false);
			CachingElementsFromWindow(hWnd, TopWindowVec, EnumConditionedElementForCaching);
		}
	}

	CoUninitialize();
}

/**
* @brief: WndProc2 is used by a transparent window which is used to paint hints
* on.
*
* @param: HWND hWnd: the handle of transparent window. And arguments below is
* similar with common win32 WndProc.
*       : UINT message
*       : WPARAM wParam
*       : LPARAM lParam
*
* @return: LRESULT
*/
LRESULT CALLBACK WndProc2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps = { 0 };
		HDC hDC = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	}
	break;
	case WM_NCHITTEST:
		return HTCAPTION;
	case WM_ERASEBKGND: {
		RECT rect;
		GetClientRect(hWnd, &rect);
		FillRect((HDC)wParam, &rect, CreateSolidBrush(RGB(255, 0, 255)));
		HWND *phMainWnd = reinterpret_cast<HWND *>(GetClassLongPtr(hWnd, 0));
		KeyMouse::Context *pCtx =
			reinterpret_cast<KeyMouse::Context *>(
				GetClassLongPtr(*phMainWnd, 0)
				);
		if (pCtx) {
			t_tagmap = std::thread(SetTagMapThread, hWnd, true);
			t_tagmap.detach();
		}
	}
		break;
	case WM_DESTROY: {
		HWND *phMainWnd = reinterpret_cast<HWND *>(GetClassLongPtr(hWnd, 0));
		delete phMainWnd;
	}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


/**
* @brief: create a transparent window to paint hints on.
*
* @param: HINSTANCE hInstance: the global HINSTANCE.
*       : HWND hMainWnd: the original window which is created when app starts
*       up. we store global context in it.
*
* @return: HWND: the handle of transparent window.
*/
HWND CreateTransparentWindow(HINSTANCE hInstance, HWND hMainWnd)
{
	HWND hWnd;
	HINSTANCE hInst = hInstance;

	DWORD Flags1 = WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TRANSPARENT;
	DWORD Flags2 = WS_VISIBLE | WS_POPUP;

	WCHAR szWindowClass[] = TEXT("Transparent Window");
	WCHAR szTitle[] = TEXT("Transparent Window");

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc2;
	wcex.cbClsExtra = sizeof(HWND *);		// Extra space for main window.
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYMOUSE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);
    // Get the resolution of the virtual screen.
	int X = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int Y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int Width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int Height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	hWnd = CreateWindowEx(Flags1, szWindowClass, szTitle, Flags2, 
            X, Y, 
            Width, Height, 
            0, 0, hInstance, 0);
	
	if (!hWnd)
        return NULL;

	// Set the main window's hWnd for getting the context.
	//std::unique_ptr<HWND> phMainWnd(new HWND(hMainWnd));
	HWND* phMainWnd = new HWND(hMainWnd);
	SetClassLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(phMainWnd));

	HRGN GGG = CreateRectRgn(X, Y, 
            Width, Height);
	InvertRgn(GetDC(hWnd), GGG);
	SetWindowRgn(hWnd, GGG, false);

	KeyMouse::Context *pCtx = reinterpret_cast<KeyMouse::Context *>(
				GetClassLongPtr(hMainWnd, 0)
				);
	int opacity = pCtx->GetProfile().opacity;
	COLORREF RRR = RGB(255, 0, 255);
	SetLayeredWindowAttributes(hWnd, RRR, (BYTE)255 * opacity / 100,  LWA_ALPHA | LWA_COLORKEY);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	
	return hWnd;
}


BOOL CALLBACK lpGetControlCoordinate(_In_ HWND hwnd, _In_ LPARAM lParam) {
	std::vector<HWND> *phWndVec = reinterpret_cast<std::vector<HWND> *>(lParam);

	HWND hChild = GetWindow(hwnd, GW_CHILD);
	//if(hChild == NULL && IsWindowVisible(hwnd))
		phWndVec->push_back(hwnd);
	return TRUE;
};

 KeyMouse::PElementVec EnumTargetWindow(HWND hMainWnd, HWND hForeWnd) {
	KeyMouse::Context *pCtx = GetContext(hMainWnd);
	std::unique_ptr<std::vector<HWND>> phWndVec(new std::vector<HWND>);
	EnumChildWindows(hForeWnd, lpGetControlCoordinate, reinterpret_cast<LPARAM>(phWndVec.get()));

	KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
	for (auto hTargetWnd : *phWndVec) {
	}
		KeyMouse::PElementVec pTempVec = EnumConditionedElement(hMainWnd, hForeWnd);
		pElementVec->insert(pElementVec->end(), 
			pTempVec->begin(), pTempVec->end());
	return pElementVec;
}

 CComPtr<IUIAutomationCondition> GetDesiredCondition(HWND hMainWnd) {
	KeyMouse::Context *pCtx = GetContext(hMainWnd);
	IUIAutomation* pAutomation = pCtx->GetAutomation();
	 HRESULT hr = S_OK;
	//----------------------create element type condition ----------------------------
	// Define the condition by pTotalCondition to find all desired items.
	std::vector<PROPERTYID> vPropertyId = {
		UIA_ListItemControlTypeId,
		UIA_ButtonControlTypeId,
		UIA_TreeItemControlTypeId,
		UIA_TabItemControlTypeId,
		UIA_HyperlinkControlTypeId,
		UIA_SplitButtonControlTypeId,
		UIA_ScrollBarControlTypeId,
		UIA_MenuItemControlTypeId
	};
	SAFEARRAY *pConditionVector = SafeArrayCreateVector(
			VT_UNKNOWN,
			0,
			vPropertyId.size()
			);

	LONG i = 0;

	// construct an condition array and use it for creating a big
	// OrCondition.
	for (PROPERTYID PropertyId : vPropertyId) {
		IUIAutomationCondition *pCondition;
		VARIANT Val;
		Val.vt = VT_I4;
		Val.lVal = PropertyId;
		hr = pAutomation->CreatePropertyCondition(UIA_ControlTypePropertyId,
			Val,
			&pCondition);
		throw_if_fail(hr);
		
		hr = SafeArrayPutElement(pConditionVector, &i, pCondition); 
		throw_if_fail(hr);

		++i;
	}
	CComPtr<IUIAutomationCondition> pTotalOrCondition;
	
	hr = pAutomation->CreateOrConditionFromArray(
			pConditionVector,
			&pTotalOrCondition
			);
	throw_if_fail(hr);

	//----------------------create bool condition ----------------------------
	std::vector<std::tuple<PROPERTYID, VARIANT_BOOL>> vPropertyTuple = {
		{UIA_IsEnabledPropertyId, VARIANT_TRUE},
		{UIA_IsOffscreenPropertyId, VARIANT_FALSE}
	 };

	pConditionVector = SafeArrayCreateVector(
			VT_UNKNOWN,
			0,
			vPropertyTuple.size()
			);

	i = 0;

	for (std::tuple<PROPERTYID, VARIANT_BOOL> PropertyTuple : vPropertyTuple) {
		IUIAutomationCondition *pCondition;
		VARIANT Val;
		Val.vt = VT_BOOL;
		Val.lVal = std::get<1>(PropertyTuple);
		hr = pAutomation->CreatePropertyCondition(std::get<0>(PropertyTuple),
			Val,
			&pCondition);
		
		hr = SafeArrayPutElement(pConditionVector, &i, pCondition); 
		throw_if_fail(hr);

		++i;
	}
	CComPtr<IUIAutomationCondition> pTotalAndCondition;
	
	hr = pAutomation->CreateAndConditionFromArray(
			pConditionVector,
			&pTotalAndCondition
			);
	throw_if_fail(hr);
//----------------------------------------------------------------------------	

	CComPtr<IUIAutomationCondition> pTotalCondition;
	hr = pAutomation->CreateAndCondition(pTotalAndCondition,
			pTotalOrCondition,
			&pTotalCondition
			);

	hr = SafeArrayDestroy(pConditionVector);
	throw_if_fail(hr);
	return pTotalCondition;
 }

 CComPtr<IUIAutomationCacheRequest> GetDesiredCacheRequest(HWND hMainWnd) {
	KeyMouse::Context *pCtx = GetContext(hMainWnd);
	IUIAutomation* pAutomation = pCtx->GetAutomation();

    CComPtr<IUIAutomationCacheRequest> pCacheRequest;
	HRESULT hr = pAutomation->CreateCacheRequest(&pCacheRequest);
	throw_if_fail(hr);

	hr = pCacheRequest->AddPattern(UIA_ScrollPatternId);
	throw_if_fail(hr);
	hr = pCacheRequest->AddPattern(UIA_InvokePatternId);
	throw_if_fail(hr);
	hr = pCacheRequest->AddProperty(UIA_BoundingRectanglePropertyId);
	throw_if_fail(hr);
	hr = pCacheRequest->AddProperty(UIA_ControlTypePropertyId);
	throw_if_fail(hr);
	hr = pCacheRequest->AddProperty(UIA_IsOffscreenPropertyId);
	throw_if_fail(hr);

	return pCacheRequest;
}
/**
* @brief: the much slow version of Findall subtree. but it doesn't
*         freeze the target window. It's used for caching.
*
* @param: HWND hMainWnd: the main window handle.
*		: int restric_depth : the search depth.
*		: IUIAutomationElement *element : the root node to walk.
*       : IUIAutomationCacheRequest * cacheRequest
*       : IUIAutomationTreeWalker *walker
*       : std::vector<CComPtr<IUIAutomationElement>> *found
*
* @return: HRESULT
*/
HRESULT WalkDesiredElementBuildCache(
	HWND hMainWnd,
	int restrict_depth,
	IUIAutomationElement *element,
	IUIAutomationCondition *condition,
	IUIAutomationCacheRequest *cacheRequest,
	IUIAutomationTreeWalker *walker,
	std::vector<CComPtr<IUIAutomationElement>> *found)
 {
	HRESULT hr = S_OK;
	KeyMouse::Context *pCtx = GetContext(hMainWnd);
	IUIAutomation* pAutomation = pCtx->GetAutomation();
	CComPtr<IUIAutomationCondition> pTrueCondition;
	pAutomation->CreateTrueCondition(&pTrueCondition);
	
	CComPtr<IUIAutomationTreeWalker> pWalker;
	pAutomation->get_RawViewWalker(&pWalker);

	int nDepth = 0;
	std::queue<CComPtr<IUIAutomationElement>> queue;
	queue.push(element);
	CComPtr<IUIAutomationElement> pCurrent;
	CComPtr<IUIAutomationElement> pExist;
	do {
		pCurrent = queue.front();
		queue.pop();
		//CComPtr<IUIAutomationElement> pFirstChild;

		//	pWalker->GetFirstChildElementBuildCache(
		//		pCurrent, cacheRequest, &pFirstChild);

		//if (pFirstChild == NULL) {	// when current has no child.
		//}
		//else {	//pCurrent is not a leaf element.
		//	queue.push(pFirstChild);
		//	IUIAutomationElement* pPrev = NULL;
		//	CComPtr<IUIAutomationElement> pSecond;
		//	pWalker->GetNextSiblingElementBuildCache(pFirstChild, cacheRequest, &pSecond);
		//	pPrev = pSecond;
		//	while (pPrev != NULL) {
		//		CComPtr<IUIAutomationElement> pNext;
		//		BOOL IsOffScreen = FALSE;
		//		pPrev->get_CachedIsOffscreen(&IsOffScreen);
		//		if (IsOffScreen == FALSE) {
		//			queue.push(pPrev);
		//		}
		//		pPrev = pNext;
		//		pWalker->GetNextSiblingElementBuildCache(pPrev, cacheRequest, &pNext);
		//	}
		//}

		if (pCurrent != nullptr) {
			HRESULT hr;

			CComPtr<IUIAutomationElementArray> pChildrenElementArray;
			hr = pCurrent->FindAllBuildCache(TreeScope_Children, 
					pTrueCondition,
					cacheRequest,
					&pChildrenElementArray);
			throw_if_fail(hr);
			
			int nChildrenNum = 0;
			if(pChildrenElementArray != nullptr) {
				hr = pChildrenElementArray->get_Length(&nChildrenNum);
				throw_if_fail(hr);
			}
			for (int i = 0; i < nChildrenNum; ++i) {
				CComPtr<IUIAutomationElement> pChild;
				pChildrenElementArray->GetElement(i, &pChild);
				if (pChild != nullptr) {
					BOOL IsOffScreen = FALSE;
					pChild->get_CachedIsOffscreen(&IsOffScreen);
					if (IsOffScreen == FALSE) {
						queue.push(pChild);
					}
				}

			}
			pCurrent->FindFirst(TreeScope_Element, condition, &pExist.p);
			if (pExist != nullptr) {
				found->push_back(pCurrent);
			}
		}


	} while (!queue.empty());
	return S_OK;
}
/**
* @brief: TODO: need to be refactored.
*         enumerate all element of foreground window which satisfies specific
*         condition and paint hint on transparent window.
*
* @param: HWND hMainWnd: the window store global context.
*       : HDC hdc: the HDC of transparent window.
*
* @return: PElementVec: a vector of all elements found in foreground window.
*/
KeyMouse::PElementVec EnumConditionedElement(HWND hMainWnd, HWND hForeWnd) {
	try {

        // Get current context.
		KeyMouse::Context *pCtx = GetContext(hMainWnd);
		if (!pCtx) {
			return nullptr;
		}
		IUIAutomation* pAutomation = pCtx->GetAutomation();
		HWND hTransWnd = pCtx->GetTransWindow();
		CComPtr<IUIAutomationElement> pElement;
		HRESULT hr = pAutomation->ElementFromHandle(hForeWnd, &pElement);
		throw_if_fail(hr);

        // this is used for fastSelectMode. add a event handler to detect
        // element tree change.
		if (pCtx->GetFastSelectState()) {
			// add event handler to detect changing when in consercutive mode.
			CComPtr<KeyMouse::EventHandler> pEHTemp(new KeyMouse::EventHandler(hMainWnd));
			hr = pAutomation->AddStructureChangedEventHandler(pElement,
				TreeScope_Subtree, NULL,
				(IUIAutomationStructureChangedEventHandler*)pEHTemp);

			pCtx->SetStructEventHandler(pEHTemp);
			pCtx->SetElement(pElement);
            // get the current foreground window's file name.
			DWORD pid;
			GetWindowThreadProcessId(hForeWnd, &pid);
			HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
			CHAR lpFileName[200] = { 0 };
			GetModuleFileNameExA(hProc, NULL, lpFileName, 200);
			pCtx->SetPrevProcessName(std::string(lpFileName));
			CloseHandle(hProc);
		}
		CComPtr<IUIAutomationElementArray> pElementArray;

        
		CComPtr<IUIAutomationCondition> pTotalCondition = GetDesiredCondition(hMainWnd);
        // add cache request for desired patterns and properties.
        CComPtr<IUIAutomationCacheRequest> pCacheRequest = GetDesiredCacheRequest(hMainWnd);

        throw_if_fail(hr);
		CComPtr<IUIAutomationElementArray> pChildrenElementArray;
        hr = pElement->FindAllBuildCache(TreeScope_Descendants, 
                pTotalCondition,
                pCacheRequest,
                &pChildrenElementArray);
        throw_if_fail(hr);

        int nChildrenNum = 0;
        if(pChildrenElementArray != nullptr) {
            hr = pChildrenElementArray->get_Length(&nChildrenNum);
            throw_if_fail(hr);
        }


		KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
		for (int i = 0; i < nChildrenNum; ++i) {
			CComPtr<IUIAutomationElement> pTempElement;
			pChildrenElementArray->GetElement(i, &pTempElement);
			pElementVec->push_back(pTempElement);

		}
		
		return pElementVec;
	}
	catch (_com_error err) {
	}
	return nullptr;
}

/**
* @brief: TODO: need to be refactored.
*         enumerate all element of foreground window which satisfies specific
*         condition and paint hint on transparent window.
*
* @param: HWND hMainWnd: the window store global context.
*       : HDC hdc: the HDC of transparent window.
*
* @return: PElementVec: a vector of all elements found in foreground window.
*/
KeyMouse::PElementVec EnumConditionedElementForCaching(HWND hMainWnd, HWND hForeWnd) {
	try {

        // Get current context.
		KeyMouse::Context *pCtx = GetContext(hMainWnd);
		if (!pCtx) {
			return nullptr;
		}
		IUIAutomation* pAutomation = pCtx->GetAutomation();
		HWND hTransWnd = pCtx->GetTransWindow();
		CComPtr<IUIAutomationElement> pElement;
		HRESULT hr = pAutomation->ElementFromHandle(hForeWnd, &pElement);
		throw_if_fail(hr);

		CComPtr<IUIAutomationElementArray> pElementArray;
		CComPtr<IUIAutomationCondition> pTotalCondition = GetDesiredCondition(hMainWnd);
        // add cache request for desired patterns and properties.
        CComPtr<IUIAutomationCacheRequest> pCacheRequest = GetDesiredCacheRequest(hMainWnd);

        throw_if_fail(hr);
		CComPtr<IUIAutomationElementArray> pChildrenElementArray;
		KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
		hr = WalkDesiredElementBuildCache(
			hMainWnd,
			20,
			pElement,
			pTotalCondition,
			pCacheRequest,
			NULL,
			pElementVec.get()
		);

		
		return pElementVec;
	}
	catch (_com_error err) {
	}
	return nullptr;
}
/**
* @brief: check whether focus on edit control.(not always works.)
*
* @param: 
*
* @return: bool: if focus on edit control, return true. else false.
*/
bool isFocusOnEdit(HWND hMainWnd) {
	KeyMouse::Context *pCtx = GetContext(hMainWnd);
	IUIAutomation* pAutomation = pCtx->GetAutomation();
    if(pAutomation == nullptr)
        return false;
    try {
        CComPtr<IUIAutomationElement> pTempElement;
        HRESULT hr = pAutomation->GetFocusedElement(&pTempElement);
        throw_if_fail(hr);
        CONTROLTYPEID iControlType;
        if(pTempElement)
            pTempElement->get_CurrentControlType(&iControlType); 
        if(iControlType == UIA_EditControlTypeId) {
			
            return true;
        }
        else
            return false;
    }
    catch (_com_error err) {
    }
    return false;
}
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

void DrawTag(HWND hMainWnd, HWND hTransWnd, HDC hdc, POINT point, const TCHAR* psText, KeyMouse::Font font) {
	KeyMouse::Context *pCtx = 
		reinterpret_cast<KeyMouse::Context *>(
				GetClassLongPtr(hMainWnd, 0)
				);

	ScreenToClient(hTransWnd, &point);

	HPEN hpenOld = static_cast<HPEN>(SelectObject(hdc, GetStockObject(DC_PEN)));
	HBRUSH hbrushOld = static_cast<HBRUSH>(SelectObject(hdc, GetStockObject(DC_BRUSH)));
	// background color.
	SetDCBrushColor(hdc, font.background_color);
	
	int nHight = MulDiv(font.font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	int PrimaryHeight = GetSystemMetrics(SM_CYSCREEN);
	// use different height on different monitors.

	HFONT hFont = CreateFont(nHight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font.font_name.c_str());

	HFONT hfontOld = static_cast<HFONT>(SelectObject(hdc, hFont));
	SIZE TextSize;

	GetTextExtentPoint(hdc, psText, _tcslen(psText), &TextSize);
	
	int WidthMax = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int HeightMax = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	point.x = point.x > WidthMax ? WidthMax : point.x;
	point.y = point.y > HeightMax ? HeightMax : point.y;
	LogicalToPhysicalPointForPerMonitorDPI(hTransWnd, &point);
	double PaddingRatio = 1.4;
	RECT Rect;
	Rect.left = point.x;
	Rect.top = point.y;
	Rect.right = static_cast<LONG>(point.x + TextSize.cx * PaddingRatio);
	Rect.bottom = static_cast<LONG>(point.y + TextSize.cy * PaddingRatio);
	RoundRect(hdc, Rect.left, Rect.top, Rect.right, Rect.bottom, TextSize.cx / 4, TextSize.cy / 4);
	SetBkMode(hdc, TRANSPARENT);
	SetBkColor(hdc, RGB(0, 0, 0));   // black
	SetTextColor(hdc, font.font_color);
	
	DrawText(hdc, psText, -1, &Rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	SelectObject(hdc, hpenOld);
	SelectObject(hdc, hbrushOld);
	SelectObject(hdc, hfontOld);

	DeleteObject(hFont);

}

/**
* @brief: return the right physical rect for multiple monitors.
*
* @param: HWND hWnd: the transparent window.
		: RECT Rect: the logical source rect.
*
* @return: Rect
*/
RECT RectForPerMonitorDPI(HWND hWnd, RECT Rect) {
	POINT LeftTop;
	LeftTop.x = Rect.left;
	LeftTop.y = Rect.top;
	LogicalToPhysicalPointForPerMonitorDPI(hWnd, &LeftTop);
	Rect.left = LeftTop.x;
	Rect.top = LeftTop.y;

	int WidthMax = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int HeightMax = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	POINT RightBottom;
	// the RightBottom point might be over the max value.
	RightBottom.x = Rect.right > WidthMax ? WidthMax : Rect.right;
	RightBottom.y = Rect.bottom > HeightMax ? HeightMax : Rect.bottom;
	LogicalToPhysicalPointForPerMonitorDPI(hWnd, &RightBottom);
	Rect.right = RightBottom.x;
	Rect.bottom = RightBottom.y;

	return Rect;
}
