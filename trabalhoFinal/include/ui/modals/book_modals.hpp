#pragma once

#include "ui/modals/base_modal.hpp"

namespace ui {

class BookRegistrationModal : public BaseModal {
public:
    InputBar* titleInput;
    InputBar* authorInput;
    InputBar* yearInput;
    InputBar* isbnInput;
    InputBar* copiesInput;
    Button* submitBtn;
    Text* titleText;
    
    app::repos::BookRepository* bookRepo;
    
    BookRegistrationModal(TUImanager& tui, app::repos::BookRepository* repo);
    ~BookRegistrationModal();
    
    void open(TUImanager& tui, container* returnTo) override;
    void close(TUImanager& tui, container* returnTo) override;
    void handleSubmit();
};

class BookEditModal : public BaseModal {
public:
    InputBar* idInput;
    InputBar* titleInput;
    InputBar* authorInput;
    InputBar* yearInput;
    InputBar* isbnInput;
    InputBar* copiesInput;
    Button* loadBtn;
    Button* saveBtn;
    Button* deleteBtn;
    Text* titleText;
    
    app::repos::BookRepository* bookRepo;
    int64_t currentBookId = 0;
    
    BookEditModal(TUImanager& tui, app::repos::BookRepository* repo);
    ~BookEditModal();
    
    void open(TUImanager& tui, container* returnTo) override;
    void close(TUImanager& tui, container* returnTo) override;
    void clearFields();
    void handleLoad();
    void handleSave();
    void handleDelete();
};

} // namespace ui
