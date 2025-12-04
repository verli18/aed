#pragma once

#include "app/db.hpp"
#include "app/models.hpp"

#include <optional>
#include <string>
#include <vector>

namespace app::repos {

/// Repository for CRUD operations on Book entities
class BookRepository {
public:
    explicit BookRepository(Database& db);

    /// Create a new book
    /// @param book Book data (id will be ignored and set to the new ID)
    /// @return The ID of the newly created book
    /// @throws std::runtime_error if insertion fails or validation fails
    int64_t create(models::Book& book);

    /// Find a book by ID
    /// @param id Book ID
    /// @return Book if found, std::nullopt otherwise
    [[nodiscard]] std::optional<models::Book> findById(int64_t id) const;

    /// Find a book by ISBN
    /// @param isbn ISBN number
    /// @return Book if found, std::nullopt otherwise
    [[nodiscard]] std::optional<models::Book> findByISBN(const std::string& isbn) const;

    /// Find a book by its hash ID (e.g., "BK-NcaKQQ")
    /// @param hashId The hash-based ID displayed to users
    /// @return Book if found, std::nullopt otherwise
    [[nodiscard]] std::optional<models::Book> findByHashId(const std::string& hashId) const;

    /// Search books by title (partial match, case-insensitive)
    /// @param title Title or part of title to search for
    /// @return Vector of matching books
    [[nodiscard]] std::vector<models::Book> searchByTitle(const std::string& title) const;

    /// Search books by author (partial match, case-insensitive)
    /// @param author Author name or part of name to search for
    /// @return Vector of matching books
    [[nodiscard]] std::vector<models::Book> searchByAuthor(const std::string& author) const;

    /// Get all books
    /// @param availableOnly If true, only return books with copies_available > 0
    /// @return Vector of all books
    [[nodiscard]] std::vector<models::Book> findAll(bool availableOnly = false) const;

    /// Update an existing book
    /// @param book Book with updated data (id must be set)
    /// @return true if the book was updated, false if not found
    /// @throws std::runtime_error if update fails or validation fails
    bool update(const models::Book& book);

    /// Delete a book by ID (hard delete)
    /// @param id Book ID
    /// @return true if deleted, false if not found
    bool deleteById(int64_t id);

    /// Increment available copies (when a book is returned)
    /// @param id Book ID
    /// @param amount Number of copies to add (default 1)
    /// @return true if updated, false if not found
    bool incrementCopies(int64_t id, int amount = 1);

    /// Decrement available copies (when a book is loaned)
    /// @param id Book ID
    /// @param amount Number of copies to remove (default 1)
    /// @return true if updated, false if not found or not enough copies
    bool decrementCopies(int64_t id, int amount = 1);

    /// Check if an ISBN already exists
    /// @param isbn ISBN to check
    /// @param excludeId Optional ID to exclude from the check (for updates)
    /// @return true if the ISBN exists
    [[nodiscard]] bool isbnExists(const std::string& isbn, 
                                   std::optional<int64_t> excludeId = std::nullopt) const;

    /// Count total books
    /// @param availableOnly If true, only count books with copies_available > 0
    /// @return Number of books
    [[nodiscard]] int64_t count(bool availableOnly = false) const;

    /// Get total number of available copies across all books
    /// @return Sum of all copies_available
    [[nodiscard]] int64_t totalAvailableCopies() const;

private:
    Database& db_;

    /// Helper to map a statement row to a Book object
    [[nodiscard]] models::Book mapRowToBook(const Statement& stmt) const;
};

}  // namespace app::repos
