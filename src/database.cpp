#include "database.hpp"
#include "heap_sort.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

Database::Database(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("Cannot open " + filename);

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (file_size % sizeof(Record) != 0)
        throw std::runtime_error("Bad file size");

    size_t count = file_size / sizeof(Record);
    records_.resize(count);
    file.read(reinterpret_cast<char*>(records_.data()), file_size);

    index_.resize(count);
    for (size_t i = 0; i < count; ++i)
        index_[i] = &records_[i];
}

void Database::sort() {
    heap_sort(index_.data(), static_cast<int>(index_.size()));
}
