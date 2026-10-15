#include "search.hpp"
#include "queue.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>
#include <vector>

static long g_allocated = 0;
static long g_freed = 0;

void* operator new(std::size_t n) {
    void* p = std::malloc(n);
    if (!p) std::abort();
    ++g_allocated;
    return p;
}

void operator delete(void* p) noexcept {
    if (p) ++g_freed;
    std::free(p);
}

void operator delete(void* p, std::size_t) noexcept {
    if (p) ++g_freed;
    std::free(p);
}

static int failures = 0;

static void expect(bool cond, const char* name) {
    if (!cond) {
        std::printf("FAIL %s\n", name);
        ++failures;
    }
}

static int ref_compare_prefix(const Record* r, const std::string& key) {
    for (size_t i = 0; i < key.size(); ++i) {
        unsigned char a = static_cast<unsigned char>(r->street[i]);
        unsigned char b = static_cast<unsigned char>(key[i]);
        if (a >= 'A' && a <= 'Z') a = static_cast<unsigned char>(a + 0x20);
        else if (a >= 0x80 && a <= 0x8F) a = static_cast<unsigned char>(a + 0x20);
        else if (a >= 0x90 && a <= 0x9F) a = static_cast<unsigned char>(a + 0x50);
        else if (a == 0xF0) a = 0xF1;
        if (b >= 'A' && b <= 'Z') b = static_cast<unsigned char>(b + 0x20);
        else if (b >= 0x80 && b <= 0x8F) b = static_cast<unsigned char>(b + 0x20);
        else if (b >= 0x90 && b <= 0x9F) b = static_cast<unsigned char>(b + 0x50);
        else if (b == 0xF0) b = 0xF1;
        if (a < b) return -1;
        if (a > b) return 1;
    }
    return 0;
}

static Record make_record(const char* street, short house) {
    Record r;
    std::memset(&r, 0, sizeof(r));
    std::memset(r.street, ' ', 18);
    size_t len = std::strlen(street);
    if (len > 17) len = 17;
    std::memcpy(r.street, street, len);
    r.house = house;
    return r;
}

static void set_street(Record& r, const unsigned char* bytes, size_t len) {
    std::memset(r.street, ' ', 18);
    std::memcpy(r.street, bytes, len);
}

static std::vector<const Record*> queue_items(const RecordQueue& q) {
    std::vector<const Record*> out;
    const tLE* p = q.head();
    while (p != nullptr) {
        out.push_back(p->data);
        p = p->next;
    }
    return out;
}

static size_t ref_first(const std::vector<Record*>& index, const std::string& key) {
    for (size_t i = 0; i < index.size(); ++i)
        if (ref_compare_prefix(index[i], key) == 0) return i;
    return index.size();
}

static bool ref_less(const Record* a, const Record* b) {
    int c = std::memcmp(a->street, b->street, 18);
    if (c != 0) return c < 0;
    return a->house < b->house;
}

static void check_against_reference(std::vector<Record*>& index,
                                    const std::string& key,
                                    const char* name) {
    std::sort(index.begin(), index.end(), ref_less);
    size_t expected = ref_first(index, key);
    size_t got = find_first_prefix(index.data(), index.size(), key);
    if (got != expected) {
        std::printf("FAIL %s: key=%s expected=%zu got=%zu\n",
                    name, key.c_str(), expected, got);
        ++failures;
        return;
    }
    RecordQueue q;
    enqueue_prefix_matches(index.data(), index.size(), got, key, q);
    size_t count = 0;
    for (size_t i = 0; i < index.size(); ++i)
        if (ref_compare_prefix(index[i], key) == 0) ++count;
    std::vector<const Record*> items = queue_items(q);
    if (items.size() != count) {
        std::printf("FAIL %s: queue size %zu expected %zu\n",
                    name, items.size(), count);
        ++failures;
        return;
    }
    for (size_t i = 0; i < items.size(); ++i)
        if (ref_compare_prefix(items[i], key) != 0) {
            std::printf("FAIL %s: queue item %zu does not match\n", name, i);
            ++failures;
            return;
        }
}

static void test_empty() {
    RecordQueue q;
    expect(q.empty(), "empty queue is empty");
    expect(q.size() == 0, "empty queue size 0");
    expect(q.head() == nullptr, "empty queue head NULL");
    expect(q.pop() == nullptr, "pop from empty queue");
    std::vector<Record*> index;
    expect(find_first_prefix(index.data(), 0, "dem") == 0, "search in empty index");
}

static void test_fifo() {
    Record a = make_record("Alpha", 1);
    Record b = make_record("Alpha", 2);
    Record c = make_record("Alpha", 3);
    RecordQueue q;
    q.push(&a);
    q.push(&b);
    q.push(&c);
    expect(!q.empty(), "fifo not empty");
    expect(q.size() == 3, "fifo size 3");
    std::vector<const Record*> items = queue_items(q);
    expect(items.size() == 3 && items[0] == &a && items[1] == &b && items[2] == &c,
           "fifo order");
    expect(q.pop() == &a, "pop first");
    expect(q.pop() == &b, "pop second");
    expect(q.size() == 1, "size after two pops");
    q.push(&a);
    expect(queue_items(q) == std::vector<const Record*>{&c, &a},
           "push to non-empty keeps tail");
    expect(q.pop() == &c && q.pop() == &a, "drain after refilled");
    expect(q.empty(), "queue empty after drain");
    expect(q.head() == nullptr, "head NULL after drain");
    q.push(&b);
    expect(q.size() == 1 && q.head()->data == &b,
           "push to emptied queue reinitializes head");
}

static void test_single_element() {
    Record r = make_record("Alpha", 5);
    std::vector<Record*> index{&r};
    expect(find_first_prefix(index.data(), 1, "alp") == 0, "single match");
    expect(find_first_prefix(index.data(), 1, "bet") == 1, "single mismatch");
    RecordQueue q;
    enqueue_prefix_matches(index.data(), 1, 0, "alp", q);
    expect(q.size() == 1, "single match queued");
    RecordQueue q2;
    enqueue_prefix_matches(index.data(), 1, 1, "bet", q2);
    expect(q2.empty(), "mismatch yields empty queue");
}

static void test_two_streets_same_prefix() {
    Record a1 = make_record("Demyanova", 1);
    Record a2 = make_record("Demyanova", 6);
    Record b1 = make_record("Demyanovoy", 2);
    Record c1 = make_record("Zeta", 3);
    std::vector<Record*> index{&a1, &a2, &b1, &c1};
    check_against_reference(index, "Dem", "two streets one prefix");

    std::sort(index.begin(), index.end(),
              [](const Record* x, const Record* y) {
                  return std::memcmp(x->street, y->street, 18) < 0;
              });
    RecordQueue q;
    enqueue_prefix_matches(index.data(), index.size(),
                           find_first_prefix(index.data(), index.size(), "dem"),
                           "dem", q);
    expect(q.size() == 3, "prefix spans two streets");
    std::vector<const Record*> items = queue_items(q);
    expect(items.size() == 3 && items[0] == &a1 && items[1] == &a2 && items[2] == &b1,
           "queue order follows sort order");
}

static void test_cyrillic_case() {
    static const unsigned char dem[] = {0x84, 0xA5, 0xAC, 'y', 'a', 'n', 'o', 'v', 'a'};
    static const unsigned char chet[] = {0x97, 0xA5, 0xE2, 'a'};
    Record d = make_record("Zeta", 2);
    set_street(d, dem, sizeof(dem));
    Record z = make_record("Zeta", 3);
    set_street(z, chet, sizeof(chet));
    std::vector<Record*> index{&z, &d};
    std::sort(index.begin(), index.end(),
              [](const Record* x, const Record* y) {
                  return std::memcmp(x->street, y->street, 18) < 0;
              });
    const std::string key_dem("\xA4\xA5\xAC", 3);
    expect(find_first_prefix(index.data(), 2, key_dem) == 0,
           "lowercase key matches uppercase street");
    RecordQueue q;
    enqueue_prefix_matches(index.data(), index.size(), 0, key_dem, q);
    expect(q.size() == 1, "cyrillic case match queued");
}

static void test_positions() {
    std::vector<Record> storage;
    storage.push_back(make_record("Alpha", 1));
    storage.push_back(make_record("Beta", 2));
    storage.push_back(make_record("Gamma", 3));
    storage.push_back(make_record("Omega", 4));
    storage.push_back(make_record("Zeta", 5));
    std::vector<Record*> index;
    for (Record& r : storage) index.push_back(&r);

    expect(find_first_prefix(index.data(), index.size(), "alp") == 0, "key at begin");
    expect(find_first_prefix(index.data(), index.size(), "ome") == 3, "key in middle");
    expect(find_first_prefix(index.data(), index.size(), "zet") == 4, "key at end");
    expect(find_first_prefix(index.data(), index.size(), "aaa") == 5, "key before all");
    expect(find_first_prefix(index.data(), index.size(), "zzz") == 5, "key after all");
    expect(find_first_prefix(index.data(), index.size(), "xyz") == 5, "key missing");

    RecordQueue q;
    enqueue_prefix_matches(index.data(), index.size(),
                           find_first_prefix(index.data(), index.size(), "zet"),
                           "zet", q);
    expect(q.size() == 1, "zeta queued");
}

static void test_all_equal() {
    std::vector<Record> storage(10, make_record("Same", 1));
    std::vector<Record*> index;
    for (Record& r : storage) index.push_back(&r);
    RecordQueue q;
    enqueue_prefix_matches(index.data(), index.size(),
                           find_first_prefix(index.data(), index.size(), "sam"),
                           "sam", q);
    expect(q.size() == 10, "all equal queued");
    expect(find_first_prefix(index.data(), index.size(), "san") == 10, "next key absent");
}

static void test_random_vs_reference() {
    std::srand(42);
    for (int iter = 0; iter < 300; ++iter) {
        int n = 1 + std::rand() % 40;
        std::vector<Record> storage;
        for (int i = 0; i < n; ++i) {
            char street[16];
            int len = 3 + std::rand() % 5;
            for (int k = 0; k < len; ++k)
                street[k] = static_cast<char>('a' + std::rand() % 4);
            street[len] = '\0';
            storage.push_back(make_record(street, static_cast<short>(i)));
        }
        std::vector<Record*> index;
        for (Record& r : storage) index.push_back(&r);
        std::sort(index.begin(), index.end(),
                  [](const Record* x, const Record* y) {
                      return std::memcmp(x->street, y->street, 18) < 0;
                  });
        for (int k = 0; k < 4; ++k) {
            char key[4];
            int len = 1 + std::rand() % 3;
            for (int c = 0; c < len; ++c)
                key[c] = static_cast<char>('a' + std::rand() % 5);
            key[len] = '\0';
            check_against_reference(index, std::string(key), "random");
        }
    }
}

static void test_no_leak() {
    {
        RecordQueue q;
        std::vector<Record> storage(100);
        for (int i = 0; i < 100; ++i)
            storage[i] = make_record("Leak", static_cast<short>(i));
        for (int i = 0; i < 100; ++i)
            q.push(&storage[i]);
        while (!q.empty())
            q.pop();
    }
    expect(g_allocated == g_freed, "queue nodes freed after drain");

    long allocated_before = g_allocated;
    long freed_before = g_freed;
    {
        RecordQueue q;
        Record r = make_record("Leak", 1);
        q.push(&r);
        q.push(&r);
        q.push(&r);
    }
    expect(g_allocated - allocated_before == g_freed - freed_before,
           "queue nodes freed by destructor");
}

int main() {
    test_empty();
    test_fifo();
    test_single_element();
    test_two_streets_same_prefix();
    test_cyrillic_case();
    test_positions();
    test_all_equal();
    test_random_vs_reference();
    test_no_leak();

    if (failures == 0)
        std::printf("all search tests passed\n");
    else
        std::printf("%d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
