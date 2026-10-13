#include "heap_sort.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static int compare_keys(const Record* x, const Record* y) {
    for (int i = 0; i < 18; ++i) {
        unsigned char cx = static_cast<unsigned char>(x->street[i]);
        unsigned char cy = static_cast<unsigned char>(y->street[i]);
        if (cx < cy) return -1;
        if (cx > cy) return 1;
    }
    if (x->house < y->house) return -1;
    if (x->house > y->house) return 1;
    return 0;
}

static int failures = 0;

static void check_sorted(const std::vector<Record>& storage,
                         const std::vector<Record*>& index,
                         const char* name) {
    for (size_t i = 1; i < index.size(); ++i)
        if (compare_keys(index[i - 1], index[i]) > 0) {
            std::printf("FAIL %s: position %zu after %zu\n", name, i, i - 1);
            ++failures;
            return;
        }
    (void)storage;
}

static void run_case(const std::vector<Record>& storage, const char* name) {
    std::vector<Record*> index(storage.size());
    for (size_t i = 0; i < storage.size(); ++i)
        index[i] = const_cast<Record*>(&storage[i]);
    heap_sort(index.data(), static_cast<int>(index.size()));
    check_sorted(storage, index, name);
}

static std::vector<Record> make_records(int n, int alphabet, unsigned seed) {
    std::srand(seed);
    std::vector<Record> storage(n);
    for (int i = 0; i < n; ++i) {
        std::memset(storage[i].street, ' ', 18);
        storage[i].street[17] = '\0';
        storage[i].street[0] = static_cast<char>('A' + std::rand() % alphabet);
        storage[i].house = static_cast<short>(std::rand() % alphabet);
    }
    return storage;
}

static void run_random(int n, int alphabet, unsigned seed, const char* name) {
    run_case(make_records(n, alphabet, seed), name);
}

int main() {
    run_case(std::vector<Record>(), "empty");

    Record one;
    std::memset(&one, 0, sizeof(one));
    run_case(std::vector<Record>(1, one), "single");

    for (int n = 1; n <= 64; ++n) {
        char name[32];
        std::snprintf(name, sizeof(name), "random n=%d alphabet=4", n);
        run_random(n, 4, 1000u + n, name);
        std::snprintf(name, sizeof(name), "random n=%d alphabet=26", n);
        run_random(n, 26, 2000u + n, name);
    }

    for (int iter = 0; iter < 2000; ++iter) {
        int n = 1 + std::rand() % 60;
        char name[32];
        std::snprintf(name, sizeof(name), "random n=%d iter=%d", n, iter);
        run_random(n, 1 + iter % 5, 3000u + iter, name);
    }

    std::vector<Record> equal(50);
    for (int i = 0; i < 50; ++i) {
        std::memset(equal[i].street, ' ', 18);
        equal[i].street[0] = 'A';
        equal[i].house = 1;
    }
    run_case(equal, "all keys equal");

    std::vector<Record> same_street(40);
    std::srand(7);
    for (int i = 0; i < 40; ++i) {
        std::memset(same_street[i].street, ' ', 18);
        same_street[i].street[0] = 'A';
        same_street[i].house = static_cast<short>(40 - i);
    }
    run_case(same_street, "same street houses descending");

    std::vector<Record> bytes(200);
    for (int i = 0; i < 200; ++i) {
        for (int k = 0; k < 18; ++k)
            bytes[i].street[k] = static_cast<char>(1 + std::rand() % 254);
        bytes[i].house = static_cast<short>(std::rand() % 100);
    }
    run_case(bytes, "random street bytes");

    if (failures == 0)
        std::printf("all tests passed\n");
    else
        std::printf("%d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
