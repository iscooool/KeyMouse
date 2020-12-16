#pragma once
// Defines an event handler for structure-changed events, and 
// listens for them on the element specifies by the user.
#include "stdafx.h"
namespace KeyMouse {
	class EventHandler :
		public IUIAutomationStructureChangedEventHandler
	{
	private:
		LONG refConut_;

	public:
		int eventCount_;
		HWND hMainWnd_;

		// Constructor.
		EventHandler(HWND hMainWnd) : 
			refConut_(0), 
			eventCount_(0), 
			hMainWnd_(hMainWnd)
		{
		}

		// IUnknown methods.
		ULONG STDMETHODCALLTYPE AddRef()
		{
			ULONG ret = InterlockedIncrement(&refConut_);
			return ret;
		}

		ULONG STDMETHODCALLTYPE Release()
		{
			ULONG ret = InterlockedDecrement(&refConut_);
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
	class CacheEventHandler :
		public IUIAutomationStructureChangedEventHandler
	{
	private:
		LONG refConut_;

	public:
		int eventCount_;
		HWND hMainWnd_;
		HWND hTargetWnd_;

		// Constructor.
		CacheEventHandler(HWND hMainWnd, HWND hTargetWnd) : 
			refConut_(0), 
			eventCount_(0), 
			hMainWnd_(hMainWnd),
			hTargetWnd_(hTargetWnd)
		{
		}

		// IUnknown methods.
		ULONG STDMETHODCALLTYPE AddRef()
		{
			ULONG ret = InterlockedIncrement(&refConut_);
			return ret;
		}

		ULONG STDMETHODCALLTYPE Release()
		{
			ULONG ret = InterlockedDecrement(&refConut_);
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


	class PropertyChangedEventHandler:
    public IUIAutomationPropertyChangedEventHandler
{
private:
    LONG refCount_;

public:
    int eventCount_;
	HWND hMainWnd_;

    //Constructor.
    PropertyChangedEventHandler(HWND hMainWnd): refCount_(0), eventCount_(0), hMainWnd_(hMainWnd)
    {
    }

    //IUnknown methods.
    ULONG STDMETHODCALLTYPE AddRef() 
    {
        ULONG ret = InterlockedIncrement(&refCount_);
        return ret;
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        ULONG ret = InterlockedDecrement(&refCount_);
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
            *ppInterface=static_cast<IUIAutomationPropertyChangedEventHandler*>(this);
        else if (riid == __uuidof(IUIAutomationPropertyChangedEventHandler)) 
            *ppInterface=static_cast<IUIAutomationPropertyChangedEventHandler*>(this);
        else 
        {
            *ppInterface = NULL;
            return E_NOINTERFACE;
        }
        this->AddRef();
        return S_OK;
    }

    // IUIAutomationPropertyChangedEventHandler methods.
	HRESULT STDMETHODCALLTYPE HandlePropertyChangedEvent(IUIAutomationElement* pSender, PROPERTYID propertyID, VARIANT newValue);
};
}
