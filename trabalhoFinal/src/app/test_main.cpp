#include "app/db_test.hpp"

#include <iostream>

int main() {
    std::cout << "Library Manager - Database Test Suite\n";
    std::cout << "======================================\n\n";
    
    try {
        app::runDatabaseTests();
        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "\nFatal error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}
