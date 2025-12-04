#pragma once

#include "app/db.hpp"
#include "app/models.hpp"

#include <optional>
#include <string>
#include <vector>

namespace app::repos {

struct LoanDetails {
	models::Loan loan;
	std::string student_name;
	std::string student_registration;
	std::string book_title;
	std::string book_author;
};

class LoanRepository {
public:
	explicit LoanRepository(Database& db);

	int64_t create(models::Loan& loan);

	bool markReturned(int64_t loanId, const std::string& returnDate);

	[[nodiscard]] std::optional<models::Loan> findById(int64_t id) const;

	/// Find a loan by its hash ID (e.g., "LN-m9Tz4a")
	[[nodiscard]] std::optional<models::Loan> findByHashId(const std::string& hashId) const;

	[[nodiscard]] std::vector<models::Loan> findActiveLoans() const;

	[[nodiscard]] std::vector<models::Loan> findByStudent(int64_t studentId, bool activeOnly = false) const;

	[[nodiscard]] std::vector<models::Loan> findByBook(int64_t bookId, bool activeOnly = false) const;

	[[nodiscard]] std::vector<LoanDetails> findActiveLoanDetails() const;

private:
	Database& db_;

	[[nodiscard]] models::Loan mapRowToLoan(const Statement& stmt) const;
};

}  // namespace app::repos
