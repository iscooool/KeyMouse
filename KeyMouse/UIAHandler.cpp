#include "stdafx.h"
#include "UIAHandler.h"
#include "ctx.h"
#include "utils.h"

namespace KeyMouse {
HRESULT STDMETHODCALLTYPE EventHandler::HandleStructureChangedEvent(IUIAutomationElement* pSender, StructureChangeType changeType, SAFEARRAY* pRuntimeID) {
	eventCount_++;
	switch (changeType)
	{
	case StructureChangeType_ChildAdded:
	case StructureChangeType_ChildRemoved:
	case StructureChangeType_ChildrenInvalidated:
	case StructureChangeType_ChildrenBulkAdded:
	case StructureChangeType_ChildrenBulkRemoved:
	case StructureChangeType_ChildrenReordered: {
		Context *pCtx = reinterpret_cast<Context *>(GetClassLongPtr(hMainWnd_, 0));
		KeybindingMap keybinding_map = pCtx->GetKeybindingMap();
		PostMessage(hMainWnd_, WM_HOTKEY, 0, keybinding_map["selectMode"].lParam);
	}
	}
	return S_OK;
};

//TODO: eventhandler needs to be cleaned up.
HRESULT STDMETHODCALLTYPE CacheEventHandler::HandleStructureChangedEvent(IUIAutomationElement* pSender, StructureChangeType changeType, SAFEARRAY* pRuntimeID) {
	eventCount_++;
	switch (changeType)
	{
	case StructureChangeType_ChildAdded:
	case StructureChangeType_ChildRemoved:
	case StructureChangeType_ChildrenInvalidated:
	case StructureChangeType_ChildrenBulkAdded:
	case StructureChangeType_ChildrenBulkRemoved:
	case StructureChangeType_ChildrenReordered: {
		Context *pCtx = reinterpret_cast<Context *>(GetClassLongPtr(hMainWnd_, 0));
		pCtx->SetCacheExpiredStructChanged(true);
		break;
	}
	}
	return S_OK;
};

    HRESULT STDMETHODCALLTYPE PropertyChangedEventHandler::HandlePropertyChangedEvent(IUIAutomationElement* pSender, PROPERTYID propertyID, VARIANT newValue) 
    {
        eventCount_++;
		switch (propertyID)
		{
		case UIA_CenterPointPropertyId:
		case UIA_BoundingRectanglePropertyId:
		case UIA_IsOffscreenPropertyId: {
			Context *pCtx = reinterpret_cast<Context *>(GetClassLongPtr(hMainWnd_, 0));
			pCtx->SetCacheExpiredState(true);
			break;
		}
	
		default:
			break;
		}

        return S_OK;
    }

}