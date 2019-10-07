#include "stdafx.h"
#include "ctx.h"

namespace KeyMouse {

Context::Context() {
    current_tag_ = string(TEXT(""));
    tag_map_ =  
        std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>>();  
    enable_state_ = true;
	mode_ = NORMAL_MODE;
}

Context::~Context() {
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
void Context::SetTagMap(
        std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>> &map
        ) {
    tag_map_ = std::move(map);
}
const std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>> &
    Context::GetTagMap() const {
    return tag_map_;
}
void Context::SetScrollVec(
        std::unique_ptr<std::vector<CComPtr<IUIAutomationElement>>> &vec
        ) {
    scroll_vec_ = std::move(vec);
}
const std::unique_ptr<std::vector<CComPtr<IUIAutomationElement>>> & 
    Context::GetScrollVec() const {
        return scroll_vec_;
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
