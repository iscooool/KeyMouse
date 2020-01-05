#pragma once
// Defines an event handler for structure-changed events, and 
// listens for them on the element specifies by the user.
#include "stdafx.h"
namespace KeyMouse {
	class EventHandler :
		public IUIAutomationStructureChangedEventHandler
	{
	private:
		LONG _refCount;

	public:
		int _eventCount;
		HWND _hWnd;

		// Constructor.
		EventHandler(HWND hWnd) : _refCount(1), _eventCount(0), _hWnd(hWnd)
		{
		}

		// IUnknown methods.
		ULONG STDMETHODCALLTYPE AddRef()
		{
			ULONG ret = InterlockedIncrement(&_refCount);
			return ret;
		}

		ULONG STDMETHODCALLTYPE Release()
		{
			ULONG ret = InterlockedDecrement(&_refCount);
			if (ret == 0)
			{
				delete this;
				return 0;
			}
			return ret;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface)
		{
			if (riid == __uuidof(IUnknown))
				*ppInterface = static_cast<IUIAutomationStructureChangedEventHandler*>(this);
			else if (riid == __uuidof(IUIAutomationStructureChangedEventHandler))
				*ppInterface = static_cast<IUIAutomationStructureChangedEventHandler*>(this);
			else
			{
				*ppInterface = NULL;
				return E_NOINTERFACE;
			}
			this->AddRef();
			return S_OK;
		}

		// IUIAutomationStructureChangedEventHandler methods
		HRESULT STDMETHODCALLTYPE HandleStructureChangedEvent(IUIAutomationElement* pSender, StructureChangeType changeType, SAFEARRAY* pRuntimeID);
	};

}
