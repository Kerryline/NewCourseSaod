#include "search.hpp"

namespace {

unsigned char lower_byte(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return static_cast<unsigned char>(c + 0x20);
    if (c >= 0x80 && c <= 0x8F) return static_cast<unsigned char>(c + 0x20);
    if (c >= 0x90 && c <= 0x9F) return static_cast<unsigned char>(c + 0x50);
    if (c == 0xF0) return 0xF1;
    return c;
}

int compare_prefix(const Record* r, const std::string& key) {
    for (std::size_t i = 0; i < key.size(); ++i) {
        unsigned char a = lower_byte(static_cast<unsigned char>(r->street[i]));
        unsigned char b = lower_byte(static_cast<unsigned char>(key[i]));
        if (a < b) return -1;
        if (a > b) return 1;
    }
    return 0;
}

}

std::size_t find_first_prefix(Record* const* index, std::size_t n, const std::string& key_cp866_lower) {
    if (n == 0) return 0;
    std::size_t l = 0;
    std::size_t r = n - 1;
    while (l < r) {
        std::size_t m = l + (r - l) / 2;
        if (compare_prefix(index[m], key_cp866_lower) < 0)
            l = m + 1;
        else
            r = m;
    }
    if (compare_prefix(index[r], key_cp866_lower) == 0) return r;
    return n;
}

void enqueue_prefix_matches(Record* const* index,
                            std::size_t n,
                            std::size_t first,
                            const std::string& key_cp866_lower,
                            RecordQueue& q) {
    for (std::size_t i = first; i < n; ++i) {
        if (compare_prefix(index[i], key_cp866_lower) != 0) break;
        q.push(index[i]);
    }
}
