#pragma once

#include "ui/ui_common.hpp"

namespace ui {

/// Base class for modal dialogs - provides common structure
class BaseModal {
public:
    container* modalContainer = nullptr;
    Button* cancelBtn = nullptr;
    bool isOpen_ = false;
    
    virtual ~BaseModal() = default;
    
    bool isOpen() const { return isOpen_; }
    
    virtual void open(TUImanager& tui, container* returnTo) = 0;
    virtual void close(TUImanager& tui, container* returnTo) = 0;
    
    void render(TUImanager& tui) {
        if (isOpen_) {
            modalContainer->render(tui);
        }
    }
    
protected:
    void setError(const std::string& msg) {
        notifyError(msg);
    }
    
    void setSuccess(const std::string& msg) {
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
