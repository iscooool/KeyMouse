#include"stdafx.h"
#include"hotkey_handler.h"
#include "wndproc_handler.h"
#include"KeyMouse.h"
#include "utils.h"
namespace KeyMouse {
BOOL RegCustomHotKey(HWND hWnd, std::string key) {
	Context *pCtx = reinterpret_cast<Context *>(GetClassLongPtr(hWnd, 0));
	KeybindingMap keybinding_map = pCtx->GetKeybindingMap();
	int id = keybinding_map[key].id;
	UINT fsModifiers = LOWORD(keybinding_map[key].lParam);
	UINT vk = HIWORD(keybinding_map[key].lParam);
	return RegisterHotKey(hWnd, id, fsModifiers, vk);

}
BOOL UnregCustomHotKey(HWND hWnd, std::string key) {
	Context *pCtx = reinterpret_cast<Context *>(GetClassLongPtr(hWnd, 0));
	KeybindingMap keybinding_map = pCtx->GetKeybindingMap();
	int id = keybinding_map[key].id;
	return UnregisterHotKey(hWnd, id);
}

BOOL RegisterAllHotKey(HWND hWnd, bool exclude_toggle) {
	BOOL flag = TRUE;
	if (!exclude_toggle) {
		flag = RegCustomHotKey(hWnd, "toggleEnable");
		auto str = GetLastErrorAsString();
	}
	return (
		flag &&
		RegCustomHotKey(hWnd, "selectMode") &&
		RegCustomHotKey(hWnd, "scrollUp") &&
		RegCustomHotKey(hWnd, "scrollDown")
		);
}
BOOL RegisterTagHotKey(HWND hWnd) {
	return (
		// unregister the conflict hotkeys first.
		UnregCustomHotKey(hWnd, "scrollDown") &&
		UnregCustomHotKey(hWnd, "scrollUp") &&
		// register hotkey for tags input.
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
BOOL UnregisterAllHotKey(HWND hWnd, bool exclude_toggle) {
	BOOL flag = TRUE;
	if (!exclude_toggle) {
		flag = UnregCustomHotKey(hWnd, "toggleEnable");
	}
	return (
		flag &&
		UnregCustomHotKey(hWnd, "selectMode") &&
		UnregCustomHotKey(hWnd, "scrollUp") &&
		UnregCustomHotKey(hWnd, "scrollDown")
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
		UnregisterHotKey(hWnd, HOTKEY_Z) &&
		// register the previous hotkeys.
		RegCustomHotKey(hWnd, "scrollDown") &&
		RegCustomHotKey(hWnd, "scrollUp")
		);
}
}
