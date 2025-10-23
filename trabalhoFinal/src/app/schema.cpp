#include "app/schema.hpp"

#include <sqlite3.h>

#include <stdexcept>
#include <string>

namespace app::schema {

namespace {

/// Helper to execute a single SQL statement
void executeSql(sqlite3* db, std::string_view sql) {
    char* errorMessage = nullptr;
    if (sqlite3_exec(db, sql.data(), nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        std::string message = errorMessage ? errorMessage : "unknown error";
        sqlite3_free(errorMessage);
        throw std::runtime_error("SQL execution failed: " + message);
    }
}

} 

void initializeSchema(sqlite3* db) {
    if (!db) {
        throw std::runtime_error("Cannot initialize schema: null database connection");
    }

    // Create tables
    executeSql(db, kCreateStudentsTable);
    executeSql(db, kCreateBooksTable);
    executeSql(db, kCreateLoansTable);

    // Create indexes
    executeSql(db, kCreateStudentRegNumberIndex);
    executeSql(db, kCreateStudentActiveIndex);
    executeSql(db, kCreateLoanStudentIndex);
    executeSql(db, kCreateLoanBookIndex);
    executeSql(db, kCreateLoanUnreturnedIndex);

    // Create triggers
    executeSql(db, kCreateStudentsUpdateTrigger);
    executeSql(db, kCreateBooksUpdateTrigger);
    executeSql(db, kCreateLoansUpdateTrigger);
}

}  // namespace app::schema
