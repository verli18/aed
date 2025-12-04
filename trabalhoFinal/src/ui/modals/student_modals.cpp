#include "ui/modals/student_modals.hpp"

namespace ui {


StudentRegistrationModal::StudentRegistrationModal(TUImanager& tui, app::repos::StudentRepository* repo)
    : studentRepo(repo) {
    
    modalContainer = createModalContainer(tui, 50, 22, "Registrar Estudante");
    
    titleText = new Text("Preencha os dados do estudante:", {0, 0});
    titleText->setPercentPosition(5, 5);
    
    nameInput = new InputBar("Nome *", {0, 0}, 40, 3);
    nameInput->setPercentPosition(10, 18);
    nameInput->setPercentW(80);
    
    regNumberInput = new InputBar("Matrícula *", {0, 0}, 40, 3);
    regNumberInput->setPercentPosition(10, 32);
    regNumberInput->setPercentW(80);
    
    emailInput = new InputBar("Email", {0, 0}, 40, 3);
    emailInput->setPercentPosition(10, 46);
    emailInput->setPercentW(80);
    
    phoneInput = new InputBar("Telefone", {0, 0}, 40, 3);
    phoneInput->setPercentPosition(10, 60);
    phoneInput->setPercentW(80);
    
    // Buttons - stacked vertically for proper navigation
    submitBtn = new Button("[ Confirmar ]", {0, 0}, 18, 3);
    submitBtn->setPercentPosition(50, 74);
    submitBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    cancelBtn = new Button("[ Cancelar ]", {0, 0}, 18, 3);
    cancelBtn->setPercentPosition(50, 84);
    cancelBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    // Add elements
    modalContainer->addElement(titleText);
    modalContainer->addElement(nameInput);
    modalContainer->addElement(regNumberInput);
    modalContainer->addElement(emailInput);
    modalContainer->addElement(phoneInput);
    modalContainer->addElement(submitBtn);
    modalContainer->addElement(cancelBtn);

    submitBtn->onClickHandler = [this](element&, TUImanager&) {
        this->handleSubmit();
    };
}

StudentRegistrationModal::~StudentRegistrationModal() {
    delete titleText;
    delete nameInput;
    delete regNumberInput;
    delete emailInput;
    delete phoneInput;
    delete submitBtn;
    delete cancelBtn;
    delete modalContainer;
}

void StudentRegistrationModal::open(TUImanager& tui, container*) {
    isOpen_ = true;
    nameInput->text.clear();
    regNumberInput->text.clear();
    emailInput->text.clear();
    phoneInput->text.clear();
    tui.focusContainer(modalContainer, 0);
}

void StudentRegistrationModal::close(TUImanager& tui, container* returnTo) {
    isOpen_ = false;
    tui.focusContainer(returnTo, 0);
}

void StudentRegistrationModal::handleSubmit() {
    if (nameInput->text.empty()) {
        setError("Nome é obrigatório");
        return;
    }
    if (regNumberInput->text.empty()) {
        setError("Matrícula é obrigatória");
        return;
    }
    
    try {
        std::optional<std::string> email = emailInput->text.empty() 
            ? std::nullopt : std::optional<std::string>(emailInput->text);
        std::optional<std::string> phone = phoneInput->text.empty() 
            ? std::nullopt : std::optional<std::string>(phoneInput->text);
        
        app::models::Student student(nameInput->text, regNumberInput->text, email, phone);
        int64_t newId = studentRepo->create(student);
        
        setSuccess("Estudante registrado! ID: " + std::to_string(newId));
    } catch (const std::exception& ex) {
        setError(ex.what());
    }
}

StudentEditModal::StudentEditModal(TUImanager& tui, app::repos::StudentRepository* repo)
    : studentRepo(repo) {
    
    modalContainer = createModalContainer(tui, 55, 40, "Editar/Excluir Estudante");
    
    titleText = new Text("Busque por matrícula:", {0, 0});
    titleText->setPercentPosition(5, 4);
    
    // Search row
    searchInput = new InputBar("Matrícula", {0, 0}, 30, 3);
    searchInput->setPercentPosition(8, 12);
    searchInput->setPercentW(55);
    
    loadBtn = new Button("Buscar", {0, 0}, 12, 3);
    loadBtn->setPercentPosition(70, 12);
    
    // Edit fields
    nameInput = new InputBar("Nome *", {0, 0}, 40, 3);
    nameInput->setPercentPosition(8, 26);
    nameInput->setPercentW(84);
    
    regNumberInput = new InputBar("Matrícula *", {0, 0}, 40, 3);
    regNumberInput->setPercentPosition(8, 38);
    regNumberInput->setPercentW(84);
    
    emailInput = new InputBar("Email", {0, 0}, 40, 3);
    emailInput->setPercentPosition(8, 50);
    emailInput->setPercentW(84);
    
    phoneInput = new InputBar("Telefone", {0, 0}, 40, 3);
    phoneInput->setPercentPosition(8, 62);
    phoneInput->setPercentW(84);
    
    // Action buttons - vertical layout for intuitive navigation
    saveBtn = new Button("[ Salvar ]", {0, 0}, 18, 3);
    saveBtn->setPercentPosition(50, 74);
    saveBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    deleteBtn = new Button("[ Excluir ]", {0, 0}, 18, 3);
    deleteBtn->setPercentPosition(50, 82);
    deleteBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    deleteBtn->setStyle({{255, 100, 100, 255}, BACKGROUND, {255, 150, 150, 255}, BACKGROUND});
    
    cancelBtn = new Button("[ Fechar ]", {0, 0}, 18, 3);
    cancelBtn->setPercentPosition(50, 90);
    cancelBtn->setAnchors(element::AnchorX::Center, element::AnchorY::Top);
    
    // Add elements
    modalContainer->addElement(titleText);
    modalContainer->addElement(searchInput);
    modalContainer->addElement(loadBtn);
    modalContainer->addElement(nameInput);
    modalContainer->addElement(regNumberInput);
    modalContainer->addElement(emailInput);
    modalContainer->addElement(phoneInput);
    modalContainer->addElement(saveBtn);
    modalContainer->addElement(deleteBtn);
    modalContainer->addElement(cancelBtn);
    
    // Callbacks
    loadBtn->onClickHandler = [this](element&, TUImanager&) { handleLoad(); };
    saveBtn->onClickHandler = [this](element&, TUImanager&) { handleSave(); };
    deleteBtn->onClickHandler = [this](element&, TUImanager&) { handleDelete(); };
}

StudentEditModal::~StudentEditModal() {
    delete titleText;
    delete searchInput;
    delete loadBtn;
    delete nameInput;
    delete regNumberInput;
    delete emailInput;
    delete phoneInput;
    delete saveBtn;
    delete deleteBtn;
    delete cancelBtn;
    delete modalContainer;
}

void StudentEditModal::open(TUImanager& tui, container*) {
    isOpen_ = true;
    clearFields();
    tui.focusContainer(modalContainer, 0);
}

void StudentEditModal::close(TUImanager& tui, container* returnTo) {
    isOpen_ = false;
    tui.focusContainer(returnTo, 0);
}

void StudentEditModal::clearFields() {
    searchInput->text.clear();
    nameInput->text.clear();
    regNumberInput->text.clear();
    emailInput->text.clear();
    phoneInput->text.clear();
    currentStudentId = 0;
}

void StudentEditModal::handleLoad() {
    if (searchInput->text.empty()) {
        setError("Digite uma matrícula ou ID");
        return;
    }
    
    std::optional<app::models::Student> studentOpt;
    
    // Try to find by hash ID first (e.g., "ST-abc123")
    if (searchInput->text.find("ST-") == 0) {
        studentOpt = studentRepo->findByHashId(searchInput->text);
    } else {
        // Try by registration number
        studentOpt = studentRepo->findByRegistrationNumber(searchInput->text);
        if (!studentOpt) {
            // Also try as hash ID in case prefix was omitted
            studentOpt = studentRepo->findByHashId(searchInput->text);
        }
    }
    
    if (!studentOpt) {
        setError("Estudante não encontrado");
        return;
    }
    
    currentStudentId = studentOpt->id;
    nameInput->text = studentOpt->name;
    regNumberInput->text = studentOpt->registration_number;
    emailInput->text = studentOpt->email.value_or("");
    phoneInput->text = studentOpt->phone.value_or("");
    setSuccess("Carregado: " + studentOpt->name);
}

void StudentEditModal::handleSave() {
    if (currentStudentId == 0) {
        setError("Busque um estudante primeiro");
        return;
    }
    if (nameInput->text.empty() || regNumberInput->text.empty()) {
        setError("Nome e matrícula são obrigatórios");
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
            setSuccess("Atualizado com sucesso!");
        } else {
            setError("Falha ao atualizar");
        }
    } catch (const std::exception& ex) {
        setError(ex.what());
    }
}

void StudentEditModal::handleDelete() {
    if (currentStudentId == 0) {
        setError("Busque um estudante primeiro");
        return;
    }
    
    try {
        // Soft delete to preserve loan history
        if (studentRepo->deactivate(currentStudentId)) {
            setSuccess("Estudante desativado!");
            clearFields();
        } else {
            setError("Falha ao desativar");
        }
    } catch (const std::exception& ex) {
        setError(ex.what());
    }
}

} // namespace ui
