#pragma once
#include <iostream>
#include "tag.h"
#include "ctx.h"


extern IUIAutomation *pAutomation;
inline void throw_if_fail(HRESULT hr);
HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation);
HWND CreateTransparentWindow(HINSTANCE hInstance, HWND hMainWnd);
KeyMouse::PElementVec EnumConditionedElement(HWND hMainWnd, HWND hForeWnd);
bool isFocusOnEdit();
std::string GetLastErrorAsString();
void DrawTag(HWND hMainWnd, HWND hTransWnd, HDC hdc, POINT point, const TCHAR* psText, KeyMouse::Font font);
RECT RectForPerMonitorDPI(HWND hWnd, RECT Rect);

