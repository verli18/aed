#pragma once

#include "ui/modals/base_modal.hpp"

namespace ui {

class LoanCreationModal : public BaseModal {
public:
    InputBar* studentRegInput;
    InputBar* bookIdInput;
    InputBar* dueDaysInput;
    Button* submitBtn;
    Text* titleText;
    
    app::repos::StudentRepository* studentRepo;
    app::repos::BookRepository* bookRepo;
    app::repos::LoanRepository* loanRepo;
    
    LoanCreationModal(TUImanager& tui,
                      app::repos::StudentRepository* sRepo,
                      app::repos::BookRepository* bRepo,
                      app::repos::LoanRepository* lRepo);
    ~LoanCreationModal();
    
    void open(TUImanager& tui, container* returnTo) override;
    void close(TUImanager& tui, container* returnTo) override;
    bool handleSubmit();  // Returns true on success
};

class LoanReturnModal : public BaseModal {
public:
    InputBar* loanIdInput;
    Button* submitBtn;
    Text* titleText;
    
    app::repos::LoanRepository* loanRepo;
    app::repos::BookRepository* bookRepo;
    
    LoanReturnModal(TUImanager& tui,
                    app::repos::LoanRepository* lRepo,
                    app::repos::BookRepository* bRepo);
    ~LoanReturnModal();
    
    void open(TUImanager& tui, container* returnTo) override;
    void close(TUImanager& tui, container* returnTo) override;
    bool handleSubmit();  // Returns true on success
};

} // namespace ui
