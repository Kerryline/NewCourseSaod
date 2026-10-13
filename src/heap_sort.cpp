#include "heap_sort.hpp"
#include <utility>

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

static void build_heap(Record** a, int L, int R) {
    Record* x = a[L];
    int i = L;
    for (;;) {
        int j = 2 * i + 1;
        if (j > R) break;
        if (j < R && compare_keys(a[j + 1], a[j]) >= 0)
            ++j;
        if (compare_keys(x, a[j]) >= 0) break;
        a[i] = a[j];
        i = j;
    }
    a[i] = x;
}

void heap_sort(Record** a, int n) {
    int L = n / 2;
    while (L > 0) {
        --L;
        build_heap(a, L, n - 1);
    }
    int R = n - 1;
    while (R > 0) {
        std::swap(a[0], a[R]);
        --R;
        build_heap(a, 0, R);
    }
}
