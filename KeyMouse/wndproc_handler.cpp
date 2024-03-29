#include "stdafx.h"
#include "ctx.h"
#include "Resource.h"
#include "utils.h"
#include "hotkey_handler.h"
#include "def.h"
namespace KeyMouse {
HKBinding WndProcHandler::select_hkbinding_[WndProcHandler::SELECT_HKBINDING_NUM] = {};
HKBinding WndProcHandler::normal_hkbinding_[WndProcHandler::NORMAL_HKBINDING_NUM] = {};

WndProcHandler::WndProcHandler() {
	// message handlers initializaion.
	event_handler_[0].Code = WM_COMMAND;
	event_handler_[0].fnPtr = fnWndProc_Command_;
	event_handler_[1].Code = WM_TRAY;
	event_handler_[1].fnPtr = fnWndProc_Tray_;
	event_handler_[2].Code = WM_HOTKEY;
	event_handler_[2].fnPtr = fnWndProc_Hotkey_;
	event_handler_[3].Code = WM_PAINT;
	event_handler_[3].fnPtr = fnWndProc_Paint_;
	event_handler_[4].Code = WM_DESTROY;
	event_handler_[4].fnPtr = fnWndProc_Destroy_;

}

WndProcHandler::~WndProcHandler() {
}

/**
* @brief: initial struct array of WM_HOTKEY massages' lParam and corresponding 
* process functions.
*
* @param: KeybindingMap& keybinding_map
*
* @return: void
*/
void WndProcHandler::InitialHKBinding(KeybindingMap& keybinding_map) {
	select_hkbinding_[0].lParam = keybinding_map["escape"].lParam;
	select_hkbinding_[0].fnPtr = fnHKProc_Escape_;
	select_hkbinding_[1].lParam = keybinding_map["rightClickPrefix"].lParam;
	select_hkbinding_[1].fnPtr = fnHKProc_RightClickPrefix_;
	select_hkbinding_[2].lParam = keybinding_map["singleClickPrefix"].lParam;
	select_hkbinding_[2].fnPtr = fnHKProc_SingleClickPrefix_;
	select_hkbinding_[3].lParam = keybinding_map["forceNotUseCache"].lParam;
	select_hkbinding_[3].fnPtr = fnHKProc_ForceNotUseCache_;

	normal_hkbinding_[0].lParam = keybinding_map["selectMode"].lParam;
	normal_hkbinding_[0].fnPtr = fnHKProc_SelectMode_;
	normal_hkbinding_[1].lParam = keybinding_map["toggleEnable"].lParam;
	normal_hkbinding_[1].fnPtr = fnHKProc_ToggleEnable_;
	normal_hkbinding_[2].lParam = keybinding_map["scrollDown"].lParam;
	normal_hkbinding_[2].fnPtr = fnHKProc_Scroll_Down_;
	normal_hkbinding_[3].lParam = keybinding_map["scrollUp"].lParam;
	normal_hkbinding_[3].fnPtr = fnHKProc_Scroll_Up_;
	normal_hkbinding_[4].lParam = keybinding_map["fastSelectMode"].lParam;
	normal_hkbinding_[4].fnPtr = fnHKProc_FastSelectMode_;
	normal_hkbinding_[5].lParam = keybinding_map["selectModeSingle"].lParam;
	normal_hkbinding_[5].fnPtr = fnHKProc_SelectModeSingle_;

}

/**
* @brief: the entrance of all windows process message.
*
* @param: UINT msg: the message from main window's WndProc.
*       : WndEventArgs& Wea: struct of window event arguments.
*
* @return: LRESULT
*/
LRESULT WndProcHandler::HandlerEntrance(UINT msg, const WndEventArgs& Wea) {

	for(int i = 0; i < EVENTHANDLER_NUM; i++) {
		if(event_handler_[i].Code==msg)
		{
		return (*event_handler_[i].fnPtr)(Wea);
		}
	}

	return (DefWindowProc(Wea.hWnd, msg, Wea.wParam, Wea.lParam));


}

LRESULT WndProcHandler::fnWndProc_Command_(const WndEventArgs& Wea) {
	int wmId = LOWORD(Wea.wParam);
	// Parse the menu selections:
	switch (wmId)
	{
	case IDM_ABOUT:
		DialogBox(Wea.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), Wea.hWnd, About_);
		break;
	case IDM_EXIT:
		DestroyWindow(Wea.hWnd);
		break;
	default:
		return DefWindowProc(Wea.hWnd, WM_COMMAND, Wea.wParam, Wea.lParam);
	}

	return 0;
}
LRESULT WndProcHandler::fnWndProc_Tray_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);

	int lmId = LOWORD(Wea.lParam);
	switch(lmId)
	{
		case WM_RBUTTONDOWN:
			{
				bool EnableState = pCtx->GetEnableState();
				UINT fState;
				if(EnableState)
					fState = MFS_UNCHECKED;
				else
					fState = MFS_CHECKED;
				MENUITEMINFO MenuItemInfo = {
						sizeof(MENUITEMINFO),
						MIIM_STATE,
						0,
						fState,
						0,
						NULL,
						NULL,
						NULL,
						NULL,
						NULL,
						0,
						0
				};
				HMENU hMenu;
				HMENU hSubMenu;
				POINT pt;
				// Get the current point of cursor.
				GetCursorPos(&pt);
				hMenu = LoadMenu(Wea.hInst, MAKEINTRESOURCE(IDR_POPUP_MENU));

				if(!hMenu)
					return -1;
				
				hSubMenu = GetSubMenu(hMenu, 0);
				// show check state.
				SetMenuItemInfo(hSubMenu, ID_TRAYMENU_DISABLE, false,
						&MenuItemInfo);

				SetForegroundWindow(Wea.hWnd);
				int cmd = TrackPopupMenu(hSubMenu, TPM_RETURNCMD,
										 pt.x, pt.y, NULL, Wea.hWnd, NULL);
				if(cmd == ID_TRAYMENU_EXIT)
					PostMessage(Wea.hWnd, WM_DESTROY, NULL, NULL);
				if(cmd == ID_TRAYMENU_ABOUT)
					DialogBox(Wea.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), 
							  Wea.hWnd, About_);

				if(cmd == ID_TRAYMENU_DISABLE) {
					if(EnableState) {
						pCtx->SetEnableState(false);
						UnregCustomHotKey(Wea.hWnd, "selectMode");
					} else {
						pCtx->SetEnableState(true);
						RegCustomHotKey(Wea.hWnd, "selectMode");
					}

				}
				DestroyMenu(hMenu);
			}
			break;
	}
	return 0;
}

LRESULT WndProcHandler::fnWndProc_Hotkey_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);

	auto Mode = pCtx->GetMode();
	if (Mode == Context::NORMAL_MODE) {
		for (int i = 0; i < NORMAL_HKBINDING_NUM; i++) {
			if (normal_hkbinding_[i].lParam == Wea.lParam) {
				return (*normal_hkbinding_[i].fnPtr)(Wea);
			}
		}
	}
	else if (Mode == Context::SELECT_MODE) {
		for (int i = 0; i < SELECT_HKBINDING_NUM; i++) {
			if (select_hkbinding_[i].lParam == Wea.lParam) {

				return (*select_hkbinding_[i].fnPtr)(Wea);
			}
		}

		// default operation which will consume all hotkeys about tags on the screen. when A <= virtualkey <= Z
		WORD VirtualKey = HIWORD(Wea.lParam);
		SelectModeHandler_(Wea.hWnd, VirtualKey);


	}

	 return 0;
}

LRESULT WndProcHandler::fnWndProc_Paint_(const WndEventArgs& Wea) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(Wea.hWnd, &ps);
	// TODO: Add any drawing code that uses hdc here...
	EndPaint(Wea.hWnd, &ps);
	return 0;
}

LRESULT WndProcHandler::fnWndProc_Destroy_(const WndEventArgs& Wea) {
	PostQuitMessage(0);
	return 0;
}

LRESULT WndProcHandler::fnHKProc_SelectMode_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);

	if (!pCtx->GetEnableState())
		return 0;
	auto Mode = pCtx->GetMode();
	// prevent from multiple selecting.
	if (Mode == Context::SELECT_MODE) {
		EscSelectMode_(Wea.hWnd);
		pCtx->SetMode(Context::NORMAL_MODE);
	}
	CompareBlackList_();

	pCtx->SetMode(Context::SELECT_MODE);
	// this will show all tags on the screen.
	CreateTransparentWindow(Wea.hInst, Wea.hWnd);

	if (pCtx->GetProfile().enable_cache) {
		RegCustomHotKey(Wea.hWnd, "forceNotUseCache");
	}
	RegCustomHotKey(Wea.hWnd, "escape");
	RegisterTagHotKey(Wea.hWnd);
	return 0;
}

LRESULT WndProcHandler::fnHKProc_SelectModeSingle_(const WndEventArgs& Wea) {
	fnHKProc_SelectMode_(Wea);
	fnHKProc_SingleClickPrefix_(Wea);
	return 0;
}

LRESULT WndProcHandler::fnHKProc_FastSelectMode_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);
	KeybindingMap keybinding_map = pCtx->GetKeybindingMap();
	pCtx->SetFastSelectState(true);
	PostMessage(Wea.hWnd, WM_HOTKEY, 0, keybinding_map["selectMode"].lParam);
	return 0;
}

LRESULT WndProcHandler::fnHKProc_RightClickPrefix_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);
	auto profile = pCtx->GetProfile();
	if (profile.invert_click_type) {
		pCtx->SetClickType(Context::SINGLE_LEFT_CLICK);
	}
	else {
		pCtx->SetClickType(Context::SINGLE_RIGHT_CLICK);
	}

	if (pCtx->GetFastSelectState()) {
		HWND hForeWnd = pCtx->GetForeWindow();
		CComPtr<IUIAutomationElement> pElement;
		pElement = pCtx->GetElement();
		CComPtr<IUnknown> pEHTemp = pCtx->GetStructEventHandler();
		if (pElement != nullptr && pEHTemp != nullptr) {

			IUIAutomation* pAutomation = pCtx->GetAutomation();
			HRESULT hr = pAutomation->RemoveAllEventHandlers();
			//HRESULT hr = pAutomation->RemoveStructureChangedEventHandler(pElement, (IUIAutomationStructureChangedEventHandler*)pEHTemp.p);
			pCtx->SetFastSelectState(false);
		}
	}
	return 0;
}
LRESULT WndProcHandler::fnHKProc_SingleClickPrefix_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);
	auto profile = pCtx->GetProfile();
	if (profile.invert_click_type) {
		pCtx->SetClickType(Context::SINGLE_RIGHT_CLICK);
	}
	else {
		pCtx->SetClickType(Context::SINGLE_LEFT_CLICK);
	}
	return 0;
}
LRESULT WndProcHandler::fnHKProc_Escape_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);

	EscSelectMode_(Wea.hWnd);
	if (pCtx->GetFastSelectState()) {
		HWND hForeWnd = pCtx->GetForeWindow();
		CComPtr<IUIAutomationElement> pElement;
		pElement = pCtx->GetElement();
		CComPtr<IUnknown> pEHTemp = pCtx->GetStructEventHandler();
		if (pElement != nullptr && pEHTemp != nullptr) {

			IUIAutomation* pAutomation = pCtx->GetAutomation();
			HRESULT hr = pAutomation->RemoveAllEventHandlers();
			//HRESULT hr = pAutomation->RemoveStructureChangedEventHandler(pElement, (IUIAutomationStructureChangedEventHandler*)pEHTemp.p);
			pCtx->SetFastSelectState(false);
		}
	}
	pCtx->SetMode(Context::NORMAL_MODE);

	return 0;
}
LRESULT WndProcHandler::fnHKProc_ToggleEnable_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);
	bool EnableState = pCtx->GetEnableState();
	if(EnableState) {
		pCtx->SetEnableState(false);
		UnregisterAllHotKey(Wea.hWnd, true);
	} else {
		pCtx->SetEnableState(true);
		RegisterAllHotKey(Wea.hWnd, true);
	}

	return 0;
}


LRESULT WndProcHandler::fnHKProc_Scroll_Up_(const WndEventArgs& Wea) {
	WORD VirtualKey = HIWORD(Wea.lParam);
	if (isFocusOnEdit(Wea.hWnd)) {
		EditInputForward_(Wea.hWnd, VirtualKey);
	}
	else {
		HWND handle = GetForegroundWindow();
		ScrollHandler_(handle, true);
	}
	return 0;
}

LRESULT WndProcHandler::fnHKProc_Scroll_Down_(const WndEventArgs& Wea) {
	WORD VirtualKey = HIWORD(Wea.lParam);
	if (isFocusOnEdit(Wea.hWnd)) {
		EditInputForward_(Wea.hWnd, VirtualKey);
	}
	else {
		HWND handle = GetForegroundWindow();
		ScrollHandler_(handle, false);
	}
	return 0;
}

LRESULT WndProcHandler::fnHKProc_ForceNotUseCache_(const WndEventArgs& Wea) {
	Context *pCtx = GetContext(Wea.hWnd);
	EscSelectMode_(Wea.hWnd);
	pCtx->SetEnableCache(false);
	fnHKProc_SelectMode_(Wea);
	pCtx->SetEnableCache(true);
	return 0;
}

void WndProcHandler::EscSelectMode_(HWND hWnd) {
	Context *pCtx = GetContext(hWnd);

    HWND handle = pCtx->GetTransWindow();
    // Enable window drawing.
    LockWindowUpdate(NULL);
    RedrawWindow(handle, NULL, NULL, 
                 RDW_INVALIDATE | RDW_ALLCHILDREN);
    DestroyWindow(handle);


	if (pCtx->GetProfile().enable_cache) {
		UnregCustomHotKey(hWnd, "forceNotUseCache");
	}
	UnregCustomHotKey(hWnd, "escape");
    UnregisterTagHotKey(hWnd);

    pCtx->SetCurrentTag(string(TEXT("")));
    pCtx->SetMaxTagLen(0);
    pCtx->SetMode(Context::NORMAL_MODE);
	auto profile = pCtx->GetProfile();
	//	clear click type.
	if (profile.invert_click_type) {
		pCtx->SetClickType(Context::RIGHT_CLICK);
	}
	else {
		pCtx->SetClickType(Context::LEFT_CLICK);
	}
}

void WndProcHandler::SelectModeHandler_(HWND hWnd, WORD VirtualKey) {
	Context *pCtx = GetContext(hWnd);
	KeybindingMap keybinding_map = pCtx->GetKeybindingMap();
    // If the VIrtualKey is out of our expectation.
    // i.e. VirtualKey is not in A - Z.
    if(VirtualKey < 0x41 || VirtualKey > 0x5A)
        return;
    
    TCHAR cInputChar = MapVirtualKey(VirtualKey, MAPVK_VK_TO_CHAR);
    size_t iMaxTagLen = pCtx->GetMaxTagLen();
    string szTag = pCtx->GetCurrentTag();

	const KeyMouse::PTagMap& pTagMap = pCtx->GetTagMap();
	const KeyMouse::PTagMap& pWindowMap = pCtx->GetWindowMap();

    szTag.append(string(1, cInputChar));
    pCtx->SetCurrentTag(szTag);
	HWND TransWindow = pCtx->GetTransWindow();

    if(pTagMap->find(szTag) != pTagMap->end()) {
        KeyMouse::ElementInfo pElement = (*pTagMap)[szTag];

		RECT Rect = pElement.rect;
		// pElement->get_CachedBoundingRectangle(&Rect);

		Rect = RectForPerMonitorDPI(TransWindow, Rect);
		if (pCtx->GetClickType() == Context::SINGLE_RIGHT_CLICK) {
			RightClick_((Rect.left + Rect.right) / 2, 
					(Rect.top + Rect.bottom) /2, 1);
		}
		else if (pCtx->GetClickType() == Context::SINGLE_LEFT_CLICK) {
			LeftClick_((Rect.left + Rect.right) / 2,
				(Rect.top + Rect.bottom) / 2, 1);

		}
		else {
			InvokeElement_(pElement, hWnd);
		}

        EscSelectMode_(hWnd);
	}
	else if (pWindowMap->find(szTag) != pWindowMap->end()) { // switch window.
		IUIAutomation* pAutomation = pCtx->GetAutomation();
        KeyMouse::ElementInfo pElement = (*pWindowMap)[szTag];
		IUIAutomationElement* pTempElement;
		pAutomation->ElementFromHandle(pElement.handle, &pTempElement);
		pTempElement->SetFocus();
		pTempElement->Release();
	}
    if(iMaxTagLen <= szTag.length())
        EscSelectMode_(hWnd);
}

void WndProcHandler::ScrollHandler_(HWND hWnd, bool Up) {
    
	GUITHREADINFO gti;
	gti.cbSize = sizeof(GUITHREADINFO);
	GetGUIThreadInfo(NULL, &gti);
	HWND hWindowToScroll = gti.hwndFocus;
	RECT r;
	GetClientRect(hWnd, &r);
	int iDirection = 0;
    if(Up)
		iDirection = 1;
    else
		iDirection = -1;
                
	SendMessage(hWindowToScroll, WM_MOUSEWHEEL, 
        MAKEWPARAM(0, WHEEL_DELTA * iDirection), 
        MAKELPARAM(r.right / 2, r.bottom / 2));

        
}
void WndProcHandler::LeftClick_(int x, int y, int time)
{
	const double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
	const double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	POINT cursorPos;
	GetCursorPos(&cursorPos);

	double cx = cursorPos.x * XSCALEFACTOR;
	double cy = cursorPos.y * YSCALEFACTOR;

	double nx = x * XSCALEFACTOR;
	double ny = y * YSCALEFACTOR;

	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;

	Input.mi.dx = (LONG)nx;
	Input.mi.dy = (LONG)ny;

	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;

	for (int i = 0; i < time; i++) {
		SendInput(1, &Input, sizeof(INPUT));
	}
	

	Input.mi.dx = (LONG)cx;
	Input.mi.dy = (LONG)cy;

	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

	SendInput(1, &Input, sizeof(INPUT));
}

void WndProcHandler::RightClick_(int x, int y, int time)
{
	const double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
	const double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	POINT cursorPos;
	GetCursorPos(&cursorPos);

	double cx = cursorPos.x * XSCALEFACTOR;
	double cy = cursorPos.y * YSCALEFACTOR;

	double nx = x * XSCALEFACTOR;
	double ny = y * YSCALEFACTOR;

	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;

	Input.mi.dx = (LONG)nx;
	Input.mi.dy = (LONG)ny;

	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;

	for (int i = 0; i < time; i++) {
		SendInput(1, &Input, sizeof(INPUT));
	}
	

	Input.mi.dx = (LONG)cx;
	Input.mi.dy = (LONG)cy;

	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

	SendInput(1, &Input, sizeof(INPUT));
}
void WndProcHandler::InvokeElement_(
	KeyMouse::ElementInfo &pElement,
	HWND hWnd) {
    try {
        CONTROLTYPEID iControlType = pElement.control_type;
		Context *pCtx = GetContext(hWnd);
		HWND TransWindow = pCtx->GetTransWindow();

        if(iControlType == UIA_ButtonControlTypeId ||
			iControlType == UIA_HyperlinkControlTypeId) {

            // Sometimes the ClickablePoint is not actually clickable, but
            // bClickable equals 1. 
            
			RECT Rect = pElement.rect;
			
			Rect = RectForPerMonitorDPI(TransWindow, Rect);
			if (pCtx->GetClickType() == Context::LEFT_CLICK) {
				LeftClick_((Rect.left + Rect.right) / 2,
					(Rect.top + Rect.bottom) / 2, 1);
			}
			else if (pCtx->GetClickType() == Context::RIGHT_CLICK) {
				RightClick_((Rect.left + Rect.right) / 2,
					(Rect.top + Rect.bottom) / 2, 1);
			}
        } else {
			RECT Rect = pElement.rect;
			Rect = RectForPerMonitorDPI(TransWindow, Rect);
			if (pCtx->GetClickType() == Context::LEFT_CLICK) {
				LeftClick_((Rect.left + Rect.right) / 2,
					(Rect.top + Rect.bottom) / 2, 2);
			}
			else if (pCtx->GetClickType() == Context::RIGHT_CLICK) {
				RightClick_((Rect.left + Rect.right) / 2,
					(Rect.top + Rect.bottom) / 2, 2);
			}
        }
    }
    catch (_com_error err) {
    }

}
// to avoid conflict when facus on text edit field.
void WndProcHandler::EditInputForward_(HWND hWnd, WORD VirtualKey) {
	UnregCustomHotKey(hWnd, "scrollDown");
	UnregCustomHotKey(hWnd, "scrollUp");
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    ip.ki.wVk = VirtualKey;
    ip.ki.dwFlags = 0;                   //Prepares key down
    SendInput(1, &ip, sizeof(INPUT));    //Key down
    ip.ki.dwFlags = KEYEVENTF_KEYUP;     //Prepares key up
    SendInput(1, &ip, sizeof(INPUT));    //Key up
	RegCustomHotKey(hWnd, "scrollDown");
	RegCustomHotKey(hWnd, "scrollUp");
}

bool WndProcHandler::CompareBlackList_() {
    HWND handle = GetForegroundWindow();
    DWORD pid;
    GetWindowThreadProcessId(handle, &pid);
    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, pid);    
    TCHAR lpFileName[100] = {0};
    GetModuleFileNameEx(
            hProc,
            NULL,
            lpFileName,
            100
            );

    CloseHandle(hProc);
    return true;
}

// Message handler for about box.
INT_PTR CALLBACK WndProcHandler::About_(
	HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam
)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
}
