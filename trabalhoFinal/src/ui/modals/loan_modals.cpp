#include "ui/modals/loan_modals.hpp"

namespace ui {

// ==================== LOAN CREATION MODAL ====================

LoanCreationModal::LoanCreationModal(TUImanager& tui,
                                     app::repos::StudentRepository* sRepo,
                                     app::repos::BookRepository* bRepo,
                                     app::repos::LoanRepository* lRepo)
    : studentRepo(sRepo), bookRepo(bRepo), loanRepo(lRepo) {
    
    modalContainer = createModalContainer(tui, 55, 24, "Novo Empréstimo", 110);
    
    titleText = new Text("Informe os dados do empréstimo:", {0, 0});
    titleText->setPercentPosition(5, 5);
    
    studentRegInput = new InputBar("Matrícula do Estudante *", {0, 0}, 40, 3);
    studentRegInput->setPercentPosition(8, 18);
    studentRegInput->setPercentW(84);
    
    bookIdInput = new InputBar("ID do Livro *", {0, 0}, 40, 3);
    bookIdInput->setPercentPosition(8, 34);
    bookIdInput->setPercentW(84);
    
    dueDaysInput = new InputBar("Dias para devolução", {0, 0}, 40, 3);
    dueDaysInput->setPercentPosition(8, 50);
    dueDaysInput->setPercentW(84);
    dueDaysInput->text = "14";
    
    // Buttons - vertical layout for proper navigation
    submitBtn = new Button("[ Confirmar ]", {0, 0}, 18, 3);
    submitBtn->setPercentPosition(50, 66);
    submitBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    cancelBtn = new Button("[ Cancelar ]", {0, 0}, 18, 3);
    cancelBtn->setPercentPosition(50, 80);
    cancelBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    statusText = new Text("", {0, 0});
    statusText->setPercentPosition(5, 85);
    
    // Add elements
    modalContainer->addElement(titleText);
    modalContainer->addElement(studentRegInput);
    modalContainer->addElement(bookIdInput);
    modalContainer->addElement(dueDaysInput);
    modalContainer->addElement(submitBtn);
    modalContainer->addElement(cancelBtn);
    modalContainer->addElement(statusText);
}

LoanCreationModal::~LoanCreationModal() {
    delete titleText;
    delete studentRegInput;
    delete bookIdInput;
    delete dueDaysInput;
    delete submitBtn;
    delete cancelBtn;
    delete statusText;
    delete modalContainer;
}

void LoanCreationModal::open(TUImanager& tui, container*) {
    state.isOpen = true;
    studentRegInput->text.clear();
    bookIdInput->text.clear();
    dueDaysInput->text = "14";
    statusText->content.clear();
    tui.focusContainer(modalContainer, 0);
}

void LoanCreationModal::close(TUImanager& tui, container* returnTo) {
    state.isOpen = false;
    tui.focusContainer(returnTo, 0);
}

bool LoanCreationModal::handleSubmit() {
    if (studentRegInput->text.empty()) {
        setError("Matrícula é obrigatória");
        return false;
    }
    if (bookIdInput->text.empty()) {
        setError("ID do livro é obrigatório");
        return false;
    }
    
    auto studentOpt = studentRepo->findByRegistrationNumber(studentRegInput->text);
    if (!studentOpt) {
        setError("Estudante não encontrado");
        return false;
    }
    
    int64_t bookId = 0;
    try {
        bookId = std::stoll(bookIdInput->text);
    } catch (...) {
        setError("ID do livro inválido");
        return false;
    }
    
    auto bookOpt = bookRepo->findById(bookId);
    if (!bookOpt) {
        setError("Livro não encontrado");
        return false;
    }
    if (bookOpt->copies_available <= 0) {
        setError("Livro sem cópias disponíveis");
        return false;
    }
    
    int dueDays = 14;
    try {
        if (!dueDaysInput->text.empty()) {
            dueDays = std::stoi(dueDaysInput->text);
        }
        if (dueDays <= 0) {
            setError("Dias deve ser > 0");
            return false;
        }
    } catch (...) {
        setError("Dias inválido");
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
            throw std::runtime_error("Falha ao atualizar cópias");
        }
        
        setSuccess("Empréstimo criado para " + studentOpt->name);
        return true;
    } catch (const std::exception& ex) {
        setError(ex.what());
        return false;
    }
}

// ==================== LOAN RETURN MODAL ====================

LoanReturnModal::LoanReturnModal(TUImanager& tui,
                                 app::repos::LoanRepository* lRepo,
                                 app::repos::BookRepository* bRepo)
    : loanRepo(lRepo), bookRepo(bRepo) {
    
    modalContainer = createModalContainer(tui, 50, 18, "Devolver Livro", 110);
    
    titleText = new Text("Informe o ID do empréstimo:", {0, 0});
    titleText->setPercentPosition(5, 10);
    
    loanIdInput = new InputBar("ID do Empréstimo *", {0, 0}, 40, 3);
    loanIdInput->setPercentPosition(10, 32);
    loanIdInput->setPercentW(80);
    
    // Buttons - vertical layout for proper navigation
    submitBtn = new Button("[ Devolver ]", {0, 0}, 18, 3);
    submitBtn->setPercentPosition(50, 52);
    submitBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    cancelBtn = new Button("[ Cancelar ]", {0, 0}, 18, 3);
    cancelBtn->setPercentPosition(50, 70);
    cancelBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    statusText = new Text("", {0, 0});
    statusText->setPercentPosition(5, 80);
    
    // Add elements
    modalContainer->addElement(titleText);
    modalContainer->addElement(loanIdInput);
    modalContainer->addElement(submitBtn);
    modalContainer->addElement(cancelBtn);
    modalContainer->addElement(statusText);
}

LoanReturnModal::~LoanReturnModal() {
    delete titleText;
    delete loanIdInput;
    delete submitBtn;
    delete cancelBtn;
    delete statusText;
    delete modalContainer;
}

void LoanReturnModal::open(TUImanager& tui, container*) {
    state.isOpen = true;
    loanIdInput->text.clear();
    statusText->content.clear();
    tui.focusContainer(modalContainer, 0);
}

void LoanReturnModal::close(TUImanager& tui, container* returnTo) {
    state.isOpen = false;
    tui.focusContainer(returnTo, 0);
}

bool LoanReturnModal::handleSubmit() {
    if (loanIdInput->text.empty()) {
        setError("ID é obrigatório");
        return false;
    }
    
    int64_t loanId = 0;
    try {
        loanId = std::stoll(loanIdInput->text);
    } catch (...) {
        setError("ID inválido");
        return false;
    }
    
    auto loanOpt = loanRepo->findById(loanId);
    if (!loanOpt) {
        setError("Empréstimo não encontrado");
        return false;
    }
    if (loanOpt->return_date.has_value()) {
        setError("Empréstimo já devolvido");
        return false;
    }
    
    try {
        if (!loanRepo->markReturned(loanId, getTodayISODate())) {
            setError("Falha ao registrar devolução");
            return false;
        }
        if (!bookRepo->incrementCopies(loanOpt->book_id)) {
            throw std::runtime_error("Falha ao atualizar cópias");
        }
        setSuccess("Livro devolvido com sucesso!");
        return true;
    } catch (const std::exception& ex) {
        setError(ex.what());
        return false;
    }
}

} // namespace ui
