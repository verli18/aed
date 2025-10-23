#include "app/repos/student_repository.hpp"

#include <stdexcept>

namespace app::repos {

StudentRepository::StudentRepository(Database& db) : db_(db) {}

int64_t StudentRepository::create(models::Student& student) {
    if (!student.isValid()) {
        throw std::runtime_error("Cannot create student: invalid data (name and registration_number required)");
    }

    // Check for duplicate registration number
    if (registrationNumberExists(student.registration_number)) {
        throw std::runtime_error("Cannot create student: registration number already exists");
    }

    constexpr auto sql = R"sql(
        INSERT INTO students (name, registration_number, email, phone, active)
        VALUES (?, ?, ?, ?, ?)
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, student.name);
    stmt.bind(2, student.registration_number);

    if (student.email.has_value()) {
        stmt.bind(3, *student.email);
    } else {
        stmt.bindNull(3);
    }

    if (student.phone.has_value()) {
        stmt.bind(4, *student.phone);
    } else {
        stmt.bindNull(4);
    }

    stmt.bind(5, student.active ? 1 : 0);

    stmt.step();

    student.id = db_.lastInsertRowId();
    return student.id;
}

std::optional<models::Student> StudentRepository::findById(int64_t id) const {
    constexpr auto sql = R"sql(
        SELECT id, name, registration_number, email, phone, active
        FROM students
        WHERE id = ?
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, id);

    if (stmt.step()) {
        return mapRowToStudent(stmt);
    }

    return std::nullopt;
}

std::optional<models::Student> StudentRepository::findByRegistrationNumber(
    const std::string& registration_number) const {
    constexpr auto sql = R"sql(
        SELECT id, name, registration_number, email, phone, active
        FROM students
        WHERE registration_number = ?
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, registration_number);

    if (stmt.step()) {
        return mapRowToStudent(stmt);
    }

    return std::nullopt;
}

std::vector<models::Student> StudentRepository::findAll(bool activeOnly) const {
    const std::string sql = activeOnly
        ? R"sql(
            SELECT id, name, registration_number, email, phone, active
            FROM students
            WHERE active = 1
            ORDER BY name
          )sql"
        : R"sql(
            SELECT id, name, registration_number, email, phone, active
            FROM students
            ORDER BY name
          )sql";

    Statement stmt(db_.handle(), sql);
    std::vector<models::Student> students;

    while (stmt.step()) {
        students.push_back(mapRowToStudent(stmt));
    }

    return students;
}

bool StudentRepository::update(const models::Student& student) {
    if (student.id == 0) {
        throw std::runtime_error("Cannot update student: ID not set");
    }

    if (!student.isValid()) {
        throw std::runtime_error("Cannot update student: invalid data");
    }

    // Check for duplicate registration number (excluding this student)
    if (registrationNumberExists(student.registration_number, student.id)) {
        throw std::runtime_error("Cannot update student: registration number already exists");
    }

    constexpr auto sql = R"sql(
        UPDATE students
        SET name = ?, registration_number = ?, email = ?, phone = ?, active = ?
        WHERE id = ?
    )sql";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, student.name);
    stmt.bind(2, student.registration_number);

    if (student.email.has_value()) {
        stmt.bind(3, *student.email);
    } else {
        stmt.bindNull(3);
    }

    if (student.phone.has_value()) {
        stmt.bind(4, *student.phone);
    } else {
        stmt.bindNull(4);
    }

    stmt.bind(5, student.active ? 1 : 0);
    stmt.bind(6, student.id);

    stmt.step();

    return db_.changes() > 0;
}

bool StudentRepository::deleteById(int64_t id) {
    constexpr auto sql = "DELETE FROM students WHERE id = ?";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, id);
    stmt.step();

    return db_.changes() > 0;
}

bool StudentRepository::deactivate(int64_t id) {
    constexpr auto sql = "UPDATE students SET active = 0 WHERE id = ?";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, id);
    stmt.step();

    return db_.changes() > 0;
}

bool StudentRepository::activate(int64_t id) {
    constexpr auto sql = "UPDATE students SET active = 1 WHERE id = ?";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, id);
    stmt.step();

    return db_.changes() > 0;
}

bool StudentRepository::registrationNumberExists(
    const std::string& registration_number,
    std::optional<int64_t> excludeId) const {
    const std::string sql = excludeId.has_value()
        ? "SELECT COUNT(*) FROM students WHERE registration_number = ? AND id != ?"
        : "SELECT COUNT(*) FROM students WHERE registration_number = ?";

    Statement stmt(db_.handle(), sql);
    stmt.bind(1, registration_number);

    if (excludeId.has_value()) {
        stmt.bind(2, *excludeId);
    }

    if (stmt.step()) {
        return stmt.getInt64(0) > 0;
    }

    return false;
}

int64_t StudentRepository::count(bool activeOnly) const {
    const std::string sql = activeOnly
        ? "SELECT COUNT(*) FROM students WHERE active = 1"
        : "SELECT COUNT(*) FROM students";

    Statement stmt(db_.handle(), sql);

    if (stmt.step()) {
        return stmt.getInt64(0);
    }

    return 0;
}

models::Student StudentRepository::mapRowToStudent(const Statement& stmt) const {
    models::Student student;
    student.id = stmt.getInt64(0);
    student.name = stmt.getText(1);
    student.registration_number = stmt.getText(2);

    if (!stmt.isNull(3)) {
        student.email = stmt.getText(3);
    }

    if (!stmt.isNull(4)) {
        student.phone = stmt.getText(4);
    }

    student.active = stmt.getInt64(5) != 0;

    return student;
}

}  // namespace app::repos
