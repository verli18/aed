#pragma once

#include "ui/modals/base_modal.hpp"

namespace ui {

class StudentRegistrationModal : public BaseModal {
public:
    InputBar* nameInput;
    InputBar* regNumberInput;
    InputBar* emailInput;
    InputBar* phoneInput;
    Button* submitBtn;
    Text* titleText;
    
    app::repos::StudentRepository* studentRepo;
    
    StudentRegistrationModal(TUImanager& tui, app::repos::StudentRepository* repo);
    ~StudentRegistrationModal();
    
    void open(TUImanager& tui, container* returnTo) override;
    void close(TUImanager& tui, container* returnTo) override;
    void handleSubmit();
};

class StudentEditModal : public BaseModal {
public:
    InputBar* searchInput;  // Search by registration number
    InputBar* nameInput;
    InputBar* regNumberInput;
    InputBar* emailInput;
    InputBar* phoneInput;
    Button* loadBtn;
    Button* saveBtn;
    Button* deleteBtn;
    Text* titleText;
    
    app::repos::StudentRepository* studentRepo;
    int64_t currentStudentId = 0;
    
    StudentEditModal(TUImanager& tui, app::repos::StudentRepository* repo);
    ~StudentEditModal();
    
    void open(TUImanager& tui, container* returnTo) override;
    void close(TUImanager& tui, container* returnTo) override;
    void clearFields();
    void handleLoad();
    void handleSave();
    void handleDelete();
};

} // namespace ui
