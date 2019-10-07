#include "stdafx.h"
#include "utils.h"
#include "KeyMouse.h"


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
LRESULT CALLBACK WndProc2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps = { 0 };
		HDC hDC = ::BeginPaint(hWnd, &ps);
		
		::SetTextColor(hDC, RGB(0, 0, 0));
		//::SetBkMode(hDC, TRANSPARENT);

		HWND *phMainWnd =
			reinterpret_cast<HWND *>(
				GetClassLongPtr(hWnd, 0)
				);
		KeyMouse::Context *pCtx =
			reinterpret_cast<KeyMouse::Context *>(
				GetClassLongPtr(*phMainWnd, 0)
				);
		pCtx->SetTransWindow(hWnd);
		EnumConditionedElement(*phMainWnd, hDC);
		//::DrawTextExW(hDC, L"Hello, World!", -1, &rc,
		//	DT_SINGLELINE | DT_CENTER | DT_VCENTER, NULL);
		::EndPaint(hWnd, &ps);
	}
	break;
	case WM_NCHITTEST:
		return HTCAPTION;
	case WM_ERASEBKGND:
		RECT rect;
		GetClientRect(hWnd, &rect);
		FillRect((HDC)wParam, &rect, CreateSolidBrush(RGB(255, 0, 255)));

		break;
	case WM_DESTROY:
		//PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HWND CreateTransparentWindow(HINSTANCE hInstance, HWND hMainWnd)
{
	HWND hMaskWindow = GetForegroundWindow();
	HWND hWnd;
	HINSTANCE hInst = hInstance;

	DWORD Flags1 = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TRANSPARENT;
	DWORD Flags2 = WS_POPUP;

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
	std::unique_ptr<HWND> phMainWnd(new HWND(hMainWnd));
	SetClassLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(phMainWnd.get()));

	HRGN GGG = CreateRectRgn(X, Y, 
            Width, Height);
	InvertRgn(GetDC(hWnd), GGG);
	SetWindowRgn(hWnd, GGG, false);

	COLORREF RRR = RGB(255, 0, 255);
	SetLayeredWindowAttributes(hWnd, RRR, (BYTE)0,  LWA_COLORKEY);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	
	return hWnd;
}
BOOL EnumConditionedElement(HWND hMainWnd, HDC hdc) {
	try {
        //DWORD start_time = GetTickCount();
        // Create Transparent window.

        // Get current context.
        KeyMouse::Context *pCtx = 
            reinterpret_cast<KeyMouse::Context *>(
                    GetClassLongPtr(hMainWnd, 0)
                    );
		HWND hForeWnd = pCtx->GetForeWindow();
		HWND hTransWnd = pCtx->GetTransWindow();
		CComPtr<IUIAutomationElement> pElement;
		HRESULT hr = pAutomation->ElementFromHandle(hForeWnd, &pElement);
		throw_if_fail(hr);

		CComPtr<IUIAutomationElementArray> pElementArray;
		// Define the condition by pTotalCondition to find all desired items.
        std::vector<PROPERTYID> vPropertyId = {
            UIA_ListItemControlTypeId,
            UIA_ButtonControlTypeId,
            UIA_TreeItemControlTypeId,
            UIA_TabItemControlTypeId,
            UIA_HyperlinkControlTypeId,
			UIA_SplitButtonControlTypeId,
            UIA_ScrollBarControlTypeId
        };
        SAFEARRAY *pConditionVector = SafeArrayCreateVector(
                VT_UNKNOWN,
                0,
                vPropertyId.size()
                );

        LONG i = 0;
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

        CComPtr<IUIAutomationCondition> pIsEnabledCondition;
        VARIANT Val;
        Val.vt = VT_BOOL;
        Val.boolVal = VARIANT_TRUE;
        hr = pAutomation->CreatePropertyCondition(UIA_IsEnabledPropertyId,
            Val,
            &pIsEnabledCondition);
        throw_if_fail(hr);
        CComPtr<IUIAutomationCondition> pIsOffscreenCondition;
        Val.vt = VT_BOOL;
        Val.boolVal = VARIANT_TRUE;
        hr = pAutomation->CreatePropertyCondition(UIA_IsOffscreenPropertyId,
            Val,
            &pIsOffscreenCondition);
        throw_if_fail(hr);

        CComPtr<IUIAutomationCondition> pBoolCondition;
        hr = pAutomation->CreateAndCondition(pIsEnabledCondition,
                pIsOffscreenCondition,
                &pBoolCondition
                );
        throw_if_fail(hr);

        CComPtr<IUIAutomationCondition> pTotalCondition;
        hr = pAutomation->CreateAndCondition(pIsEnabledCondition,
                pTotalOrCondition,
                &pTotalCondition
                );

        CComPtr<IUIAutomationCacheRequest> pCacheRequest;
        hr = pAutomation->CreateCacheRequest(&pCacheRequest);
        throw_if_fail(hr);

        //hr = pCacheRequest->AddPattern(UIA_SelectionItemPatternId);
        //throw_if_fail(hr);
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
            int length;
            hr = pThreadElementArray[j]->get_Length(&length);
            nTotalLength += length;
            throw_if_fail(hr);

        }

        hr = SafeArrayDestroy(pConditionVector);
        throw_if_fail(hr);

        std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>>
            TagMap(new std::map<string, CComPtr<IUIAutomationElement>>);
        std::unique_ptr<std::vector<CComPtr<IUIAutomationElement>>>
            ScrollVec(new std::vector<CComPtr<IUIAutomationElement>>);

		KeyMouse::TagCreator TC;
        std::queue<string> TagQueue = TC.AllocTag(nTotalLength);
        // the last one of the queue must be the longest one.
        pCtx->SetMaxTagLen(TagQueue.back().length());
        /*DWORD end_time = GetTickCount();
        DWORD total_time = end_time - start_time;
        cout<<total_time<<std::endl;*/

		
		// Traverse the items of ElementArray.
        for(int i = 0; i < nChildrenNum; ++i) {
            int length;
            hr = pThreadElementArray[i]->get_Length(&length);
            throw_if_fail(hr);
            IUIAutomationElementArray *pElementArray = pThreadElementArray[i];
            for (int j = 0; j < length; ++j) {
                CComPtr<IUIAutomationElement> pTempElement;
                pElementArray->GetElement(j, &pTempElement);
                RECT Rect;
                pTempElement->get_CachedBoundingRectangle(&Rect);
                string szTemp = TagQueue.front();
                TagQueue.pop();
                // print the tag on the screen.
                const TCHAR *psText = szTemp.c_str();
                POINT point;
                point.x = Rect.left;
                point.y = Rect.top;
				ScreenToClient(hTransWnd, &point);
                BOOL result = TextOut(hdc,
                        point.x,
                        point.y,
                        psText, _tcslen(psText));
                // insert the tag and Element into the keymap.
                TagMap->insert(std::pair<string, CComPtr<IUIAutomationElement>>(
                             szTemp, pTempElement)); 

            }
        }
        pCtx->SetScrollVec(ScrollVec);
        pCtx->SetTagMap(TagMap);
        LockWindowUpdate(hForeWnd);
		
	}
	catch (_com_error err) {
	}
	return true;
}
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

