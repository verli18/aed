#include "chrmaTUI.hpp"
#include "elements.hpp"
#include "app/db.hpp"
#include "app/models.hpp"
#include "app/repos/student_repository.hpp"
#include "app/repos/book_repository.hpp"

#include <vector>
#include <string>
#include <optional>

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
    NONE
};

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

// Dummy data generators for Loans (will be replaced later)

std::vector<std::string> generateDummyLoans() {
    return {
        "No active loans",
    };
}

std::vector<RichListItem> generateDummyRichLoans() {
    std::vector<RichListItem> loans;
    
    // Normal loan theme
    standardStyle normalTheme = {TEXT, BACKGROUND, TEXT_HIGHLIGHT, BACKGROUND};
    
    loans.push_back(RichListItem({
        "No active loans",
        "Register books and students to create loans",
        ""
    }, normalTheme));
    
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
        titleText = new Text("Fill in student information:", {0, 0});
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

void runTestUI(app::Database& db) {
    TUImanager tui;
    
    // Create repositories
    app::repos::StudentRepository studentRepo(db);
    app::repos::BookRepository bookRepo(db);
    
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
    
    // Create action buttons
    Button viewBooksBtn("Ver livros", {0, 0}, 20, 3);
    viewBooksBtn.setPercentPosition(10, 10);
    
    Button viewStudentsBtn("Ver estudantes", {0, 0}, 20, 3);
    viewStudentsBtn.setPercentPosition(10, 20);
    
    Button viewLoansBtn("Ver empréstimos", {0, 0}, 20, 3);
    viewLoansBtn.setPercentPosition(10, 30);
    
    Button registerBookBtn("Registrar livro", {0, 0}, 20, 3);
    registerBookBtn.setPercentPosition(10, 45);
    
    Button registerStudentBtn("Registrar estudante", {0, 0}, 20, 3);
    registerStudentBtn.setPercentPosition(10, 55);
    
    Button loanBookBtn("Realizar empréstimo", {0, 0}, 20, 3);
    loanBookBtn.setPercentPosition(10, 65);
    
    Button returnBookBtn("Devolver livro", {0, 0}, 20, 3);
    returnBookBtn.setPercentPosition(10, 75);
    
    // Load real data from database
    std::vector<std::string> studentsData = loadStudentsFromDatabase(studentRepo);
    std::vector<std::string> booksData = loadBooksFromDatabase(bookRepo);
    
    // Create ListViews for each view container
    ListView booksList("Livros Disponíveis", booksData, {0, 0}, rightW - 4, tui.rows - 6, 15);
    booksList.setPercentPosition(1, 5);
    booksList.setPercentW(95);

    ListView studentsList("Estudantes Registrados", studentsData, {0, 0}, rightW - 4, tui.rows - 6, 15);
    studentsList.setPercentPosition(1, 5);
    studentsList.setPercentW(95);
    
    // Use RichListView for loans to show multi-line with custom themes
    RichListView richLoansList("Empréstimos Ativos", generateDummyRichLoans(), {0, 0}, rightW - 4, tui.rows - 4, 5);
    richLoansList.setPercentPosition(1, 3);
    richLoansList.setPercentW(95);
    richLoansList.setPercentH(95);
    
    // Add elements to containers
    actionsMenu.addElement(&viewBooksBtn);
    actionsMenu.addElement(&viewStudentsBtn);
    actionsMenu.addElement(&viewLoansBtn);
    actionsMenu.addElement(&registerBookBtn);
    actionsMenu.addElement(&registerStudentBtn);
    actionsMenu.addElement(&loanBookBtn);
    actionsMenu.addElement(&returnBookBtn);
    
    booksView.addElement(&booksList);
    studentsView.addElement(&studentsList);
    loansView.addElement(&richLoansList);
    
    // Create modals with repositories
    StudentRegistrationModal studentModal(tui, &studentRepo);
    studentModal.modalContainer->tui = &tui;
    
    BookRegistrationModal bookModal(tui, &bookRepo);
    bookModal.modalContainer->tui = &tui;
    
    // Current view state
    ViewType currentView = ViewType::BOOKS;
    container* currentRightContainer = &booksView;
    
    // Setup container navigation
    actionsMenu.setRight(&booksView);
    booksView.setLeft(&actionsMenu);
    studentsView.setLeft(&actionsMenu);
    loansView.setLeft(&actionsMenu);
    
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
    
    loanBookBtn.onClickHandler = [](element&, TUImanager&) {
        // TODO: Open modal for loan creation
    };
    
    returnBookBtn.onClickHandler = [](element&, TUImanager&) {
        // TODO: Open modal for loan return
    };
    
    // Initialize TUI state
    tui.containerID = &actionsMenu;
    actionsMenu.tui = &tui;
    booksView.tui = &tui;
    studentsView.tui = &tui;
    loansView.tui = &tui;
    
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
        
        // Run end-of-frame callbacks and render
        tui.runEndOfFrame();
        if (tui.hasDirty()) {
            tui.render();
        }
    }
}

} // namespace ui
