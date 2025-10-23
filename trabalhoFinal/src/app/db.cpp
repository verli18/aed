#include "app/db.hpp"

#include <stdexcept>
#include <utility>

namespace app {

Database::Database(const std::string& path, int flags) {
    if (sqlite3_open_v2(path.c_str(), &connection_, flags, nullptr) != SQLITE_OK) {
        std::string message = connection_ ? sqlite3_errmsg(connection_) : "unknown error";
        if (connection_) {
            sqlite3_close(connection_);
        }
        throw std::runtime_error("Failed to open database: " + message);
    }

    // Enable foreign keys by default
    execute("PRAGMA foreign_keys = ON;");
}

Database::~Database() {
    if (connection_) {
        sqlite3_close(connection_);
    }
}

Database::Database(Database&& other) noexcept : connection_(other.connection_) {
    other.connection_ = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        if (connection_) {
            sqlite3_close(connection_);
        }
        connection_ = other.connection_;
        other.connection_ = nullptr;
    }
    return *this;
}

void Database::beginTransaction() {
    execute("BEGIN TRANSACTION;");
}

void Database::commit() {
    execute("COMMIT;");
}

void Database::rollback() {
    execute("ROLLBACK;");
}

void Database::execute(const std::string& sql) {
    char* errorMessage = nullptr;
    if (sqlite3_exec(connection_, sql.c_str(), nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        std::string message = errorMessage ? errorMessage : "unknown error";
        sqlite3_free(errorMessage);
        throw std::runtime_error("SQL execution failed: " + message);
    }
}

int64_t Database::lastInsertRowId() const {
    return sqlite3_last_insert_rowid(connection_);
}

int Database::changes() const {
    return sqlite3_changes(connection_);
}

Statement::Statement(sqlite3* db, const std::string& sql) {
    if (!db) {
        throw std::runtime_error("Cannot prepare statement: null database connection");
    }

    if (sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()), &stmt_, nullptr) != SQLITE_OK) {
        std::string message = sqlite3_errmsg(db);
        throw std::runtime_error("Failed to prepare statement: " + message);
    }
}

Statement::~Statement() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
    }
}

Statement::Statement(Statement&& other) noexcept : stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (stmt_) {
            sqlite3_finalize(stmt_);
        }
        stmt_ = other.stmt_;
        other.stmt_ = nullptr;
    }
    return *this;
}

void Statement::reset() {
    if (sqlite3_reset(stmt_) != SQLITE_OK) {
        throw std::runtime_error("Failed to reset statement");
    }
}

void Statement::clearBindings() {
    if (sqlite3_clear_bindings(stmt_) != SQLITE_OK) {
        throw std::runtime_error("Failed to clear bindings");
    }
}

void Statement::bind(int index, int64_t value) {
    if (sqlite3_bind_int64(stmt_, index, value) != SQLITE_OK) {
        throw std::runtime_error("Failed to bind int64 parameter");
    }
}

void Statement::bind(int index, const std::string& value) {
    if (sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT) != SQLITE_OK) {
        throw std::runtime_error("Failed to bind text parameter");
    }
}

void Statement::bindNull(int index) {
    if (sqlite3_bind_null(stmt_, index) != SQLITE_OK) {
        throw std::runtime_error("Failed to bind NULL parameter");
    }
}

bool Statement::step() {
    int result = sqlite3_step(stmt_);
    if (result == SQLITE_ROW) {
        return true;
    } else if (result == SQLITE_DONE) {
        return false;
    } else {
        throw std::runtime_error("Statement execution failed: " + 
                                 std::string(sqlite3_errmsg(sqlite3_db_handle(stmt_))));
    }
}

int64_t Statement::getInt64(int column) const {
    return sqlite3_column_int64(stmt_, column);
}

std::string Statement::getText(int column) const {
    const unsigned char* text = sqlite3_column_text(stmt_, column);
    return text ? reinterpret_cast<const char*>(text) : "";
}

bool Statement::isNull(int column) const {
    return sqlite3_column_type(stmt_, column) == SQLITE_NULL;
}

}  // namespace app
