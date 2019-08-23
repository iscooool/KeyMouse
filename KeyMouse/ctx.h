#pragma once
#include "stdafx.h"
#include "def.h"

namespace KeyMouse {
class Context
{
private:
    string current_tag_;
    size_t max_tag_len_;
    std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>> tag_map_;
    bool enable_state_;
    HWND transparent_window_;

public:
    Context ();
    ~Context ();
    void SetCurrentTag(const string &tag);
    const string &GetCurrentTag() const;
    void SetMaxTagLen(const size_t len);
    const size_t &GetMaxTagLen() const;
    void SetTagMap(std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>> &map);
    const std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>> &
        GetTagMap() const;
    void SetEnableState(const bool flag);
    const bool &GetEnableState() const;
    void SetTransWindow(const HWND hWnd);
    const HWND &GetTransWindow() const;
};
}
