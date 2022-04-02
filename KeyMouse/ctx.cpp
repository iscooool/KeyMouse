#include <sstream>
#include "stdafx.h"
#include "ctx.h"

#include "hotkey_handler.h"
#include "utils.h"
namespace KeyMouse {
const int MAX_PATH_LEN = 200;
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
	if (elems.empty()) {
		elems.push_back(std::string(""));
	}
	return elems;
}

std::wstring Str2Wstr(std::string& src) {
	size_t size = src.size() + 1;
	std::wstring temp(size, L' ');
	size_t out_size;
	mbstowcs_s(&out_size , &temp[0], size, src.c_str(), size - 1);
	temp.resize(out_size - 1);
	return temp;
}

COLORREF Str2RGB(std::string color) {
	COLORREF BGR = 0;
	COLORREF RGB = 0;
	color.erase(color.begin());

	if (!std::all_of(color.begin(), color.end(), 
		[](unsigned char c) {return std::isxdigit(c); }) || 
		color.length() > 6) { 
		return 0;
	}
	BGR = std::stol(color, nullptr, 16);
	long xxx = (BGR & 0xFF) << 4;
	RGB = (BGR & 0xFF) << 16 | (BGR & 0xFF00) | (BGR & 0xFF0000) >> 16;
	return RGB;
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
	{"escape", CLEANTAG},
	{"fastSelectMode", FASTSELECTMODE},
	{"rightClickPrefix", RIGHTCLICKPREFIX},
	{"singleClickPrefix", SINGLELEFTCLICKPREFIX},
	{"forceNotUseCache", FORCENOTUSECACHE},
	{"selectModeSingle", SELECTMODESINGLE}
};
Config::Config() {

}

Config::Config(const std::wstring& json_name) {
	has_error_ = false;
	default_json_ = json({
		{"profile", {
			{"runOnStartUp", true},
			{"fontColor", "#000000"},
			{"fontSize", 10},
			{"font", "Arial Rounded MT Bold"},
			{"backgroundColor", "#CCFFCC"},
			{"windowFontColor", "#000000"},
			{"windowFontSize", 12},
			{"windowFont", "Arial Rounded MT Bold"},
			{"windowBkgdColor", "#CCBBAA"},
			{"opacity", 100},
			{"invertClickType", false},
			{"onlyForeWindow", true},
			{"enableWindowSwitching", true},
			{"enableCache", false}
		}},
		{"keybindings", {
			{"toggleEnable", "alt+["},
			{"scrollUp", "alt+k"},
			{"scrollDown", "alt+j"},
			{"selectMode", "alt+;"},
			{"escape", "esc"},
			{"fastSelectMode", "alt+j"},
			{"rightClickPrefix", "shift+a"},
			{"singleClickPrefix", "shift+s"},
			{"forceNotUseCache", "space"},
			{"selectModeSingle", "disabled"}
		}}
		});
	if (!LoadJson(json_name)) {
		// set defalut configuration.
		config_json_ = default_json_;
		
	}
	else {
		// this step guarantees the left side are valid options, but not 
		// guarantees valid option values on the right.
		PatchCustomJson_();
	}

}

Config::~Config() {

}

bool Config::LoadJson(const std::wstring& json_name) {
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

bool Config::WriteJson(const std::wstring& json_name) {
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

// patch the custom configure json with default json.
void Config::PatchCustomJson_() {
	
	json diff = json::diff(config_json_, default_json_);
	json::iterator it = diff.begin();
	while (it != diff.end()) {
		// don't replace user's custom option.
		if ((*it)["op"] == "replace") {
			it = diff.erase(it);
		}
		else {
			if ((*it)["op"] == "remove") {
				if (!has_error_) {		// only warn user one time.
					has_error_ = true;
					std::wstring temp = Str2Wstr((*it)["path"].get<std::string>());
					std::wstring output_str = TEXT("Unrecognized option: ") +
						temp + TEXT(". Please check your configuration.");
					MessageBox(nullptr, output_str.c_str(), TEXT("Warning"), MB_OK);
				}
			}
			++it;
		}

	}
	config_json_ = config_json_.patch(diff);

}
LPARAM Config::ExtractSingleKeyBinding_(std::string key, std::string binding) {
	// remove all space in binding.
	binding.erase(std::remove_if(binding.begin(), binding.end(), ::isspace), binding.end());

	std::vector<std::string> split_keys = split(binding, '+');
	WORD lo = 0;
	WORD hi = 0;
	if (binding == "disabled")
		return MAKELPARAM(lo, hi);

	for (auto& key_str : split_keys) {
		auto search = lo_map_.find(key_str);
		if (search != lo_map_.end()) {
			lo = lo | lo_map_[key_str];
		} 
		else if (hi_map_.find(key_str) != hi_map_.end()) {
			hi = hi | hi_map_[key_str];
		}
		else {
			if (!has_error_) {		// only warn user one time.
				has_error_ = true;
				std::wstring temp = Str2Wstr(binding);
				std::wstring output_str = TEXT("Unrecognized key binding: ") +
					temp + TEXT(". Please check your configuration.");
				MessageBox(nullptr, output_str.c_str(), TEXT("Warning"), MB_OK);
				std::string defalut_binding = default_json_["keybindings"][key].get<std::string>();
				return ExtractSingleKeyBinding_(key, defalut_binding);
			}
		}
	}

	return MAKELPARAM(lo, hi);

}

KeybindingMap Config::ExtractKeyBinding() {
	json keybinding_json = config_json_["keybindings"];
	KeybindingMap keybinding_map;

	for (auto& keybinding : keybinding_json.items()) {
		std::string str = keybinding.value();
		LPARAM lp = ExtractSingleKeyBinding_(keybinding.key(), str);

		auto key = keybinding.key();
		if (command_id_map_.find(key) != command_id_map_.end()) {
			if (str == "disabled") {
				IdlParam id_lParam = { DISABLED, lp };
				keybinding_map.insert(
					std::pair<std::string, IdlParam>(keybinding.key(), id_lParam)
				);
			}
			IdlParam id_lParam = { command_id_map_[key], lp};
			keybinding_map.insert(
				std::pair<std::string, IdlParam>(keybinding.key(), id_lParam)
			);
		} else {
			if (!has_error_) {		// only warn user one time.
				has_error_ = true;
				std::wstring temp = Str2Wstr(key);
				std::wstring output_str = TEXT("Unrecognized key binding command: ") +
					temp + TEXT(". Please check your configuration.");
				MessageBox(nullptr, output_str.c_str(), TEXT("Warning"), MB_OK);
			}
		}

	}
	return keybinding_map;

}

Profile Config::ExtractProfile() {
	json profile_json = config_json_["profile"];
	Profile profile;
	profile.run_startup = profile_json["runOnStartUp"].get<bool>();
	profile.font.background_color = Str2RGB(profile_json["backgroundColor"].get<std::string>());
	profile.font.font_name = Str2Wstr(profile_json["font"].get<std::string>());
	profile.font.font_size = profile_json["fontSize"].get<int>();
	profile.font.font_color = Str2RGB(profile_json["fontColor"].get<std::string>());
	profile.window_tag_font.background_color = Str2RGB(profile_json["windowBkgdColor"].get<std::string>());
	profile.window_tag_font.font_name = Str2Wstr(profile_json["windowFont"].get<std::string>());
	profile.window_tag_font.font_size = profile_json["windowFontSize"].get<int>();
	profile.window_tag_font.font_color = Str2RGB(profile_json["windowFontColor"].get<std::string>());
	profile.opacity = profile_json["opacity"].get<int>();
	profile.invert_click_type = profile_json["invertClickType"].get<bool>();
	profile.only_forewindow = profile_json["onlyForeWindow"].get<bool>();
	profile.enable_window_switching = profile_json["enableWindowSwitching"].get<bool>();
	profile.enable_cache = profile_json["enableCache"].get<bool>();

	return profile;
}

//------------------------------------------------------------------------
Context::Context() : 
	wndProc_handler_(WndProcHandler()),
    current_tag_(string(TEXT(""))),
	tag_map_(PTagMap()),
    enable_state_(true),
	on_fast_select_mode_(false),
	mode_(NORMAL_MODE),
	is_cache_expired_(false),
	cache_expired_count_(0),
	cache_expired_struct_changed_(false),
	last_input_tick_(0),
	last_cache_tick_(0),
	cache_window_mtx_(),
	cache_window_cv_(),
	doing_caching_(false) {

	app_directory_ = AppDir();
	SetCurrentDirectory(app_directory_.c_str());

	json_name_ = std::wstring(L"./config.json");
	config_ = Config(json_name_);
	keybinding_map_ = config_.ExtractKeyBinding();
	profile_ = config_.ExtractProfile();
	ApplyProfile_();

	// initalize uiautomation.
	CoCreateInstance(CLSID_CUIAutomation, nullptr,
		CLSCTX_INPROC_SERVER, IID_IUIAutomation,
		reinterpret_cast<void**>(&automation_));

	event_handlers_.reset(new std::map<HWND, std::vector<CComPtr<IUnknown>>>);
	wndProc_handler_.InitialHKBinding(keybinding_map_);
	cache_window_.reset(new std::map<HWND, std::map<std::string, PElementVec>>);
	if (profile_.invert_click_type) {
		click_type_ = RIGHT_CLICK;
	}
	else {
		click_type_ = LEFT_CLICK;
	}
}

Context::~Context() {
}

void Context::ApplyProfile_() {
	if (profile_.run_startup) {
		WriteRegistryRUN_();
	}
	else {
		DeleteRegistryRUN_();
	}
}
std::wstring Context::AppDir() {
	TCHAR buffer[MAX_PATH_LEN];
    GetModuleFileName( NULL, buffer, MAX_PATH_LEN);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of( L"\\/" );
    return std::wstring(buffer).substr(0, pos);
}
bool Context::WriteRegistryRUN_()
{
	HKEY hKey;
	TCHAR pPath [MAX_PATH_LEN];

	GetModuleFileName(0, pPath, MAX_PATH_LEN);

	RegOpenKeyExA (HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
	if (RegSetValueEx(hKey, L"KeyMouse", 0, REG_SZ, (BYTE*)pPath, MAX_PATH_LEN)) {
		return false;
	}
	

	RegCloseKey(hKey);
	return true;
}

bool Context::DeleteRegistryRUN_()
{
	HKEY hKey;

	RegOpenKeyExA (HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
	if (RegDeleteValue(hKey, L"KeyMouse")) {
		return false;
	}
	
	RegCloseKey(hKey);
	return true;
}

void Context::InitTimer(HWND hwnd) {
	timer_.reset(new TimedExecution(CacheThread, std::chrono::milliseconds(1), hwnd));
}

void Context::WaitForCacheWindow() {
	std::unique_lock<std::mutex> lck(cache_window_mtx_);
	cache_window_cv_.wait(lck, [this] { return !doing_caching_; });
}
void Context::LockCacheWindow() {
	doing_caching_ = true;
	cache_window_mtx_.lock();
}

void Context::UnlockCacheWindow() {
	cache_window_mtx_.unlock();
	doing_caching_ = false;
	cache_window_cv_.notify_one();
}

const  WndProcHandler& Context::GetWndProcHandler() const {
    return wndProc_handler_;
}

const KeybindingMap& Context::GetKeybindingMap() const {
	return keybinding_map_;
}

const Profile& Context::GetProfile() const {
	return profile_;
}

IUIAutomation* Context::GetAutomation() const {
	return automation_;
}

void Context::SetEventHandlers(PEventHandlers& event_handlers) {
	event_handlers_ = std::move(event_handlers);
}

PEventHandlers Context::GetEventHandlers() const {
	return event_handlers_;
}

void Context::SetStructEventHandler(CComPtr<EventHandler> &event_handler) {
    pStruct_event_handler_ = event_handler;
}
CComPtr<EventHandler> Context::GetStructEventHandler() const {
    return pStruct_event_handler_;
}

void Context::SetElement(CComPtr<IUIAutomationElement>& pElement) {
	pElement_ = pElement;
}
const CComPtr<IUIAutomationElement>& Context::GetElement() const {
	return pElement_;
}

void Context::SetPrevProcessName(const std::string &name) {
	prev_process_name_ = name;
}
const std::string &Context::GetPrevProcessName() const {
	return prev_process_name_;
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

void Context::ClearTagMap() {
	if (tag_map_ != nullptr) {
		tag_map_->clear();
	}
}

void Context::MergeTagMap(PTagMap& src_map) {
	if (tag_map_ == nullptr) {
		tag_map_ = std::move(src_map);
	}
	else {
		tag_map_->insert(src_map->begin(), src_map->end());
	}
}

const PTagMap& Context::GetTagMap() const {
    return tag_map_;
}

void Context::SetWindowMap(PTagMap& map) {
	window_map_ = std::move(map);
}

const PTagMap& Context::GetWindowMap() const {
	return window_map_;
}

void Context::SetEnableState(const bool flag) {
    enable_state_ = flag;
}

const bool &Context::GetEnableState() const {
    return enable_state_;
}

void Context::SetEnableCache(const bool stat) {
	profile_.enable_cache = stat;
}
const bool &Context::GetEnableCache() const {
	return profile_.enable_cache;
}

void Context::SetFastSelectState(const bool flag) {
	on_fast_select_mode_ = flag;
}

const bool& Context::GetFastSelectState() const {
	return on_fast_select_mode_;
}

void Context::SetClickType(const ClickType type) {
	click_type_ = type;
}

Context::ClickType Context::GetClickType() const {
	return click_type_;
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

void Context::SetCacheExpiredState(const bool stat) {
	if (!stat) {
		cache_expired_count_ = 0;
		last_input_tick_ = 0;
	}
	else {
		cache_expired_count_++;
		last_input_tick_ = GetTickCount();
	}
	is_cache_expired_ = stat;
}

const bool &Context::GetCacheExpiredState() const {
	return is_cache_expired_;
}

void Context::SetCacheExpiredStructChanged(const bool stat) {
	cache_expired_struct_changed_ = stat;
}

const bool &Context::GetCacheExpiredStructChanged() const {
	return cache_expired_struct_changed_;
}

void Context::SetLastInputTick(const DWORD interval) {
	last_input_tick_ = interval;
}

const DWORD &Context::GetLastInputTick() const {
	return last_input_tick_;
}

void Context::SetLastcacheTick(const DWORD time) {
	last_cache_tick_ = time;
}

const DWORD &Context::GetLastCacheTick() const {
	return last_cache_tick_;
}

void Context::SetCacheWindow(PCacheWindow& cache_window) {
	cache_window_ = std::move(cache_window);
}

const PCacheWindow& Context::GetCacheWindow() const {
	return cache_window_;
}
const bool &Context::GetDoingCachingState() const {
	return doing_caching_;
}
void Context::SetDoingCachingState(const bool state) {
	doing_caching_ = state;
}
}
