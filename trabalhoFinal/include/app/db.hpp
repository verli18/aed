#pragma once

#include <sqlite3.h>

#include <memory>
#include <string>

namespace app {

/// RAII wrapper for SQLite database connections
class Database {
public:
    /// Opens or creates a database at the specified path
    /// @param path Path to the database file
    /// @param flags SQLite open flags (default: READWRITE | CREATE | FULLMUTEX)
    /// @throws std::runtime_error if the database cannot be opened
    explicit Database(const std::string& path, 
                     int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX);

    ~Database();

    // Disable copying
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Allow moving
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    /// Get the raw SQLite connection handle
    /// @return Raw sqlite3* pointer (never null while Database is valid)
    [[nodiscard]] sqlite3* handle() const { return connection_; }

    /// Check if the database is open
    [[nodiscard]] bool isOpen() const { return connection_ != nullptr; }

    /// Begin a transaction
    void beginTransaction();

    /// Commit the current transaction
    void commit();

    /// Rollback the current transaction
    void rollback();

    /// Execute a simple SQL statement with no parameters or result
    /// @param sql The SQL statement to execute
    /// @throws std::runtime_error if execution fails
    void execute(const std::string& sql);

    /// Get the last inserted row ID
    [[nodiscard]] int64_t lastInsertRowId() const;

    /// Get the number of rows modified by the last statement
    [[nodiscard]] int changes() const;

private:
    sqlite3* connection_ = nullptr;
};

/// RAII wrapper for SQLite prepared statements
class Statement {
public:
    /// Prepares a statement for the given database
    /// @param db Database connection
    /// @param sql SQL statement with optional ? placeholders
    /// @throws std::runtime_error if preparation fails
    Statement(sqlite3* db, const std::string& sql);

    /// Destructor finalizes the statement
    ~Statement();

    // Disable copying
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    // Allow moving
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

    /// Reset the statement to be re-executed
    void reset();

    /// Clear all parameter bindings
    void clearBindings();

    /// Bind a 64-bit integer to a parameter (1-indexed)
    void bind(int index, int64_t value);

    /// Bind a string to a parameter (1-indexed)
    void bind(int index, const std::string& value);

    /// Bind NULL to a parameter (1-indexed)
    void bindNull(int index);

    /// Execute the statement and step to the first/next row
    /// @return true if a row is available, false if done
    [[nodiscard]] bool step();

    /// Get an integer column value (0-indexed)
    [[nodiscard]] int64_t getInt64(int column) const;

    /// Get a text column value (0-indexed)
    [[nodiscard]] std::string getText(int column) const;

    /// Check if a column is NULL (0-indexed)
    [[nodiscard]] bool isNull(int column) const;

    /// Get the raw statement handle
    [[nodiscard]] sqlite3_stmt* handle() const { return stmt_; }

private:
    sqlite3_stmt* stmt_ = nullptr;
};

}  // namespace app
