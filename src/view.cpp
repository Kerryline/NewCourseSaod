#include "view.hpp"
#include "database.hpp"
#include "encoding.hpp"
#include "queue.hpp"
#include "search.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

const size_t kPageSize = 20;
const size_t kLeftPad = 2;
const size_t kGap = 3;
const size_t kNumWidth = 5;
const size_t kFioWidth = 36;
const size_t kStreetWidth = 18;
const size_t kHouseWidth = 4;
const size_t kAptWidth = 5;
const size_t kDateWidth = 10;
const size_t kInnerWidth = 95;

size_t utf8_width(const std::string& text) {
    size_t width = 0;
    for (unsigned char c : text)
        if ((c & 0xC0) != 0x80)
            ++width;
    return width;
}

void print_field(const std::string& text, size_t width) {
    std::cout << text;
    for (size_t i = utf8_width(text); i < width; ++i)
        std::cout << ' ';
}

void clear_screen() {
    std::cout << "\x1B[2J\x1B[H";
}

void print_border(const char* left, const char* fill, const char* right) {
    std::cout << left;
    for (size_t i = 0; i < kInnerWidth; ++i)
        std::cout << fill;
    std::cout << right << '\n';
}

void print_top() { print_border("╔", "═", "╗"); }
void print_mid() { print_border("╠", "═", "╣"); }
void print_bottom() { print_border("╚", "═", "╝"); }

void print_line(const std::string& text) {
    std::cout << "║" << std::string(kLeftPad, ' ') << text;
    size_t used = kLeftPad + utf8_width(text);
    for (size_t i = used; i < kInnerWidth; ++i)
        std::cout << ' ';
    std::cout << "║\n";
}

void print_columns(const std::string& num, const std::string& fio, const std::string& street,
                   const std::string& house, const std::string& apt,
                   const std::string& date) {
    std::cout << "║" << std::string(kLeftPad, ' ');
    print_field(num, kNumWidth);
    std::cout << std::string(kGap, ' ');
    print_field(fio, kFioWidth);
    std::cout << std::string(kGap, ' ');
    print_field(street, kStreetWidth);
    std::cout << std::string(kGap, ' ');
    print_field(house, kHouseWidth);
    std::cout << std::string(kGap, ' ');
    print_field(apt, kAptWidth);
    std::cout << std::string(kGap, ' ');
    print_field(date, kDateWidth);
    std::cout << "║\n";
}

void print_header_row() {
    print_columns("№", "ФИО", "Улица", "Дом", "Кв", "Дата");
}

void print_record_row(const Record& r, size_t number) {
    print_columns(std::to_string(number),
                  cp866_display_field(r.fio, sizeof(r.fio)),
                  cp866_display_field(r.street, sizeof(r.street)),
                  std::to_string(r.house),
                  std::to_string(r.apartment),
                  cp866_display_field(r.date, sizeof(r.date)));
}

void print_page(const Database& db, size_t page, size_t pages,
                const std::string& title, bool sorted_view) {
    clear_screen();
    print_top();
    print_line(title);
    print_mid();
    print_header_row();
    print_mid();
    size_t start = page * kPageSize;
    size_t end = std::min(start + kPageSize, db.size());
    for (size_t i = start; i < end; ++i)
        print_record_row(sorted_view ? db.at(i) : db.source_at(i), i + 1);
    print_mid();
    std::string menu = "Страница " + std::to_string(page + 1) + "/" + std::to_string(pages)
        + " │ N-след │ Р-пред │ B-назад │ R-случайная │ I-по номеру │ A-вся БД";
    if (sorted_view) menu += " │ S-поиск";
    print_line(menu);
    print_bottom();
}

void pause() {
    std::cout << "\n<Enter> — продолжить: " << std::flush;
    std::string dummy;
    std::getline(std::cin, dummy);
}

void show_record(const Record& r, const std::string& title, size_t number) {
    clear_screen();
    print_top();
    print_line(title);
    print_mid();
    print_header_row();
    print_mid();
    print_record_row(r, number);
    print_bottom();
}

void show_random(const Database& db, bool sorted_view) {
    if (db.size() == 0) {
        std::cout << "База данных пуста.\n";
        return;
    }
    size_t number = 1 + static_cast<size_t>(std::rand()) % db.size();
    show_record(sorted_view ? db.at(number - 1) : db.source_at(number - 1),
                "Случайная запись № " + std::to_string(number), number);
    pause();
}

void show_by_number(const Database& db, bool sorted_view) {
    std::cout << "Номер записи (1-" << db.size() << "): " << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) return;
    char* end = nullptr;
    long number = std::strtol(line.c_str(), &end, 10);
    if (end == line.c_str() || *end != '\0' || number < 1
        || number > static_cast<long>(db.size())) {
        std::cout << "Неверный номер записи.\n";
        pause();
        return;
    }
    show_record(sorted_view ? db.at(static_cast<size_t>(number) - 1)
                            : db.source_at(static_cast<size_t>(number) - 1),
                "Запись № " + std::to_string(number),
                static_cast<size_t>(number));
    pause();
}

void show_all(const Database& db, const std::string& title, bool sorted_view) {
    clear_screen();
    print_top();
    print_line(title + " (вся)");
    print_mid();
    print_header_row();
    print_mid();
    for (size_t i = 0; i < db.size(); ++i)
        print_record_row(sorted_view ? db.at(i) : db.source_at(i), i + 1);
    print_bottom();
    pause();
}

void show_queue(const RecordQueue& q, const std::string& key) {
    std::size_t found = q.size();
    clear_screen();
    print_top();
    print_line("Очередь: ключ \"" + key + "\" — найдено " + std::to_string(found) + " записей");
    print_mid();
    if (found > 0) {
        print_header_row();
        print_mid();
        const tLE* p = q.head();
        std::size_t pos = 1;
        while (p != nullptr) {
            print_record_row(*p->data, pos);
            p = p->next;
            ++pos;
        }
        print_mid();
    }
    print_line(found == 0
                   ? "Очередь пуста. Клавиша: B-назад"
                   : "В очереди записей: " + std::to_string(found) + " │ B-назад");
    print_bottom();
    pause();
}

void run_search(const Database& db) {
    std::cout << "Ключ (первые 3 буквы улицы): " << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) return;
    size_t begin = line.find_first_not_of(" \t");
    size_t end = line.find_last_not_of(" \t");
    std::string display = (begin == std::string::npos) ? std::string() : line.substr(begin, end - begin + 1);
    if (display.empty() || utf8_width(display) > 3) {
        std::cout << "Ключ должен содержать от 1 до 3 букв.\n";
        pause();
        return;
    }
    std::string key = utf8_to_cp866(display);
    if (key.empty()) {
        std::cout << "Ключ не удалось преобразовать в кодировку базы.\n";
        pause();
        return;
    }
    RecordQueue q;
    std::size_t first = find_first_prefix(db.index_data(), db.size(), key);
    enqueue_prefix_matches(db.index_data(), db.size(), first, key, q);
    show_queue(q, display);
}

char normalize_command(const std::string& input) {
    unsigned char c = static_cast<unsigned char>(input[0]);
    if (c >= 'A' && c <= 'Z') return static_cast<char>(c + ('a' - 'A'));
    if (c < 0x80) return static_cast<char>(c);
    if ((c == 0xD0 || c == 0xD1) && input.size() >= 2) {
        unsigned char d = static_cast<unsigned char>(input[1]);
        if (d >= 0x80 && d <= 0xBF) {
            unsigned code = ((c & 0x1F) << 6) | (d & 0x3F);
            if (code >= 0x410 && code <= 0x42F) code += 0x20;
            if (code == 0x451) code = 0x435;
            switch (code) {
                case 0x430: return 'a';
                case 0x432: return 'b';
                case 0x435: return 'e';
                case 0x43A: return 'k';
                case 0x43C: return 'm';
                case 0x43D: return 'n';
                case 0x43E: return 'o';
                case 0x440: return 'p';
                case 0x441: return 'c';
                case 0x442: return 't';
                case 0x443: return 'y';
                case 0x445: return 'x';
                case 0x44B: return 's';
            }
        }
    }
    return '?';
}

void print_main_menu(size_t records, bool sorted) {
    clear_screen();
    print_top();
    print_line("База данных \"Населенный пункт\": " + std::to_string(records) + " записей");
    print_mid();
    print_line("1 - Исходная база данных");
    print_line("2 - Отсортированная база данных");
    print_line("0 - Выход");
    print_bottom();
}

void browse(const Database& db, const std::string& title, bool sorted_view) {
    size_t pages = (db.size() + kPageSize - 1) / kPageSize;
    if (pages == 0) pages = 1;
    size_t page = 0;
    for (;;) {
        print_page(db, page, pages, title, sorted_view);
        std::cout << "Выбор: " << std::flush;
        std::string input;
        if (!std::getline(std::cin, input)) return;
        if (input.empty()) continue;
        char cmd = normalize_command(input);
        if (cmd == 'n') {
            if (page + 1 < pages) ++page;
        } else if (cmd == 'p') {
            if (page > 0) --page;
        } else if (cmd == 'b' || cmd == 'x') {
            return;
        } else if (cmd == 'r') {
            show_random(db, sorted_view);
        } else if (cmd == 'i') {
            show_by_number(db, sorted_view);
        } else if (cmd == 'a') {
            show_all(db, title, sorted_view);
        } else if (cmd == 's') {
            if (sorted_view) {
                run_search(db);
            } else {
                std::cout << "Поиск доступен только в отсортированной базе.\n";
                pause();
            }
        } else {
            std::cout << "Неизвестная команда.\n";
            pause();
        }
    }
}

}

int run_app(Database& db) {
    bool sorted = false;
    for (;;) {
        print_main_menu(db.size(), sorted);
        std::cout << "Выбор: " << std::flush;
        std::string input;
        if (!std::getline(std::cin, input)) return 0;
        if (input.empty()) continue;
        char cmd = normalize_command(input);
        if (cmd == '1') {
            browse(db, "Исходная база данных", false);
        } else if (cmd == '2') {
            if (!sorted) {
                db.sort();
                sorted = true;
            }
            browse(db, "Отсортированная база данных", true);
        } else if (cmd == '0') {
            return 0;
        } else {
            std::cout << "Неизвестная команда.\n";
            pause();
        }
    }
}
