#pragma once
#include<fstream>
#include "stdafx.h"
#include "def.h"
#include "wndproc_handler.h"
#include "UIAHandler.h"

#include "json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

namespace KeyMouse {
using PTagMap = std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>>;

struct Font {
	std::wstring font_name;
	COLORREF font_color;
	int font_size;
};
struct Profile {
	bool run_startup;
	Font font;
	COLORREF background_color;
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
    Context ();
    ~Context ();
	std::wstring AppDir();
	bool WriteRegistryRUN_();
	bool DeleteRegistryRUN_();
	void ApplyProfile_();
    const WndProcHandler& GetWndProcHandler () const;
    const KeybindingMap& GetKeybindingMap () const;
	const Profile& GetProfile() const;

	void SetStructEventHandler(EventHandler* event_handler);
	const EventHandler* GetStructEventHandler() const;
    void SetElement(CComPtr<IUIAutomationElement>& pElement);
    const CComPtr<IUIAutomationElement>& GetElement() const;
    void SetPrevProcessName(const std::string &tag);
    const std::string &GetPrevProcessName() const;

    void SetCurrentTag(const string &tag);
    const string &GetCurrentTag() const;
    void SetMaxTagLen(const size_t len);
    const size_t &GetMaxTagLen() const;
    void SetTagMap(PTagMap& map);
    const PTagMap& GetTagMap() const;
    void SetEnableState(const bool flag);
    const bool &GetEnableState() const;
    void SetFastSelectState(const bool flag);
    const bool &GetFastSelectState() const;
    void SetTransWindow(const HWND hWnd);
    const HWND &GetTransWindow() const;
	void SetForeWindow(const HWND hWnd);
	const HWND &GetForeWindow() const;
    void SetMode(const Mode mode);
    const Mode &GetMode() const;

private:
	std::wstring app_directory_;
    // the configure file name.
	std::wstring json_name_;
	Config config_;
    // the (command: IdlParam)map of hotkey binding.
	KeybindingMap keybinding_map_;
	Profile profile_;
    // window process handler which will handle all message from windows.
	WndProcHandler wndProc_handler_;
	EventHandler* pStruct_event_handler_;
	CComPtr<IUIAutomationElement> pElement_;
	std::string prev_process_name_;
	
    string current_tag_;
    size_t max_tag_len_;
    // (hints: UIAutomationElement) map.
    PTagMap tag_map_;
    
    // current status.
    bool enable_state_;
	bool on_fast_select_mode_;

    HWND transparent_window_;
	HWND fore_window_;
    Mode mode_;

};
}
