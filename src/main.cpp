#include "database.hpp"
#include "view.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        std::string path = argc > 1 ? argv[1] : "testBase4.dat";
        Database db(path);
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        return run_app(db);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
