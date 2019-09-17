#pragma once
#include "stdafx.h"
#include "def.h"

namespace KeyMouse {
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
    void SetCurrentTag(const string &tag);
    const string &GetCurrentTag() const;
    void SetMaxTagLen(const size_t len);
    const size_t &GetMaxTagLen() const;
    void SetTagMap(std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>> &map);
    const std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>> &
        GetTagMap() const;
    void SetScrollVec(std::unique_ptr<std::vector<CComPtr<IUIAutomationElement>>> &vec);
    const std::unique_ptr<std::vector<CComPtr<IUIAutomationElement>>> & 
        GetScrollVec() const;
    void SetEnableState(const bool flag);
    const bool &GetEnableState() const;
    void SetTransWindow(const HWND hWnd);
    const HWND &GetTransWindow() const;
    void SetMode(const Mode mode);
    const Mode &GetMode() const;

private:
    string current_tag_;
    size_t max_tag_len_;
    std::unique_ptr<std::map<string, CComPtr<IUIAutomationElement>>> tag_map_;
    // TODO: scroll_vec_ is unused. delete it later.
    std::unique_ptr<std::vector<CComPtr<IUIAutomationElement>>> scroll_vec_;
    bool enable_state_;
    HWND transparent_window_;
    Mode mode_;

};
}
