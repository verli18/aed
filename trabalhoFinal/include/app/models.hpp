#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace app::models {

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
};

}  // namespace app::models
