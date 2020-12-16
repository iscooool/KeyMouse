#pragma once
#include <iostream>
#include "tag.h"
#include "ctx.h"



extern IUIAutomation *pAutomation;
inline void throw_if_fail(HRESULT hr);
HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation);
KeyMouse::Context *GetContext(HWND hMainWnd);
KeyMouse::PElementVec CachingElementsFromWindow(HWND hMainWnd, std::vector<HWND> TopWindowVec, KeyMouse::PElementVec(*lpEnumFunc)(HWND, HWND));
void CacheThread(HWND hWnd);
HWND CreateTransparentWindow(HINSTANCE hInstance, HWND hMainWnd);
KeyMouse::PElementVec EnumConditionedElement(HWND hMainWnd, HWND hForeWnd);
KeyMouse::PElementVec EnumConditionedElementForCaching(HWND hMainWnd, HWND hForeWnd);
bool isFocusOnEdit(HWND hMainWnd);
std::string GetLastErrorAsString();
void DrawTag(HWND hMainWnd, HWND hTransWnd, HDC hdc, POINT point, const TCHAR* psText, KeyMouse::Font font);
RECT RectForPerMonitorDPI(HWND hWnd, RECT Rect);

KeyMouse::PElementVec EnumTargetWindow(HWND hMainWnd, HWND hForeWnd);
