#pragma once
#include<fstream>
#include "stdafx.h"
#include "def.h"
#include "wndproc_handler.h"

#include "json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

namespace KeyMouse {
using PTagMap = std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>>;

class Config {
public:
	Config();
	Config(const std::string& json_name);
	~Config();
	bool LoadJson(const std::string& json_name);
	bool WriteJson(const std::string& json_name);
	KeybindingMap ExtractKeyBinding();


private:
	static std::map<std::string, WORD> lo_map_;
	static std::map<std::string, WORD> hi_map_;
	static std::map<std::string, int> command_id_map_;
	json config_json_;

};


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
    const WndProcHandler& GetWndProcHandler () const;
    const KeybindingMap& GetKeybindingMap () const;
    void SetCurrentTag(const string &tag);
    const string &GetCurrentTag() const;
    void SetMaxTagLen(const size_t len);
    const size_t &GetMaxTagLen() const;
    void SetTagMap(PTagMap& map);
    const PTagMap& GetTagMap() const;
    void SetEnableState(const bool flag);
    const bool &GetEnableState() const;
    void SetTransWindow(const HWND hWnd);
    const HWND &GetTransWindow() const;
	void SetForeWindow(const HWND hWnd);
	const HWND &GetForeWindow() const;
    void SetMode(const Mode mode);
    const Mode &GetMode() const;

private:
	std::string json_name_;
	Config config_;
	KeybindingMap keybinding_map_;

	WndProcHandler wndProc_handler_;
    string current_tag_;
    size_t max_tag_len_;
    PTagMap tag_map_;
    bool enable_state_;
    HWND transparent_window_;
	HWND fore_window_;
    Mode mode_;

};
}
