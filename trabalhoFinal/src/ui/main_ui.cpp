#include "ui/ui_common.hpp"
#include "ui/modals/student_modals.hpp"
#include "ui/modals/book_modals.hpp"
#include "ui/modals/loan_modals.hpp"

namespace ui {

NotificationManager* globalNotifications = nullptr;

void runTestUI(app::Database& db) {
    TUImanager tui;
    NotificationManager notifications;
    globalNotifications = &notifications;
    
    app::repos::StudentRepository studentRepo(db);
    app::repos::BookRepository bookRepo(db);
    app::repos::LoanRepository loanRepo(db);
    

    int menuWidth = tui.cols * 30 / 100;
    container actionsMenu({0, 0}, {menuWidth, tui.rows}, defaultModalStyle(), "Ações");
    actionsMenu.setDefaultElementStyle(defaultModalStyle());
    actionsMenu.setInheritStyle(true);
    
    int rightX = menuWidth;
    int rightW = tui.cols - rightX;

    //initializing container contexts
    container booksView({rightX, 0}, {rightW, tui.rows}, defaultModalStyle(), "Livros");
    booksView.setDefaultElementStyle(defaultModalStyle());
    booksView.setInheritStyle(true);
    
    container studentsView({rightX, 0}, {rightW, tui.rows}, defaultModalStyle(), "Estudantes");
    studentsView.setDefaultElementStyle(defaultModalStyle());
    studentsView.setInheritStyle(true);
    
    container loansView({rightX, 0}, {rightW, tui.rows}, defaultModalStyle(), "Empréstimos Ativos");
    loansView.setDefaultElementStyle(defaultModalStyle());
    loansView.setInheritStyle(true);
    
    container searchView({rightX, 0}, {rightW, tui.rows}, defaultModalStyle(), "Buscar Livros");
    searchView.setDefaultElementStyle(defaultModalStyle());
    searchView.setInheritStyle(true);
    
    
    Text viewSectionLabel("─── Visualizar ───", {0, 0});
    viewSectionLabel.setPercentPosition(5, 3);
    
    Button viewBooksBtn("Livros", {0, 0}, 16, 3);
    viewBooksBtn.setPercentPosition(8, 8);
    
    Button viewStudentsBtn("Estudantes", {0, 0}, 16, 3);
    viewStudentsBtn.setPercentPosition(8, 15);
    
    Button viewLoansBtn("Empréstimos", {0, 0}, 16, 3);
    viewLoansBtn.setPercentPosition(8, 22);
    
    Button searchBooksBtn("Buscar", {0, 0}, 16, 3);
    searchBooksBtn.setPercentPosition(8, 29);
    
    Text registerSectionLabel("─── Cadastrar ───", {0, 0});
    registerSectionLabel.setPercentPosition(5, 38);
    
    Button registerBookBtn("+ Livro", {0, 0}, 16, 3);
    registerBookBtn.setPercentPosition(8, 43);
    
    Button registerStudentBtn("+ Estudante", {0, 0}, 16, 3);
    registerStudentBtn.setPercentPosition(8, 50);
    
    Text editSectionLabel("─── Gerenciar ───", {0, 0});
    editSectionLabel.setPercentPosition(5, 59);
    
    Button editBookBtn("Editar Livro", {0, 0}, 16, 3);
    editBookBtn.setPercentPosition(8, 64);
    
    Button editStudentBtn("Editar Estudante", {0, 0}, 18, 3);
    editStudentBtn.setPercentPosition(8, 71);
    
    Text loanSectionLabel("─── Empréstimos ───", {0, 0});
    loanSectionLabel.setPercentPosition(5, 80);
    
    Button loanBookBtn("Emprestar", {0, 0}, 14, 3);
    loanBookBtn.setPercentPosition(8, 85);
    
    Button returnBookBtn("Devolver", {0, 0}, 14, 3);
    returnBookBtn.setPercentPosition(8, 92);
    
    //list views
    std::vector<std::string> booksData = loadBooksFromDatabase(bookRepo);
    std::vector<std::string> studentsData = loadStudentsFromDatabase(studentRepo);
    std::vector<std::string> searchData = searchBooksFromDatabase(bookRepo, "");
    std::vector<RichListItem> loansData = loadLoansRichFromDatabase(loanRepo);
    
    ListView booksList("", booksData, {0, 0}, rightW - 4, tui.rows - 4, 15);
    booksList.setPercentPosition(2, 5);
    booksList.setPercentW(96);
    booksList.setPercentH(90);
    
    ListView studentsList("", studentsData, {0, 0}, rightW - 4, tui.rows - 4, 15);
    studentsList.setPercentPosition(2, 5);
    studentsList.setPercentW(96);
    studentsList.setPercentH(90);
    
    RichListView richLoansList("", loansData, {0, 0}, rightW - 4, tui.rows - 4, 5);
    richLoansList.setPercentPosition(2, 5);
    richLoansList.setPercentW(96);
    richLoansList.setPercentH(90);
    
    // Search view components
    InputBar searchInput("Título, autor ou ISBN", {0, 0}, rightW - 12, 3);
    searchInput.setPercentPosition(3, 8);
    searchInput.setPercentW(75);
    
    Button executeSearchBtn("Buscar", {0, 0}, 12, 3);
    executeSearchBtn.setPercentPosition(82, 8);
    
    ListView searchResultsList("", searchData, {0, 0}, rightW - 4, tui.rows - 8, 15);
    searchResultsList.setPercentPosition(2, 20);
    searchResultsList.setPercentW(96);
    searchResultsList.setPercentH(75);
    
    actionsMenu.addElement(&viewSectionLabel);
    actionsMenu.addElement(&viewBooksBtn);
    actionsMenu.addElement(&viewStudentsBtn);
    actionsMenu.addElement(&viewLoansBtn);
    actionsMenu.addElement(&searchBooksBtn);
    actionsMenu.addElement(&registerSectionLabel);
    actionsMenu.addElement(&registerBookBtn);
    actionsMenu.addElement(&registerStudentBtn);
    actionsMenu.addElement(&editSectionLabel);
    actionsMenu.addElement(&editBookBtn);
    actionsMenu.addElement(&editStudentBtn);
    actionsMenu.addElement(&loanSectionLabel);
    actionsMenu.addElement(&loanBookBtn);
    actionsMenu.addElement(&returnBookBtn);
    
    booksView.addElement(&booksList);
    studentsView.addElement(&studentsList);
    loansView.addElement(&richLoansList);
    searchView.addElement(&searchInput);
    searchView.addElement(&executeSearchBtn);
    searchView.addElement(&searchResultsList);
    
    StudentRegistrationModal studentModal(tui, &studentRepo);
    studentModal.modalContainer->tui = &tui;
    
    BookRegistrationModal bookModal(tui, &bookRepo);
    bookModal.modalContainer->tui = &tui;
    
    LoanCreationModal loanModal(tui, &studentRepo, &bookRepo, &loanRepo);
    loanModal.modalContainer->tui = &tui;
    
    LoanReturnModal returnModal(tui, &loanRepo, &bookRepo);
    returnModal.modalContainer->tui = &tui;
    
    BookEditModal bookEditModal(tui, &bookRepo);
    bookEditModal.modalContainer->tui = &tui;
    
    StudentEditModal studentEditModal(tui, &studentRepo);
    studentEditModal.modalContainer->tui = &tui;
    
    ViewType currentView = ViewType::BOOKS;
    container* currentRightContainer = &booksView;
    
    actionsMenu.setRight(&booksView);
    booksView.setLeft(&actionsMenu);
    studentsView.setLeft(&actionsMenu);
    loansView.setLeft(&actionsMenu);
    searchView.setLeft(&actionsMenu);
    
    // button callbacks w/ lambdas
    
    // View buttons
    viewBooksBtn.onClickHandler = [&](element&, TUImanager&) {
        currentView = ViewType::BOOKS;
        currentRightContainer = &booksView;
        actionsMenu.setRight(&booksView);
        booksList.setItems(loadBooksFromDatabase(bookRepo));
    };
    
    viewStudentsBtn.onClickHandler = [&](element&, TUImanager&) {
        currentView = ViewType::STUDENTS;
        currentRightContainer = &studentsView;
        actionsMenu.setRight(&studentsView);
        studentsList.setItems(loadStudentsFromDatabase(studentRepo));
    };
    
    viewLoansBtn.onClickHandler = [&](element&, TUImanager&) {
        currentView = ViewType::LOANS;
        currentRightContainer = &loansView;
        actionsMenu.setRight(&loansView);
        richLoansList.setItems(loadLoansRichFromDatabase(loanRepo));
    };
    
    searchBooksBtn.onClickHandler = [&](element&, TUImanager&) {
        currentView = ViewType::SEARCH;
        currentRightContainer = &searchView;
        actionsMenu.setRight(&searchView);
    };
    
    // Registration buttons
    registerBookBtn.onClickHandler = [&](element&, TUImanager& t) {
        bookModal.open(t, &actionsMenu);
    };
    
    registerStudentBtn.onClickHandler = [&](element&, TUImanager& t) {
        studentModal.open(t, &actionsMenu);
    };
    
    // Edit buttons
    editBookBtn.onClickHandler = [&](element&, TUImanager& t) {
        bookEditModal.open(t, &actionsMenu);
    };
    
    editStudentBtn.onClickHandler = [&](element&, TUImanager& t) {
        studentEditModal.open(t, &actionsMenu);
    };
    
    // Loan buttons
    loanBookBtn.onClickHandler = [&](element&, TUImanager& t) {
        loanModal.open(t, &actionsMenu);
    };
    
    returnBookBtn.onClickHandler = [&](element&, TUImanager& t) {
        returnModal.open(t, &actionsMenu);
    };
    
    // Search button
    executeSearchBtn.onClickHandler = [&](element&, TUImanager&) {
        searchResultsList.setItems(searchBooksFromDatabase(bookRepo, searchInput.text));
    };
        
    // Helper to refresh lists
    auto refreshLists = [&]() {
        booksList.setItems(loadBooksFromDatabase(bookRepo));
        studentsList.setItems(loadStudentsFromDatabase(studentRepo));
        richLoansList.setItems(loadLoansRichFromDatabase(loanRepo));
    };
    
    // Book modal
    bookModal.cancelBtn->onClickHandler = [&](element&, TUImanager& t) {
        bookModal.close(t, &actionsMenu);
        refreshLists();
    };
    
    bookModal.submitBtn->onClickHandler = [&](element&, TUImanager& t) {
        bookModal.handleSubmit();
        // Stay open to show status, user closes with cancel
    };
    
    studentModal.cancelBtn->onClickHandler = [&](element&, TUImanager& t) {
        studentModal.close(t, &actionsMenu);
        refreshLists();
    };
    
    studentModal.submitBtn->onClickHandler = [&](element&, TUImanager&) {
        studentModal.handleSubmit();
    };
    
    loanModal.cancelBtn->onClickHandler = [&](element&, TUImanager& t) {
        loanModal.close(t, &actionsMenu);
        refreshLists();
    };
    
    loanModal.submitBtn->onClickHandler = [&](element&, TUImanager& t) {
        if (loanModal.handleSubmit()) {
            // Success - close after a moment
            refreshLists();
        }
    };
    
    returnModal.cancelBtn->onClickHandler = [&](element&, TUImanager& t) {
        returnModal.close(t, &actionsMenu);
        refreshLists();
    };
    
    returnModal.submitBtn->onClickHandler = [&](element&, TUImanager& t) {
        if (returnModal.handleSubmit()) {
            refreshLists();
        }
    };
    
    // Book edit modal
    bookEditModal.cancelBtn->onClickHandler = [&](element&, TUImanager& t) {
        bookEditModal.close(t, &actionsMenu);
        refreshLists();
    };
    
    // Student edit modal
    studentEditModal.cancelBtn->onClickHandler = [&](element&, TUImanager& t) {
        studentEditModal.close(t, &actionsMenu);
        refreshLists();
    };
    
    //finally initialize tui
    tui.containerID = &actionsMenu;
    actionsMenu.tui = &tui;
    booksView.tui = &tui;
    studentsView.tui = &tui;
    loansView.tui = &tui;
    searchView.tui = &tui;
    
    if (!actionsMenu.elements.empty()) {
        // Focus first focusable element
        for (size_t i = 0; i < actionsMenu.elements.size(); ++i) {
            if (actionsMenu.elements[i]->canBeFocused()) {
                actionsMenu.focusedIndex = static_cast<int>(i);
                actionsMenu.elements[i]->notifyHover(tui, true);
                break;
            }
        }
    }
    
    tui.clearScreen(BACKGROUND);
    
    // ==================== MAIN EVENT LOOP ====================
    while (true) {
        // Update notifications even without input (for auto-removal of expired ones)
        // This will mark the notification area dirty if any were removed
        notifications.update(tui);
        
        if (!tui.hasDirty() && notifications.empty()) {
            if (!tui.waitForInput(16)) {
                continue;
            }
        }
        
        if (tui.pollInput()) break;
        
        // Render containers
        actionsMenu.render(tui);
        if (currentRightContainer) {
            currentRightContainer->render(tui);
        }
        
        // Render modals (higher z-index)
        if (studentModal.isOpen()) studentModal.render(tui);
        if (bookModal.isOpen()) bookModal.render(tui);
        if (loanModal.isOpen()) loanModal.render(tui);
        if (returnModal.isOpen()) returnModal.render(tui);
        if (bookEditModal.isOpen()) bookEditModal.render(tui);
        if (studentEditModal.isOpen()) studentEditModal.render(tui);
        
        // Render notifications (highest z-index, top-right corner)
        notifications.render(tui);
        
        tui.runEndOfFrame();
        if (tui.hasDirty()) {
            tui.render();
        }
    }
    
    // Cleanup
    globalNotifications = nullptr;
}
} // namespace ui