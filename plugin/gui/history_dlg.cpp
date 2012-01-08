//
// $Id: history_dlg.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Display a history of debugged programs. User can remove entries from
// history, or load a program in the debugger.
//
#include "config.h"
#ifdef HAVE_SYS_STAT_H
 #include <sys/stat.h>
#endif
#include <signal.h>
#include <cstdlib>
#include <vector>
#include "gtkmm/button.h"
#include "gtkmm/color.h"
#include "gtkmm/connect.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/flags.h"
#include "gtkmm/clist.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/separator.h"
#include "gtkmm/stock.h"
#include "zdk/check_ptr.h"
#include "entry_dialog.h"
#include "env_dialog.h"
#include "history_dlg.h"
#include "main_window.h"
#include "popup_menu.h"

using namespace std;
using namespace Gtk;


////////////////////////////////////////////////////////////////
BEGIN_SLOT(MainWindow::on_menu_history,())
{
    RefPtr<History> hist = interface_cast<History*>(
        CHKPTR(debugger().properties())->get_object("hist"));

    const char* current = 0;
    if (RefPtr<Thread> thread = current_thread())
    {
        current = thread->filename();
    }
    HistoryDialog dlg(debugger(), hist, "Recent Programs", current);

    dlg.execute.connect(Gtk_SLOT(this, &MainWindow::on_execute));
    dlg.attach_to_program.connect(Gtk_SLOT(this, &MainWindow::do_attach));
    dlg.set_transient_for(*this);
    dlg.run(this);

    compute_and_update_state(current_thread());
}
END_SLOT()


////////////////////////////////////////////////////////////////////////////////
HistoryDialog::HistoryDialog
(
    Debugger& debugger,
    const RefPtr<History>& hist,
    const char* title,  // dialog's title
    const char* current // path to current program
)
  : DialogBox(btn_none, title)
  , debugger_(debugger)
  , list_(0)
  , history_(hist)
  , index_(-1)  // nothing selected
  , btnExec_(0)
  , btnAttach_(0)
  , btnDelete_(0)
  , btnLoad_(0)
  , btnClearAll_(0)
{
    set_has_separator();
    if (current)
    {
        current_ = current;
    }
    Gtk::HBox* hbox = manage(new Gtk::HBox);
    get_vbox()->pack_start(*hbox);

    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow);
    hbox->pack_start(*sw);

    Gtk::VSeparator* sep = manage(new Gtk::VSeparator);
    hbox->pack_start(*sep, false, false);

    Gtk::ButtonBox* bbox = manage(new Gtk::VButtonBox);
    hbox->pack_end(*bbox, false, false);
    bbox->set_spacing(3);
    bbox->set_layout(Gtk_FLAG(BUTTONBOX_START));

    btnClearAll_ = add_button("Reset History And _Settings", get_action_area());

    // Gtk::Button* btn = add_button("_Close", get_action_area());
    Gtk::Button* btn = add_button(Gtk::Stock::CLOSE, get_action_area());
    Gtk_CONNECT_1(btn, clicked, this, &HistoryDialog::on_button, btn_close);

    btnExec_ = add_button(Gtk::Stock::EXECUTE, bbox);
    btnAttach_ = add_button("_Attach", bbox);
    btnLoad_ = add_button("_Load Core", bbox);
    btnDelete_ = add_button(Gtk::Stock::DELETE, bbox);

    gray_out_all_buttons();

    Gtk_CONNECT_0(btnExec_, clicked, this, &HistoryDialog::on_exec);
    Gtk_CONNECT_0(btnAttach_, clicked, this, &HistoryDialog::on_attach);
    Gtk_CONNECT_0(btnDelete_, clicked, this, &HistoryDialog::on_delete);
    Gtk_CONNECT_0(btnLoad_, clicked, this, &HistoryDialog::on_exec);
    Gtk_CONNECT_0(btnClearAll_, clicked, this, &HistoryDialog::on_reset_all);

    sw->set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));
    Gtk_set_size(hbox, 600, 240);

    static const char* columns[] =
    {
        "Path", "Last Debugged", "Last Modified", 0
    };
    list_ = manage(new List(columns));
    Gtk_add_with_viewport(sw, *list_);

    list_->column(0).set_width(200);
    list_->column(1).set_width(170);
    list_->column(2).set_width(170);
    list_->column(0).set_passive();
    list_->column(1).set_passive();
    list_->column(2).set_passive();

#ifdef GTKMM_2
    list_->signal_cursor_changed().connect(
        sigc::mem_fun(this, &HistoryDialog::on_selection_change));
#else
    list_->set_selection_mode(Gtk_FLAG(SELECTION_SINGLE));
    list_->select_row.connect(slot(this, &HistoryDialog::on_selection_change));
    list_->unselect_row.connect(slot(this, &HistoryDialog::on_selection_change));

    get_action_area()->destroy();
#endif
    Gtk_set_resizable(this, true);
}


DialogBox::ButtonID HistoryDialog::run(const Widget* parent)
{
    populate_list();
    return DialogBox::run(parent);
}


void HistoryDialog::populate_list()
{
    static const char* timefmt = "%a %b %d %Y %X";

    if (history_)
    {
        const size_t count = history_->entry_count();
        for (size_t i = 0; i != count; ++i)
        {
            const HistoryEntry* entry = history_->entry(i);

            vector<string> cols;
            cols.push_back(entry->name());

            char timebuf[512];
            time_t t = entry->last_debugged();
            struct tm* loct = localtime(&t);
            strftime(timebuf, 512, timefmt, loct);
            cols.push_back(timebuf);

            struct stat st;
            if (stat(entry->name(), &st) < 0)
            {
                cols.push_back("n/a");
            }
            else
            {
                loct = localtime(&st.st_mtime);
                strftime(timebuf, 512, timefmt, loct);
                cols.push_back(timebuf);
            }
            list_->rows().push_back(cols);
            CList::Row row = list_->rows().back();
            row.set_data((gpointer)entry);

            if (current_ == entry->name())
            {
                row.set_selectable(false);
                row.set_background(Gdk_Color("green"));
            }
        }
    }
}


void HistoryDialog::gray_out_all_buttons()
{
    btnExec_->set_sensitive(false);
    btnAttach_->set_sensitive(false);
    btnDelete_->set_sensitive(false);
    btnLoad_->set_sensitive(false);
}


static inline bool process_exists(pid_t pid)
{
    return pid && (kill(pid, 0) == 0);
}


#ifndef GTKMM_2
BEGIN_SLOT(HistoryDialog::on_selection_change,(int nrow, int, GdkEvent*))
{
#else
BEGIN_SLOT(HistoryDialog::on_selection_change,())
{
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column = NULL;

    list_->get_cursor(path, column);
    TreeIter iter = list_->get_iter(path);
    if (!iter)
    {
        return;
    }

    int nrow = Gtk::get_row_num(path);

#endif // GTKMM_2
    gray_out_all_buttons();

    CList::SelectionList selection = CHKPTR(list_)->selection();
    if (selection.empty())
    {
        entry_.reset();
        index_ = -1;
    }
    else
    {
        assert(selection.size() == 1);
#ifdef GTKMM_2
        CList::Row row(get_list_row(*iter, *list_));
#else
        CList::Row row(get_list_row(*selection.begin(), *list_));
#endif
        HistoryEntry* entry = (HistoryEntry*)CHKPTR(row.get_data());

        entry_ = entry;
        index_ = nrow;

        if (process_exists(entry->pid()) && current_ != entry->name())
        {
            btnAttach_->set_sensitive(true);
        }
        if (entry->is_live())
        {
            btnExec_->set_sensitive(true);
        }
        else
        {
            btnLoad_->set_sensitive(true);
        }
        btnDelete_->set_sensitive(true);
    }
}
END_SLOT()


BEGIN_SLOT(HistoryDialog::on_attach,())
{
    assert(index_ >= 0);
    assert(entry_.get());

    attach_to_program(entry_->pid(), entry_->target_param());
    on_button(btn_ok);
}
END_SLOT()


BEGIN_SLOT(HistoryDialog::on_delete,())
{
    assert(index_ >= 0);
    assert(entry_);
// NOTE: we assume that the list shows the history
// entries in the same order they are stored internally
// -- this is why the list columns are passive, so that
// we cannot resort the list
    history_->remove_entry(index_);
    CHKPTR(list_)->remove_row(index_);
}
END_SLOT()


BEGIN_SLOT(HistoryDialog::on_exec,())
{
    assert(index_ >= 0);
    assert(entry_);

    on_button(btn_ok);

    execute(entry_.get());
}
END_SLOT()


BEGIN_SLOT(HistoryDialog::on_reset_all,())
{
    debugger_.reset_properties();
    on_button(btn_ok);
}
END_SLOT()


event_result_t
HistoryDialog::on_button_press_event(GdkEventButton* event)
{
    if ((event != NULL)                   &&
        (event->type == GDK_BUTTON_PRESS) &&
        (event->button == 3))
    {
        Gtk::RowHandle row;
        int col = 0; // not used

        const int x = static_cast<int>(event->x);
        const int y = static_cast<int>(event->y);

        if (list_ && list_->get_selection_info(x, y, &row, &col))
        {
            if (gpointer data = list_->row(row).get_data())
            {
                HistoryEntry* entry = reinterpret_cast<HistoryEntry*>(data);
                popup_menu(*event, *entry);
            }
        }
    }
    return 0;
}


void
HistoryDialog::popup_menu(GdkEventButton& event, HistoryEntry& entry)
{
    std::auto_ptr<PopupMenu> menu(new PopupMenu);
    Gtk::MenuItem* item =
        menu->add_manage_item(new Gtk::MenuItem("Environment..."));
    Gtk_CONNECT_1(item,
                  activate,
                  this,
                  &HistoryDialog::edit_environment,
                  &entry);
    item = menu->add_manage_item(new Gtk::MenuItem("Command Line..."));
    Gtk_CONNECT_1(item,
                  activate,
                  this,
                  &HistoryDialog::edit_command_line,
                  &entry);
    menu.release()->popup(event.button, event.time);
}


///
/// Helper for HistoryDialog::edit_environment
///
class ZDK_LOCAL HistoryEnvironment : public Environment
{
    HistoryEntry& entry_;

public:
    explicit HistoryEnvironment(HistoryEntry& entry) : entry_(entry) { }

    const char* const* get(bool reset)
    {
        if (reset)
        {
            entry_.set_environ(environ);
        }
        return entry_.environ();
    }
    void set(const char* const* env)
    {
        entry_.set_environ(env);
    }
};


void HistoryDialog::edit_environment(HistoryEntry* entry)
{
    if (entry)
    {
        HistoryEnvironment env(*entry);
        EnvDialog dlg(env);
        dlg.set_transient_for(*this);
        dlg.run();
    }
}


void HistoryDialog::edit_command_line(HistoryEntry* entry)
{
    if (entry)
    {
        EntryDialog dlg("Edit Command Line Arguments",
                        debugger_.properties(),
                        "Command Line");
        dlg.text_entry()->set_text(entry->command_line());
        Gtk_set_size(dlg.text_entry(), 400, -1);
        dlg.set_transient_for(*this);
        string args = dlg.run();
        if (!args.empty())
        {
            entry->set_command_line(args.c_str());
        }
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
