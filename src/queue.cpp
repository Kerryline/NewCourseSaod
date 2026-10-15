#include "queue.hpp"

RecordQueue::RecordQueue() : head_(nullptr), tail_(nullptr) {}

RecordQueue::~RecordQueue() {
    while (head_ != nullptr)
        pop();
}

void RecordQueue::push(const Record* r) {
    tLE* p = new tLE;
    p->next = nullptr;
    p->data = r;
    if (head_ != nullptr)
        tail_->next = p;
    else
        head_ = p;
    tail_ = p;
}

const Record* RecordQueue::pop() {
    if (head_ == nullptr) return nullptr;
    tLE* p = head_;
    const Record* r = p->data;
    head_ = p->next;
    if (head_ == nullptr) tail_ = nullptr;
    delete p;
    return r;
}

bool RecordQueue::empty() const {
    return head_ == nullptr;
}

std::size_t RecordQueue::size() const {
    std::size_t n = 0;
    for (const tLE* p = head_; p != nullptr; p = p->next)
        ++n;
    return n;
}

const tLE* RecordQueue::head() const {
    return head_;
}
