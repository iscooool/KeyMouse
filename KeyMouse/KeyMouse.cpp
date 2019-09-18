// KeyMouse.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "KeyMouse.h"


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
NOTIFYICONDATA structNID;                       // The tray struct

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                InitTray(HINSTANCE hInstance, HWND hwnd);
BOOL                UnregisterAllHotKey(HWND hWnd);
BOOL                RegisterAllHotKey(HWND hWnd);
void                InvokeElement(CComPtr<IUIAutomationElement> &pElement);
void                ScrollElement(CComPtr<IUIAutomationElement> &pElement, 
                    ScrollAmount amount);
BOOL                RegisterTagHotKey(HWND hWnd);
BOOL                UnregisterTagHotKey(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KEYMOUSE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    
    HWND hWnd;
    // Perform application initialization:
    if (!(hWnd = InitInstance (hInstance, nCmdShow)))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYMOUSE));
    
    std::unique_ptr<KeyMouse::Context> pCtx(new KeyMouse::Context());
    // Store a Context pointer in extra space of window hWnd.
    SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(pCtx.get()));
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	
    UnregisterAllHotKey(hWnd);
	pAutomation->Release();
	CoUninitialize();
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = sizeof(KeyMouse::Context*); // Extra space for Context.
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYMOUSE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_KEYMOUSE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

void InitTray(HINSTANCE hInstance, HWND hwnd) {
    structNID.cbSize = sizeof(NOTIFYICONDATA);
    structNID.hWnd = hwnd;
    structNID.uID = IDI_TRAY;
    structNID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    structNID.uCallbackMessage = WM_TRAY;
    structNID.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY));
    lstrcpy(structNID.szTip, TEXT("KeyMouse"));

    Shell_NotifyIcon(NIM_ADD, &structNID);
}
   
BOOL RegisterAllHotKey(HWND hWnd) {
    return (
        RegisterHotKey(hWnd, SHOWTAG, MOD_ALT | MOD_NOREPEAT, VK_OEM_1 /* ; */) &&
        RegisterHotKey(hWnd, TOGGLEENABLE, 0, VK_F11) &&
        RegisterHotKey(hWnd, SCROLLDOWN, 0, 0x4A /* J */) &&
        RegisterHotKey(hWnd, SCROLLUP, 0, 0x4B /* K */) 
        );
}
BOOL RegisterTagHotKey(HWND hWnd) {
    return (
        RegisterHotKey(hWnd, HOTKEY_A, 0, 0x41 /* A */) && 
        RegisterHotKey(hWnd, HOTKEY_B, 0, 0x42 /* B */) && 
        RegisterHotKey(hWnd, HOTKEY_C, 0, 0x43 /* C */) && 
        RegisterHotKey(hWnd, HOTKEY_D, 0, 0x44 /* D */) && 
        RegisterHotKey(hWnd, HOTKEY_E, 0, 0x45 /* E */) && 
        RegisterHotKey(hWnd, HOTKEY_F, 0, 0x46 /* F */) && 
        RegisterHotKey(hWnd, HOTKEY_G, 0, 0x47 /* G */) && 
        RegisterHotKey(hWnd, HOTKEY_H, 0, 0x48 /* H */) && 
        RegisterHotKey(hWnd, HOTKEY_I, 0, 0x49 /* I */) && 
        //RegisterHotKey(hWnd, HOTKEY_J, 0, 0x4A /* J */) && 
        //RegisterHotKey(hWnd, HOTKEY_K, 0, 0x4B /* K */) && 
        RegisterHotKey(hWnd, HOTKEY_L, 0, 0x4C /* L */) && 
        RegisterHotKey(hWnd, HOTKEY_M, 0, 0x4D /* M */) && 
        RegisterHotKey(hWnd, HOTKEY_N, 0, 0x4E /* N */) && 
        RegisterHotKey(hWnd, HOTKEY_O, 0, 0x4F /* O */) && 
        RegisterHotKey(hWnd, HOTKEY_P, 0, 0x50 /* P */) && 
        RegisterHotKey(hWnd, HOTKEY_Q, 0, 0x51 /* Q */) && 
        RegisterHotKey(hWnd, HOTKEY_R, 0, 0x52 /* R */) && 
        RegisterHotKey(hWnd, HOTKEY_S, 0, 0x53 /* S */) && 
        RegisterHotKey(hWnd, HOTKEY_T, 0, 0x54 /* T */) && 
        RegisterHotKey(hWnd, HOTKEY_U, 0, 0x55 /* U */) && 
        RegisterHotKey(hWnd, HOTKEY_V, 0, 0x56 /* V */) && 
        RegisterHotKey(hWnd, HOTKEY_W, 0, 0x57 /* W */) && 
        RegisterHotKey(hWnd, HOTKEY_X, 0, 0x58 /* X */) && 
        RegisterHotKey(hWnd, HOTKEY_Y, 0, 0x59 /* Y */) && 
        RegisterHotKey(hWnd, HOTKEY_Z, 0, 0x5A /* Z */)
        );
}
BOOL UnregisterAllHotKey(HWND hWnd) {
    return (
        UnregisterHotKey(hWnd, SHOWTAG) &&
        UnregisterHotKey(hWnd, TOGGLEENABLE) &&
        UnregisterHotKey(hWnd, SCROLLDOWN) &&
        UnregisterHotKey(hWnd, SCROLLUP)
        );
}
BOOL UnregisterTagHotKey(HWND hWnd) {
    return (
        UnregisterHotKey(hWnd, HOTKEY_A) && 
        UnregisterHotKey(hWnd, HOTKEY_B) && 
        UnregisterHotKey(hWnd, HOTKEY_C) && 
        UnregisterHotKey(hWnd, HOTKEY_D) && 
        UnregisterHotKey(hWnd, HOTKEY_E) && 
        UnregisterHotKey(hWnd, HOTKEY_F) && 
        UnregisterHotKey(hWnd, HOTKEY_G) && 
        UnregisterHotKey(hWnd, HOTKEY_H) && 
        UnregisterHotKey(hWnd, HOTKEY_I) && 
        //UnregisterHotKey(hWnd, HOTKEY_J) && 
        //UnregisterHotKey(hWnd, HOTKEY_K) && 
        UnregisterHotKey(hWnd, HOTKEY_L) && 
        UnregisterHotKey(hWnd, HOTKEY_M) && 
        UnregisterHotKey(hWnd, HOTKEY_N) && 
        UnregisterHotKey(hWnd, HOTKEY_O) && 
        UnregisterHotKey(hWnd, HOTKEY_P) && 
        UnregisterHotKey(hWnd, HOTKEY_Q) && 
        UnregisterHotKey(hWnd, HOTKEY_R) && 
        UnregisterHotKey(hWnd, HOTKEY_S) && 
        UnregisterHotKey(hWnd, HOTKEY_T) && 
        UnregisterHotKey(hWnd, HOTKEY_U) && 
        UnregisterHotKey(hWnd, HOTKEY_V) && 
        UnregisterHotKey(hWnd, HOTKEY_W) && 
        UnregisterHotKey(hWnd, HOTKEY_X) && 
        UnregisterHotKey(hWnd, HOTKEY_Y) && 
        UnregisterHotKey(hWnd, HOTKEY_Z)
        );
}

void EscSelectMode(HWND hWnd) {

    //HWND handle = GetForegroundWindow();
    // Get current context.
    KeyMouse::Context *pCtx = 
        reinterpret_cast<KeyMouse::Context *>(
                GetWindowLongPtr(hWnd, 0)
                );
    HWND handle = pCtx->GetTransWindow();
    // Enable window drawing.
    LockWindowUpdate(NULL);
    RedrawWindow(handle, NULL, NULL, 
                 RDW_INVALIDATE | RDW_ALLCHILDREN);
    DestroyWindow(handle);
    UnregisterHotKey(hWnd, CLEANTAG);
    UnregisterTagHotKey(hWnd);
    pCtx->SetCurrentTag(string(TEXT("")));
    pCtx->SetMaxTagLen(0);
    pCtx->SetMode(KeyMouse::Context::NORMAL_MODE);
}

void SelectModeHandle(HWND hWnd, WORD VirtualKey) {
    // Get current context.
    KeyMouse::Context *pCtx = 
        reinterpret_cast<KeyMouse::Context *>(
                GetWindowLongPtr(hWnd, 0)
                );
    // If the VIrtualKey is out of our expectation.
    // i.e. VirtualKey is not in A - Z.
    if(VirtualKey < 0x41 || VirtualKey > 0x5A)
        return;
    
    TCHAR cInputChar = MapVirtualKey(VirtualKey, MAPVK_VK_TO_CHAR);
    size_t iMaxTagLen = pCtx->GetMaxTagLen();
    string szTag = pCtx->GetCurrentTag();
    const std::unique_ptr<
        std::map<string, CComPtr<IUIAutomationElement>>> 
        &pTagMap = pCtx->GetTagMap();

    szTag.append(string(1, cInputChar));
    pCtx->SetCurrentTag(szTag);
    if(pTagMap->find(szTag) != pTagMap->end()) {
        //MessageBox(nullptr, TEXT("test"), 
                //TEXT("contains"),  MB_OK);
        CComPtr<IUIAutomationElement> pElement =
            (*pTagMap)[szTag];
        InvokeElement(pElement);

        EscSelectMode(hWnd);
    }
    if(iMaxTagLen <= szTag.length())
        EscSelectMode(hWnd);
}

void NormalModeHandle(HWND hWnd, WORD VirtualKey) {
    switch (VirtualKey) {
        case KM_SCROLLUP:
        case KM_SCROLLDOWN:
            {
                GUITHREADINFO gti;
                gti.cbSize = sizeof(GUITHREADINFO);
                GetGUIThreadInfo(NULL, &gti);
                HWND hWindowToScroll = gti.hwndFocus;
				RECT r;
				GetClientRect(hWnd, &r);
                int iDirection = 0;
                if(VirtualKey == KM_SCROLLDOWN)
                    iDirection = -1;
                else if (VirtualKey == KM_SCROLLUP)
                    iDirection = 1;
                
				SendMessage(hWindowToScroll, WM_MOUSEWHEEL, 
                        MAKEWPARAM(0, WHEEL_DELTA * iDirection), 
                        MAKELPARAM(r.right / 2, r.bottom / 2));

            }
            break;
        default:
            {
            }
    }
}
void SingleClick(int x, int y)
{
	const double XSCALEFACTOR = 65535 / (GetSystemMetrics(SM_CXSCREEN) - 1);
	const double YSCALEFACTOR = 65535 / (GetSystemMetrics(SM_CYSCREEN) - 1);

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

	SendInput(1, &Input, sizeof(INPUT));
	

	Input.mi.dx = (LONG)cx;
	Input.mi.dy = (LONG)cy;

	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

	SendInput(1, &Input, sizeof(INPUT));
}

void InvokeElement(CComPtr<IUIAutomationElement> &pElement) {
    try {
        CONTROLTYPEID iControlType;
        HRESULT hr = pElement->get_CachedControlType(&iControlType); 
        throw_if_fail(hr); 
        if(iControlType == UIA_TabItemControlTypeId ||
           iControlType == UIA_TreeItemControlTypeId ||
		   iControlType == UIA_ButtonControlTypeId) {

            // Sometimes the ClickablePoint is not actually clickable, but
            // bClickable equals 1. So comment the code.
            
            //POINT point;
            //BOOL bClickable;
            //pElement->GetClickablePoint(&point, &bClickable);
            //if(bClickable) {
                //SingleClick(point.x, point.y);
            //} else {
                RECT Rect;
                pElement->get_CachedBoundingRectangle(&Rect);
                SingleClick((Rect.left + Rect.right) / 2, 
                        (Rect.top + Rect.bottom) /2);
            //}
        } else {
            CComPtr<IUIAutomationInvokePattern> pInvoke;
            hr = pElement->GetCachedPatternAs(
                    UIA_InvokePatternId,
                    __uuidof(IUIAutomationInvokePattern),
                    reinterpret_cast<void **>(&pInvoke)
                    );
			if(pInvoke)
				pInvoke->Invoke();
			else {
				RECT Rect;
				pElement->get_CachedBoundingRectangle(&Rect);
				SingleClick((Rect.left + Rect.right) / 2,
					(Rect.top + Rect.bottom) / 2);
			}
        }
    }
    catch (_com_error err) {
    }

}
// to avoid conflict when facus on text edit field.
void EditInputProxy(HWND hWnd, WORD VirtualKey) {
    UnregisterHotKey(hWnd, SCROLLDOWN);
    UnregisterHotKey(hWnd, SCROLLUP);
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
    RegisterHotKey(hWnd, SCROLLDOWN, 0, 0x4A /* J */);
    RegisterHotKey(hWnd, SCROLLUP, 0, 0x4B /* K */); 
}

bool CompareBlackList() {
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

// TODO: unused. delete it later.
void ScrollElement(CComPtr<IUIAutomationElement> &pElement, ScrollAmount amount) {
    try {
        CComPtr<IUIAutomationScrollPattern> pScroll;
        HRESULT hr = pElement->GetCurrentPatternAs(
                UIA_ScrollPatternId,
                __uuidof(IUIAutomationScrollPattern),
                reinterpret_cast<void **>(&pScroll)
                );
        if(pScroll) {
            BOOL bVScrollable;
            BOOL bHScrollable;    
            pScroll->get_CurrentVerticallyScrollable(&bVScrollable);
            pScroll->get_CurrentHorizontallyScrollable(&bHScrollable);
            if(bVScrollable)
                pScroll->Scroll(ScrollAmount_NoAmount, amount);
            else if(bHScrollable) {
                pScroll->Scroll(amount, ScrollAmount_NoAmount);
            }
                
        }
    }
    catch (_com_error err) {
    }

}
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
      return hWnd;
    }

    //ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    InitTray(hInst, hWnd);

    RegisterAllHotKey(hWnd);
	// Initialize COM before using UI Automation.
	//HRESULT hr = CoInitialize(nullptr);
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	hr = InitializeUIAutomation(&pAutomation);
    return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Get current context.
    KeyMouse::Context *pCtx = 
        reinterpret_cast<KeyMouse::Context *>(
                GetWindowLongPtr(hWnd, 0)
                );
    
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TRAY:   // display tray menu.
        {
            int lmId = LOWORD(lParam);
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
                        hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPUP_MENU));

                        if(!hMenu)
                            return -1;
                        
                        hSubMenu = GetSubMenu(hMenu, 0);
                        // show check state.
                        SetMenuItemInfo(hSubMenu, ID_TRAYMENU_DISABLE, false,
                                &MenuItemInfo);

                        SetForegroundWindow(hWnd);
                        int cmd = TrackPopupMenu(hSubMenu, TPM_RETURNCMD,
                                                 pt.x, pt.y, NULL, hWnd, NULL);
                        if(cmd == ID_TRAYMENU_EXIT)
                            PostMessage(hWnd, WM_DESTROY, NULL, NULL);
                        if(cmd == ID_TRAYMENU_ABOUT)
                            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), 
                                      hWnd, About);
                        if(cmd == ID_TRAYMENU_DISABLE) {
                            if(EnableState) {
                                pCtx->SetEnableState(false);
                                UnregisterHotKey(hWnd, SHOWTAG);
                            } else {
                                pCtx->SetEnableState(true);
                                RegisterHotKey(hWnd, SHOWTAG, 
                                        MOD_ALT | MOD_NOREPEAT, 
                                        VK_OEM_1 /* ; */); 
                            }

                        }
                        DestroyMenu(hMenu);
                    }
                    break;
            }
        }
        break;
    case WM_HOTKEY:
         {
             switch(LOWORD(lParam))
             {
                 case MOD_ALT:
                     {
                     switch(HIWORD(lParam))
                     {
                         case VK_OEM_1: // enter selcet mode.
                             {
                                if(!pCtx->GetEnableState())
                                    break;
                                CompareBlackList();

                                pCtx->SetMode(KeyMouse::Context::SELECT_MODE);
                                HWND handle = GetForegroundWindow();
                                EnumConditionedElement(handle, hWnd, hInst);
                                RegisterHotKey(hWnd, CLEANTAG, 0, VK_ESCAPE);
                                RegisterTagHotKey(hWnd);

                             }
                             break;
                     }
                     }
                     break;
                 default:
                     {
                     WORD VirtualKey = HIWORD(lParam);
                     switch(VirtualKey)
                     {
                         case VK_ESCAPE:
                             {
                                 EscSelectMode(hWnd);
                                 pCtx->SetMode(KeyMouse::Context::NORMAL_MODE);
                             }
                             break;
                         case VK_F11:   //enable or disable program.
                             {
                                bool EnableState = pCtx->GetEnableState();
                                if(EnableState) {
                                    pCtx->SetEnableState(false);
                                    UnregisterHotKey(hWnd, SHOWTAG);
                                    UnregisterHotKey(hWnd, SCROLLDOWN);
                                    UnregisterHotKey(hWnd, SCROLLUP);
                                } else {
                                    pCtx->SetEnableState(true);
                                    RegisterHotKey(hWnd, SHOWTAG, 
                                            MOD_ALT | MOD_NOREPEAT, 
                                            VK_OEM_1 /* ; */); 
                                    RegisterHotKey(hWnd, SCROLLDOWN, 0, 0x4A /* J */);
                                    RegisterHotKey(hWnd, SCROLLUP, 0, 0x4B /* K */); 
                                }
                             }
                             break;
                         default:   // for entering tags in select mode.
                             {
                                auto mode = pCtx->GetMode();
                                if(mode == KeyMouse::Context::SELECT_MODE) {
                                    SelectModeHandle(hWnd, VirtualKey);
								}
								else if (isFocusOnEdit()) {
									EditInputProxy(hWnd, VirtualKey);
								}
                                else if ( mode == KeyMouse::Context::NORMAL_MODE) {
									HWND handle = GetForegroundWindow();
                                    NormalModeHandle(handle, VirtualKey);
                                }

                             }
                     }

                     }
             }
         }
         break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
