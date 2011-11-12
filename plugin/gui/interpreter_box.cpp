// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: interpreter_box.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#if defined (GTKMM_2)
#include <assert.h>
#include <ctype.h>
#include <iostream>
#include <stdexcept>
#include <gdk/gdkkeysyms.h>
#include "zdk/zero.h"
#include "generic/temporary.h"
#include "app_slots.h"
#include "interpreter_box.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/text.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "text_mark.h"
#include "text_entry.h"
#include "toolbar.h"


using namespace std;

/**
 * A minimalistic console
 */
class ZDK_LOCAL Console
    : public Gtk::Text
    , public RefCountedImpl<Interpreter::Output>
{
    AppSlots* app_;
    RefPtr<Interpreter> interp_;
    Glib::RefPtr<Gtk::SourceBuffer> buf_;
    Glib::RefPtr<Gtk::TextTagTable> tags_;
    Glib::RefPtr<Gtk::TextBuffer::Mark> mark_;

    vector<string> history_;
    size_t hpos_;

    string output_;
    TextEntry* entry_; // for command history

private:
    event_result_t on_key_press_event(GdkEventKey*);
    event_result_t on_button_press_event(GdkEventButton*);

    /**
     * handle carriage return
     */
    bool enter_line(GdkEventKey*,
                    Gtk::TextBuffer::iterator,
                    Gtk::TextBuffer::iterator);

    Gtk::TextBuffer::iterator get_insert_iter()
    {
        return buf_->get_iter_at_mark(buf_->get_insert());
    }

    Gtk::TextBuffer::iterator move_mark_to_cursor()
    {
        buf_->move_mark(buf_->get_insert(), buf_->end());

        Gtk::TextBuffer::iterator i = get_insert_iter();
        buf_->move_mark(mark_, i);
        scroll_to(mark_);

        return i;
    }

    // Interpreter::Output interface
    void print(const char* text, size_t len);

public:
    ~Console() throw()
    { }

    Console(AppSlots* app, Interpreter* interp)
        : app_(app)
        , interp_(interp)
        , buf_(get_buffer())
        , hpos_(0)
        , entry_(0)
    {
        if (!buf_)
        {
            throw runtime_error(string(interp_->name()) + ": null text buffer");
        }
        tags_ = buf_->get_tag_table();
        if (!tags_)
        {
            throw runtime_error(string(interp_->name()) + ": null tags table");
        }

        Gtk::TextBuffer::iterator i = get_insert_iter();
        mark_ = buf_->create_mark(i);
        set_show_line_markers(true);
        buf_->begin_not_undoable_action(); // no undo

        move_mark_to_cursor();
        set_auto_indent(false);
        set_flags(Gtk_FLAG(CAN_DEFAULT));
    }

    void output_history(const string& hstr)
    {
        Gtk::TextBuffer::iterator i = buf_->get_iter_at_mark(mark_);
        buf_->erase(i, buf_->end());
        move_mark_to_cursor();

        buf_->insert(buf_->get_iter_at_mark(mark_), hstr);
    }

    void output_history(int dir)
    {
        if (const size_t n = history_.size())
        {
            hpos_ = (hpos_ + n + dir) % n;
            const string& hstr = history_[hpos_];

            output_history(hstr);
        }
    }

    void output_history_entry()
    {
        if (entry_)
        {
            output_history(entry_->get_text(false, false));
        }
    }

    void add_to_history(const string& str)
    {
        if(!str.empty())
        {
            if ((hpos_ < history_.size()) && (history_[hpos_] == str))
            {
                history_.erase(history_.begin() + hpos_);
            }
            history_.push_back(str);
            hpos_ = history_.size();
            if (entry_)
            {
                entry_->add_to_list(str);
            }
        }
    }

    void clear()
    {
        buf_->erase(buf_->begin(), buf_->end());
        move_mark_to_cursor();
        grab_focus();
    }

    void clear_history()
    {
        if (entry_)
        {
            entry_->clear();
            history_.clear();
            // make sure focus is returned to the console
            grab_default();
            grab_focus();
        }
    }

    void set_entry(TextEntry* entry)
    {
        entry_ = entry;
        if (entry)
        {
            Gtk_CONNECT_0(entry, activate, this, &Console::run_history);
        }
    }

    /// Run command from history drop down
    void run_history()
    {
        output_history_entry();
        Gtk::TextBuffer::iterator last = buf_->end(),
                                  first = buf_->get_iter_at_mark(mark_);
        enter_line(NULL, first, last);
    }
};


static void strip_trailing_space(string& str)
{
    while (size_t n = str.size())
    {
        if (isspace(str[--n]))
        {
            str.resize(n);
        }
        else
        {
            break;
        }
    }
}


void Console::print(const char* text, size_t len)
{
    if (!is_ui_thread())
    {
        output_.assign(text, text + len);
        app_->post_request(command(&Console::print, this, output_.c_str(), len));
    }
    else if (len)
    {
        assert(buf_);
        buf_->begin_not_undoable_action();

        Glib::ustring ustr;
                      ustr += text;
        if (text[len - 1] != '\n')
        {
            ustr += '\n';
        }

        Glib::RefPtr<Gtk::TextTag> plain = tags_->lookup("__plain__");
        if (!plain)
        {
            plain = buf_->create_tag("__plain__");
            plain->property_foreground() = "navy";
            plain->property_font() = "fixed";
            plain->set_priority(tags_->get_size() - 1);
        }
        buf_->insert_with_tag(get_insert_iter(), ustr, plain);

        move_mark_to_cursor();
        buf_->end_not_undoable_action();
    }
}



namespace
{
    /**
     * Invokes interpreter on main thread.
     */
    class ZDK_LOCAL Runner : public ZObjectImpl<InterThreadCommand>
    {
        AppSlots& app_;
        Console& console_;
        RefPtr<Interpreter> interp_;
        string command_;

    public:
        Runner(AppSlots& app,
               Console& console,
               RefPtr<Interpreter>& interp,
               const string& command
              )
            : app_(app)
            , console_(console)
            , interp_(interp)
            , command_(command)
        { }

        ~Runner() throw() { }

        /// execute on main thread
        bool execute()
        {
            Thread* thread = app_.current_thread().get();
            Debugger* dbg = thread ? thread->debugger() : NULL;
            interp_->run(thread, command_.c_str(), &console_);

            const bool ret = dbg ? dbg->is_resumed() : false;
            return ret;
        }

        const char* name() const { return "Runner"; }
    };
}


bool Console::enter_line(GdkEventKey* event,
                         Gtk::TextBuffer::iterator first,
                         Gtk::TextBuffer::iterator last)
{
    if (last != buf_->end())
    {
        last = buf_->end();
        buf_->move_mark(buf_->get_insert(), last);
        //buf_->move_mark(buf_->get_selection_bound(), last);
    }
    buf_->move_mark(buf_->get_selection_bound(), buf_->end());

    string str = buf_->get_slice(first, last);

    strip_trailing_space(str);

    if (event || !str.empty())
    {
        if (str == "clear")
        {
            clear();
            return true;
        }

        if (event)
        {
            Gtk::Text::on_key_press_event(event);
        }
        else
        {
            buf_->insert(get_insert_iter(), "\n");
        }
        last = move_mark_to_cursor();
        add_to_history(str);

        CommandPtr runner(new Runner(*app_, *this, interp_, str));
        app_->post_response(runner);
    }
    return true;
}


/**
 * Prevent users from inserting and deleting text using the mouse
 */
event_result_t Console::on_button_press_event(GdkEventButton* event)
{
    Gtk::TextBuffer::iterator last = get_insert_iter();
    Gtk::TextBuffer::iterator first = buf_->get_iter_at_mark(mark_);

    set_editable (first <= last);
    bool result = Gtk::Text::on_button_press_event(event);

    return result;
}


event_result_t Console::on_key_press_event(GdkEventKey* event)
{
    bool handled = false;

    if (history_.empty())
    {
        if (entry_)
        {
            history_.assign(entry_->drop_list()->items().begin(),
                            entry_->drop_list()->items().end());
        }
        hpos_ = history_.size();
        history_.push_back(string());
    }
    if (event)
    {
        assert(buf_);
        assert(mark_);

        Gtk::TextBuffer::iterator last = get_insert_iter();
        Gtk::TextBuffer::iterator first = buf_->get_iter_at_mark(mark_);

        const int state = event->state & (Gdk_FLAG(CONTROL_MASK)|
                                          Gdk_FLAG(SHIFT_MASK)  |
                                          Gdk_FLAG(MOD1_MASK));

        if (state)
        {
            set_editable (first <= last);
        }
        else
        {
            // mark_ is set at the start of the last line, reject any
            // character if the cursor is positioned elsewhere other
            // than within the last line of text
            if (last < first)
            {
                handled = true;
            }

            switch (event->keyval)
            {
            case GDK_Return:
                handled = enter_line(event, first, last);
                break;

            case GDK_Home:
            case GDK_End:
            case GDK_Right:
                handled = false;
                break;

            case GDK_Left:
                handled = first == last;
                break;

            case GDK_BackSpace:
                if (first == last)
                {
                    handled = true;
                }
                break;

            case GDK_Up:
            case GDK_Down:
                //up/down arrows are treated separately within
                //the last line only
                if (!handled)
                {
                    output_history(event->keyval == GDK_Up ? -1 : 1);
                }
                handled ^= true;
                break;

            default:
                set_editable (first <= last);
                break;
            }
        }
    }
    if (!handled)
    {
        handled = Gtk::Text::on_key_press_event(event);
    }
    return handled;
}



InterpreterBox::InterpreterBox(AppSlots* app, Interpreter* interp)
    : interp_(interp)
{
    ToolBar* toolbar = manage(new ToolBar);
    pack_start(*toolbar, false, false);

    Console* console = new Console(app, interp);
    console_ = console;

    console->set_language(interp->lang_name());

    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow);
    sw->add(*console);
    console->set_cursor_visible(true);
    console->set_wrap_mode(Gtk_FLAG(WRAP_NONE));

    add(*sw);

    Gtk::ToolItem* item = manage(new Gtk::ToolItem);
    RefPtr<Properties> prop = app->debugger().properties();
#if DEBUG
    if (!prop)
    {
        clog << "null properties object\n";
    }
#endif
    toolbar->add_button(NULL, "Erase console",
                        Gtk_SLOT(console, &Console::clear),
                        0, "Clear", GTK_STOCK_CLEAR);
    //
    // add a drop-down combo for the command history
    //
    TextEntry* entry = manage(new TextEntry(prop, interp_->name()));
    item->add(*entry);
    Gtk_set_size(item, 350, -1);
    toolbar->add(*item);
    //entry->get_entry()->set_editable(false);

    console->set_entry(entry);

    app->register_object(interp_->name(), entry->drop_list());

    toolbar->add_button(NULL, "Execute command",
                        Gtk_SLOT(console, &Console::run_history),
                        0, "Run", GTK_STOCK_EXECUTE);
    toolbar->add_button(NULL, "Delete history",
                        Gtk_SLOT(console, &Console::clear_history),
                        0, "Delete", GTK_STOCK_DELETE);

    toolbar->add_separator();
    toolbar->add_button(NULL, "Help",
                        Gtk_BIND(Gtk_SLOT(this, &InterpreterBox::help), app),
                        0, "Help", GTK_STOCK_HELP);

}


const char* InterpreterBox::name() const
{
    return interp_ ? interp_->name() : "???";
}



void InterpreterBox::help(AppSlots* app)
{
    if (interp_)
    {
        app->on_help(interp_->lang_name());
    }
}
#endif // GTKMM_2
