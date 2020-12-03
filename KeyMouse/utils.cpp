#include "stdafx.h"
#include "utils.h"
#include "KeyMouse.h"
#include "UIAHandler.h"

#include <mutex>
inline void throw_if_fail(HRESULT hr) {
	if (FAILED(hr))
		throw _com_error(hr);
}

IUIAutomation *pAutomation;

HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation) {
	return CoCreateInstance(CLSID_CUIAutomation, nullptr,
		CLSCTX_INPROC_SERVER, IID_IUIAutomation,
		reinterpret_cast<void**>(ppAutomation));
}

std::thread t_tagmap;
std::mutex mtx;
void SetTagMapThread(HWND hWnd) {
	mtx.lock();
	
	HWND *phMainWnd = reinterpret_cast<HWND *>(GetClassLongPtr(hWnd, 0));
	KeyMouse::Context *pCtx =
		reinterpret_cast<KeyMouse::Context *>(
			GetClassLongPtr(*phMainWnd, 0)
			);
	if (pCtx == nullptr)
		return;
	KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
	KeyMouse::PElementVec pTempElementVec;
	pCtx->ClearTagMap();
	pCtx->SetTransWindow(hWnd);
	HWND hForeWnd = pCtx->GetForeWindow();
	// find the elements of froeground window.
	pTempElementVec = EnumConditionedElement(*phMainWnd, hForeWnd);
	if (pTempElementVec) {
		pElementVec->insert(pElementVec->end(), 
			pTempElementVec->begin(), pTempElementVec->end());
	}
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

	KeyMouse::PElementVec pWindowVec(new std::vector<CComPtr<IUIAutomationElement>>);
	for (int i = 0; i < nWindowNum; ++i) {
		IUIAutomationElement *pTempElement;
		pWindowElementArray->GetElement(i, &pTempElement);
		pWindowVec->push_back(pTempElement);
	}
	//-------------------------------------------------------------------
	KeyMouse::PTagMap TagMap(new std::map<string, CComPtr<IUIAutomationElement>>);
	KeyMouse::PTagMap WindowMap(new std::map<string, CComPtr<IUIAutomationElement>>);
	KeyMouse::TagCreator TC;
	std::queue<string> TagQueue = TC.AllocTag(pElementVec->size() + pWindowVec->size());
	// the last one of the queue must be the longest one.
	pCtx->SetMaxTagLen(TagQueue.back().length());

	// Traverse the items of ElementArray and paint all hints on the screen.
	for(auto& pTempElement : *pElementVec) {
		string szTemp = TagQueue.front();
		TagQueue.pop();

		// insert the tag and Element into the keymap.
		TagMap->insert(std::pair<string, CComPtr<IUIAutomationElement>>(
					 szTemp, pTempElement)); 
	}
	pCtx->SetTagMap(TagMap);
	for(auto& pTempElement : *pWindowVec) {
		string szTemp = TagQueue.front();
		TagQueue.pop();

		// insert the tag and Element into the keymap.
		WindowMap->insert(std::pair<string, CComPtr<IUIAutomationElement>>(
					 szTemp, pTempElement)); 
	}
	pCtx->SetWindowMap(WindowMap);

	VariantClear(&Val);
	mtx.unlock();
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);

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
		HWND *phMainWnd = reinterpret_cast<HWND *>(GetClassLongPtr(hWnd, 0));
		KeyMouse::Context *pCtx =
			reinterpret_cast<KeyMouse::Context *>(
				GetClassLongPtr(*phMainWnd, 0)
				);
		if (mtx.try_lock() && pCtx) {
			auto profile = pCtx->GetProfile();
			const KeyMouse::PTagMap& pWindowMap = pCtx->GetWindowMap();
			for (auto& item : *pWindowMap) {
				RECT Rect;
				item.second->get_CachedBoundingRectangle(&Rect);
			
				// print the tag on the screen.
				const TCHAR *psText = item.first.c_str();
				POINT point;
				point.x = (Rect.left + Rect.right) / 2;
				point.y = Rect.top;
				KeyMouse::Font font = profile.window_tag_font;
				DrawTag(*phMainWnd, hWnd, hDC, point, psText, font);
				
			}
			const KeyMouse::PTagMap& pTagMap = pCtx->GetTagMap();
			for (auto& item : *pTagMap) {
				RECT Rect;
				item.second->get_CachedBoundingRectangle(&Rect);
			
				// print the tag on the screen.
				const TCHAR *psText = item.first.c_str();
				POINT point;
				point.x = Rect.left;
				point.y = Rect.top;

				DrawTag(*phMainWnd, hWnd, hDC, point, psText, profile.font);
				
			}
			mtx.unlock();
		}
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
			t_tagmap = std::thread(SetTagMapThread, hWnd);
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
	HWND hMaskWindow = GetForegroundWindow();
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
    // Get the resolution of th screen.
	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    int X = 0;
    int Y = 0;
	int Width = dm.dmPelsWidth;
	int Height = dm.dmPelsHeight;
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
        DWORD start_time = GetTickCount();

        // Get current context.
        KeyMouse::Context *pCtx = 
            reinterpret_cast<KeyMouse::Context *>(
                    GetClassLongPtr(hMainWnd, 0)
                    );
		if (!pCtx) {
			return nullptr;
		}
		HWND hTransWnd = pCtx->GetTransWindow();
		CComPtr<IUIAutomationElement> pElement;
		HRESULT hr = pAutomation->ElementFromHandle(hForeWnd, &pElement);
		throw_if_fail(hr);

        // this is used for fastSelectMode. add a event handler to detect
        // element tree change.
		if (pCtx->GetFastSelectState()) {
			// add event handler to detect changing when in consercutive mode.
			KeyMouse::EventHandler* pEHTemp = new KeyMouse::EventHandler(hMainWnd);
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
        
        // add cache request for desired patterns and properties.
        CComPtr<IUIAutomationCacheRequest> pCacheRequest;
        hr = pAutomation->CreateCacheRequest(&pCacheRequest);
        throw_if_fail(hr);

        hr = pCacheRequest->AddPattern(UIA_ScrollPatternId);
        throw_if_fail(hr);
        hr = pCacheRequest->AddPattern(UIA_InvokePatternId);
        throw_if_fail(hr);
        hr = pCacheRequest->AddProperty(UIA_BoundingRectanglePropertyId);
        throw_if_fail(hr);
        hr = pCacheRequest->AddProperty(UIA_ControlTypePropertyId);
        throw_if_fail(hr);

        CComPtr<IUIAutomationCondition> pTrueCondition;
        hr = pAutomation->CreateTrueCondition(&pTrueCondition);
        throw_if_fail(hr);
		CComPtr<IUIAutomationElementArray> pChildrenElementArray;
        // Find all children of focus window for multi-threads task
        // partition.
        hr = pElement->FindAllBuildCache(TreeScope_Children, 
                pTrueCondition,
                pCacheRequest,
                &pChildrenElementArray);
        throw_if_fail(hr);
        
        int nChildrenNum = 0;
        if(pChildrenElementArray != nullptr) {
            hr = pChildrenElementArray->get_Length(&nChildrenNum);
            throw_if_fail(hr);
        }
        std::unique_ptr<std::thread[]> pThread(new std::thread[nChildrenNum]);
        std::unique_ptr<CComPtr<IUIAutomationElementArray>[]> 
            pThreadElementArray(new CComPtr<IUIAutomationElementArray>[nChildrenNum]);
        // Task assignment for multi-threads.
        for(int j = 0; j < nChildrenNum; ++j) {
            IUIAutomationElement *pTempElement;
            pChildrenElementArray->GetElement(j, &pTempElement);
            pThread[j] = std::thread(&IUIAutomationElement::FindAllBuildCache,
                    pTempElement,
                    TreeScope_Subtree,
                    pTotalCondition,
                    pCacheRequest,
                    &pThreadElementArray[j]
                    );
        }

        int nTotalLength = 0;
        for(int j = 0; j < nChildrenNum; ++j) {
            pThread[j].join();
            int length = 0;
			if (pThreadElementArray[j] != nullptr) {
				hr = pThreadElementArray[j]->get_Length(&length);
			}
            nTotalLength += length;
            throw_if_fail(hr);

        }

        hr = SafeArrayDestroy(pConditionVector);
        throw_if_fail(hr);

		KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
        DWORD end_time = GetTickCount();
        DWORD total_time = end_time - start_time;
        cout<<total_time<<std::endl;

		
		// Traverse the items of ElementArray and paint all hints on the screen.
        for(int i = 0; i < nChildrenNum && nTotalLength > 0; ++i) {
            int length;
			if (pThreadElementArray[i] == nullptr) {
				continue;
			}
            hr = pThreadElementArray[i]->get_Length(&length);
            throw_if_fail(hr);
            IUIAutomationElementArray *pElementArray = pThreadElementArray[i];
            for (int j = 0; j < length; ++j) {
                CComPtr<IUIAutomationElement> pTempElement;
                pElementArray->GetElement(j, &pTempElement);
				pElementVec->push_back(pTempElement);

            }
        }
       // LockWindowUpdate(hForeWnd);
		
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
bool isFocusOnEdit() {
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
    
	int nHight = -MulDiv(font.font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	HFONT hFont = CreateFont(nHight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font.font_name.c_str());

	HFONT hfontOld = static_cast<HFONT>(SelectObject(hdc, hFont));
	SIZE text_size;

	GetTextExtentPoint(hdc, psText, _tcslen(psText), &text_size);
	
	double padding_ratio = 1.4;
	RECT rect;
	rect.left = point.x;
	rect.top = point.y;
	rect.right = static_cast<LONG>(point.x + text_size.cx * padding_ratio);
	rect.bottom = static_cast<LONG>(point.y + text_size.cy * padding_ratio);
	RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, text_size.cx / 4, text_size.cy / 4);
	SetBkMode(hdc, TRANSPARENT);
	SetBkColor(hdc, RGB(0, 0, 0));   // black
	SetTextColor(hdc, font.font_color);
	
	DrawText(hdc, psText, -1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	SelectObject(hdc, hpenOld);
	SelectObject(hdc, hbrushOld);
	SelectObject(hdc, hfontOld);

	DeleteObject(hFont);

}

