#pragma once
#include <iostream>
#include "tag.h"
#include "ctx.h"


extern IUIAutomation *pAutomation;
inline void throw_if_fail(HRESULT hr);
HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation);
HWND CreateTransparentWindow(HINSTANCE hInstance, HWND hMainWnd);
BOOL  EnumConditionedElementTest(HWND hMainWnd, HDC hdc);
BOOL  EnumConditionedElement(HWND hMainWnd, HDC hdc);
bool isFocusOnEdit();
std::string GetLastErrorAsString();
void DrawTag(HWND hMainWnd, HWND hTransWnd, HDC hdc, POINT point, const TCHAR* psText);

