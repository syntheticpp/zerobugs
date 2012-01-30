// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#if defined GTKMM_2
 #error This file contains Gtkmm-1.2 specific code
#endif

#include <math.h>
#include "command.h"
#include "dharma/system_error.h"
#include "gtkmm/scrollbar.h"
#include "gtkmm/table.h"
#include "code_view_common.h"
#include "gui.h"
#include "popup_menu.h"

using namespace std;

// C/C++ delimiters... TODO: are these all?
static const std::string cpp_delim(".,;+-/*%|&~^=<>()[]{}!\"' \t\n");

//
// Gtk-- 1.2 only implementation part
//
void CodeView::customize_popup_menu(Gtk::Menu&)
{
}


void CodeView::init()
{
}

////////////////////////////////////////////////////////////////
bool CodeView::is_line_visible(size_t line) const
{
    bool result = true;
    if (!line)
    {
        result = false;
    }
    else if (Gtk::Adjustment* adj = get_vadjustment())
    {
        --line;
        assert(line_count());

        // points per line
        const double ppl = adj->get_upper() / line_count();

        size_t top = static_cast<size_t>(rint(adj->get_value() / ppl));
        size_t linesInView = static_cast<size_t>(floor(height() / ppl));

        result = (line >= top) && (line < top + linesInView);
    }
    return result;
}

////////////////////////////////////////////////////////////////
int CodeView::button_press_event_impl(GdkEventButton* event)
{
    assert(event);

    if (event->type == GDK_BUTTON_PRESS)
    {
        searchResult_ = 0;
    }

    // the implementation in the base class positions the cursor
    const int result = Gtk::Text::button_press_event_impl(event);

    if (event->type == GDK_BUTTON_PRESS &&
        event->button == 3              &&
        can_interact())
    {
        const size_t savePos = get_position();
        move_to_column_impl(0);

        const size_t pos = get_position();

        rClick_.clear();
        rClick_.set_position(pos);

        run_on_main_thread(command(&CodeView::get_addrs_at_offs, this, pos));

        // Simulate a button release, since the real release
        // event goes to the menu widget, once we pop it up.
        Gtk::Text::button_release_event_impl(event);

        ensure_rclick_line();
        rClick_.set_position(savePos);

        popup_context_menu(*event);
    }
    return result;
}


////////////////////////////////////////////////////////////////
static string cpp_token_at_cursor(
    const CodeView& view,
    size_t begin,
    size_t end,
    size_t pos)
{
    assert(pos >= begin);
    pos -= begin;

    // line cannot be empty, there is at least the line number
    assert(end > begin);

    // retrieve entire line
    string lineAtCursor = view.get_chars(begin, end);

    size_t n = lineAtCursor.find_last_of(cpp_delim, pos);
    ++n;

    size_t m = lineAtCursor.find_first_of(cpp_delim, pos);
    if (m == string::npos)
    {
        m = end;
    }
    return lineAtCursor.substr(n, m - n);
}


////////////////////////////////////////////////////////////////
string CodeView::get_word_at_position(size_t currentPos) const
{
    // assume that we are at the beginning of the line
    const unsigned int begin = get_position();
    // goto end-of-line
    const_cast<CodeView*>(this)->move_to_column_impl(-1);

    // get offset from beginning of text, in chars
    const unsigned int end = get_position();

    return cpp_token_at_cursor(*this, begin, end, currentPos);
}


////////////////////////////////////////////////////////////////
void CodeView::popup_context_menu(GdkEventButton& event)
{
    std::auto_ptr<Gtk::Menu> menu(new PopupMenu);
    on_populate_popup(menu.get());

    menu->accelerate(*get_toplevel());
    menu.release()->popup(event.button, event.time);
}


///////////////////////////////////////////////////////////////
/*
void CodeView::context_menu_add_item(Gtk::Menu& menu)
{
    wordAtCursor_ = get_word_at_position(rClick_.position());

    if (!wordAtCursor_.empty())
    {
        context_menu_add_quick_view(menu);
    }
}
*/


int CodeView::scroll_to_line_internal(size_t line)
{
    Gtk::Adjustment* adj = get_vadjustment();
    assert(adj);

    assert(line_count());
    const double pointsPerLine = adj->get_upper() / line_count();
    // compute the top-most visible line
    const size_t top = static_cast<size_t>(rint(adj->get_value() / pointsPerLine));
    const size_t linesInView = static_cast<size_t>(floor(height() / pointsPerLine));

    if ((line < top) || (line >= top + linesInView))
    {
        if (line)
        {
            --line;
        }
        const double a = floor(line * pointsPerLine);
        adj->set_value(a);
    }
    return 0;
}


bool
CodeView::read_source_file(const RefPtr<SharedString>& fname, addr_t)
{
    assert(fname);

    vector<char> buf(4096);

    ifstream fs(fname->c_str());
    if (!fs)
    {
        //throw SystemError("Could not open: " + string(fname->c_str()));
        //SEE comment in homonyme function in code_view_2.cpp
        return false;
    }

    set_file(fname);
    file_changed(this);

    ScopedFreeze<Gtk::Text> freeze(*this);

    reset();

    for (size_t pos = 0, i = 1; ; ++i)
    {
        fs.getline(&buf[0], buf.size());
        if (!fs)
        {
            if (fs.bad())
            {
                throw SystemError("Error reading: " + string(fname->c_str()));
            }
            break;
        }
        dbgout(1) << "  [" << pos << ", " << i << "]" << endl;
        lineMap_.insert(make_pair(i, pos));
        reverseLineMap_.insert(make_pair(pos, i));

        ostringstream lineNum;
        lineNum << setw(line_number_width()) << i << ' ';

        size_t len = lineNum.str().length();
        this->insert(font_, lineCol_, backGnd_, lineNum.str(), -1);

        string line(&buf[0]);
        ensure_new_line(line);

        len += line.length();
        pos += len;

        this->insert(font_, foreGnd_, backGnd_, line, -1);
    }

    if (BreakPointManager* mgr = interface_cast<BreakPointManager*>(&debugger_))
    {
        EnumBreakPoints observ(*this);
        mgr->enum_breakpoints(&observ);
    }

    return true;
}


void CodeView::reset_text()
{
    delete_text(0, -1);
}


void CodeView::hilite_line(
    size_t          line,
    bool            hilite,
    BreakPointState state)
{
    static const Gdk_Color yellow("yellow");
    static const Gdk_Color black("black");
    static const Gdk_Color pink("pink");
    static const Gdk_Color red("red");
    static const Gdk_Color gray("gray");

    if (line)
    {
        const Gdk_Color* bg = &backGnd_;
        const Gdk_Color* fg = &lineCol_;

        if (hilite)
        {
            bg = &black;
            if (state == B_ENABLED)
            {
                bg = &red;
            }
            else if (state == B_DISABLED)
            {
                bg = &gray;
            }
            fg = &yellow;
        }
        else
        {
            if (state == B_ENABLED)
            {
                bg = &pink;
            }
            else if (state == B_DISABLED)
            {
                bg = &gray;
            }
            if (have_disassembly())
            {
                fg = &foreGnd_;
            }
        }
        mark_line(line, *fg, *bg);
    }
}


Gtk::Widget* CodeView::add_scrolled_window()
{
    Gtk::Table* table = new Gtk::Table(2, 2, false);
    table->attach(*this, 0, 1, 0, 1);

    Gtk::VScrollbar* sb =
        manage(new Gtk::VScrollbar(*get_vadjustment()));
    table->attach(*sb, 1, 2, 0, 1, Gtk_FLAG(FILL));

    return table;
}


void CodeView::check_before_insert_asm_line(size_t currentPos)
{
    assert(get_point() == currentPos);
}


bool CodeView::tabstops(size_t* first, size_t* second) const
{
    if (first)
    {
        *first = 2 * sizeof(addr_t) + 4;
    }
    if (second)
    {
        *second = 2 * sizeof(addr_t) + 34;
    }
    return true;
}


void CodeView::set_asm_tabstops()
{
}


void CodeView::reset_tips()
{
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
