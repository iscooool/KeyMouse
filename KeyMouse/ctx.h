#pragma once
#include<fstream>
#include "stdafx.h"
#include "def.h"
#include "wndproc_handler.h"
#include "UIAHandler.h"

#include "json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

namespace KeyMouse {
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

using PTagMap = std::shared_ptr<std::map<string, CComPtr<IUIAutomationElement>>>;
using PElementVec = std::shared_ptr<std::vector<CComPtr<IUIAutomationElement>>>;
using PCacheWindow = std::unique_ptr<std::map<HWND, std::map<std::string, PElementVec>>>;
using PEventHandlers = std::shared_ptr<std::map<HWND, std::vector<CComPtr<IUnknown>>>>;

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
	bool cache_expired_struct_changed_;

	DWORD last_input_tick_;
	DWORD last_cache_tick_;
	
	std::unique_ptr<TimedExecution> timer_;
};
}
