#include <sqlite3.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "chrmaTUI.hpp"
#include "elements.hpp"

#define TEXT {4, 99, 116, 255}
#define TEXT_HIGHLIGHT {4, 191, 181, 255}
#define TEXT_BACKGROUND {4, 33, 49, 255}
#define TEXT_BACKGROUND_HIGHLIGHT {4, 99, 116, 255}
#define BACKGROUND {4, 14, 22, 255}
#define TRANSPARENT {0, 0, 0, 0}

namespace app {

constexpr auto kDefaultDatabasePath = "library_manager.db";

sqlite3* openOrCreateDatabase(const std::string& path) {
    sqlite3* connection = nullptr;
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
    if (sqlite3_open_v2(path.c_str(), &connection, flags, nullptr) != SQLITE_OK) {
        std::string message = sqlite3_errmsg(connection);
        sqlite3_close(connection);
        throw std::runtime_error("Failed to open SQLite database: " + message);
    }
    return connection;
}

void ensureSchema(sqlite3* connection) {
    static constexpr auto createTableSql = R"sql(
        CREATE TABLE IF NOT EXISTS books (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            author TEXT NOT NULL,
            published_year INTEGER,
            isbn TEXT UNIQUE,
            copies_available INTEGER NOT NULL DEFAULT 1
        );
    )sql";

    char* errorMessage = nullptr;
    if (sqlite3_exec(connection, createTableSql, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        std::string message = errorMessage ? errorMessage : "unknown error";
        sqlite3_free(errorMessage);
        throw std::runtime_error("Failed to initialize schema: " + message);
    }
}

}  // namespace app

int main(int argc, char** argv) {
    const std::string databasePath = (argc > 1) ? argv[1] : app::kDefaultDatabasePath;

    try {
        auto* db = app::openOrCreateDatabase(databasePath);
        app::ensureSchema(db);
        sqlite3_close(db);
    } catch (const std::exception& ex) {
        std::cerr << "Startup error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Database ready at " << std::filesystem::absolute(databasePath) << "\n";

    TUImanager tui;

    container mainContainer({0,0}, {tui.cols,tui.rows}, {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND}, "Main Menu");
    // Set default element style and enable inheritance
    mainContainer.setDefaultElementStyle({TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND});
    mainContainer.setInheritStyle(true);

    tui.containerID = &mainContainer;
    mainContainer.tui = &tui;

    while (true) {
                if (!tui.hasDirty()) {
            bool hasInput = tui.waitForInput(16);
            if (!hasInput) {
                // nothing happened; continue loop without render
                continue;
            }
        }
        if (tui.pollInput()) break;
        mainContainer.render(tui);

        tui.runEndOfFrame();
        if (tui.hasDirty()) {
            tui.render();
        }
    }
    return EXIT_SUCCESS;
}
