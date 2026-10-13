#include "view.hpp"
#include "database.hpp"
#include "encoding.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

const size_t kPageSize = 20;
const size_t kLeftPad = 2;
const size_t kGap = 3;
const size_t kFioWidth = 34;
const size_t kStreetWidth = 18;
const size_t kHouseWidth = 4;
const size_t kAptWidth = 5;
const size_t kDateWidth = 10;
const size_t kInnerWidth = 85;

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

void print_columns(const std::string& fio, const std::string& street,
                   const std::string& house, const std::string& apt,
                   const std::string& date) {
    std::cout << "║" << std::string(kLeftPad, ' ');
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
    print_columns("ФИО", "Улица", "Дом", "Кв", "Дата");
}

void print_record_row(const Record& r) {
    print_columns(cp866_display_field(r.fio, sizeof(r.fio)),
                  cp866_display_field(r.street, sizeof(r.street)),
                  std::to_string(r.house),
                  std::to_string(r.apartment),
                  cp866_display_field(r.date, sizeof(r.date)));
}

void print_page(const Database& db, size_t page, size_t pages, const std::string& title) {
    clear_screen();
    print_top();
    print_line(title);
    print_mid();
    print_header_row();
    print_mid();
    size_t start = page * kPageSize;
    size_t end = std::min(start + kPageSize, db.size());
    for (size_t i = start; i < end; ++i)
        print_record_row(db.at(i));
    print_mid();
    print_line("Страница " + std::to_string(page + 1) + "/" + std::to_string(pages)
        + " │ N-след │ Р-пред │ B-назад │ R-случайная │ I-по номеру │ A-вся БД");
    print_bottom();
}

void pause() {
    std::cout << "\n<Enter> — продолжить: " << std::flush;
    std::string dummy;
    std::getline(std::cin, dummy);
}

void show_record(const Record& r, const std::string& title) {
    clear_screen();
    print_top();
    print_line(title);
    print_mid();
    print_header_row();
    print_mid();
    print_record_row(r);
    print_bottom();
}

void show_random(const Database& db) {
    if (db.size() == 0) {
        std::cout << "База данных пуста.\n";
        return;
    }
    size_t number = 1 + static_cast<size_t>(std::rand()) % db.size();
    show_record(db.at(number - 1), "Случайная запись № " + std::to_string(number));
    pause();
}

void show_by_number(const Database& db) {
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
    show_record(db.at(static_cast<size_t>(number) - 1),
                "Запись № " + std::to_string(number));
    pause();
}

void show_all(const Database& db, const std::string& title) {
    clear_screen();
    print_top();
    print_line(title + " (вся)");
    print_mid();
    print_header_row();
    print_mid();
    for (size_t i = 0; i < db.size(); ++i)
        print_record_row(db.at(i));
    print_bottom();
    pause();
}

char normalize_command(const std::string& input) {
    if (input.empty()) return '\n';
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
            }
        }
    }
    return '?';
}

void print_main_menu(size_t records, bool sorted) {
    clear_screen();
    print_top();
    print_line("База данных «Населенный пункт»: " + std::to_string(records) + " записей");
    print_mid();
    print_line("1 - Исходная база данных");
    print_line("2 - Отсортированная база данных" + std::string(sorted ? " (уже упорядочена)" : ""));
    print_line("0 - Выход");
    print_bottom();
}

void browse(const Database& db, const std::string& title) {
    size_t pages = (db.size() + kPageSize - 1) / kPageSize;
    if (pages == 0) pages = 1;
    size_t page = 0;
    for (;;) {
        print_page(db, page, pages, title);
        std::cout << "Выбор: " << std::flush;
        std::string input;
        if (!std::getline(std::cin, input)) return;
        char cmd = normalize_command(input);
        if (cmd == '\n' || cmd == 'n') {
            if (page + 1 < pages) ++page;
        } else if (cmd == 'p') {
            if (page > 0) --page;
        } else if (cmd == 'b' || cmd == 'x') {
            return;
        } else if (cmd == 'r') {
            show_random(db);
        } else if (cmd == 'i') {
            show_by_number(db);
        } else if (cmd == 'a') {
            show_all(db, title);
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
        char cmd = normalize_command(input);
        if (cmd == '1') {
            browse(db, "Исходная база данных");
        } else if (cmd == '2') {
            if (!sorted) {
                db.sort();
                sorted = true;
            }
            browse(db, "Отсортированная база данных");
        } else if (cmd == '0') {
            return 0;
        } else {
            std::cout << "Неизвестная команда.\n";
            pause();
        }
    }
}
