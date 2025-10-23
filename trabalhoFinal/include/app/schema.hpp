#pragma once

#include <sqlite3.h>
#include <string_view>

namespace app::schema {

/// SQL statement to create the students table
constexpr std::string_view kCreateStudentsTable = R"sql(
    CREATE TABLE IF NOT EXISTS students (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        registration_number TEXT NOT NULL UNIQUE,
        email TEXT,
        phone TEXT,
        active INTEGER NOT NULL DEFAULT 1,
        created_at TEXT NOT NULL DEFAULT (datetime('now')),
        updated_at TEXT NOT NULL DEFAULT (datetime('now'))
    );
)sql";

/// SQL statement to create the books table
constexpr std::string_view kCreateBooksTable = R"sql(
    CREATE TABLE IF NOT EXISTS books (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        title TEXT NOT NULL,
        author TEXT NOT NULL,
        published_year INTEGER,
        isbn TEXT UNIQUE,
        copies_available INTEGER NOT NULL DEFAULT 1,
        created_at TEXT NOT NULL DEFAULT (datetime('now')),
        updated_at TEXT NOT NULL DEFAULT (datetime('now'))
    );
)sql";

/// SQL statement to create the loans table
constexpr std::string_view kCreateLoansTable = R"sql(
    CREATE TABLE IF NOT EXISTS loans (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        student_id INTEGER NOT NULL,
        book_id INTEGER NOT NULL,
        loan_date TEXT NOT NULL DEFAULT (date('now')),
        due_date TEXT NOT NULL,
        return_date TEXT,
        created_at TEXT NOT NULL DEFAULT (datetime('now')),
        updated_at TEXT NOT NULL DEFAULT (datetime('now')),
        FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,
        FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE
    );
)sql";

/// Index on registration_number for faster lookups
constexpr std::string_view kCreateStudentRegNumberIndex = R"sql(
    CREATE INDEX IF NOT EXISTS idx_students_reg_number 
    ON students(registration_number);
)sql";

/// Index on active students
constexpr std::string_view kCreateStudentActiveIndex = R"sql(
    CREATE INDEX IF NOT EXISTS idx_students_active 
    ON students(active);
)sql";

/// Index on loans by student
constexpr std::string_view kCreateLoanStudentIndex = R"sql(
    CREATE INDEX IF NOT EXISTS idx_loans_student 
    ON loans(student_id);
)sql";

/// Index on loans by book
constexpr std::string_view kCreateLoanBookIndex = R"sql(
    CREATE INDEX IF NOT EXISTS idx_loans_book 
    ON loans(book_id);
)sql";

/// Index on unreturned loans
constexpr std::string_view kCreateLoanUnreturnedIndex = R"sql(
    CREATE INDEX IF NOT EXISTS idx_loans_unreturned 
    ON loans(return_date) WHERE return_date IS NULL;
)sql";

/// Trigger to update updated_at on students table
constexpr std::string_view kCreateStudentsUpdateTrigger = R"sql(
    CREATE TRIGGER IF NOT EXISTS update_students_timestamp 
    AFTER UPDATE ON students
    BEGIN
        UPDATE students SET updated_at = datetime('now') 
        WHERE id = NEW.id;
    END;
)sql";

/// Trigger to update updated_at on books table
constexpr std::string_view kCreateBooksUpdateTrigger = R"sql(
    CREATE TRIGGER IF NOT EXISTS update_books_timestamp 
    AFTER UPDATE ON books
    BEGIN
        UPDATE books SET updated_at = datetime('now') 
        WHERE id = NEW.id;
    END;
)sql";

/// Trigger to update updated_at on loans table
constexpr std::string_view kCreateLoansUpdateTrigger = R"sql(
    CREATE TRIGGER IF NOT EXISTS update_loans_timestamp 
    AFTER UPDATE ON loans
    BEGIN
        UPDATE loans SET updated_at = datetime('now') 
        WHERE id = NEW.id;
    END;
)sql";

/// Initialize all tables, indexes, and triggers
/// @param db SQLite database connection
/// @throws std::runtime_error if schema creation fails
void initializeSchema(sqlite3* db);

}  // namespace app::schema
