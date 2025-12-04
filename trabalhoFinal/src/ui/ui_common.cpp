#include "ui/ui_common.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace ui {

// ==================== STRING UTILITIES ====================

std::string toLowerCopy(const std::string& input) {
    std::string lowered = input;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lowered;
}

bool containsCaseInsensitive(const std::string& text, const std::string& query) {
    if (query.empty()) {
        return true;
    }
    return toLowerCopy(text).find(toLowerCopy(query)) != std::string::npos;
}

// ==================== QUICKSORT IMPLEMENTATION ====================
// Classic QuickSort with Lomuto partition scheme
// Time Complexity: O(n log n) average, O(n²) worst case
// Space Complexity: O(log n) due to recursion stack

static int partitionStrings(std::vector<std::string>& arr, int low, int high) {
    const std::string& pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (arr[j] <= pivot) {
            ++i;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

void quickSortStrings(std::vector<std::string>& arr, int low, int high) {
    if (low < high) {
        int pivotIndex = partitionStrings(arr, low, high);
        if (pivotIndex > 0) {
            quickSortStrings(arr, low, pivotIndex - 1);
        }
        quickSortStrings(arr, pivotIndex + 1, high);
    }
}

// ==================== DATE UTILITIES ====================

std::string formatDateISO(const std::chrono::system_clock::time_point& tp) {
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string getTodayISODate() {
    return formatDateISO(std::chrono::system_clock::now());
}

std::string getFutureISODate(int daysAhead) {
    auto now = std::chrono::system_clock::now();
    auto future = now + std::chrono::hours(24 * daysAhead);
    return formatDateISO(future);
}

// ==================== DATA LOADERS ====================

std::vector<std::string> loadStudentsFromDatabase(app::repos::StudentRepository& studentRepo) {
    std::vector<std::string> studentStrings;
    auto students = studentRepo.findAll(true); // Active only
    
    for (const auto& student : students) {
        // Use hash-based ID for display (e.g., "ST-a3Bf9x")
        std::string entry = "[" + student.hashId() + "] " + student.name + " - " + student.registration_number;
        if (student.email.has_value()) {
            entry += " (" + *student.email + ")";
        }
        studentStrings.push_back(entry);
    }
    
    if (studentStrings.empty()) {
        studentStrings.push_back("Nenhum estudante registrado");
    }
    
    return studentStrings;
}

std::vector<std::string> loadBooksFromDatabase(app::repos::BookRepository& bookRepo) {
    std::vector<std::string> bookStrings;
    auto books = bookRepo.findAll();
    
    for (const auto& book : books) {
        // Use hash-based ID for display (e.g., "BK-x7Kp2m")
        std::string entry = "[" + book.hashId() + "] " + book.title + " - " + book.author;
        entry += " (" + std::to_string(book.copies_available) + " disponíveis)";
        bookStrings.push_back(entry);
    }
    
    if (bookStrings.empty()) {
        bookStrings.push_back("Nenhum livro registrado");
    }
    
    return bookStrings;
}

std::vector<std::string> searchBooksFromDatabase(app::repos::BookRepository& bookRepo, const std::string& query) {
    std::vector<std::string> bookStrings;
    auto books = bookRepo.findAll();
    
    for (const auto& book : books) {
        if (containsCaseInsensitive(book.title, query) ||
            containsCaseInsensitive(book.author, query) ||
            (book.isbn.has_value() && containsCaseInsensitive(*book.isbn, query))) {
            // Use hash-based ID for display
            std::string entry = "[" + book.hashId() + "] " + book.title + " - " + book.author;
            entry += " (" + std::to_string(book.copies_available) + " disponíveis)";
            bookStrings.push_back(entry);
        }
    }

    if (bookStrings.size() > 1) {
        quickSortStrings(bookStrings, 0, static_cast<int>(bookStrings.size()) - 1);
    }

    if (bookStrings.empty()) {
        bookStrings.push_back("Nenhum livro encontrado");
    }

    return bookStrings;
}

std::vector<RichListItem> loadLoansRichFromDatabase(app::repos::LoanRepository& loanRepo) {
    auto loanDetails = loanRepo.findActiveLoanDetails();
    std::vector<RichListItem> loans;

    standardStyle normalTheme = {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};
    standardStyle overdueTheme = {{255, 140, 140, 255}, BACKGROUND, {255, 200, 200, 255}, BACKGROUND};

    for (const auto& detail : loanDetails) {
        std::vector<std::string> lines;
        // Use hash-based IDs for display
        lines.push_back("Empréstimo " + detail.loan.hashId() + " - " + detail.student_name + " (" + detail.student_registration + ")");
        
        // Create a temporary book to generate its hash ID
        app::models::Book tempBook;
        tempBook.title = detail.book_title;
        tempBook.author = detail.book_author;
        lines.push_back("Livro: " + detail.book_title + " - " + detail.book_author + " [" + tempBook.hashId() + "]");
        
        lines.push_back("Retirado em: " + detail.loan.loan_date + " | Devolver até: " + detail.loan.due_date);
        lines.push_back(detail.loan.is_overdue ? "⚠ ATRASADO" : "✓ No prazo");

        loans.emplace_back(lines, detail.loan.is_overdue ? overdueTheme : normalTheme);
    }

    if (loans.empty()) {
        loans.emplace_back(
            std::vector<std::string>{
                "Nenhum empréstimo ativo",
                "Registre livros e estudantes para criar empréstimos"
            },
            normalTheme
        );
    }

    return loans;
}

} // namespace ui
