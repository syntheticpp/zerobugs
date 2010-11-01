#ifndef PROGRESS_BOX_H__92EA368F_E4D8_4CFB_BA83_788BBEA9802D
#define PROGRESS_BOX_H__92EA368F_E4D8_4CFB_BA83_788BBEA9802D
//
// $Id: progress_box.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <string>
#include "zdk/mutex.h"
#include "dialog_box.h"


namespace Gtk
{
    class Label;
    class Window;
}


class ProgressBox : public DialogBox
{
    class Bar;

public:
    explicit ProgressBox(
        const std::string& msg,
        const char* title = "Please wait");

    void update(Gtk::Window&, const std::string&, double);

    bool cancelled() const;

    bool done() const;

    void reset();

protected:
    virtual void close_dialog();

    void run();

    event_result_t delete_event_impl(GdkEventAny*);

private:
    Gtk::Label*     label_;
    Bar*            bar_;
    mutable Mutex   mutex_;
    bool            cancelled_;
    bool            done_;
};


#endif // PROGRESS_BOX_H__92EA368F_E4D8_4CFB_BA83_788BBEA9802D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
