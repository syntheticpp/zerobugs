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

class Fl_Widget;
class Fl_Window;


class FlDialog : public ui::Dialog
{
public:
    FlDialog(ui::Controller&, int x, int y, int w, int hi, const char* = nullptr);

    ~FlDialog();

protected:
    void center();

    virtual void close() {
        delete this;
    }

    void hide();
    void set_resizable();
    void show();

private:
    static void close_callback(Fl_Widget*, void*);

    std::unique_ptr<Fl_Window> window_;
};


#endif // FLDIALOG_H__FD2A8E2A_CB60_41CD_BBF5_CACFCA2217CB

