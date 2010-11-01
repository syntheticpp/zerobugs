//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: update.cpp 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "config.h"
#include "dialog_box.h"
#include "html_view.h"
#include "main_window.h"
#include "slot_macros.h"
#include "gtkmm/resize.h"
#include "zdk/update.h"
#include <sstream>
#include <time.h>


using namespace std;

CLASS UpdateView : public HTMLView
{
    bool clicked_;

public:
    UpdateView() : clicked_(false) { }

    void on_click(string url)
    {
        clicked_ = true;
        HTMLView::on_click(url);
    }

    bool url_clicked() const { return clicked_; }
};


CLASS UpdateDialog : public DialogBox, public EnumCallback<Update*>
{
    UpdateView* htmlView_;
    string      src_;

public:
    UpdateDialog()
        : DialogBox(btn_close, "Update")
        , htmlView_(manage(new UpdateView))
    {
        get_vbox()->add(*htmlView_);
        Gtk_set_size(htmlView_, 400, 180);
    }

    void notify(Update* update)
    {
        if (update)
        {
            if (!src_.empty())
            {
                src_ += "<hr/>";
            }
            src_ += update->url();
            src_ += " ";
            src_ += update->description();
        }
    }

    void show_updates(size_t count)
    {
        ostringstream html;

        html << "<html>" << count << " update";
        html << (count > 1 ? "s are" : " is");
        html << " available for your product: <br/><br/>";
        html << src_ << "</html>";
        src_ = html.str();
        HTMLView::make_hrefs(src_);
        htmlView_->source(src_);
        htmlView_->show_all();
        htmlView_->grab_focus();
    }

    bool url_clicked() const
    {
        return htmlView_ ? htmlView_->url_clicked() : false;
    }
};


BEGIN_SLOT(MainWindow::on_menu_check_for_updates,())
{
    bool markCurrentTime = false;
    UpdateDialog dlg;

    if (size_t count = interface_cast<Updateable&>(debugger()).check_for_updates(&dlg))
    {
        dlg.set_transient_for(*this);
        dlg.show_updates(count);
        dlg.run(this);

        if (dlg.url_clicked())
        {
            markCurrentTime = true;
        }
    }
    else
    {
        info_message("Your product is up to date");
        markCurrentTime = true;
    }
    if (markCurrentTime)
    {
        //save the current time; compute_and_update_state will gray out the
        //"Check for Updates" menu to prevent checking more than once a day
        debugger().properties()->set_word("upchk", time(NULL));
    }
    compute_and_update_state(NULL);
}
END_SLOT()
