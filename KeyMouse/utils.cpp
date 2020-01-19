#include "stdafx.h"
#include "utils.h"
#include "KeyMouse.h"
#include "UIAHandler.h"


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
		SetTextColor(hDC, RGB(0, 0, 0));
		
		HWND *phMainWnd = reinterpret_cast<HWND *>(GetClassLongPtr(hWnd, 0));
		KeyMouse::Context *pCtx =
			reinterpret_cast<KeyMouse::Context *>(
				GetClassLongPtr(*phMainWnd, 0)
				);
		if (pCtx == nullptr)
			break;
        KeyMouse::PTagMap TagMap(new std::map<string, CComPtr<IUIAutomationElement>>);
		KeyMouse::PElementVec pElementVec(new std::vector<CComPtr<IUIAutomationElement>>);
		KeyMouse::PElementVec pTempElementVec;
		pCtx->ClearTagMap();
		pCtx->SetTransWindow(hWnd);
		HWND hForeWnd = pCtx->GetForeWindow();
		// find the elements of froeground window.
		pTempElementVec = EnumConditionedElement(*phMainWnd, hForeWnd, hDC);
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
				pTempElementVec = EnumConditionedElement(*phMainWnd, static_cast<HWND>(hTaskBar), hDC);
				if (pTempElementVec) {
					pElementVec->insert(pElementVec->end(),
						pTempElementVec->begin(), pTempElementVec->end());
				}
			}
		}
		KeyMouse::TagCreator TC;
        std::queue<string> TagQueue = TC.AllocTag(pElementVec->size());
        // the last one of the queue must be the longest one.
        pCtx->SetMaxTagLen(TagQueue.back().length());

		// Traverse the items of ElementArray and paint all hints on the screen.
        for(auto& pTempElement : *pElementVec) {
			RECT Rect;
			pTempElement->get_CachedBoundingRectangle(&Rect);
			string szTemp = TagQueue.front();
			TagQueue.pop();
			// print the tag on the screen.
			const TCHAR *psText = szTemp.c_str();
			POINT point;
			point.x = Rect.left;
			point.y = Rect.top;

			DrawTag(*phMainWnd, hWnd, hDC, point, psText);
			// insert the tag and Element into the keymap.
			TagMap->insert(std::pair<string, CComPtr<IUIAutomationElement>>(
						 szTemp, pTempElement)); 
        }
		pCtx->SetTagMap(TagMap);

		VariantClear(&Val);
	
		EndPaint(hWnd, &ps);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

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
	std::unique_ptr<HWND> phMainWnd(new HWND(hMainWnd));
	SetClassLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(phMainWnd.get()));

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
*         (this function is not finished yet.)a depth first search of element 
*         tree to find desierd elements.(can't do better than FindAll.)
*
* @param: int restric_depth : the search depth.
*		: IUIAutomationElement *element : the root node to walk.
*       : IUIAutomationCacheRequest * cacheRequest
*       : IUIAutomationTreeWalker *walker
*       : std::vector<CComPtr<IUIAutomationElement>> *found
*
* @return: HRESULT
*/
HRESULT WalkDesiredElementBuildCache(
	int restrict_depth,
	IUIAutomationElement *element,
	IUIAutomationCondition *condition,
	IUIAutomationCacheRequest *cacheRequest,
	IUIAutomationTreeWalker *walker,
	std::vector<CComPtr<IUIAutomationElement>> *found)
 {


	CONTROLTYPEID ControlType;
	element->get_CurrentControlType(&ControlType);
	// TODO: DocumentControlType is used for skipping chrome mainwindow. need to
	// optimize.
	if (element == nullptr || ControlType == UIA_DocumentControlTypeId)
		return S_OK;

	int nDepth = 0;
	std::queue<IUIAutomationElement*> queue;
	queue.push(element);
	// the pointer to the next depth's first element on the element tree.
	// For caculating the depth of current element.
	BSTR szNextDepthFirstId;
	element->get_CurrentAutomationId(&szNextDepthFirstId);
	IUIAutomationElement *pCurrent;
	do {
		pCurrent = queue.front();
		queue.pop();
		IUIAutomationElement *pFirstChild;

		// find the next depth's first element.
		//BSTR szCurrentId;
		//pCurrent->get_CurrentAutomationId(&szCurrentId);
		//if (StrCmp(szCurrentId, szNextDepthFirstId) == 0) {
		//	do {
		//		walker->GetFirstChildElementBuildCache(
		//			pCurrent, cacheRequest, &pFirstChild);
		//		if (pFirstChild == nullptr && !queue.empty()) {	// when current has no child.
		//			found->push_back(CComPtr<IUIAutomationElement>(pCurrent));
		//			pCurrent = queue.front();
		//			queue.pop();
		//		}
		//		else {
		//			szNextDepthFirstId = szCurrentId;
		//			nDepth++;
		//			break;
		//		}
		//	} while (pFirstChild == nullptr);
		//}
		// push all children of pCurrent to the queue.
		if (pCurrent != nullptr) {
			HRESULT hr;

			CComPtr<IUIAutomationElementArray> pChildrenElementArray;
			hr = pCurrent->FindAllBuildCache(TreeScope_Children, 
					condition,
					cacheRequest,
					&pChildrenElementArray);
			throw_if_fail(hr);
			
			int nChildrenNum = 0;
			if(pChildrenElementArray != nullptr) {
				hr = pChildrenElementArray->get_Length(&nChildrenNum);
				throw_if_fail(hr);
			}
			for (int i = 0; i < nChildrenNum; ++i) {
				IUIAutomationElement *pChild;
				pChildrenElementArray->GetElement(i, &pChild);
				if (pChild != nullptr) {
					// for debug.
					BSTR szTemp;
					pChild->get_CurrentName(&szTemp);
					OutputDebugString(szTemp);

					pChild->get_CurrentControlType(&ControlType);
					if (!(ControlType == UIA_DocumentControlTypeId)) {
						queue.push(pChild);
					}
				}

			}
			found->push_back(CComPtr<IUIAutomationElement>(pCurrent));
		}

		if (nDepth >= restrict_depth) {
			break;
		}
	} while (!queue.empty());
	return S_OK;
}

// TODO: need to be refactored.
// use WalkDesiredElementBuildCache in this function. But performance is worse 
// than FindAll. 
BOOL EnumConditionedElementTest(HWND hMainWnd, HDC hdc) {
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
        auto pThreadElementVector(
			new std::vector<CComPtr<IUIAutomationElement>>[nChildrenNum]
		);
		CComPtr<IUIAutomationTreeWalker> pWalker;
		pAutomation->get_RawViewWalker(&pWalker);
		//pAutomation->CreateTreeWalker(pTrueCondition, &pWalker);
        // Task assignment for multi-threads.
        for(int j = 0; j < nChildrenNum; ++j) {
            IUIAutomationElement *pTempElement;
            pChildrenElementArray->GetElement(j, &pTempElement);
            pThread[j] = std::thread(&WalkDesiredElementBuildCache,
                    20,
                    pTempElement,
					pTotalCondition,
                    pCacheRequest,
					pWalker,
                    &pThreadElementVector[j]
                    );
        }

        int nTotalLength = 0;
        for(int j = 0; j < nChildrenNum; ++j) {
            pThread[j].join();
            int length;
            length = pThreadElementVector[j].size();
            nTotalLength += length;
        }

        hr = SafeArrayDestroy(pConditionVector);
        throw_if_fail(hr);

        std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>>
            TagMap(new std::map<string, CComPtr<IUIAutomationElement>>);

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
            length = pThreadElementVector[i].size();
            throw_if_fail(hr);
            auto ElementVector = pThreadElementVector[i];
            for (int j = 0; j < length; ++j) {
                CComPtr<IUIAutomationElement> pTempElement;
				pTempElement = ElementVector[j];
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
        pCtx->SetTagMap(TagMap);
        LockWindowUpdate(hForeWnd);
		
	}
	catch (_com_error err) {
	}
	return true;
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
KeyMouse::PElementVec EnumConditionedElement(HWND hMainWnd,HWND hForeWnd, HDC hdc) {
	try {
        DWORD start_time = GetTickCount();

        // Get current context.
        KeyMouse::Context *pCtx = 
            reinterpret_cast<KeyMouse::Context *>(
                    GetClassLongPtr(hMainWnd, 0)
                    );

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
        
        // add cache request for desired patterns and properties.
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
        LockWindowUpdate(hForeWnd);
		
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

void DrawTag(HWND hMainWnd, HWND hTransWnd, HDC hdc, POINT point, const TCHAR* psText) {
	KeyMouse::Context *pCtx = 
		reinterpret_cast<KeyMouse::Context *>(
				GetClassLongPtr(hMainWnd, 0)
				);
	auto profile = pCtx->GetProfile();

	ScreenToClient(hTransWnd, &point);

	HPEN hpenOld = static_cast<HPEN>(SelectObject(hdc, GetStockObject(DC_PEN)));
	HBRUSH hbrushOld = static_cast<HBRUSH>(SelectObject(hdc, GetStockObject(DC_BRUSH)));
	// background color.
	SetDCBrushColor(hdc, profile.background_color);
    
	int nHight = -MulDiv(profile.font.font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	HFONT hFont = CreateFont(nHight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, profile.font.font_name.c_str());

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
	SetTextColor(hdc, profile.font.font_color);
	
	DrawText(hdc, psText, -1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	SelectObject(hdc, hpenOld);
	SelectObject(hdc, hbrushOld);
	SelectObject(hdc, hfontOld);

	DeleteObject(hFont);

}

