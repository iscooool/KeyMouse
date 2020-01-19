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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
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
    SetClassLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(pCtx.get()));
    KeyMouse::RegisterAllHotKey(hWnd);

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
	
    KeyMouse::UnregisterAllHotKey(hWnd);
	pAutomation->Release();
	CoUninitialize();

	pCtx.reset();
	_CrtDumpMemoryLeaks();

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
    wcex.cbClsExtra     = sizeof(KeyMouse::Context*); // Extra space for Context.
	wcex.cbWndExtra     = 0;
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

	// Initialize COM before using UI Automation.
	//HRESULT hr = CoInitialize(nullptr);
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	hr = InitializeUIAutomation(&pAutomation);
    return hWnd;
}

//
//
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	KeyMouse::WndEventArgs Wea;
	Wea.hWnd = hWnd;
	Wea.wParam = wParam;
	Wea.lParam = lParam;
	Wea.hInst = hInst;

    // Get current context.
    KeyMouse::Context *pCtx = 
        reinterpret_cast<KeyMouse::Context *>(
                GetClassLongPtr(hWnd, 0)
                );
	if (pCtx) {
		KeyMouse::WndProcHandler WPHandler = pCtx->GetWndProcHandler();
		return WPHandler.HandlerEntrance(message, Wea);
	}

    return DefWindowProc(hWnd, message, wParam, lParam);
    
}

