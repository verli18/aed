#include "ui/modals/book_modals.hpp"

namespace ui {

BookRegistrationModal::BookRegistrationModal(TUImanager& tui, app::repos::BookRepository* repo)
    : bookRepo(repo) {
    
    modalContainer = createModalContainer(tui, 52, 26, "Registrar Livro");
    
    titleText = new Text("Preencha os dados do livro:", {0, 0});
    titleText->setPercentPosition(5, 4);
    
    titleInput = new InputBar("Título *", {0, 0}, 40, 3);
    titleInput->setPercentPosition(8, 14);
    titleInput->setPercentW(84);
    
    authorInput = new InputBar("Autor *", {0, 0}, 40, 3);
    authorInput->setPercentPosition(8, 26);
    authorInput->setPercentW(84);
    
    yearInput = new InputBar("Ano", {0, 0}, 15, 3);
    yearInput->setPercentPosition(8, 38);
    yearInput->setPercentW(38);
    
    copiesInput = new InputBar("Cópias", {0, 0}, 15, 3);
    copiesInput->setPercentPosition(54, 38);
    copiesInput->setPercentW(38);
    copiesInput->text = "1";
    
    isbnInput = new InputBar("ISBN", {0, 0}, 40, 3);
    isbnInput->setPercentPosition(8, 50);
    isbnInput->setPercentW(84);
    
    submitBtn = new Button("[ Confirmar ]", {0, 0}, 18, 3);
    submitBtn->setPercentPosition(50, 64);
    submitBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    cancelBtn = new Button("[ Cancelar ]", {0, 0}, 18, 3);
    cancelBtn->setPercentPosition(50, 76);
    cancelBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    // Add elements
    modalContainer->addElement(titleText);
    modalContainer->addElement(titleInput);
    modalContainer->addElement(authorInput);
    modalContainer->addElement(yearInput);
    modalContainer->addElement(copiesInput);
    modalContainer->addElement(isbnInput);
    modalContainer->addElement(submitBtn);
    modalContainer->addElement(cancelBtn);
    
    submitBtn->onClickHandler = [this](element&, TUImanager&) {
        this->handleSubmit();
    };
}

BookRegistrationModal::~BookRegistrationModal() {
    delete titleText;
    delete titleInput;
    delete authorInput;
    delete yearInput;
    delete isbnInput;
    delete copiesInput;
    delete submitBtn;
    delete cancelBtn;
    delete modalContainer;
}

void BookRegistrationModal::open(TUImanager& tui, container*) {
    isOpen_ = true;
    titleInput->text.clear();
    authorInput->text.clear();
    yearInput->text.clear();
    isbnInput->text.clear();
    copiesInput->text = "1";
    tui.focusContainer(modalContainer, 0);
}

void BookRegistrationModal::close(TUImanager& tui, container* returnTo) {
    isOpen_ = false;
    tui.focusContainer(returnTo, 0);
}

void BookRegistrationModal::handleSubmit() {
    if (titleInput->text.empty()) {
        setError("Título é obrigatório");
        return;
    }
    if (authorInput->text.empty()) {
        setError("Autor é obrigatório");
        return;
    }
    
    int copies = 1;
    try {
        if (!copiesInput->text.empty()) {
            copies = std::stoi(copiesInput->text);
            if (copies < 0) {
                setError("Cópias deve ser >= 0");
                return;
            }
        }
    } catch (...) {
        setError("Número de cópias inválido");
        return;
    }
    
    std::optional<int> year;
    if (!yearInput->text.empty()) {
        try {
            year = std::stoi(yearInput->text);
        } catch (...) {
            setError("Ano inválido");
            return;
        }
    }
    
    try {
        std::optional<std::string> isbn = isbnInput->text.empty() 
            ? std::nullopt : std::optional<std::string>(isbnInput->text);
        
        app::models::Book book(titleInput->text, authorInput->text, year, isbn, copies);
        int64_t newId = bookRepo->create(book);
        
        setSuccess("Livro registrado! ID: " + std::to_string(newId));
    } catch (const std::exception& ex) {
        setError(ex.what());
    }
}

// ==================== BOOK EDIT MODAL ====================

BookEditModal::BookEditModal(TUImanager& tui, app::repos::BookRepository* repo)
    : bookRepo(repo) {
    
    modalContainer = createModalContainer(tui, 55, 30, "Editar/Excluir Livro");
    
    titleText = new Text("Busque pelo ID do livro:", {0, 0});
    titleText->setPercentPosition(5, 3);
    
    // Search row
    idInput = new InputBar("ID", {0, 0}, 20, 3);
    idInput->setPercentPosition(8, 10);
    idInput->setPercentW(50);
    
    loadBtn = new Button("Buscar", {0, 0}, 12, 3);
    loadBtn->setPercentPosition(65, 10);
    
    // Edit fields
    titleInput = new InputBar("Título *", {0, 0}, 40, 3);
    titleInput->setPercentPosition(8, 22);
    titleInput->setPercentW(84);
    
    authorInput = new InputBar("Autor *", {0, 0}, 40, 3);
    authorInput->setPercentPosition(8, 33);
    authorInput->setPercentW(84);
    
    yearInput = new InputBar("Ano", {0, 0}, 15, 3);
    yearInput->setPercentPosition(8, 44);
    yearInput->setPercentW(38);
    
    copiesInput = new InputBar("Cópias", {0, 0}, 15, 3);
    copiesInput->setPercentPosition(54, 44);
    copiesInput->setPercentW(38);
    
    isbnInput = new InputBar("ISBN", {0, 0}, 40, 3);
    isbnInput->setPercentPosition(8, 55);
    isbnInput->setPercentW(84);
    
    // Action buttons - vertical layout for proper navigation
    saveBtn = new Button("[ Salvar ]", {0, 0}, 16, 3);
    saveBtn->setPercentPosition(50, 68);
    saveBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    deleteBtn = new Button("[ Excluir ]", {0, 0}, 16, 3);
    deleteBtn->setPercentPosition(50, 78);
    deleteBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    deleteBtn->setStyle({{255, 100, 100, 255}, BACKGROUND, {255, 150, 150, 255}, BACKGROUND});
    
    cancelBtn = new Button("[ Fechar ]", {0, 0}, 16, 3);
    cancelBtn->setPercentPosition(50, 88);
    cancelBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    // Add elements
    modalContainer->addElement(titleText);
    modalContainer->addElement(idInput);
    modalContainer->addElement(loadBtn);
    modalContainer->addElement(titleInput);
    modalContainer->addElement(authorInput);
    modalContainer->addElement(yearInput);
    modalContainer->addElement(copiesInput);
    modalContainer->addElement(isbnInput);
    modalContainer->addElement(saveBtn);
    modalContainer->addElement(deleteBtn);
    modalContainer->addElement(cancelBtn);
    
    // Callbacks
    loadBtn->onClickHandler = [this](element&, TUImanager&) { handleLoad(); };
    saveBtn->onClickHandler = [this](element&, TUImanager&) { handleSave(); };
    deleteBtn->onClickHandler = [this](element&, TUImanager&) { handleDelete(); };
}

BookEditModal::~BookEditModal() {
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
    delete modalContainer;
}

void BookEditModal::open(TUImanager& tui, container*) {
    isOpen_ = true;
    clearFields();
    tui.focusContainer(modalContainer, 0);
}

void BookEditModal::close(TUImanager& tui, container* returnTo) {
    isOpen_ = false;
    tui.focusContainer(returnTo, 0);
}

void BookEditModal::clearFields() {
    idInput->text.clear();
    titleInput->text.clear();
    authorInput->text.clear();
    yearInput->text.clear();
    isbnInput->text.clear();
    copiesInput->text.clear();
    currentBookId = 0;
}

void BookEditModal::handleLoad() {
    if (idInput->text.empty()) {
        setError("Digite um ID");
        return;
    }
    
    std::optional<app::models::Book> bookOpt;
    
    // Try to find by hash ID first (e.g., "BK-NcaKQQ")
    if (idInput->text.find("BK-") == 0) {
        bookOpt = bookRepo->findByHashId(idInput->text);
    } else {
        // Try numeric ID
        try {
            int64_t id = std::stoll(idInput->text);
            bookOpt = bookRepo->findById(id);
        } catch (...) {

            bookOpt = bookRepo->findByHashId(idInput->text);
        }
    }
    
    if (!bookOpt) {
        setError("Livro não encontrado");
        return;
    }
    
    currentBookId = bookOpt->id;
    titleInput->text = bookOpt->title;
    authorInput->text = bookOpt->author;
    yearInput->text = bookOpt->published_year ? std::to_string(*bookOpt->published_year) : "";
    isbnInput->text = bookOpt->isbn.value_or("");
    copiesInput->text = std::to_string(bookOpt->copies_available);
    setSuccess("Carregado: " + bookOpt->title);
}

void BookEditModal::handleSave() {
    if (currentBookId == 0) {
        setError("Busque um livro primeiro");
        return;
    }
    if (titleInput->text.empty() || authorInput->text.empty()) {
        setError("Título e autor são obrigatórios");
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
            setSuccess("Atualizado com sucesso!");
        } else {
            setError("Falha ao atualizar");
        }
    } catch (const std::exception& ex) {
        setError(ex.what());
    }
}

void BookEditModal::handleDelete() {
    if (currentBookId == 0) {
        setError("Busque um livro primeiro");
        return;
    }
    
    try {
        if (bookRepo->deleteById(currentBookId)) {
            setSuccess("Livro excluído!");
            clearFields();
        } else {
            setError("Falha ao excluir");
        }
    } catch (const std::exception& ex) {
        setError(ex.what());
    }
}

} // namespace ui
