#pragma once
#include "record.hpp"
#include <vector>
#include <string>

class Database {
public:
    explicit Database(const std::string& filename);

    void sort();
    size_t size() const { return records_.size(); }
    const Record& at(size_t i) const { return *index_[i]; }
    const Record& source_at(size_t i) const { return records_[i]; }
    Record* const* index_data() const { return index_.data(); }

private:
    std::vector<Record> records_;
    std::vector<Record*> index_;
};
