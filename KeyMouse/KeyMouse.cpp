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
        RegisterHotKey(hWnd, SHOWTAG, MOD_SHIFT | MOD_NOREPEAT, 0x46 /* F */) &&
        RegisterHotKey(hWnd, TOGGLEENABLE, 0, VK_F11)
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
        RegisterHotKey(hWnd, HOTKEY_J, 0, 0x4A /* J */) && 
        RegisterHotKey(hWnd, HOTKEY_K, 0, 0x4B /* K */) && 
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
        UnregisterHotKey(hWnd, TOGGLEENABLE)
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
        UnregisterHotKey(hWnd, HOTKEY_J) && 
        UnregisterHotKey(hWnd, HOTKEY_K) && 
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
        HRESULT hr = pElement->get_CurrentControlType(&iControlType); 
        throw_if_fail(hr); 
        if(iControlType == UIA_TreeItemControlTypeId ||
           iControlType == UIA_TabItemControlTypeId) {
            CComPtr<IUIAutomationSelectionItemPattern> pInvoke;
            hr = pElement->GetCurrentPatternAs(
                    UIA_SelectionItemPatternId,
                    __uuidof(IUIAutomationSelectionItemPattern),
                    reinterpret_cast<void **>(&pInvoke)
                    );
            pInvoke->Select();

        } else if(iControlType == UIA_PaneControlTypeId) {
            POINT point;
            BOOL bClickable;
            pElement->GetClickablePoint(&point, &bClickable);
            if(bClickable) {
                SingleClick(point.x, point.y);

            }
        } else {
            CComPtr<IUIAutomationInvokePattern> pInvoke;
            hr = pElement->GetCurrentPatternAs(
                    UIA_InvokePatternId,
                    __uuidof(IUIAutomationInvokePattern),
                    reinterpret_cast<void **>(&pInvoke)
                    );
            pInvoke->Invoke();
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
	HRESULT hr = CoInitialize(nullptr);
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
    case WM_TRAY:
        {
            int lmId = LOWORD(lParam);
            switch(lmId)
            {
                case WM_RBUTTONDOWN:
                    {
                        // Get current context.
                        KeyMouse::Context *pCtx = 
                            reinterpret_cast<KeyMouse::Context *>(
                                    GetWindowLongPtr(hWnd, 0)
                                    );
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
                                        MOD_SHIFT | MOD_NOREPEAT, 
                                        0x46 /* F */); 
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
                 case MOD_SHIFT:
                     {
                     switch(HIWORD(lParam))
                     {
                         case 0x46 /* F */:
                             {
                                // Get current context.
                                KeyMouse::Context *pCtx = 
                                    reinterpret_cast<KeyMouse::Context *>(
                                            GetWindowLongPtr(hWnd, 0)
                                            );
                                if(!pCtx->GetEnableState())
                                    break;

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
                             }
                             break;
                         case VK_F11:
                             {
                                // Get current context.
                                KeyMouse::Context *pCtx = 
                                    reinterpret_cast<KeyMouse::Context *>(
                                            GetWindowLongPtr(hWnd, 0)
                                            );
                                bool EnableState = pCtx->GetEnableState();
                                if(EnableState) {
                                    pCtx->SetEnableState(false);
                                    UnregisterHotKey(hWnd, SHOWTAG);
                                } else {
                                    pCtx->SetEnableState(true);
                                    RegisterHotKey(hWnd, SHOWTAG, 
                                            MOD_SHIFT | MOD_NOREPEAT, 
                                            0x46 /* F */); 
                                }
                             }
                             break;
                         default:
                             {
                                // If the VIrtualKey is out of our expection.
                                // i.e. VirtualKey is not in A - Z.
                                if(VirtualKey < 0x41 || VirtualKey > 0x5A)
                                    break;
                                
                                TCHAR cInputChar = MapVirtualKey(VirtualKey,
                                                    MAPVK_VK_TO_CHAR);
                                // Get current context.
                                KeyMouse::Context *pCtx = 
                                    reinterpret_cast<KeyMouse::Context *>(
                                            GetWindowLongPtr(hWnd, 0)
                                            );
                                size_t iMaxTagLen = pCtx->GetMaxTagLen();
                                string szTag = pCtx->GetCurrentTag();
                                const std::unique_ptr<
                                    std::map<string, CComPtr<IUIAutomationElement>>
                                    > &pTagMap = pCtx->GetTagMap();

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
