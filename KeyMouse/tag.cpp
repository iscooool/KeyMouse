#include"tag.h"
namespace KeyMouse {

TagCreator::TagCreator() {
    count_ = 0;
    zero_ = 'A';
    period_ = 26;
}
TagCreator::~TagCreator() {
}


std::queue<string> TagCreator::AllocTag(int iCount) {
    count_ = iCount;
    string szPrefix(TEXT(""));
    // Clear the queue.
    std::queue<string> empty;
    std::swap(tag_queue_, empty);
    tag_queue_.push(szPrefix);
    for(int i = 0; i < iCount; ++i) {
        int remainer = i % period_;
        if(remainer == 0) {
            szPrefix.clear();
            szPrefix = string(TEXT("")) + tag_queue_.front();
            tag_queue_.pop();
            // Something is dequeued form queue.
            iCount += 1;
        }
        tag_queue_.push(szPrefix + string(1, zero_ + remainer));
        
    }
    return tag_queue_;

}

void TagCreator::Clear() {
    count_ = 0;
}


}

