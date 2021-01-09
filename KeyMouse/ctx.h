#pragma once
#include<fstream>
#include "stdafx.h"
#include "def.h"
#include "UIAHandler.h"

#include "json/single_include/nlohmann/json.hpp"


// user defined message.
#define WM_TRAY WM_USER + 1

// user defined virtual key mapping.
#define KM_SCROLLDOWN 0x4A  // J
#define KM_SCROLLUP 0x4B    // K
using json = nlohmann::json;

namespace KeyMouse {
struct ElementInfo {
	UIA_HWND handle;
	RECT rect;
	CONTROLTYPEID control_type;
};
struct IdlParam{
	// id is used for RegisterHotKey.
	int id;
	LPARAM lParam;
};
using KeybindingMap = std::map<std::string, IdlParam>;

typedef struct    WindowsEventArguments {
	HWND             hWnd;
	WPARAM           wParam;
	LPARAM           lParam;
	HINSTANCE        hInst;
}WndEventArgs, *lpWndEventArgs;


struct EVENTHANDLER {
	unsigned int    Code;
	LRESULT(*fnPtr)(const WndEventArgs&);
};

struct HKBinding {
	LPARAM lParam;
	LRESULT(*fnPtr)(const WndEventArgs&);
};

/**
* @brief: handle all window messages.
*
*/
class WndProcHandler
{
public:
	static constexpr int EVENTHANDLER_NUM = 5;
	static constexpr int SELECT_HKBINDING_NUM = 4;
	static constexpr int NORMAL_HKBINDING_NUM = 5;

	WndProcHandler();
	~WndProcHandler();
	void InitialHKBinding(KeybindingMap& keybinding_map);
	LRESULT HandlerEntrance(UINT msg, const WndEventArgs& Wea);
	static LRESULT fnWndProc_Command_(const WndEventArgs& Wea);
	static LRESULT fnWndProc_Tray_(const WndEventArgs& Wea);
	static LRESULT fnWndProc_Hotkey_(const WndEventArgs& Wea);
	static LRESULT fnWndProc_Paint_(const WndEventArgs& Wea);
	static LRESULT fnWndProc_Destroy_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_SelectMode_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_FastSelectMode_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_RightClickPrefix_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_SingleClickPrefix_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_Escape_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_ToggleEnable_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_Scroll_(const WndEventArgs& Wea);
	static LRESULT fnHKProc_ForceNotUseCache_(const WndEventArgs& Wea);

	static void EscSelectMode_(HWND hWnd);
	static void SelectModeHandler_(HWND hWnd, WORD VirtualKey);
	static void ScrollHandler_(HWND hWnd, WORD VirtualKey);
	static void LeftClick_(int x, int y, int time);
	static void RightClick_(int x, int y, int time);
	static void InvokeElement_(KeyMouse::ElementInfo &pElement, HWND hWnd);
	static void EditInputForward_(HWND hWnd, WORD VirtualKey);
	static bool CompareBlackList_();
	static INT_PTR CALLBACK About_(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
	EVENTHANDLER event_handler_[EVENTHANDLER_NUM];
	static HKBinding select_hkbinding_[SELECT_HKBINDING_NUM];
	static HKBinding normal_hkbinding_[NORMAL_HKBINDING_NUM];

};

// a snippet from https://stackoverflow.com/questions/25104305/how-to-make-a-function-execute-at-the-desired-periods-using-c-11
struct TimedExecution {
    typedef void (*func_type)(HWND);
	TimedExecution() {};
    TimedExecution (func_type func, const std::chrono::milliseconds period, HWND hwnd) 
        : func_(func)
        , period_(period)
        , thread_(std::bind(&TimedExecution::threadFunc,this))
		, hwnd_(hwnd)
		, kill_(false)
    {
	}
	~TimedExecution() {
		kill_ = true;
		thread_.join();
	};
private:        
    void threadFunc() {
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);
        while(true) {
            std::this_thread::sleep_for(period_);
            func_(hwnd_);
			if (kill_) {
				CoUninitialize();
				break;
			}
        }
    }
    func_type func_;
    std::chrono::milliseconds period_;
    std::thread thread_;
	HWND hwnd_;
	bool kill_;
};


using PTagMap = std::shared_ptr<std::map<string, ElementInfo>>;
using PElementVec = std::shared_ptr<std::vector<ElementInfo>>;
using PCacheWindow = std::unique_ptr<std::map<HWND, std::map<std::string, PElementVec>>>;
using PEventHandlers = std::shared_ptr<std::map<HWND, std::vector<CComPtr<IUnknown>>>>;
using KeybindingMap = std::map<std::string, IdlParam>;

struct Font {
	std::wstring font_name;
	COLORREF font_color;
	COLORREF background_color;
	int font_size;
};
struct Profile {
	bool run_startup;
	Font font;
	Font window_tag_font;
	int opacity;
	bool invert_click_type;
	bool only_forewindow;
	bool enable_window_switching;
	bool enable_cache;
};

/**
* @brief: extract configuration from json format file.
*/
class Config {
public:
	Config();
	Config(const std::wstring& json_name);
	~Config();
	bool LoadJson(const std::wstring& json_name);
	bool WriteJson(const std::wstring& json_name);
	// fill the custom json with default json.
	void PatchCustomJson_();
    // extract keybinding map from json.
	LPARAM ExtractSingleKeyBinding_(std::string key, std::string binding);
	KeybindingMap ExtractKeyBinding();
	Profile ExtractProfile();


private:
	static std::map<std::string, WORD> lo_map_;
	static std::map<std::string, WORD> hi_map_;
	static std::map<std::string, int> command_id_map_;
	bool has_error_;
	json default_json_;
	json config_json_;

};

/**
* @brief: Context will include most of global context of this app.
*/
class Context
{
public:
    enum Mode {
        NORMAL_MODE,
        SELECT_MODE,
        INSERT_MODE
    };
	enum ClickType {
		RIGHT_CLICK,
		LEFT_CLICK,
		SINGLE_RIGHT_CLICK,
		SINGLE_LEFT_CLICK
	};
    Context ();
    ~Context ();
	std::wstring AppDir();
	bool WriteRegistryRUN_();
	bool DeleteRegistryRUN_();
	void ApplyProfile_();
	void InitTimer(HWND hwnd);
	void WaitForCacheWindow();
	void LockCacheWindow();
	void UnlockCacheWindow();
    const WndProcHandler& GetWndProcHandler () const;
    const KeybindingMap& GetKeybindingMap () const;
	const Profile& GetProfile() const;

	IUIAutomation* GetAutomation() const;
    void SetEventHandlers(PEventHandlers& event_handlers);
    PEventHandlers GetEventHandlers() const;
	void SetStructEventHandler(CComPtr<EventHandler>& event_handler);
	CComPtr<EventHandler> GetStructEventHandler() const;
    void SetElement(CComPtr<IUIAutomationElement>& pElement);
    const CComPtr<IUIAutomationElement>& GetElement() const;
    void SetPrevProcessName(const std::string &tag);
    const std::string &GetPrevProcessName() const;

    void SetCurrentTag(const string &tag);
    const string &GetCurrentTag() const;
    void SetMaxTagLen(const size_t len);
    const size_t &GetMaxTagLen() const;
    void SetTagMap(PTagMap& map);
	void ClearTagMap();
	void MergeTagMap(PTagMap& src_map);
    const PTagMap& GetTagMap() const;
    void SetWindowMap(PTagMap& map);
    const PTagMap& GetWindowMap() const;
    void SetEnableState(const bool flag);
    const bool &GetEnableState() const;
    void SetEnableCache(const bool stat);
    const bool &GetEnableCache() const;
    void SetFastSelectState(const bool flag);
    const bool &GetFastSelectState() const;
	void SetClickType(const ClickType type);
	ClickType GetClickType() const;
    void SetTransWindow(const HWND hWnd);
    const HWND &GetTransWindow() const;
	void SetForeWindow(const HWND hWnd);
	const HWND &GetForeWindow() const;
    void SetMode(const Mode mode);
    const Mode &GetMode() const;
    void SetCacheExpiredState(const bool stat);
    const bool &GetCacheExpiredState() const;
    void SetCacheExpiredStructChanged(const bool stat);
    const bool &GetCacheExpiredStructChanged() const;
    void SetLastInputTick(const DWORD interval);
    const DWORD &GetLastInputTick() const;
    void SetLastcacheTick(const DWORD time);
    const DWORD &GetLastCacheTick() const;
    void SetCacheWindow(PCacheWindow& cache_window);
    const PCacheWindow& GetCacheWindow() const;
    const bool &GetDoingCachingState() const;
    void SetDoingCachingState(const bool state);

private:
	std::wstring app_directory_;
    // the configure file name.
	std::wstring json_name_;
	Config config_;
    // the (command: IdlParam)map of hotkey binding.
	KeybindingMap keybinding_map_;
	Profile profile_;

	IUIAutomation* automation_;
    // window process handler which will handle all message from windows.
	WndProcHandler wndProc_handler_;
	PEventHandlers event_handlers_;
	CComPtr<EventHandler> pStruct_event_handler_;
	CComPtr<IUIAutomationElement> pElement_;
	std::string prev_process_name_;
	
    string current_tag_;
    size_t max_tag_len_;
    // (hints: UIAutomationElement) map.
    PTagMap tag_map_;
    PTagMap window_map_;

	PCacheWindow cache_window_;
	std::mutex cache_window_mtx_;
	std::condition_variable cache_window_cv_;
	bool doing_caching_;


    
    // current status.
    bool enable_state_;
	bool on_fast_select_mode_;
	ClickType click_type_;

    HWND transparent_window_;
	HWND fore_window_;
    Mode mode_;

	// cache will expires when keyboard strokes are detected
	// and target window's struct and property changing.
	bool is_cache_expired_;		
	int cache_expired_count_;
	bool cache_expired_struct_changed_;

	DWORD last_input_tick_;
	DWORD last_cache_tick_;
	
	std::unique_ptr<TimedExecution> timer_;
};
}
