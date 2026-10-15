#pragma once

#include <cstddef>
#include "record.hpp"

struct tLE {
    tLE* next;
    const Record* data;
};

class RecordQueue {
public:
    RecordQueue();
    ~RecordQueue();

    RecordQueue(const RecordQueue&) = delete;
    RecordQueue& operator=(const RecordQueue&) = delete;

    void push(const Record* r);
    const Record* pop();
    bool empty() const;
    std::size_t size() const;
    const tLE* head() const;

private:
    tLE* head_;
    tLE* tail_;
};
