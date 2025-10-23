#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "app/db.hpp"
#include "app/schema.hpp"
#include "chrmaTUI.hpp"
#include "elements.hpp"

namespace {

constexpr auto kDefaultDatabasePath = "library_manager.db";

}  // anonymous namespace

// Forward declaration
namespace ui { void runTestUI(app::Database& db); }

int main(int argc, char** argv) {
    const std::string databasePath = (argc > 1) ? argv[1] : kDefaultDatabasePath;

    try {
        // Open/create database and initialize schema
        app::Database db(databasePath);
        app::schema::initializeSchema(db.handle());
        
        std::cout << "Database ready at " << std::filesystem::absolute(databasePath) << "\n";
        std::cout << "Launching test UI...\n";

        // Launch the test UI with database connection
        ui::runTestUI(db);

        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "Startup error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}
