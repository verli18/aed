#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace app::models {

// ============================================================================
// HASH ID UTILITIES
// ============================================================================
// FNV-1a hash function for generating display IDs
// This creates human-readable, hash-based identifiers from entity data
// ============================================================================

namespace hash_utils {
    // FNV-1a constants (64-bit)
    constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME = 1099511628211ULL;
    
    // Base62 alphabet for compact encoding
    constexpr const char* BASE62 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    /// FNV-1a hash - O(n) time, O(1) space
    inline uint64_t fnv1a(const std::string& data) {
        uint64_t hash = FNV_OFFSET;
        for (unsigned char c : data) {
            hash ^= c;
            hash *= FNV_PRIME;
        }
        return hash;
    }
    
    /// Convert to base62 - O(k) where k is output length
    inline std::string toBase62(uint64_t value, size_t len = 6) {
        std::string result(len, '0');
        for (size_t i = len; i > 0 && value > 0; --i) {
            result[i - 1] = BASE62[value % 62];
            value /= 62;
        }
        return result;
    }
    
    /// Generate hash ID with prefix
    inline std::string hashId(const std::string& content, const std::string& prefix) {
        return prefix + toBase62(fnv1a(content));
    }
}

/// Represents a student in the library system
struct Student {
    int64_t id = 0;                          ///< Primary key (0 = not yet persisted)
    std::string name;                         ///< Full name
    std::string registration_number;          ///< Unique registration number (e.g., "251018907")
    std::optional<std::string> email;         ///< Optional email address
    std::optional<std::string> phone;         ///< Optional phone number
    bool active = true;                       ///< Whether the student is currently active

    /// Default constructor
    Student() = default;

    /// Constructor for creating a new student (before DB insertion)
    Student(std::string name_, std::string registration_number_,
            std::optional<std::string> email_ = std::nullopt,
            std::optional<std::string> phone_ = std::nullopt)
        : name(std::move(name_)),
          registration_number(std::move(registration_number_)),
          email(std::move(email_)),
          phone(std::move(phone_)) {}

    /// Validates student data (checks required fields)
    [[nodiscard]] bool isValid() const {
        return !name.empty() && !registration_number.empty();
    }
    
    /// Generate hash-based display ID (e.g., "ST-a3Bf9x")
    [[nodiscard]] std::string hashId() const {
        return hash_utils::hashId(name + "|" + registration_number, "ST-");
    }
};

/// Represents a book in the library catalogue
struct Book {
    int64_t id = 0;
    std::string title;
    std::string author;
    std::optional<int> published_year;
    std::optional<std::string> isbn;
    int copies_available = 1;

    Book() = default;

    /// Constructor for creating a new book (before DB insertion)
    Book(std::string title_, std::string author_,
         std::optional<int> published_year_ = std::nullopt,
         std::optional<std::string> isbn_ = std::nullopt,
         int copies_available_ = 1)
        : title(std::move(title_)),
          author(std::move(author_)),
          published_year(published_year_),
          isbn(std::move(isbn_)),
          copies_available(copies_available_) {}

    /// Validates book data (checks required fields)
    [[nodiscard]] bool isValid() const {
        return !title.empty() && !author.empty() && copies_available >= 0;
    }
    
    /// Generate hash-based display ID (e.g., "BK-x7Kp2m")
    [[nodiscard]] std::string hashId() const {
        return hash_utils::hashId(title + "|" + author, "BK-");
    }
};

/// Represents a loan (book borrowed by a student)
struct Loan {
    int64_t id = 0;
    int64_t student_id = 0;
    int64_t book_id = 0;
    std::string loan_date;        ///< ISO 8601 date string
    std::string due_date;         ///< ISO 8601 date string
    std::optional<std::string> return_date;  ///< Null if not yet returned
    bool is_overdue = false;

    Loan() = default;
    
    /// Generate hash-based display ID (e.g., "LN-m9Tz4a")
    [[nodiscard]] std::string hashId() const {
        return hash_utils::hashId(
            std::to_string(student_id) + "|" + std::to_string(book_id) + "|" + loan_date,
            "LN-"
        );
    }
};

}  // namespace app::models
