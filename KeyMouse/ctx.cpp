#include <sstream>
#include "stdafx.h"
#include "ctx.h"
#include "hotkey_handler.h"

namespace KeyMouse {
// split std::string by delim.
template <typename Out>
void split(const std::string &s, char delim, Out result) {
	std::istringstream iss(s);
	std::string item;
	while (std::getline(iss, item, delim)) {
		*result++ = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}


std::map<std::string, WORD> Config::lo_map_ = {
	{"alt", MOD_ALT},
	{"ctrl", MOD_CONTROL},
	{"norepeat", MOD_NOREPEAT},
	{"shift", MOD_SHIFT},
	{"win", MOD_WIN}
};

std::map<std::string, WORD> Config::hi_map_ = {
	{"tab", VK_TAB},
	{"enter", VK_RETURN},
	{"esc", VK_ESCAPE},
	{"space", VK_SPACE},
	{"end", VK_END},
	{"home", VK_HOME},
	{"left", VK_LEFT},
	{"right", VK_RIGHT},
	{"up", VK_UP},
	{"down", VK_DOWN},
	{"insert", VK_INSERT},
	{"0", 0x30},
	{"1", 0x31},
	{"2", 0x32},
	{"3", 0x33},
	{"4", 0x34},
	{"5", 0x35},
	{"6", 0x36},
	{"7", 0x37},
	{"8", 0x38},
	{"9", 0x39},
	{"a", 0x41},
	{"b", 0x42},
	{"c", 0x43},
	{"d", 0x44},
	{"e", 0x45},
	{"f", 0x46},
	{"g", 0x47},
	{"h", 0x48},
	{"i", 0x49},
	{"j", 0x4A},
	{"k", 0x4B},
	{"l", 0x4C},
	{"m", 0x4D},
	{"n", 0x4E},
	{"o", 0x4F},
	{"p", 0x50},
	{"q", 0x51},
	{"r", 0x52},
	{"s", 0x53},
	{"t", 0x54},
	{"u", 0x55},
	{"v", 0x56},
	{"w", 0x57},
	{"x", 0x58},
	{"y", 0x59},
	{"z", 0x5A},
	{"f1", VK_F1},
	{"f2", VK_F2},
	{"f3", VK_F3},
	{"f4", VK_F4},
	{"f5", VK_F5},
	{"f6", VK_F6},
	{"f7", VK_F7},
	{"f8", VK_F8},
	{"f9", VK_F9},
	{"f10", VK_F10},
	{"f11", VK_F11},
	{"f12", VK_F12},
	{";", VK_OEM_1},
	{"=", VK_OEM_PLUS},
	{",", VK_OEM_COMMA},
	{"-", VK_OEM_MINUS},
	{".", VK_OEM_PERIOD},
	{"/", VK_OEM_2},
	{"`", VK_OEM_3},
	{"[", VK_OEM_4},
	{"\\", VK_OEM_5},
	{"]", VK_OEM_6},
	{"'", VK_OEM_7}
};
std::map<std::string, int> Config::command_id_map_{
	{"toggleEnable", TOGGLEENABLE},
	{"scrollUp", SCROLLUP},
	{"scrollDown", SCROLLDOWN},
	{"selectMode", SHOWTAG},
	{"escape", CLEANTAG}
};
Config::Config() {

}

Config::Config(const std::string& json_name) {
	if (!LoadJson(json_name)) {
		// set defalut configuration.
		config_json_ = json({
			{"profile", {
				{"runOnStartUp", true}
			}},
			{"keybindings", {
				{"toggleEnable", "f11"},
				{"scrollUp", "k"},
				{"scrollDown", "j"},
				{"selectMode", "alt+;"},
				{"escape", "esc"}
			}}
			});
	}

}

Config::~Config() {

}

bool Config::LoadJson(const std::string& json_name) {
	std::fstream json_file(json_name, std::ios::in);

	if (!json_file.is_open()) {
		return false;
	}
	try {
		json_file >> config_json_;
	}
	catch (json::exception& e) {
		// output exception information
		cout << "message: " << e.what() << '\n'
			 << "exception id: " << e.id << std::endl;
		return false;

	}
	json_file.close();
	return true;
}

bool Config::WriteJson(const std::string& json_name) {
	std::fstream json_file(json_name, std::ios::out);

	if (!json_file.is_open()) {
		return false;
	}
	try {
		json_file << config_json_;
	}
	catch (json::exception& e) {
		// output exception information
		cout << "message: " << e.what() << '\n'
			 << "exception id: " << e.id << std::endl;
		return false;

	}
	json_file.close();
	return true;
}

KeybindingMap Config::ExtractKeyBinding() {
	json keybinding_json = config_json_["keybindings"];
	KeybindingMap keybinding_map;

	for (auto& keybinding : keybinding_json.items()) {
		auto str = keybinding.value();
		std::vector<std::string> split_keys = split(str, '+');
		WORD lo = 0;
		WORD hi = 0;
		for (auto& key : split_keys) {
			auto search = lo_map_.find(key);
			if (search != lo_map_.end()) {
				lo = lo | lo_map_[key];
			} 
			else if (hi_map_.find(key) != hi_map_.end()) {
				hi = hi | hi_map_[key];
			}
		}

		auto key = keybinding.key();
		IdlParam id_lParam = { command_id_map_[key], MAKELPARAM(lo, hi) };
		keybinding_map.insert(
			std::pair<std::string, IdlParam>(keybinding.key(), id_lParam)
		);

	}
	return keybinding_map;

}

Context::Context() {
	json_name_ = std::string("./config.json");
	config_ = Config(json_name_);
	keybinding_map_ = config_.ExtractKeyBinding();

	wndProc_handler_ = WndProcHandler();
	wndProc_handler_.InitialHKBinding(keybinding_map_);
    current_tag_ = string(TEXT(""));
	tag_map_ = PTagMap();
    enable_state_ = true;
	mode_ = NORMAL_MODE;
}

Context::~Context() {
}

const  WndProcHandler& Context::GetWndProcHandler() const {
    return wndProc_handler_;
}

const KeybindingMap& Context::GetKeybindingMap() const {
	return keybinding_map_;
}

void Context::SetCurrentTag(const string &tag) {
    current_tag_ = tag;
}

const string &Context::GetCurrentTag() const {
    return current_tag_;
}

void Context::SetMaxTagLen(const size_t len) {
    max_tag_len_ = len;
}

const size_t &Context::GetMaxTagLen() const {
    return max_tag_len_;
}
void Context::SetTagMap(PTagMap& map) {
    tag_map_ = std::move(map);
}
const PTagMap& Context::GetTagMap() const {
    return tag_map_;
}

void Context::SetEnableState(const bool flag) {
    enable_state_ = flag;
}

const bool &Context::GetEnableState() const {
    return enable_state_;
}

void Context::SetTransWindow(const HWND hWnd) {
    transparent_window_ = hWnd;
}

const HWND &Context::GetTransWindow() const {
    return transparent_window_;
}
void Context::SetForeWindow(const HWND hWnd) {
	fore_window_ = hWnd;
}

const HWND &Context::GetForeWindow() const {
	return fore_window_;
}
void Context::SetMode(const Mode mode) {
    mode_ = mode;
}

const Context::Mode &Context::GetMode() const {
    return mode_;
}
}
