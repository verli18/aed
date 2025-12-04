#pragma once

#include "ui/ui_common.hpp"

namespace ui {

/// Base class for modal dialogs - provides common structure
class BaseModal {
public:
    container* modalContainer = nullptr;
    Text* statusText = nullptr;
    Button* cancelBtn = nullptr;
    ModalState state;
    
    virtual ~BaseModal() = default;
    
    bool isOpen() const { return state.isOpen; }
    
    virtual void open(TUImanager& tui, container* returnTo) = 0;
    virtual void close(TUImanager& tui, container* returnTo) = 0;
    
    void render(TUImanager& tui) {
        if (state.isOpen) {
            modalContainer->render(tui);
        }
    }
    
protected:
    // Set status text in modal AND push notification
    void setStatus(const std::string& msg, color fg) {
        if (statusText) {
            setStatusText(statusText, msg, fg);
        }
    }
    
    void setError(const std::string& msg) {
        setStatus("Erro: " + msg, COLOR_ERROR);
        notifyError(msg);
    }
    
    void setSuccess(const std::string& msg) {
        setStatus(msg, COLOR_SUCCESS);
        notifySuccess(msg);
    }
    
    // Create a standard modal container
    static container* createModalContainer(TUImanager& tui, int w, int h, const std::string& title, int zIndex = 100) {
        int x = tui.cols / 2 - w / 2;
        int y = tui.rows / 2 - h / 2;
        auto* c = new container({x, y}, {w, h}, defaultModalStyle(), title);
        c->setZIndex(zIndex);
        c->setDefaultElementStyle(defaultModalStyle());
        c->setInheritStyle(true);
        return c;
    }
};

} // namespace ui
