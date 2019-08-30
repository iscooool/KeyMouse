#pragma once
#include <iostream>
#include "tag.h"
#include "ctx.h"


extern IUIAutomation *pAutomation;
inline void throw_if_fail(HRESULT hr);
HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation);
BOOL EnumConditionedElement(HWND handle, HWND hWnd, HINSTANCE hInst);