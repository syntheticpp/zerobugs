#ifndef FLDIALOG_H__FD2A8E2A_CB60_41CD_BBF5_CACFCA2217CB
#define FLDIALOG_H__FD2A8E2A_CB60_41CD_BBF5_CACFCA2217CB
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "dialog.h"
#include <memory>

class Fl_Group;
class Fl_Widget;
class Fl_Window;


/**
 * Base class for dialogs. Implemented as a top-level Fl_Window
 * containing a Fl_Group. All dialog elements are children of
 * the group.
 */
class FlDialog : public ui::Dialog
{
public:
    FlDialog(ui::Controller&, int x, int y, int w, int hi, const char* = nullptr);

    ~FlDialog();

protected:
    void center();

    void set_resizable(int minWidth, int minHeight);

    void show(bool);

    Fl_Group* group() {
        return group_;
    }
private:
    static void close_callback(Fl_Widget*, void*);

    // dialog owns the window
    std::unique_ptr<Fl_Window> window_;
    Fl_Group* group_;
};


#endif // FLDIALOG_H__FD2A8E2A_CB60_41CD_BBF5_CACFCA2217CB

