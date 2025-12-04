#pragma once

#include "chrmaTUI.hpp"
#include "elements.hpp"
#include "app/db.hpp"
#include "app/models.hpp"
#include "app/repos/student_repository.hpp"
#include "app/repos/book_repository.hpp"
#include "app/repos/loan_repository.hpp"

#include <vector>
#include <string>
#include <optional>
#include <chrono>

namespace ui {

// ==================== COLOR PALETTE ====================
// Teal/cyan dark theme
inline constexpr color TEXT = {4, 99, 116, 255};
inline constexpr color TEXT_HIGHLIGHT = {4, 191, 181, 255};
inline constexpr color TEXT_BACKGROUND = {4, 33, 49, 255};
inline constexpr color TEXT_BACKGROUND_HIGHLIGHT = {4, 99, 116, 255};
inline constexpr color BACKGROUND = {4, 14, 22, 255};
inline constexpr color TRANSPARENT = {0, 0, 0, 0};

// Status colors
inline constexpr color COLOR_SUCCESS = {100, 255, 100, 255};
inline constexpr color COLOR_ERROR = {255, 100, 100, 255};
inline constexpr color COLOR_WARNING = {255, 200, 100, 255};

// Default modal style
inline standardStyle defaultModalStyle() {
    return {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};
}

// ==================== VIEW TYPES ====================
enum class ViewType {
    BOOKS,
    STUDENTS,
    LOANS,
    SEARCH,
    NONE
};

// ==================== MODAL STATE ====================
struct ModalState {
    bool isOpen = false;
    std::string statusMessage = "";
    color statusColor = TEXT;
};

// ==================== HELPER FUNCTIONS ====================

// String utilities
std::string toLowerCopy(const std::string& input);
bool containsCaseInsensitive(const std::string& text, const std::string& query);

// ==================== SORTING ALGORITHMS ====================
// QuickSort for strings - O(n log n) average, O(n²) worst case
// Uses Lomuto partition scheme
void quickSortStrings(std::vector<std::string>& arr, int low, int high);

// MergeSort for strings - O(n log n) guaranteed, stable sort
// Better for nearly sorted data or when stability matters
void mergeSortStrings(std::vector<std::string>& arr, int left, int right);

// ==================== SEARCH ALGORITHMS ====================
// Binary Search - O(log n) search in sorted array
// Precondition: array must be sorted
// Returns index if found, -1 otherwise
int binarySearchString(const std::vector<std::string>& arr, const std::string& target);

// Date formatting
std::string formatDateISO(const std::chrono::system_clock::time_point& tp);
std::string getTodayISODate();
std::string getFutureISODate(int daysAhead);

// ==================== DATA LOADERS ====================
std::vector<std::string> loadStudentsFromDatabase(app::repos::StudentRepository& repo);
std::vector<std::string> loadBooksFromDatabase(app::repos::BookRepository& repo);
std::vector<std::string> searchBooksFromDatabase(app::repos::BookRepository& repo, const std::string& query);
std::vector<RichListItem> loadLoansRichFromDatabase(app::repos::LoanRepository& repo);

// ==================== UI HELPERS ====================

// Helper to set status text with color
inline void setStatusText(Text* statusText, const std::string& msg, color fg) {
    statusText->content = msg;
    statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
}

// Global notification manager for toast-style notifications
// Define this in the main UI file
extern NotificationManager* globalNotifications;

// Helper functions to push notifications from anywhere
inline void notifyInfo(const std::string& msg) {
    if (globalNotifications) globalNotifications->pushInfo(msg);
}
inline void notifySuccess(const std::string& msg) {
    if (globalNotifications) globalNotifications->pushSuccess(msg);
}
inline void notifyWarning(const std::string& msg) {
    if (globalNotifications) globalNotifications->pushWarning(msg);
}
inline void notifyError(const std::string& msg) {
    if (globalNotifications) globalNotifications->pushError(msg);
}

} // namespace ui
