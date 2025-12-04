#pragma once

#include "app/db.hpp"
#include "app/models.hpp"

#include <optional>
#include <string>
#include <vector>

namespace app::repos {

/// Repository for CRUD operations on Student entities
class StudentRepository {
public:
    /// Constructor takes a database connection
    explicit StudentRepository(Database& db);

    /// Create a new student
    /// @param student Student data (id will be ignored and set to the new ID)
    /// @return The ID of the newly created student
    /// @throws std::runtime_error if insertion fails or validation fails
    int64_t create(models::Student& student);

    /// Find a student by ID
    /// @param id Student ID
    /// @return Student if found, std::nullopt otherwise
    [[nodiscard]] std::optional<models::Student> findById(int64_t id) const;

    /// Find a student by registration number
    /// @param registration_number Unique registration number
    /// @return Student if found, std::nullopt otherwise
    [[nodiscard]] std::optional<models::Student> findByRegistrationNumber(
        const std::string& registration_number) const;

    /// Find a student by its hash ID (e.g., "ST-abc123")
    /// @param hashId The hash-based ID displayed to users
    /// @return Student if found, std::nullopt otherwise
    [[nodiscard]] std::optional<models::Student> findByHashId(const std::string& hashId) const;

    /// Get all students
    /// @param activeOnly If true, only return active students
    /// @return Vector of all students
    [[nodiscard]] std::vector<models::Student> findAll(bool activeOnly = false) const;

    /// Update an existing student
    /// @param student Student with updated data (id must be set)
    /// @return true if the student was updated, false if not found
    /// @throws std::runtime_error if update fails or validation fails
    bool update(const models::Student& student);

    /// Delete a student by ID (hard delete)
    /// @param id Student ID
    /// @return true if deleted, false if not found
    bool deleteById(int64_t id);

    /// Soft-delete a student (mark as inactive)
    /// @param id Student ID
    /// @return true if deactivated, false if not found
    bool deactivate(int64_t id);

    /// Reactivate a student
    /// @param id Student ID
    /// @return true if reactivated, false if not found
    bool activate(int64_t id);

    /// Check if a registration number already exists
    /// @param registration_number Registration number to check
    /// @param excludeId Optional ID to exclude from the check (for updates)
    /// @return true if the registration number exists
    [[nodiscard]] bool registrationNumberExists(
        const std::string& registration_number, 
        std::optional<int64_t> excludeId = std::nullopt) const;

    /// Count total students
    /// @param activeOnly If true, only count active students
    /// @return Number of students
    [[nodiscard]] int64_t count(bool activeOnly = false) const;

private:
    Database& db_;

    /// Helper to map a statement row to a Student object
    [[nodiscard]] models::Student mapRowToStudent(const Statement& stmt) const;
};

}  // namespace app::repos
