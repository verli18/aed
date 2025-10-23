#include "app/repos/book_repository.hpp"

#include <stdexcept>

namespace app::repos {

BookRepository::BookRepository(Database& db) : db_(db) {}

int64_t BookRepository::create(models::Book& book) {
    if (book.title.empty() || book.author.empty()) {
        throw std::runtime_error("Cannot create book: title and author are required");
    }

    // Check for duplicate ISBN if provided
    if (book.isbn.has_value() && !book.isbn->empty()) {
        if (isbnExists(*book.isbn)) {
            throw std::runtime_error("Cannot create book: ISBN already exists");
        }
    }

    constexpr auto sql = R"sql(
        INSERT INTO books (title, author, published_year, isbn, copies_available)
        VALUES (?, ?, ?, ?, ?)
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, book.title);
    stmt.bind(2, book.author);

    if (book.published_year.has_value()) {
        stmt.bind(3, static_cast<int64_t>(*book.published_year));
    } else {
        stmt.bindNull(3);
    }

    if (book.isbn.has_value() && !book.isbn->empty()) {
        stmt.bind(4, *book.isbn);
    } else {
        stmt.bindNull(4);
    }

    stmt.bind(5, static_cast<int64_t>(book.copies_available));

    stmt.step();

    book.id = db_.lastInsertRowId();
    return book.id;
}

std::optional<models::Book> BookRepository::findById(int64_t id) const {
    constexpr auto sql = R"sql(
        SELECT id, title, author, published_year, isbn, copies_available
        FROM books
        WHERE id = ?
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, id);

    if (stmt.step()) {
        return mapRowToBook(stmt);
    }

    return std::nullopt;
}

std::optional<models::Book> BookRepository::findByISBN(const std::string& isbn) const {
    constexpr auto sql = R"sql(
        SELECT id, title, author, published_year, isbn, copies_available
        FROM books
        WHERE isbn = ?
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, isbn);

    if (stmt.step()) {
        return mapRowToBook(stmt);
    }

    return std::nullopt;
}

std::vector<models::Book> BookRepository::searchByTitle(const std::string& title) const {
    constexpr auto sql = R"sql(
        SELECT id, title, author, published_year, isbn, copies_available
        FROM books
        WHERE title LIKE ? COLLATE NOCASE
        ORDER BY title
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, "%" + title + "%");

    std::vector<models::Book> books;
    while (stmt.step()) {
        books.push_back(mapRowToBook(stmt));
    }

    return books;
}

std::vector<models::Book> BookRepository::searchByAuthor(const std::string& author) const {
    constexpr auto sql = R"sql(
        SELECT id, title, author, published_year, isbn, copies_available
        FROM books
        WHERE author LIKE ? COLLATE NOCASE
        ORDER BY author, title
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, "%" + author + "%");

    std::vector<models::Book> books;
    while (stmt.step()) {
        books.push_back(mapRowToBook(stmt));
    }

    return books;
}

std::vector<models::Book> BookRepository::findAll(bool availableOnly) const {
    const std::string sql = availableOnly
        ? R"sql(
            SELECT id, title, author, published_year, isbn, copies_available
            FROM books
            WHERE copies_available > 0
            ORDER BY title
          )sql"
        : R"sql(
            SELECT id, title, author, published_year, isbn, copies_available
            FROM books
            ORDER BY title
          )sql";

    Statement stmt(db_.handle(), sql);
    std::vector<models::Book> books;

    while (stmt.step()) {
        books.push_back(mapRowToBook(stmt));
    }

    return books;
}

bool BookRepository::update(const models::Book& book) {
    if (book.id == 0) {
        throw std::runtime_error("Cannot update book: ID not set");
    }

    if (book.title.empty() || book.author.empty()) {
        throw std::runtime_error("Cannot update book: title and author are required");
    }

    // Check for duplicate ISBN (excluding this book)
    if (book.isbn.has_value() && !book.isbn->empty()) {
        if (isbnExists(*book.isbn, book.id)) {
            throw std::runtime_error("Cannot update book: ISBN already exists");
        }
    }

    constexpr auto sql = R"sql(
        UPDATE books
        SET title = ?, author = ?, published_year = ?, isbn = ?, copies_available = ?
        WHERE id = ?
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, book.title);
    stmt.bind(2, book.author);

    if (book.published_year.has_value()) {
        stmt.bind(3, static_cast<int64_t>(*book.published_year));
    } else {
        stmt.bindNull(3);
    }

    if (book.isbn.has_value() && !book.isbn->empty()) {
        stmt.bind(4, *book.isbn);
    } else {
        stmt.bindNull(4);
    }

    stmt.bind(5, static_cast<int64_t>(book.copies_available));
    stmt.bind(6, book.id);

    stmt.step();

    return db_.changes() > 0;
}

bool BookRepository::deleteById(int64_t id) {
    constexpr auto sql = "DELETE FROM books WHERE id = ?";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, id);
    stmt.step();

    return db_.changes() > 0;
}

bool BookRepository::incrementCopies(int64_t id, int amount) {
    if (amount <= 0) {
        throw std::runtime_error("Cannot increment copies: amount must be positive");
    }

    constexpr auto sql = "UPDATE books SET copies_available = copies_available + ? WHERE id = ?";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, static_cast<int64_t>(amount));
    stmt.bind(2, id);
    stmt.step();

    return db_.changes() > 0;
}

bool BookRepository::decrementCopies(int64_t id, int amount) {
    if (amount <= 0) {
        throw std::runtime_error("Cannot decrement copies: amount must be positive");
    }

    // First check if we have enough copies
    auto book = findById(id);
    if (!book) {
        return false;
    }

    if (book->copies_available < amount) {
        throw std::runtime_error("Cannot decrement copies: not enough copies available");
    }

    constexpr auto sql = "UPDATE books SET copies_available = copies_available - ? WHERE id = ?";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, static_cast<int64_t>(amount));
    stmt.bind(2, id);
    stmt.step();

    return db_.changes() > 0;
}

bool BookRepository::isbnExists(const std::string& isbn, std::optional<int64_t> excludeId) const {
    if (isbn.empty()) {
        return false;
    }

    const std::string sql = excludeId.has_value()
        ? "SELECT COUNT(*) FROM books WHERE isbn = ? AND id != ?"
        : "SELECT COUNT(*) FROM books WHERE isbn = ?";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, isbn);

    if (excludeId.has_value()) {
        stmt.bind(2, *excludeId);
    }

    if (stmt.step()) {
        return stmt.getInt64(0) > 0;
    }

    return false;
}

int64_t BookRepository::count(bool availableOnly) const {
    const std::string sql = availableOnly
        ? "SELECT COUNT(*) FROM books WHERE copies_available > 0"
        : "SELECT COUNT(*) FROM books";

    Statement stmt(db_.handle(), sql);

    if (stmt.step()) {
        return stmt.getInt64(0);
    }

    return 0;
}

int64_t BookRepository::totalAvailableCopies() const {
    constexpr auto sql = "SELECT COALESCE(SUM(copies_available), 0) FROM books";

    Statement stmt(db_.handle(), sql);

    if (stmt.step()) {
        return stmt.getInt64(0);
    }

    return 0;
}

models::Book BookRepository::mapRowToBook(const Statement& stmt) const {
    models::Book book;
    book.id = stmt.getInt64(0);
    book.title = stmt.getText(1);
    book.author = stmt.getText(2);

    if (!stmt.isNull(3)) {
        book.published_year = static_cast<int>(stmt.getInt64(3));
    }

    if (!stmt.isNull(4)) {
        book.isbn = stmt.getText(4);
    }

    book.copies_available = static_cast<int>(stmt.getInt64(5));

    return book;
}

}  // namespace app::repos
