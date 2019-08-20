#pragma once
#include "stdafx.h"
#include<queue>
#include<tchar.h>
#include "def.h"
namespace KeyMouse {
    
class TagCreator
{
private:
    int count_;             // The tags' count.
    TCHAR zero_;            // The start tag in each period.
    int period_;            // Use 26 english character.
    std::queue<string> tag_queue_;
    
    

public:
    TagCreator();
    ~TagCreator();
    std::queue<string> AllocTag(int iCount);
    void Clear();
    
    
};

} 
