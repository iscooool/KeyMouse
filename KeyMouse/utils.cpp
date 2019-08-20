#include "stdafx.h"
#include "utils.h"

inline void throw_if_fail(HRESULT hr) {
	if (FAILED(hr))
		throw _com_error(hr);
}

IUIAutomation *pAutomation;

HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation) {
	return CoCreateInstance(CLSID_CUIAutomation, nullptr,
		CLSCTX_INPROC_SERVER, IID_IUIAutomation,
		reinterpret_cast<void**>(ppAutomation));
}
BOOL EnumConditionedElement(HWND handle, HWND hWnd) {
	try {
		//IUIAutomationElement *pElement = nullptr;
		CComPtr<IUIAutomationElement> pElement;
		HRESULT hr = pAutomation->ElementFromHandle(handle, &pElement);
		throw_if_fail(hr);

		BSTR pStr;
		pElement->get_CurrentName(&pStr);
		cout << pStr << std::endl;

		CComPtr<IUIAutomationElementArray> pElementArray;
		// Define the condition by pTotalCondition to find all desired items.
        SAFEARRAY *pConditionVector = SafeArrayCreateVector(
                VT_UNKNOWN,
                0,
                CONDITION_NUM
                );
        std::vector<PROPERTYID> vPropertyId = {
            UIA_ListItemControlTypeId,
            UIA_ButtonControlTypeId,
            UIA_TreeItemControlTypeId,
            UIA_TabItemControlTypeId,
            UIA_HyperlinkControlTypeId
        };

        LONG i = 0;
        for (PROPERTYID PropertyId : vPropertyId) {
            IUIAutomationCondition *pCondition;
            VARIANT Val;
            Val.vt = VT_I4;
            Val.lVal = PropertyId;
            hr = pAutomation->CreatePropertyCondition(UIA_ControlTypePropertyId,
                Val,
                &pCondition);
            throw_if_fail(hr);
            
            hr = SafeArrayPutElement(pConditionVector, &i, pCondition); 
            throw_if_fail(hr);

            ++i;
        }
        CComPtr<IUIAutomationCondition> pTotalCondition;
        
        hr = pAutomation->CreateOrConditionFromArray(
                pConditionVector,
                &pTotalCondition
                );
        throw_if_fail(hr);
		hr = pElement->FindAll(TreeScope_Descendants, pTotalCondition,
			&pElementArray);
		throw_if_fail(hr);

        hr = SafeArrayDestroy(pConditionVector);
        throw_if_fail(hr);

		std::unique_ptr<int> pLength(new int());
		hr = pElementArray->get_Length(pLength.get());
		throw_if_fail(hr);

        // Get current context.
        KeyMouse::Context *pCtx = 
            reinterpret_cast<KeyMouse::Context *>(
                    GetWindowLongPtr(hWnd, 0)
                    );
        std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>>
            TagMap(new std::map<string, CComPtr<IUIAutomationElement>>);

		KeyMouse::TagCreator TC;
        std::queue<string> TagQueue = TC.AllocTag(*pLength);
        // the last one of the queue must be the longest one.
        pCtx->SetMaxTagLen(TagQueue.back().length());
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = GetDC(handle);
		// Traverse the items of ListControlType.
		for (int i = 0; i < *pLength; ++i) {
			//IUIAutomationElement *pTempElement;
			CComPtr<IUIAutomationElement> pTempElement;
			pElementArray->GetElement(i, &pTempElement);
			RECT Rect;
			pTempElement->get_CurrentBoundingRectangle(&Rect);
			string szTemp = TagQueue.front();
            TagQueue.pop();
            // print the tag on the screen.
			const TCHAR *psText = szTemp.c_str();
			POINT point;
			point.x = Rect.left;
			point.y = Rect.top;
			ScreenToClient(handle, &point);

			TextOut(hdc,
				point.x,
				point.y,
				psText, _tcslen(psText));
            // insert the tag and Element into the keymap.
            TagMap->insert(std::pair<string, CComPtr<IUIAutomationElement>>(
                         szTemp, pTempElement)); 

		}
        pCtx->SetTagMap(TagMap);
		ReleaseDC(handle, hdc);
	}
	catch (_com_error err) {
	}
	return true;
}
