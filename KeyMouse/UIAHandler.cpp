#include "stdafx.h"
#include "UIAHandler.h"
#include "ctx.h"

namespace KeyMouse {
HRESULT STDMETHODCALLTYPE EventHandler::HandleStructureChangedEvent(IUIAutomationElement* pSender, StructureChangeType changeType, SAFEARRAY* pRuntimeID) {
	_eventCount++;
	switch (changeType)
	{
	case StructureChangeType_ChildAdded:
	case StructureChangeType_ChildRemoved:
	case StructureChangeType_ChildrenInvalidated:
	case StructureChangeType_ChildrenBulkAdded:
	case StructureChangeType_ChildrenBulkRemoved:
	case StructureChangeType_ChildrenReordered:
		Context *pCtx = reinterpret_cast<Context *>(GetClassLongPtr(_hWnd, 0));
		KeybindingMap keybinding_map = pCtx->GetKeybindingMap();
		PostMessage(_hWnd, WM_HOTKEY, 0, keybinding_map["selectMode"].lParam);
	}
	return S_OK;
};


}