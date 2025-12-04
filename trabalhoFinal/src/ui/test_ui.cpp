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
#include <iomanip>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <cctype>

#define TEXT {4, 99, 116, 255}
#define TEXT_HIGHLIGHT {4, 191, 181, 255}
#define TEXT_BACKGROUND {4, 33, 49, 255}
#define TEXT_BACKGROUND_HIGHLIGHT {4, 99, 116, 255}
#define BACKGROUND {4, 14, 22, 255}
#define TRANSPARENT {0, 0, 0, 0}

namespace ui {

enum class ViewType {
    BOOKS,
    STUDENTS,
    LOANS,
    SEARCH,
    NONE
};

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
    std::string loweredText = toLowerCopy(text);
    std::string loweredQuery = toLowerCopy(query);
    return loweredText.find(loweredQuery) != std::string::npos;
}

int partitionStrings(std::vector<std::string>& arr, int low, int high) {
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

// Load students from database
std::vector<std::string> loadStudentsFromDatabase(app::repos::StudentRepository& studentRepo) {
    std::vector<std::string> studentStrings;
    auto students = studentRepo.findAll(true); // Active only
    
    for (const auto& student : students) {
        std::string entry = student.name + " - " + student.registration_number;
        if (student.email.has_value()) {
            entry += " (" + *student.email + ")";
        }
        studentStrings.push_back(entry);
    }
    
    if (studentStrings.empty()) {
        studentStrings.push_back("No students registered yet");
    }
    
    return studentStrings;
}

// Load books from database
std::vector<std::string> loadBooksFromDatabase(app::repos::BookRepository& bookRepo) {
    std::vector<std::string> bookStrings;
    auto books = bookRepo.findAll();
    
    for (const auto& book : books) {
        std::string entry = book.title + " - " + book.author;
        entry += " (" + std::to_string(book.copies_available) + " available)";
        bookStrings.push_back(entry);
    }
    
    if (bookStrings.empty()) {
        bookStrings.push_back("No books registered yet");
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
            std::string entry = book.title + " - " + book.author;
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
        lines.push_back("Empréstimo #" + std::to_string(detail.loan.id) + " - " + detail.student_name + " (" + detail.student_registration + ")");
        lines.push_back("Livro: " + detail.book_title + " - " + detail.book_author + " [ID " + std::to_string(detail.loan.book_id) + "]");
        lines.push_back("Retirar em: " + detail.loan.loan_date + "; Devolver até: " + detail.loan.due_date);
        if (detail.loan.is_overdue) {
            lines.push_back("Status: ATRASADO");
        } else {
            lines.push_back("Status: No prazo");
        }

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


// Modal state management
struct ModalState {
    bool isOpen = false;
    std::string statusMessage = "";
    color statusColor = TEXT;
};

// Student registration modal
class StudentRegistrationModal {
public:
    container* modalContainer;
    InputBar* nameInput;
    InputBar* regNumberInput;
    InputBar* emailInput;
    InputBar* phoneInput;
    Button* submitBtn;
    Button* cancelBtn;
    Text* statusText;
    Text* titleText;
    
    ModalState state;
    app::repos::StudentRepository* studentRepo; // Repository reference
    
    StudentRegistrationModal(TUImanager& tui, app::repos::StudentRepository* repo) 
        : studentRepo(repo) {
        
        int modalW = 50;
        int modalH = 20;
        int modalX = tui.cols / 2 - modalW / 2;
        int modalY = tui.rows / 2 - modalH / 2;
        
        // Create modal container with higher Z-index
        standardStyle modalStyle = {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};
        modalContainer = new container({modalX, modalY}, {modalW, modalH}, modalStyle, "Register Student");
        modalContainer->setZIndex(100);
        modalContainer->setDefaultElementStyle(modalStyle);
        modalContainer->setInheritStyle(true);
        
        // Title/instructions
        titleText = new Text("Preencha as informações do estudante:", {0, 0});
        titleText->setPercentPosition(5, 5);
        
        // Input fields
        nameInput = new InputBar("Nome", {0, 0}, modalW - 8, 3);
        nameInput->setPercentPosition(10, 15);
        nameInput->setPercentW(80);
        
        regNumberInput = new InputBar("Matrícula", {0, 0}, modalW - 8, 3);
        regNumberInput->setPercentPosition(10, 30);
        regNumberInput->setPercentW(80);
        
        emailInput = new InputBar("Email (opcional)", {0, 0}, modalW - 8, 3);
        emailInput->setPercentPosition(10, 45);
        emailInput->setPercentW(80);
        
        phoneInput = new InputBar("Telefone (opcional)", {0, 0}, modalW - 8, 3);
        phoneInput->setPercentPosition(10, 60);
        phoneInput->setPercentW(80);
        
        // Buttons
        submitBtn = new Button("Confirmar", {0, 0}, 12, 3);
        submitBtn->setPercentPosition(20, 78);

        cancelBtn = new Button("Cancelar", {0, 0}, 12, 3);
        cancelBtn->setPercentPosition(60, 78);
        
        // Status message text
        statusText = new Text("", {0, 0});
        statusText->setPercentPosition(5, 90);
        
        // Add elements to modal
        modalContainer->addElement(titleText);
        modalContainer->addElement(nameInput);
        modalContainer->addElement(regNumberInput);
        modalContainer->addElement(emailInput);
        modalContainer->addElement(phoneInput);
        modalContainer->addElement(submitBtn);
        modalContainer->addElement(cancelBtn);
        modalContainer->addElement(statusText);
        
        // Setup callbacks
        submitBtn->onClickHandler = [this](element&, TUImanager& tui) {
            this->handleSubmit(tui);
        };
        
        cancelBtn->onClickHandler = [this](element&, TUImanager& tui) {
            this->handleCancel(tui);
        };
    }
    
    ~StudentRegistrationModal() {
        delete titleText;
        delete nameInput;
        delete regNumberInput;
        delete emailInput;
        delete phoneInput;
        delete submitBtn;
        delete cancelBtn;
        delete statusText;
        delete modalContainer;
    }
    
    void open(TUImanager& tui, container* returnTo) {
        state.isOpen = true;
        state.statusMessage = "";
        
        // Clear all inputs
        nameInput->text.clear();
        regNumberInput->text.clear();
        emailInput->text.clear();
        phoneInput->text.clear();
        statusText->content = "";
        
        // Focus the modal
        tui.focusContainer(modalContainer, 0);
    }
    
    void close(TUImanager& tui, container* returnTo) {
        state.isOpen = false;
        tui.focusContainer(returnTo, 0);
    }
    
    void handleSubmit(TUImanager& tui) {
        // Validate inputs
        if (nameInput->text.empty()) {
            statusText->content = "Erro: Nome é obrigatório";
            statusText->setStyle({
                {255, 100, 100, 255}, // red fg
                TRANSPARENT,
                {255, 100, 100, 255},
                TRANSPARENT
            });
            return;
        }
        
        if (regNumberInput->text.empty()) {
            statusText->content = "Erro: Matrícula é obrigatória";
            statusText->setStyle({
                {255, 100, 100, 255},
                TRANSPARENT,
                {255, 100, 100, 255},
                TRANSPARENT
            });
            return;
        }
        
        // Try to save to database
        try {
            // Create student model
            std::optional<std::string> email = emailInput->text.empty() 
                ? std::nullopt 
                : std::optional<std::string>(emailInput->text);
            std::optional<std::string> phone = phoneInput->text.empty() 
                ? std::nullopt 
                : std::optional<std::string>(phoneInput->text);
            
            app::models::Student student(nameInput->text, regNumberInput->text, email, phone);
            
            // Save to database
            int64_t newId = studentRepo->create(student);
            
            // Show success message
            statusText->content = "Estudante registrado com matrícula: " + std::to_string(newId);
            statusText->setStyle({
                {100, 255, 100, 255}, // green fg
                TRANSPARENT,
                {100, 255, 100, 255},
                TRANSPARENT
            });
            
        } catch (const std::exception& ex) {
            // Show error message
            statusText->content = "Erro: " + std::string(ex.what());
            statusText->setStyle({
                {255, 100, 100, 255},
                TRANSPARENT,
                {255, 100, 100, 255},
                TRANSPARENT
            });
        }
    }
    
    void handleCancel(TUImanager& tui) {
        // Just close without saving
        state.isOpen = false;
    }
    
    void render(TUImanager& tui) {
        if (state.isOpen) {
            modalContainer->render(tui);
        }
    }
};

// Book registration modal
class BookRegistrationModal {
public:
    container* modalContainer;
    InputBar* titleInput;
    InputBar* authorInput;
    InputBar* yearInput;
    InputBar* isbnInput;
    InputBar* copiesInput;
    Button* submitBtn;
    Button* cancelBtn;
    Text* statusText;
    Text* titleText;
    
    ModalState state;
    app::repos::BookRepository* bookRepo;
    
    BookRegistrationModal(TUImanager& tui, app::repos::BookRepository* repo) 
        : bookRepo(repo) {
        
        int modalW = 50;
        int modalH = 24;
        int modalX = tui.cols / 2 - modalW / 2;
        int modalY = tui.rows / 2 - modalH / 2;
        
        standardStyle modalStyle = {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};
        modalContainer = new container({modalX, modalY}, {modalW, modalH}, modalStyle, "Registrar Livro");
        modalContainer->setZIndex(100);
        modalContainer->setDefaultElementStyle(modalStyle);
        modalContainer->setInheritStyle(true);
        
        titleText = new Text("Preencha as informações do livro:", {0, 0});
        titleText->setPercentPosition(5, 3);
        
        titleInput = new InputBar("Título", {0, 0}, modalW - 8, 3);
        titleInput->setPercentPosition(10, 12);
        titleInput->setPercentW(80);
        
        authorInput = new InputBar("Autor", {0, 0}, modalW - 8, 3);
        authorInput->setPercentPosition(10, 24);
        authorInput->setPercentW(80);
        
        yearInput = new InputBar("Ano (opcional)", {0, 0}, modalW - 8, 3);
        yearInput->setPercentPosition(10, 36);
        yearInput->setPercentW(80);
        
        isbnInput = new InputBar("ISBN (opcional)", {0, 0}, modalW - 8, 3);
        isbnInput->setPercentPosition(10, 48);
        isbnInput->setPercentW(80);
        
        copiesInput = new InputBar("Cópias Disponíveis", {0, 0}, modalW - 8, 3);
        copiesInput->setPercentPosition(10, 60);
        copiesInput->setPercentW(80);
        copiesInput->text = "1"; // Default to 1 copy
        
        submitBtn = new Button("Confirmar", {0, 0}, 12, 3);
        submitBtn->setPercentPosition(20, 75);

        cancelBtn = new Button("Cancelar", {0, 0}, 12, 3);
        cancelBtn->setPercentPosition(60, 75);
        
        statusText = new Text("", {0, 0});
        statusText->setPercentPosition(5, 88);
        
        modalContainer->addElement(titleText);
        modalContainer->addElement(titleInput);
        modalContainer->addElement(authorInput);
        modalContainer->addElement(yearInput);
        modalContainer->addElement(isbnInput);
        modalContainer->addElement(copiesInput);
        modalContainer->addElement(submitBtn);
        modalContainer->addElement(cancelBtn);
        modalContainer->addElement(statusText);
        
        submitBtn->onClickHandler = [this](element&, TUImanager& tui) {
            this->handleSubmit(tui);
        };
        
        cancelBtn->onClickHandler = [this](element&, TUImanager& tui) {
            this->handleCancel(tui);
        };
    }
    
    ~BookRegistrationModal() {
        delete titleText;
        delete titleInput;
        delete authorInput;
        delete yearInput;
        delete isbnInput;
        delete copiesInput;
        delete submitBtn;
        delete cancelBtn;
        delete statusText;
        delete modalContainer;
    }
    
    void open(TUImanager& tui, container*) {
        state.isOpen = true;
        state.statusMessage = "";
        
        titleInput->text.clear();
        authorInput->text.clear();
        yearInput->text.clear();
        isbnInput->text.clear();
        copiesInput->text = "1";
        statusText->content = "";
        
        tui.focusContainer(modalContainer, 0);
    }
    
    void close(TUImanager& tui, container* returnTo) {
        state.isOpen = false;
        tui.focusContainer(returnTo, 0);
    }
    
    void handleSubmit(TUImanager&) {
        // Validate inputs
        if (titleInput->text.empty()) {
            statusText->content = "Erro: Título é obrigatório";
            statusText->setStyle({{255, 100, 100, 255}, TRANSPARENT, {255, 100, 100, 255}, TRANSPARENT});
            return;
        }
        
        if (authorInput->text.empty()) {
            statusText->content = "Erro: Autor é obrigatório";
            statusText->setStyle({{255, 100, 100, 255}, TRANSPARENT, {255, 100, 100, 255}, TRANSPARENT});
            return;
        }
        
        // Parse copies
        int copies = 1;
        try {
            if (!copiesInput->text.empty()) {
                copies = std::stoi(copiesInput->text);
                if (copies < 0) {
                    statusText->content = "Erro: Cópias devem ser >= 0";
                    statusText->setStyle({{255, 100, 100, 255}, TRANSPARENT, {255, 100, 100, 255}, TRANSPARENT});
                    return;
                }
            }
        } catch (...) {
            statusText->content = "Erro: Número inválido para cópias";
            statusText->setStyle({{255, 100, 100, 255}, TRANSPARENT, {255, 100, 100, 255}, TRANSPARENT});
            return;
        }
        
        try {
            // Parse optional year
            std::optional<int> year;
            if (!yearInput->text.empty()) {
                try {
                    year = std::stoi(yearInput->text);
                } catch (...) {
                    statusText->content = "Erro: Formato de ano inválido";
                    statusText->setStyle({{255, 100, 100, 255}, TRANSPARENT, {255, 100, 100, 255}, TRANSPARENT});
                    return;
                }
            }
            
            std::optional<std::string> isbn = isbnInput->text.empty() 
                ? std::nullopt 
                : std::optional<std::string>(isbnInput->text);
            
            app::models::Book book(titleInput->text, authorInput->text, year, isbn, copies);
            
            int64_t newId = bookRepo->create(book);

            statusText->content = "Livro registrado com ID: " + std::to_string(newId);
            statusText->setStyle({{100, 255, 100, 255}, TRANSPARENT, {100, 255, 100, 255}, TRANSPARENT});
            
        } catch (const std::exception& ex) {
            statusText->content = "Erro: " + std::string(ex.what());
            statusText->setStyle({{255, 100, 100, 255}, TRANSPARENT, {255, 100, 100, 255}, TRANSPARENT});
        }
    }
    
    void handleCancel(TUImanager&) {
        state.isOpen = false;
    }
    
    void render(TUImanager& tui) {
        if (state.isOpen) {
            modalContainer->render(tui);
        }
    }
};

class LoanCreationModal {
public:
    container* modalContainer;
    InputBar* studentRegInput;
    InputBar* bookIdInput;
    InputBar* dueDaysInput;
    Button* submitBtn;
    Button* cancelBtn;
    Text* statusText;
    Text* titleText;

    ModalState state;
    app::repos::StudentRepository* studentRepo;
    app::repos::BookRepository* bookRepo;
    app::repos::LoanRepository* loanRepo;

    LoanCreationModal(TUImanager& tui,
                      app::repos::StudentRepository* sRepo,
                      app::repos::BookRepository* bRepo,
                      app::repos::LoanRepository* lRepo)
        : studentRepo(sRepo), bookRepo(bRepo), loanRepo(lRepo) {

        int modalW = 60;
        int modalH = 22;
        int modalX = tui.cols / 2 - modalW / 2;
        int modalY = tui.rows / 2 - modalH / 2;
        standardStyle modalStyle = {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};

        modalContainer = new container({modalX, modalY}, {modalW, modalH}, modalStyle, "Novo Empréstimo");
        modalContainer->setZIndex(110);
        modalContainer->setDefaultElementStyle(modalStyle);
        modalContainer->setInheritStyle(true);

        titleText = new Text("Informe matrícula e ID do livro:", {0, 0});
        titleText->setPercentPosition(5, 5);

        studentRegInput = new InputBar("Matrícula do estudante", {0, 0}, modalW - 8, 3);
        studentRegInput->setPercentPosition(10, 18);
        studentRegInput->setPercentW(80);

        bookIdInput = new InputBar("ID do livro", {0, 0}, modalW - 8, 3);
        bookIdInput->setPercentPosition(10, 35);
        bookIdInput->setPercentW(80);

        dueDaysInput = new InputBar("Dias para devolução", {0, 0}, modalW - 8, 3);
        dueDaysInput->setPercentPosition(10, 52);
        dueDaysInput->setPercentW(80);
        dueDaysInput->text = "14";

        submitBtn = new Button("Confirmar", {0, 0}, 14, 3);
        submitBtn->setPercentPosition(20, 73);

        cancelBtn = new Button("Cancelar", {0, 0}, 14, 3);
        cancelBtn->setPercentPosition(60, 73);

        statusText = new Text("", {0, 0});
        statusText->setPercentPosition(5, 88);

        modalContainer->addElement(titleText);
        modalContainer->addElement(studentRegInput);
        modalContainer->addElement(bookIdInput);
        modalContainer->addElement(dueDaysInput);
        modalContainer->addElement(submitBtn);
        modalContainer->addElement(cancelBtn);
        modalContainer->addElement(statusText);
    }

    ~LoanCreationModal() {
        delete titleText;
        delete studentRegInput;
        delete bookIdInput;
        delete dueDaysInput;
        delete submitBtn;
        delete cancelBtn;
        delete statusText;
        delete modalContainer;
    }

    void open(TUImanager& tui, container* /*returnTo*/) {
        state.isOpen = true;
        statusText->content.clear();
        studentRegInput->text.clear();
        bookIdInput->text.clear();
        dueDaysInput->text = "14";
        tui.focusContainer(modalContainer, 0);
    }

    void close(TUImanager& tui, container* returnTo) {
        state.isOpen = false;
        tui.focusContainer(returnTo, 0);
    }

    bool handleSubmit(TUImanager&) {
        auto setStatus = [this](const std::string& msg, color fg) {
            statusText->content = msg;
            statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
        };

        if (studentRegInput->text.empty()) {
            setStatus("Erro: Matrícula obrigatória", {255, 100, 100, 255});
            return false;
        }
        if (bookIdInput->text.empty()) {
            setStatus("Erro: ID do livro obrigatório", {255, 100, 100, 255});
            return false;
        }

        auto studentOpt = studentRepo->findByRegistrationNumber(studentRegInput->text);
        if (!studentOpt) {
            setStatus("Erro: Estudante não encontrado", {255, 100, 100, 255});
            return false;
        }

        int64_t bookId = 0;
        try {
            bookId = std::stoll(bookIdInput->text);
        } catch (...) {
            setStatus("Erro: ID do livro inválido", {255, 100, 100, 255});
            return false;
        }

        auto bookOpt = bookRepo->findById(bookId);
        if (!bookOpt) {
            setStatus("Erro: Livro não encontrado", {255, 100, 100, 255});
            return false;
        }
        if (bookOpt->copies_available <= 0) {
            setStatus("Erro: Livro sem cópias disponíveis", {255, 100, 100, 255});
            return false;
        }

        int dueDays = 14;
        try {
            if (!dueDaysInput->text.empty()) {
                dueDays = std::stoi(dueDaysInput->text);
            }
            if (dueDays <= 0) {
                throw std::runtime_error("invalid");
            }
        } catch (...) {
            setStatus("Erro: Dias para devolução inválido", {255, 100, 100, 255});
            return false;
        }

        try {
            app::models::Loan loan;
            loan.student_id = studentOpt->id;
            loan.book_id = bookOpt->id;
            loan.loan_date = getTodayISODate();
            loan.due_date = getFutureISODate(dueDays);
            loanRepo->create(loan);
            if (!bookRepo->decrementCopies(bookOpt->id)) {
                throw std::runtime_error("Não foi possível atualizar as cópias disponíveis");
            }

            setStatus("Empréstimo criado para " + studentOpt->name, {100, 255, 100, 255});
            return true;
        } catch (const std::exception& ex) {
            setStatus(std::string("Erro: ") + ex.what(), {255, 100, 100, 255});
            return false;
        }
    }

    void render(TUImanager& tui) {
        if (state.isOpen) {
            modalContainer->render(tui);
        }
    }
};

class LoanReturnModal {
public:
    container* modalContainer;
    InputBar* loanIdInput;
    Button* submitBtn;
    Button* cancelBtn;
    Text* statusText;
    Text* titleText;

    ModalState state;
    app::repos::LoanRepository* loanRepo;
    app::repos::BookRepository* bookRepo;

    LoanReturnModal(TUImanager& tui,
                    app::repos::LoanRepository* lRepo,
                    app::repos::BookRepository* bRepo)
        : loanRepo(lRepo), bookRepo(bRepo) {
        int modalW = 50;
        int modalH = 16;
        int modalX = tui.cols / 2 - modalW / 2;
        int modalY = tui.rows / 2 - modalH / 2;

        standardStyle modalStyle = {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};
        modalContainer = new container({modalX, modalY}, {modalW, modalH}, modalStyle, "Devolver Livro");
        modalContainer->setZIndex(110);
        modalContainer->setDefaultElementStyle(modalStyle);
        modalContainer->setInheritStyle(true);

        titleText = new Text("Informe o ID do empréstimo ativo:", {0, 0});
        titleText->setPercentPosition(5, 10);

        loanIdInput = new InputBar("ID do empréstimo", {0, 0}, modalW - 8, 3);
        loanIdInput->setPercentPosition(10, 35);
        loanIdInput->setPercentW(80);

        submitBtn = new Button("Confirmar", {0, 0}, 14, 3);
        submitBtn->setPercentPosition(20, 65);

        cancelBtn = new Button("Cancelar", {0, 0}, 14, 3);
        cancelBtn->setPercentPosition(60, 65);

        statusText = new Text("", {0, 0});
        statusText->setPercentPosition(5, 85);

        modalContainer->addElement(titleText);
        modalContainer->addElement(loanIdInput);
        modalContainer->addElement(submitBtn);
        modalContainer->addElement(cancelBtn);
        modalContainer->addElement(statusText);
    }

    ~LoanReturnModal() {
        delete titleText;
        delete loanIdInput;
        delete submitBtn;
        delete cancelBtn;
        delete statusText;
        delete modalContainer;
    }

    void open(TUImanager& tui, container* /*returnTo*/) {
        state.isOpen = true;
        statusText->content.clear();
        loanIdInput->text.clear();
        tui.focusContainer(modalContainer, 0);
    }

    void close(TUImanager& tui, container* returnTo) {
        state.isOpen = false;
        tui.focusContainer(returnTo, 0);
    }

    bool handleSubmit(TUImanager&) {
        auto setStatus = [this](const std::string& msg, color fg) {
            statusText->content = msg;
            statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
        };

        if (loanIdInput->text.empty()) {
            setStatus("Erro: ID é obrigatório", {255, 100, 100, 255});
            return false;
        }

        int64_t loanId = 0;
        try {
            loanId = std::stoll(loanIdInput->text);
        } catch (...) {
            setStatus("Erro: ID inválido", {255, 100, 100, 255});
            return false;
        }

        auto loanOpt = loanRepo->findById(loanId);
        if (!loanOpt) {
            setStatus("Erro: Empréstimo não encontrado", {255, 100, 100, 255});
            return false;
        }
        if (loanOpt->return_date.has_value()) {
            setStatus("Erro: Empréstimo já devolvido", {255, 100, 100, 255});
            return false;
        }

        try {
            if (!loanRepo->markReturned(loanId, getTodayISODate())) {
                setStatus("Erro: Empréstimo já devolvido", {255, 100, 100, 255});
                return false;
            }
            if (!bookRepo->incrementCopies(loanOpt->book_id)) {
                throw std::runtime_error("Falha ao atualizar cópias do livro");
            }
            setStatus("Livro devolvido com sucesso", {100, 255, 100, 255});
            return true;
        } catch (const std::exception& ex) {
            setStatus(std::string("Erro: ") + ex.what(), {255, 100, 100, 255});
            return false;
        }
    }

    void render(TUImanager& tui) {
        if (state.isOpen) {
            modalContainer->render(tui);
        }
    }
};

// ==================== EDIT MODALS ====================

class BookEditModal {
public:
    container* modalContainer;
    InputBar* idInput;
    InputBar* titleInput;
    InputBar* authorInput;
    InputBar* yearInput;
    InputBar* isbnInput;
    InputBar* copiesInput;
    Button* loadBtn;
    Button* saveBtn;
    Button* deleteBtn;
    Button* cancelBtn;
    Text* statusText;
    Text* titleText;
    
    ModalState state;
    app::repos::BookRepository* bookRepo;
    int64_t currentBookId = 0;
    
    BookEditModal(TUImanager& tui, app::repos::BookRepository* repo) 
        : bookRepo(repo) {
        
        int modalW = 60;
        int modalH = 30;
        int modalX = tui.cols / 2 - modalW / 2;
        int modalY = tui.rows / 2 - modalH / 2;
        
        standardStyle modalStyle = {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};
        modalContainer = new container({modalX, modalY}, {modalW, modalH}, modalStyle, "Editar/Excluir Livro");
        modalContainer->setZIndex(100);
        modalContainer->setDefaultElementStyle(modalStyle);
        modalContainer->setInheritStyle(true);
        
        titleText = new Text("Digite o ID do livro para carregar:", {0, 0});
        titleText->setPercentPosition(5, 3);
        
        idInput = new InputBar("ID do Livro", {0, 0}, modalW - 8, 3);
        idInput->setPercentPosition(10, 10);
        idInput->setPercentW(50);
        
        loadBtn = new Button("Carregar", {0, 0}, 12, 3);
        loadBtn->setPercentPosition(65, 10);
        
        titleInput = new InputBar("Título", {0, 0}, modalW - 8, 3);
        titleInput->setPercentPosition(10, 22);
        titleInput->setPercentW(80);
        
        authorInput = new InputBar("Autor", {0, 0}, modalW - 8, 3);
        authorInput->setPercentPosition(10, 32);
        authorInput->setPercentW(80);
        
        yearInput = new InputBar("Ano", {0, 0}, modalW - 8, 3);
        yearInput->setPercentPosition(10, 42);
        yearInput->setPercentW(35);
        
        isbnInput = new InputBar("ISBN", {0, 0}, modalW - 8, 3);
        isbnInput->setPercentPosition(55, 42);
        isbnInput->setPercentW(35);
        
        copiesInput = new InputBar("Cópias", {0, 0}, modalW - 8, 3);
        copiesInput->setPercentPosition(10, 52);
        copiesInput->setPercentW(30);
        
        saveBtn = new Button("Salvar", {0, 0}, 12, 3);
        saveBtn->setPercentPosition(10, 65);
        
        deleteBtn = new Button("Excluir", {0, 0}, 12, 3);
        deleteBtn->setPercentPosition(40, 65);
        deleteBtn->setStyle({{255, 100, 100, 255}, BACKGROUND, {255, 150, 150, 255}, BACKGROUND});

        cancelBtn = new Button("Cancelar", {0, 0}, 12, 3);
        cancelBtn->setPercentPosition(70, 65);
        
        statusText = new Text("", {0, 0});
        statusText->setPercentPosition(5, 80);
        
        modalContainer->addElement(titleText);
        modalContainer->addElement(idInput);
        modalContainer->addElement(loadBtn);
        modalContainer->addElement(titleInput);
        modalContainer->addElement(authorInput);
        modalContainer->addElement(yearInput);
        modalContainer->addElement(isbnInput);
        modalContainer->addElement(copiesInput);
        modalContainer->addElement(saveBtn);
        modalContainer->addElement(deleteBtn);
        modalContainer->addElement(cancelBtn);
        modalContainer->addElement(statusText);
        
        loadBtn->onClickHandler = [this](element&, TUImanager&) {
            this->handleLoad();
        };
        
        saveBtn->onClickHandler = [this](element&, TUImanager&) {
            this->handleSave();
        };
        
        deleteBtn->onClickHandler = [this](element&, TUImanager&) {
            this->handleDelete();
        };
        
        cancelBtn->onClickHandler = [this](element&, TUImanager&) {
            this->state.isOpen = false;
        };
    }
    
    ~BookEditModal() {
        delete titleText;
        delete idInput;
        delete loadBtn;
        delete titleInput;
        delete authorInput;
        delete yearInput;
        delete isbnInput;
        delete copiesInput;
        delete saveBtn;
        delete deleteBtn;
        delete cancelBtn;
        delete statusText;
        delete modalContainer;
    }
    
    void open(TUImanager& tui, container*) {
        state.isOpen = true;
        clearFields();
        tui.focusContainer(modalContainer, 0);
    }
    
    void close(TUImanager& tui, container* returnTo) {
        state.isOpen = false;
        tui.focusContainer(returnTo, 0);
    }
    
    void clearFields() {
        idInput->text.clear();
        titleInput->text.clear();
        authorInput->text.clear();
        yearInput->text.clear();
        isbnInput->text.clear();
        copiesInput->text.clear();
        statusText->content.clear();
        currentBookId = 0;
    }
    
    void handleLoad() {
        auto setStatus = [this](const std::string& msg, color fg) {
            statusText->content = msg;
            statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
        };
        
        if (idInput->text.empty()) {
            setStatus("Erro: Digite um ID", {255, 100, 100, 255});
            return;
        }
        
        int64_t id = 0;
        try {
            id = std::stoll(idInput->text);
        } catch (...) {
            setStatus("Erro: ID inválido", {255, 100, 100, 255});
            return;
        }
        
        auto bookOpt = bookRepo->findById(id);
        if (!bookOpt) {
            setStatus("Erro: Livro não encontrado", {255, 100, 100, 255});
            return;
        }
        
        currentBookId = id;
        titleInput->text = bookOpt->title;
        authorInput->text = bookOpt->author;
        yearInput->text = bookOpt->published_year ? std::to_string(*bookOpt->published_year) : "";
        isbnInput->text = bookOpt->isbn.value_or("");
        copiesInput->text = std::to_string(bookOpt->copies_available);
        setStatus("Livro carregado: " + bookOpt->title, {100, 255, 100, 255});
    }
    
    void handleSave() {
        auto setStatus = [this](const std::string& msg, color fg) {
            statusText->content = msg;
            statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
        };
        
        if (currentBookId == 0) {
            setStatus("Erro: Carregue um livro primeiro", {255, 100, 100, 255});
            return;
        }
        
        if (titleInput->text.empty() || authorInput->text.empty()) {
            setStatus("Erro: Título e autor são obrigatórios", {255, 100, 100, 255});
            return;
        }
        
        try {
            app::models::Book book;
            book.id = currentBookId;
            book.title = titleInput->text;
            book.author = authorInput->text;
            book.published_year = yearInput->text.empty() ? std::nullopt : std::optional<int>(std::stoi(yearInput->text));
            book.isbn = isbnInput->text.empty() ? std::nullopt : std::optional<std::string>(isbnInput->text);
            book.copies_available = copiesInput->text.empty() ? 1 : std::stoi(copiesInput->text);
            
            if (bookRepo->update(book)) {
                setStatus("Livro atualizado com sucesso!", {100, 255, 100, 255});
            } else {
                setStatus("Erro: Falha ao atualizar", {255, 100, 100, 255});
            }
        } catch (const std::exception& ex) {
            setStatus("Erro: " + std::string(ex.what()), {255, 100, 100, 255});
        }
    }
    
    void handleDelete() {
        auto setStatus = [this](const std::string& msg, color fg) {
            statusText->content = msg;
            statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
        };
        
        if (currentBookId == 0) {
            setStatus("Erro: Carregue um livro primeiro", {255, 100, 100, 255});
            return;
        }
        
        try {
            if (bookRepo->deleteById(currentBookId)) {
                setStatus("Livro excluído com sucesso!", {100, 255, 100, 255});
                clearFields();
            } else {
                setStatus("Erro: Falha ao excluir", {255, 100, 100, 255});
            }
        } catch (const std::exception& ex) {
            setStatus("Erro: " + std::string(ex.what()), {255, 100, 100, 255});
        }
    }
    
    void render(TUImanager& tui) {
        if (state.isOpen) {
            modalContainer->render(tui);
        }
    }
};

class StudentEditModal {
public:
    container* modalContainer;
    InputBar* idInput;
    InputBar* nameInput;
    InputBar* regNumberInput;
    InputBar* emailInput;
    InputBar* phoneInput;
    Button* loadBtn;
    Button* saveBtn;
    Button* deleteBtn;
    Button* cancelBtn;
    Text* statusText;
    Text* titleText;
    
    ModalState state;
    app::repos::StudentRepository* studentRepo;
    int64_t currentStudentId = 0;
    
    StudentEditModal(TUImanager& tui, app::repos::StudentRepository* repo) 
        : studentRepo(repo) {
        
        int modalW = 60;
        int modalH = 28;
        int modalX = tui.cols / 2 - modalW / 2;
        int modalY = tui.rows / 2 - modalH / 2;
        
        standardStyle modalStyle = {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};
        modalContainer = new container({modalX, modalY}, {modalW, modalH}, modalStyle, "Editar/Excluir Estudante");
        modalContainer->setZIndex(100);
        modalContainer->setDefaultElementStyle(modalStyle);
        modalContainer->setInheritStyle(true);
        
        titleText = new Text("Digite a matrícula para carregar:", {0, 0});
        titleText->setPercentPosition(5, 3);
        
        idInput = new InputBar("Matrícula", {0, 0}, modalW - 8, 3);
        idInput->setPercentPosition(10, 12);
        idInput->setPercentW(50);
        
        loadBtn = new Button("Carregar", {0, 0}, 12, 3);
        loadBtn->setPercentPosition(65, 12);
        
        nameInput = new InputBar("Nome", {0, 0}, modalW - 8, 3);
        nameInput->setPercentPosition(10, 26);
        nameInput->setPercentW(80);
        
        regNumberInput = new InputBar("Matrícula", {0, 0}, modalW - 8, 3);
        regNumberInput->setPercentPosition(10, 38);
        regNumberInput->setPercentW(80);
        
        emailInput = new InputBar("Email", {0, 0}, modalW - 8, 3);
        emailInput->setPercentPosition(10, 50);
        emailInput->setPercentW(80);
        
        phoneInput = new InputBar("Telefone", {0, 0}, modalW - 8, 3);
        phoneInput->setPercentPosition(10, 62);
        phoneInput->setPercentW(80);
        
        saveBtn = new Button("Salvar", {0, 0}, 12, 3);
        saveBtn->setPercentPosition(10, 76);
        
        deleteBtn = new Button("Excluir", {0, 0}, 12, 3);
        deleteBtn->setPercentPosition(40, 76);
        deleteBtn->setStyle({{255, 100, 100, 255}, BACKGROUND, {255, 150, 150, 255}, BACKGROUND});

        cancelBtn = new Button("Cancelar", {0, 0}, 12, 3);
        cancelBtn->setPercentPosition(70, 76);
        
        statusText = new Text("", {0, 0});
        statusText->setPercentPosition(5, 88);
        
        modalContainer->addElement(titleText);
        modalContainer->addElement(idInput);
        modalContainer->addElement(loadBtn);
        modalContainer->addElement(nameInput);
        modalContainer->addElement(regNumberInput);
        modalContainer->addElement(emailInput);
        modalContainer->addElement(phoneInput);
        modalContainer->addElement(saveBtn);
        modalContainer->addElement(deleteBtn);
        modalContainer->addElement(cancelBtn);
        modalContainer->addElement(statusText);
        
        loadBtn->onClickHandler = [this](element&, TUImanager&) {
            this->handleLoad();
        };
        
        saveBtn->onClickHandler = [this](element&, TUImanager&) {
            this->handleSave();
        };
        
        deleteBtn->onClickHandler = [this](element&, TUImanager&) {
            this->handleDelete();
        };
        
        cancelBtn->onClickHandler = [this](element&, TUImanager&) {
            this->state.isOpen = false;
        };
    }
    
    ~StudentEditModal() {
        delete titleText;
        delete idInput;
        delete loadBtn;
        delete nameInput;
        delete regNumberInput;
        delete emailInput;
        delete phoneInput;
        delete saveBtn;
        delete deleteBtn;
        delete cancelBtn;
        delete statusText;
        delete modalContainer;
    }
    
    void open(TUImanager& tui, container*) {
        state.isOpen = true;
        clearFields();
        tui.focusContainer(modalContainer, 0);
    }
    
    void close(TUImanager& tui, container* returnTo) {
        state.isOpen = false;
        tui.focusContainer(returnTo, 0);
    }
    
    void clearFields() {
        idInput->text.clear();
        nameInput->text.clear();
        regNumberInput->text.clear();
        emailInput->text.clear();
        phoneInput->text.clear();
        statusText->content.clear();
        currentStudentId = 0;
    }
    
    void handleLoad() {
        auto setStatus = [this](const std::string& msg, color fg) {
            statusText->content = msg;
            statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
        };
        
        if (idInput->text.empty()) {
            setStatus("Erro: Digite uma matrícula", {255, 100, 100, 255});
            return;
        }
        
        auto studentOpt = studentRepo->findByRegistrationNumber(idInput->text);
        if (!studentOpt) {
            setStatus("Erro: Estudante não encontrado", {255, 100, 100, 255});
            return;
        }
        
        currentStudentId = studentOpt->id;
        nameInput->text = studentOpt->name;
        regNumberInput->text = studentOpt->registration_number;
        emailInput->text = studentOpt->email.value_or("");
        phoneInput->text = studentOpt->phone.value_or("");
        setStatus("Estudante carregado: " + studentOpt->name, {100, 255, 100, 255});
    }
    
    void handleSave() {
        auto setStatus = [this](const std::string& msg, color fg) {
            statusText->content = msg;
            statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
        };
        
        if (currentStudentId == 0) {
            setStatus("Erro: Carregue um estudante primeiro", {255, 100, 100, 255});
            return;
        }
        
        if (nameInput->text.empty() || regNumberInput->text.empty()) {
            setStatus("Erro: Nome e matrícula são obrigatórios", {255, 100, 100, 255});
            return;
        }
        
        try {
            app::models::Student student;
            student.id = currentStudentId;
            student.name = nameInput->text;
            student.registration_number = regNumberInput->text;
            student.email = emailInput->text.empty() ? std::nullopt : std::optional<std::string>(emailInput->text);
            student.phone = phoneInput->text.empty() ? std::nullopt : std::optional<std::string>(phoneInput->text);
            student.active = true;
            
            if (studentRepo->update(student)) {
                setStatus("Estudante atualizado com sucesso!", {100, 255, 100, 255});
            } else {
                setStatus("Erro: Falha ao atualizar", {255, 100, 100, 255});
            }
        } catch (const std::exception& ex) {
            setStatus("Erro: " + std::string(ex.what()), {255, 100, 100, 255});
        }
    }
    
    void handleDelete() {
        auto setStatus = [this](const std::string& msg, color fg) {
            statusText->content = msg;
            statusText->setStyle({fg, TRANSPARENT, fg, TRANSPARENT});
        };
        
        if (currentStudentId == 0) {
            setStatus("Erro: Carregue um estudante primeiro", {255, 100, 100, 255});
            return;
        }
        
        try {
            // Use deactivate instead of hard delete to preserve loan history
            if (studentRepo->deactivate(currentStudentId)) {
                setStatus("Estudante desativado com sucesso!", {100, 255, 100, 255});
                clearFields();
            } else {
                setStatus("Erro: Falha ao desativar", {255, 100, 100, 255});
            }
        } catch (const std::exception& ex) {
            setStatus("Erro: " + std::string(ex.what()), {255, 100, 100, 255});
        }
    }
    
    void render(TUImanager& tui) {
        if (state.isOpen) {
            modalContainer->render(tui);
        }
    }
};

// ==================== END EDIT MODALS ====================

void runTestUI(app::Database& db) {
    TUImanager tui;
    
    // Create repositories
    app::repos::StudentRepository studentRepo(db);
    app::repos::BookRepository bookRepo(db);
    app::repos::LoanRepository loanRepo(db);
    
    // Left panel - Actions menu
    container actionsMenu({0, 0}, {tui.cols / 2, tui.rows}, 
                          {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND}, "Ações");
    actionsMenu.setDefaultElementStyle({TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND});
    actionsMenu.setInheritStyle(true);
    
    // Right panel containers for different views
    int rightX = tui.cols / 2;
    int rightW = tui.cols - rightX;
    
    container booksView({rightX, 0}, {rightW, tui.rows}, 
                        {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND}, "Livros");
    booksView.setDefaultElementStyle({TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND});
    booksView.setInheritStyle(true);
    
    container studentsView({rightX, 0}, {rightW, tui.rows}, 
                           {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND}, "Estudantes");
    studentsView.setDefaultElementStyle({TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND});
    studentsView.setInheritStyle(true);
    
    container loansView({rightX, 0}, {rightW, tui.rows}, 
                        {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND}, "Empréstimos Ativos");
    loansView.setDefaultElementStyle({TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND});
    loansView.setInheritStyle(true);

    container searchView({rightX, 0}, {rightW, tui.rows},
                         {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND}, "Buscar Livros");
    searchView.setDefaultElementStyle({TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND});
    searchView.setInheritStyle(true);
    
    // Create action buttons - View section
    Button viewBooksBtn("Ver livros", {0,0}, 20, 3);
    viewBooksBtn.setPercentPosition(10, 5);
    
    Button viewStudentsBtn("Ver estudantes", {0, 0}, 20, 3);
    viewStudentsBtn.setPercentPosition(10, 12);
    
    Button viewLoansBtn("Ver empréstimos", {0, 0}, 20, 3);
    viewLoansBtn.setPercentPosition(10, 19);
    
    Button searchBooksBtn("Buscar livros", {0, 0}, 20, 3);
    searchBooksBtn.setPercentPosition(10, 26);
    
    // Create action buttons - Register section
    Button registerBookBtn("Registrar livro", {0, 0}, 20, 3);
    registerBookBtn.setPercentPosition(10, 38);
    
    Button registerStudentBtn("Registrar estudante", {0, 0}, 20, 3);
    registerStudentBtn.setPercentPosition(10, 45);
    
    // Create action buttons - Edit section
    Button editBookBtn("Editar/Excluir livro", {0, 0}, 22, 3);
    editBookBtn.setPercentPosition(10, 57);
    
    Button editStudentBtn("Editar/Excluir estudante", {0, 0}, 26, 3);
    editStudentBtn.setPercentPosition(10, 64);
    
    // Create action buttons - Loan section
    Button loanBookBtn("Realizar empréstimo", {0, 0}, 20, 3);
    loanBookBtn.setPercentPosition(10, 76);
    
    Button returnBookBtn("Devolver livro", {0, 0}, 20, 3);
    returnBookBtn.setPercentPosition(10, 83);
    
    // Load real data from database
    std::vector<std::string> studentsData = loadStudentsFromDatabase(studentRepo);
    std::vector<std::string> booksData = loadBooksFromDatabase(bookRepo);
    std::vector<std::string> searchData = searchBooksFromDatabase(bookRepo, "");
    std::vector<RichListItem> loansData = loadLoansRichFromDatabase(loanRepo);
    
    // Create ListViews for each view container
    ListView booksList("Livros Disponíveis", booksData, {0, 0}, rightW - 4, tui.rows - 6, 15);
    booksList.setPercentPosition(1, 5);
    booksList.setPercentW(95);

    ListView studentsList("Estudantes Registrados", studentsData, {0, 0}, rightW - 4, tui.rows - 6, 15);
    studentsList.setPercentPosition(1, 5);
    studentsList.setPercentW(95);
    
    // Use RichListView for loans to show multi-line with custom themes
    RichListView richLoansList("Empréstimos Ativos", loansData, {0, 0}, rightW - 4, tui.rows - 4, 5);
    richLoansList.setPercentPosition(1, 3);
    richLoansList.setPercentW(95);
    richLoansList.setPercentH(95);

    InputBar searchInput("Título, autor ou ISBN", {0, 0}, rightW - 12, 3);
    searchInput.setPercentPosition(5, 8);
    searchInput.setPercentW(80);

    Button executeSearchBtn("Buscar", {0, 0}, 12, 3);
    executeSearchBtn.setPercentPosition(87, 8);

    ListView searchResultsList("Resultados da Busca", searchData, {0, 0}, rightW - 4, tui.rows - 10, 15);
    searchResultsList.setPercentPosition(1, 20);
    searchResultsList.setPercentW(95);
    searchResultsList.setPercentH(75);
    
    // Add elements to containers
    actionsMenu.addElement(&viewBooksBtn);
    actionsMenu.addElement(&viewStudentsBtn);
    actionsMenu.addElement(&viewLoansBtn);
    actionsMenu.addElement(&searchBooksBtn);
    actionsMenu.addElement(&registerBookBtn);
    actionsMenu.addElement(&registerStudentBtn);
    actionsMenu.addElement(&editBookBtn);
    actionsMenu.addElement(&editStudentBtn);
    actionsMenu.addElement(&loanBookBtn);
    actionsMenu.addElement(&returnBookBtn);
    
    booksView.addElement(&booksList);
    studentsView.addElement(&studentsList);
    loansView.addElement(&richLoansList);
    searchView.addElement(&searchInput);
    searchView.addElement(&executeSearchBtn);
    searchView.addElement(&searchResultsList);
    
    // Create modals with repositories
    StudentRegistrationModal studentModal(tui, &studentRepo);
    studentModal.modalContainer->tui = &tui;
    
    BookRegistrationModal bookModal(tui, &bookRepo);
    bookModal.modalContainer->tui = &tui;

    LoanCreationModal loanModal(tui, &studentRepo, &bookRepo, &loanRepo);
    loanModal.modalContainer->tui = &tui;

    LoanReturnModal returnModal(tui, &loanRepo, &bookRepo);
    returnModal.modalContainer->tui = &tui;
    
    // Create edit modals
    BookEditModal bookEditModal(tui, &bookRepo);
    bookEditModal.modalContainer->tui = &tui;
    
    StudentEditModal studentEditModal(tui, &studentRepo);
    studentEditModal.modalContainer->tui = &tui;
    
    // Current view state
    ViewType currentView = ViewType::BOOKS;
    container* currentRightContainer = &booksView;
    
    // Setup container navigation
    actionsMenu.setRight(&booksView);
    booksView.setLeft(&actionsMenu);
    studentsView.setLeft(&actionsMenu);
    loansView.setLeft(&actionsMenu);
    searchView.setLeft(&actionsMenu);
    
    // Setup button callbacks to switch views
    viewBooksBtn.onClickHandler = [&](element&, TUImanager& tui) {
        currentView = ViewType::BOOKS;
        currentRightContainer = &booksView;
        actionsMenu.setRight(&booksView);
    };
    
    viewStudentsBtn.onClickHandler = [&](element&, TUImanager& tui) {
        currentView = ViewType::STUDENTS;
        currentRightContainer = &studentsView;
        actionsMenu.setRight(&studentsView);
    };
    
    viewLoansBtn.onClickHandler = [&](element&, TUImanager& tui) {
        currentView = ViewType::LOANS;
        currentRightContainer = &loansView;
        actionsMenu.setRight(&loansView);
    };

    searchBooksBtn.onClickHandler = [&](element&, TUImanager& tui) {
        currentView = ViewType::SEARCH;
        currentRightContainer = &searchView;
        actionsMenu.setRight(&searchView);
    };
    
    // Book and Student registration callbacks
    registerBookBtn.onClickHandler = [&bookModal, &actionsMenu](element&, TUImanager& tui) {
        bookModal.open(tui, &actionsMenu);
    };
    
    registerStudentBtn.onClickHandler = [&studentModal, &actionsMenu](element&, TUImanager& tui) {
        studentModal.open(tui, &actionsMenu);
    };
    
    // Book modal callbacks
    bookModal.cancelBtn->onClickHandler = [&bookModal, &actionsMenu, &bookRepo, &booksList](element&, TUImanager& tui) {
        bookModal.close(tui, &actionsMenu);
        auto updatedData = loadBooksFromDatabase(bookRepo);
        booksList.setItems(updatedData);
    };
    
    bookModal.submitBtn->onClickHandler = [&bookModal, &actionsMenu, &bookRepo, &booksList](element&, TUImanager& tui) {
        bookModal.handleSubmit(tui);
        
        if (bookModal.statusText->content.find("Success") != std::string::npos) {
            bookModal.close(tui, &actionsMenu);
            auto updatedData = loadBooksFromDatabase(bookRepo);
            booksList.setItems(updatedData);
        }
    };
    
    // Student modal callbacks
    studentModal.cancelBtn->onClickHandler = [&studentModal, &actionsMenu, &studentRepo, &studentsList](element&, TUImanager& tui) {
        studentModal.close(tui, &actionsMenu);
        // Reload student list from database
        auto updatedData = loadStudentsFromDatabase(studentRepo);
        studentsList.setItems(updatedData);
    };
    
    // Update submit to close on success and refresh list
    studentModal.submitBtn->onClickHandler = [&studentModal, &actionsMenu, &studentRepo, &studentsList](element&, TUImanager& tui) {
        studentModal.handleSubmit(tui);
        
        // If success message shown, close modal and refresh list
        if (studentModal.statusText->content.find("Success") != std::string::npos) {
            studentModal.close(tui, &actionsMenu);
            // Reload student list from database
            auto updatedData = loadStudentsFromDatabase(studentRepo);
            studentsList.setItems(updatedData);
        }
    };
    
    loanBookBtn.onClickHandler = [&loanModal, &actionsMenu](element&, TUImanager& tui) {
        loanModal.open(tui, &actionsMenu);
    };
    
    returnBookBtn.onClickHandler = [&returnModal, &actionsMenu](element&, TUImanager& tui) {
        returnModal.open(tui, &actionsMenu);
    };
    
    loanModal.cancelBtn->onClickHandler = [&loanModal, &actionsMenu](element&, TUImanager& tui) {
        loanModal.close(tui, &actionsMenu);
    };
    
    loanModal.submitBtn->onClickHandler = [&loanModal, &actionsMenu, &bookRepo, &loanRepo, &booksList, &richLoansList](element&, TUImanager& tui) {
        if (loanModal.handleSubmit(tui)) {
            loanModal.close(tui, &actionsMenu);
            booksList.setItems(loadBooksFromDatabase(bookRepo));
            richLoansList.setItems(loadLoansRichFromDatabase(loanRepo));
        }
    };
    
    returnModal.cancelBtn->onClickHandler = [&returnModal, &actionsMenu](element&, TUImanager& tui) {
        returnModal.close(tui, &actionsMenu);
    };
    
    returnModal.submitBtn->onClickHandler = [&returnModal, &actionsMenu, &bookRepo, &loanRepo, &booksList, &richLoansList](element&, TUImanager& tui) {
        if (returnModal.handleSubmit(tui)) {
            returnModal.close(tui, &actionsMenu);
            booksList.setItems(loadBooksFromDatabase(bookRepo));
            richLoansList.setItems(loadLoansRichFromDatabase(loanRepo));
        }
    };

    executeSearchBtn.onClickHandler = [&searchInput, &bookRepo, &searchResultsList](element&, TUImanager&) {
        auto results = searchBooksFromDatabase(bookRepo, searchInput.text);
        searchResultsList.setItems(results);
    };
    
    // Edit modal handlers
    editBookBtn.onClickHandler = [&bookEditModal, &actionsMenu](element&, TUImanager& tui) {
        bookEditModal.open(tui, &actionsMenu);
    };
    
    editStudentBtn.onClickHandler = [&studentEditModal, &actionsMenu](element&, TUImanager& tui) {
        studentEditModal.open(tui, &actionsMenu);
    };
    
    bookEditModal.cancelBtn->onClickHandler = [&bookEditModal, &actionsMenu, &bookRepo, &booksList](element&, TUImanager& tui) {
        bookEditModal.close(tui, &actionsMenu);
        booksList.setItems(loadBooksFromDatabase(bookRepo));
    };
    
    studentEditModal.cancelBtn->onClickHandler = [&studentEditModal, &actionsMenu, &studentRepo, &studentsList](element&, TUImanager& tui) {
        studentEditModal.close(tui, &actionsMenu);
        studentsList.setItems(loadStudentsFromDatabase(studentRepo));
    };
    
    // Initialize TUI state
    tui.containerID = &actionsMenu;
    actionsMenu.tui = &tui;
    booksView.tui = &tui;
    studentsView.tui = &tui;
    loansView.tui = &tui;
    searchView.tui = &tui;
    
    // Initial focus
    if (!actionsMenu.elements.empty()) {
        actionsMenu.elements[actionsMenu.focusedIndex]->notifyHover(tui, true);
    }
    
    // Clear background once
    tui.clearScreen(BACKGROUND);
    
    // Main event loop
    while (true) {
        // Wait for input with timeout
        if (!tui.hasDirty()) {
            bool hasInput = tui.waitForInput(16);
            if (!hasInput) {
                continue;
            }
        }
        
        // Poll input and check for exit
        if (tui.pollInput()) break;
        
        // Render active containers
        actionsMenu.render(tui);
        if (currentRightContainer) {
            currentRightContainer->render(tui);
        }
        
        // Render modals on top if open
        if (studentModal.state.isOpen) {
            studentModal.render(tui);
        }
        if (bookModal.state.isOpen) {
            bookModal.render(tui);
        }
        if (loanModal.state.isOpen) {
            loanModal.render(tui);
        }
        if (returnModal.state.isOpen) {
            returnModal.render(tui);
        }
        if (bookEditModal.state.isOpen) {
            bookEditModal.render(tui);
        }
        if (studentEditModal.state.isOpen) {
            studentEditModal.render(tui);
        }
        
        // Run end-of-frame callbacks and render
        tui.runEndOfFrame();
        if (tui.hasDirty()) {
            tui.render();
        }
    }
}

} // namespace ui
