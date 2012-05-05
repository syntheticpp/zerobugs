#ifndef FLDIALOG_H__FD2A8E2A_CB60_41CD_BBF5_CACFCA2217CB
#define FLDIALOG_H__FD2A8E2A_CB60_41CD_BBF5_CACFCA2217CB
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "dialog.h"
#include <memory>

class Fl_Group;
class Fl_Widget;
class Fl_Window;

typedef void (Fl_Callback)(Fl_Widget*, void*);


/**
 * Base class for dialogs. Implemented as a top-level Fl_Window
 * containing a Fl_Group. All dialog elements are children of
 * the group.
 */
class FlDialog : public ui::Dialog
{
public:
    FlDialog(ui::Controller&, int x, int y, int w, int h, const char* = nullptr);

    ~FlDialog();

protected:
    template<typename F>
    void add_button(int x, int y, int w, int h, const char* label, F f) {
        add_action(label, f);
        add_button(x, y, w, h, label, action_callback);
    }

    void add_button(int x, int y, int w, int h, const char* label, Fl_Callback);

    void center();

    void close_impl();

    void set_resizable(int minWidth, int minHeight);

    void show(bool);

    Fl_Group* group() {
        return group_;
    }

    const Fl_Window& window() const {
        assert(window_);
        return *window_;
    }

private:
    static void action_callback(Fl_Widget*, void*);
    static void close_callback(Fl_Widget*, void*);

    // dialog owns the window
    std::unique_ptr<Fl_Window>  window_;
    Fl_Group*                   group_;
};


#endif // FLDIALOG_H__FD2A8E2A_CB60_41CD_BBF5_CACFCA2217CB

