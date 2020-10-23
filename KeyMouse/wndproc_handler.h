#pragma once
#include "stdafx.h"
#include "Resource.h"

// user defined message.
#define WM_TRAY WM_USER + 1

// user defined virtual key mapping.
#define KM_SCROLLDOWN 0x4A  // J
#define KM_SCROLLUP 0x4B    // K

namespace KeyMouse {
struct IdlParam{
	// id is used for RegisterHotKey.
	int id;
	LPARAM lParam;
};
using KeybindingMap = std::map<std::string, IdlParam>;

typedef struct    WindowsEventArguments {
	HWND             hWnd;
	WPARAM           wParam;
	LPARAM           lParam;
	HINSTANCE        hInst;
}WndEventArgs, *lpWndEventArgs;


struct EVENTHANDLER {
	unsigned int    Code;
	LRESULT(*fnPtr)(const WndEventArgs&);
};

struct HKBinding {
	LPARAM lParam;
	LRESULT(*fnPtr)(const WndEventArgs&);
};

/**
* @brief: handle all window messages.
*
*/
class WndProcHandler
{
public:
	static constexpr int EVENTHANDLER_NUM = 5;
	static constexpr int SELECT_HKBINDING_NUM = 3;
	static constexpr int NORMAL_HKBINDING_NUM = 5;

	WndProcHandler();
	~WndProcHandler();
	void InitialHKBinding(KeybindingMap& keybinding_map);
	LRESULT HandlerEntrance(UINT msg, const WndEventArgs& Wea);
	static LRESULT fnWndProc_Command_(const WndEventArgs& Wea);
	static LRESULT fnWndProc_Tray_(const WndEventArgs& Wea);
	static LRESULT fnWndProc_Hotkey_(const WndEventArgs& Wea);
	static LRESULT fnWndProc_Paint_(const WndEventArgs& Wea);
	static LRESULT fnWndProc_Destroy_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_SelectMode_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_FastSelectMode_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_RightClickPrefix_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_SingleClickPrefix_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_Escape_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_ToggleEnable_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_Scroll_(const WndEventArgs& Wea);

	static void EscSelectMode_(HWND hWnd);
	static void SelectModeHandler_(HWND hWnd, WORD VirtualKey);
	static void ScrollHandler_(HWND hWnd, WORD VirtualKey);
	static void LeftClick_(int x, int y, int time);
	static void RightClick_(int x, int y, int time);
	static void InvokeElement_(CComPtr<IUIAutomationElement> &pElement, HWND hWnd);
	static void EditInputForward_(HWND hWnd, WORD VirtualKey);
	static bool CompareBlackList_();
	static INT_PTR CALLBACK About_(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	EVENTHANDLER event_handler_[EVENTHANDLER_NUM];
	static HKBinding select_hkbinding_[SELECT_HKBINDING_NUM];
	static HKBinding normal_hkbinding_[NORMAL_HKBINDING_NUM];

};
}

