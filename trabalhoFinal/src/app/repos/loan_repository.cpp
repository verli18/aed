#include "app/repos/loan_repository.hpp"

#include <stdexcept>

namespace app::repos {

namespace {

std::string defaultTodayDate() {
	// SQLite's date('now') is UTC; we can rely on database default when empty
	// but we want explicit value for consistency. We'll keep empty string meaning use default.
	return "";
}

}

LoanRepository::LoanRepository(Database& db) : db_(db) {}

int64_t LoanRepository::create(models::Loan& loan) {
	if (loan.student_id == 0 || loan.book_id == 0) {
		throw std::runtime_error("Cannot create loan: student_id and book_id are required");
	}

	if (loan.due_date.empty()) {
		throw std::runtime_error("Cannot create loan: due_date is required");
	}

	if (loan.loan_date.empty()) {
		loan.loan_date = defaultTodayDate();
	}

	const char* sql = R"sql(
		INSERT INTO loans (student_id, book_id, loan_date, due_date, return_date)
		VALUES (?, ?, COALESCE(?, date('now')), ?, NULL)
	)sql";

	Statement stmt(db_.handle(), sql);
	stmt.bind(1, loan.student_id);
	stmt.bind(2, loan.book_id);

	if (!loan.loan_date.empty()) {
		stmt.bind(3, loan.loan_date);
	} else {
		stmt.bindNull(3);
	}

	stmt.bind(4, loan.due_date);
	(void)stmt.step();

	loan.id = db_.lastInsertRowId();
	loan.return_date.reset();
	loan.is_overdue = false;
	return loan.id;
}

bool LoanRepository::markReturned(int64_t loanId, const std::string& returnDate) {
	if (loanId == 0) {
		throw std::runtime_error("Cannot return loan: invalid ID");
	}

	const char* sql = R"sql(
		UPDATE loans
		SET return_date = COALESCE(?, date('now'))
		WHERE id = ? AND return_date IS NULL
	)sql";

	Statement stmt(db_.handle(), sql);
	if (!returnDate.empty()) {
		stmt.bind(1, returnDate);
	} else {
		stmt.bindNull(1);
	}
	stmt.bind(2, loanId);
	(void)stmt.step();

	return db_.changes() > 0;
}

std::optional<models::Loan> LoanRepository::findById(int64_t id) const {
	constexpr auto sql = R"sql(
		SELECT id, student_id, book_id, loan_date, due_date, return_date,
			   CASE WHEN return_date IS NULL AND DATE(due_date) < DATE('now') THEN 1 ELSE 0 END AS overdue
		FROM loans
		WHERE id = ?
	)sql";

	Statement stmt(db_.handle(), sql);
	stmt.bind(1, id);

	if (stmt.step()) {
		return mapRowToLoan(stmt);
	}

	return std::nullopt;
}

std::vector<models::Loan> LoanRepository::findActiveLoans() const {
	constexpr auto sql = R"sql(
		SELECT id, student_id, book_id, loan_date, due_date, return_date,
			   CASE WHEN DATE(due_date) < DATE('now') THEN 1 ELSE 0 END AS overdue
		FROM loans
		WHERE return_date IS NULL
		ORDER BY due_date ASC
	)sql";

	Statement stmt(db_.handle(), sql);
	std::vector<models::Loan> loans;
	while (stmt.step()) {
		loans.push_back(mapRowToLoan(stmt));
	}
	return loans;
}

std::vector<models::Loan> LoanRepository::findByStudent(int64_t studentId, bool activeOnly) const {
	const std::string sql = activeOnly
		? R"sql(
			SELECT id, student_id, book_id, loan_date, due_date, return_date,
				   CASE WHEN return_date IS NULL AND DATE(due_date) < DATE('now') THEN 1 ELSE 0 END AS overdue
			FROM loans
			WHERE student_id = ? AND return_date IS NULL
			ORDER BY due_date ASC
		)sql"
		: R"sql(
			SELECT id, student_id, book_id, loan_date, due_date, return_date,
				   CASE WHEN return_date IS NULL AND DATE(due_date) < DATE('now') THEN 1 ELSE 0 END AS overdue
			FROM loans
			WHERE student_id = ?
			ORDER BY loan_date DESC
		)sql";

	Statement stmt(db_.handle(), sql);
	stmt.bind(1, studentId);

	std::vector<models::Loan> loans;
	while (stmt.step()) {
		loans.push_back(mapRowToLoan(stmt));
	}
	return loans;
}

std::vector<models::Loan> LoanRepository::findByBook(int64_t bookId, bool activeOnly) const {
	const std::string sql = activeOnly
		? R"sql(
			SELECT id, student_id, book_id, loan_date, due_date, return_date,
				   CASE WHEN return_date IS NULL AND DATE(due_date) < DATE('now') THEN 1 ELSE 0 END AS overdue
			FROM loans
			WHERE book_id = ? AND return_date IS NULL
			ORDER BY due_date ASC
		)sql"
		: R"sql(
			SELECT id, student_id, book_id, loan_date, due_date, return_date,
				   CASE WHEN return_date IS NULL AND DATE(due_date) < DATE('now') THEN 1 ELSE 0 END AS overdue
			FROM loans
			WHERE book_id = ?
			ORDER BY loan_date DESC
		)sql";

	Statement stmt(db_.handle(), sql);
	stmt.bind(1, bookId);

	std::vector<models::Loan> loans;
	while (stmt.step()) {
		loans.push_back(mapRowToLoan(stmt));
	}
	return loans;
}

std::vector<LoanDetails> LoanRepository::findActiveLoanDetails() const {
	constexpr auto sql = R"sql(
		SELECT l.id, l.student_id, l.book_id, l.loan_date, l.due_date, l.return_date,
			   CASE WHEN DATE(l.due_date) < DATE('now') THEN 1 ELSE 0 END AS overdue,
			   s.name, s.registration_number, b.title, b.author
		FROM loans l
		INNER JOIN students s ON s.id = l.student_id
		INNER JOIN books b ON b.id = l.book_id
		WHERE l.return_date IS NULL
		ORDER BY DATE(l.due_date) ASC, l.id ASC
	)sql";

	Statement stmt(db_.handle(), sql);
	std::vector<LoanDetails> details;
	while (stmt.step()) {
		LoanDetails detail;
		detail.loan = mapRowToLoan(stmt);
		detail.student_name = stmt.getText(7);
		detail.student_registration = stmt.getText(8);
		detail.book_title = stmt.getText(9);
		detail.book_author = stmt.getText(10);
		details.push_back(std::move(detail));
	}
	return details;
}

models::Loan LoanRepository::mapRowToLoan(const Statement& stmt) const {
	models::Loan loan;
	loan.id = stmt.getInt64(0);
	loan.student_id = stmt.getInt64(1);
	loan.book_id = stmt.getInt64(2);
	loan.loan_date = stmt.getText(3);
	loan.due_date = stmt.getText(4);
	if (!stmt.isNull(5)) {
		loan.return_date = stmt.getText(5);
	} else {
		loan.return_date.reset();
	}
	loan.is_overdue = stmt.getInt64(6) != 0;
	return loan;
}

}  // namespace app::repos