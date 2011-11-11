//
// $Id: code_view_2.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#ifndef GTKMM_2
 #error This file contains Gtkmm-2.x specific code
#endif
//
// Gtkmm-2.x implementation part for CodeView
//
#include "config.h"
#include <gdkmm/window.h>
#include <gtkmm/scrolledwindow.h>
#include <pangomm/tabarray.h>
#include <dwarf.h>
#include "code_view_common.h"
#include "gtkmm/connect.h"
#include "gtkmm/tooltips.h"
#include "ensure_font.h"
#include "fixed_font.h"
#include "gui.h"
#include "dharma/system_error.h"
#include "gtksourceview/gtksourcebuffer.h"
#include "gtksourceview/gtksourceview.h"
#include "icons/stop_pink.xpm"
#include "icons/stop_red.xpm"
#include "icons/right.xpm"
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/disasm.h"
#include "zdk/translation_unit.h"
#include "zdk/types.h"
#include "command.h"
#include "is_reference.h"
#include "right_click_context.h"
#include "slot_macros.h"
#include "text_mark.h"

#define ENABLED     "enabled"
#define DISABLED    "disabled"
#define CURRENT     "current"

using namespace Gtk;
using namespace std;

static Glib::RefPtr<Gdk::Pixbuf> g_pcounter;
static Glib::RefPtr<Gdk::Pixbuf> g_break_disabled;
static Glib::RefPtr<Gdk::Pixbuf> g_break_enabled;

//
// prototype may be missing in some gtksourceview.h versions
//
extern "C" void
gtk_source_view_set_highlight_current_line(GtkSourceView*, gboolean);



////////////////////////////////////////////////////////////////
void CodeView::customize_popup_menu(Menu& menu)
{
    MenuShell::MenuList& mlist = menu.items();
    mlist.erase(mlist.begin());
    MenuShell::MenuList::iterator mi = mlist.end();
    mi = mlist.erase(--mi);
    for (size_t count = 3; count && !mlist.empty(); --count)
    {
        mi = mlist.erase(--mi);
    }
}


////////////////////////////////////////////////////////////////
void CodeView::init()
{
    g_pcounter = Gdk::Pixbuf::create_from_xpm_data(right_xpm);
    g_break_enabled = Gdk::Pixbuf::create_from_xpm_data(stop_red);
    g_break_disabled = Gdk::Pixbuf::create_from_xpm_data(stop_pink);

    set_marker_pixbuf(CURRENT, g_pcounter);
    set_marker_pixbuf(ENABLED, g_break_enabled);
    set_marker_pixbuf(DISABLED, g_break_disabled);
    set_show_line_markers(true);
    set_cursor_visible(false);

#if HAVE_GTK_SOURCE_VIEW_SET_HIGHLIGHT_CURRENT_LINE
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(gobj()), true);
#endif
}


////////////////////////////////////////////////////////////////
bool CodeView::is_line_visible(size_t line) const
{
    bool result = true;
    if (!line)
    {
        result = false;
    }
    else
    {
        Glib::RefPtr<const Gtk::SourceBuffer> buf = get_buffer();
        Gdk::Rectangle rect;
        get_visible_rect(rect);

        result = ((int)line >= rect.get_y()
               && (int)line < rect.get_y() + rect.get_height());
    }
    return result;
}


////////////////////////////////////////////////////////////////
bool CodeView::on_button_press_event(GdkEventButton* event)
{
    reset_tips();
    if (CHKPTR(event)->type == GDK_BUTTON_PRESS)
    {
        searchResult_ = 0;
    }
    rClick_.clear();

    if (can_interact()
        && (event->type == GDK_BUTTON_PRESS)
        && (event->button == 3))
    {
        int x = 0, y = 0;
        window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT,
                                static_cast<int>(event->x),
                                static_cast<int>(event->y),
                                x, y);
        Gtk::TextBuffer::iterator iter;
        get_iter_at_location(iter, x, y);
        rClick_.set_position(iter.get_offset());
        wordAtCursor_ = get_word_at_iter(iter);
    #if 0
        backward_display_line(iter);
        if (iter)
        {
            ++iter;
        }
    #endif
        const size_t pos = iter.get_offset();
        run_on_main_thread(command(&CodeView::get_addrs_at_offs, this, pos));
    }
    // chain to the event handler in the base class,
    // which in turn calls on_populate_popup()
    return Gtk::Text::on_button_press_event(event);
}


////////////////////////////////////////////////////////////////
int CodeView::scroll_to_line_internal(size_t line)
{
    if (gobj()->parent.onscreen_validated)
    {
        Text::scroll(line);
    }
    else
    {
        Glib::signal_idle().connect(Gtk_BIND(Gtk_SLOT(this, &Text::scroll), line));
    }
    return 0;
}



////////////////////////////////////////////////////////////////
void
CodeView::set_language_from_file_ext(const string& str, addr_t addr)
{
    RefPtr<TranslationUnit> unit;

    if (thread_)
    {
        RefPtr<Process> proc = thread_->process();
        unit = addr ? debugger_.lookup_unit_by_addr(proc.get(), addr)
                    : debugger_.lookup_unit_by_name(proc.get(), str.c_str());
    }
    if (unit)
    {
        switch (unit->language())
        {
        case DW_LANG_C89:
        case DW_LANG_C:
        case DW_LANG_C_plus_plus:
        case DW_LANG_C99:
            set_language("c++");
            return;

        case DW_LANG_D:
            set_language("d");
            return;
        }
    }

    if (str.rfind(".d") == str.size() - 2)
    {
        set_language("d");
    }
    else if (
        str.rfind(".cpp") < str.size()  ||
        str.rfind(".h") < str.size()    ||
        str.rfind(".hpp") < str.size()  ||
        str.rfind(".cc") < str.size()   ||
        str.rfind(".CC") < str.size()   ||
        str.rfind(".cxx") < str.size())
    {
        set_language("c++");
    }
    else if (str.rfind(".c") < str.size() ||
             str.rfind(".C") < str.size())
    {
        set_language("c");
    }
    else
    {
        set_language("text");
    }
}



////////////////////////////////////////////////////////////////
bool CodeView::tabstops(size_t*, size_t*) const
{
    // tell the disassembler not to worry about formatting tabs
    return false;
}


////////////////////////////////////////////////////////////////
int CodeView::get_font_width()
{
    int size = font_.get_size();

    if (Glib::RefPtr<Pango::Context> ctxt = get_pango_context())
    {
        Pango::FontMetrics fm =
            ctxt->get_metrics(font_, Pango::Language("common"));

        const double w = fm.get_approximate_digit_width();
        const double z = fm.get_approximate_char_width();

        size = static_cast<int>((w + z) / 2);
    }
    return size;
}


////////////////////////////////////////////////////////////////
void CodeView::set_asm_tabstops()
{
    const int size = get_font_width();
    Pango::TabArray tabs(2, false);
    //
    // todo: make tabstops user-configurable
    //
    tabs.set_tab(0, Pango::TAB_LEFT, (2 * sizeof(addr_t) + 4) * size);
    tabs.set_tab(1, Pango::TAB_LEFT, (2 * sizeof(addr_t) + 38) * size);
    set_tabs(tabs);
}


////////////////////////////////////////////////////////////////
void CodeView::apply_tab_width()
{
    Pango::TabArray tabs(1, false);
    int tabSize = debugger_.properties()->get_word("tab_size", 4);
    tabSize *= get_font_width();
    tabs.set_tab(0, Pango::TAB_LEFT, tabSize);
    set_tabs(tabs);
}


////////////////////////////////////////////////////////////////
static Glib::RefPtr<Gtk::SourceBuffer::Tag>
apply_font(Glib::RefPtr<SourceBuffer> buf, const Gdk_Font& font)
{
    Glib::RefPtr<Gtk::SourceBuffer::Tag> tagFont;

    if (Glib::RefPtr<TextBuffer::TagTable> tagTable = buf->get_tag_table())
    {
        tagFont = tagTable->lookup(font.to_string());

        if (!tagFont)
        {
            tagFont = TextBuffer::Tag::create(font.to_string());
            tagFont->property_font_desc() = font;
            tagTable->add(tagFont);
        }
    }
    return tagFont;
}


////////////////////////////////////////////////////////////////
bool
CodeView::read_source_file(const RefPtr<SharedString>& fname, addr_t addr)
{
    ScopedFreeze<CodeView> freeze(*this);
    apply_tab_width();

    assert(fname);
    assert(fname->c_str());
    const string str(fname->c_str());

    ifstream fs(fname->c_str());
    if (!fs)
    {
        SystemError err("Could not open: " + str);
        debugger_.message(err.what(), Debugger::MSG_ERROR);
        // returning false rather than throwing an exception
        // allows the caller to fallback to disassembly
        return false;
    }
    set_file(fname);
    file_changed(this);
    this->reset();

    set_language_from_file_ext(str, addr);

    Glib::RefPtr<SourceBuffer> buf = get_buffer();
    if (!buf)
    {
        debugger_.message("failed getting source view buffer", Debugger::MSG_ERROR);
        return false;
    }
    NotUndoableScope notUndoable(buf);
    Glib::RefPtr<Gtk::SourceBuffer::Tag> tagFont = ::apply_font(buf, font_);

    Glib::RefPtr<TextBuffer::Mark> mark = buf->get_insert();
    TextBuffer::iterator iter = buf->get_iter_at_mark(mark);

    vector<char> readbuf(4096);

    for (size_t pos = 0, lineNum = 1; buf; ++lineNum)
    {
        if (!getline(fs, &readbuf[0], readbuf.size()))
        {
            return false;
        }
        if (!fs)
        {
            if (fs.bad())
            {
                SystemError err("Error reading: " + str);

                debugger_.message(err.what(), Debugger::MSG_ERROR);
                return false;
            }
            break;
        }
        lineMap_.insert(make_pair(lineNum, pos));
        reverseLineMap_.insert(make_pair(pos, lineNum));

        Glib::ustring line(&readbuf[0]);
        if (!line.validate())
        {
            if (!accept_invalid_utf8(&readbuf[0], lineNum))
            {
                return false;
            }
            // remove all invalid UTF8 characters
            for (Glib::ustring::iterator i = line.begin(); !line.validate(i);)
            {
                line.erase(i);
            }
        }
        line += '\n';

        if (tagFont)
        {
            iter = buf->insert_with_tag(iter, line, tagFont);
        }
        else
        {
            buf->insert_at_cursor(line);
            iter = buf->end();
        }
        pos += line.length();
    }
    if (BreakPointManager* mgr = debugger_.breakpoint_manager())
    {
        EnumBreakPoints observ(*this);
        mgr->enum_breakpoints(&observ);
    }
    set_show_line_numbers(true);
    return true;
}


////////////////////////////////////////////////////////////////
void CodeView::reset_text()
{
    Glib::RefPtr<Gtk::SourceBuffer> buf = get_buffer();
    if (buf)
    {
        buf->set_text("");
    }
    set_show_line_numbers(false);
}


////////////////////////////////////////////////////////////////
void CodeView::hilite_line
(
    size_t          line,
    bool            hilite,
    BreakPointState state
)
{
    assert(line);

    Glib::RefPtr<Gtk::SourceBuffer> buf = get_buffer();
    if (!buf)
    {
        return;
    }
    Gtk::TextBuffer::iterator iter = buf->get_iter_at_line(line - 1);
    if (!iter)
    {
        return;
    }

    if (hilite || state == B_NONE)
    {
        remove_marker(buf, iter, line, DISABLED);
        remove_marker(buf, iter, line, ENABLED);
    }
    if (hilite)
    {
        //buf->move_mark(buf->get_insert(), iter);
        //buf->move_mark(buf->get_selection_bound(), iter);

        //remove_marker(buf, CURRENT);
        create_marker(buf, iter, line, CURRENT);
    }
    else
    {
        remove_marker(buf, iter, line, CURRENT);
    }

    switch (state)
    {
    case B_AUTO:
    case B_NONE:
        break;

    case B_ENABLED:
        remove_marker(buf, iter, line, DISABLED);
        create_marker(buf, iter, line, ENABLED);
        break;

    case B_DISABLED:
        remove_marker(buf, iter, line, ENABLED);
        create_marker(buf, iter, line, DISABLED);
        break;
    }
}


////////////////////////////////////////////////////////////////
BEGIN_SLOT_(bool, CodeView::on_map_event,(GdkEventAny* event))
{
    if (font_.to_string() == fixed_font())
    {
        ensure_monospace(font_, *this);
    }
    return Text::on_map_event(event);
}
END_SLOT_(false)


////////////////////////////////////////////////////////////////
Gtk::Widget* CodeView::add_scrolled_window()
{
    Gtk::ScrolledWindow* sw = new Gtk::ScrolledWindow;
    sw->set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));
    sw->add(*this);
    return sw;
}


////////////////////////////////////////////////////////////////
void CodeView::check_before_insert_asm_line(size_t)
{
    assert_eq(get_buffer()->get_line_count(), line_count() + 1);
}


////////////////////////////////////////////////////////////////
BEGIN_SLOT_(bool, CodeView::on_motion_notify_event,(GdkEventMotion* event))
{
    if (event)
    {
        reset_tips();
        timer_ = Glib::signal_timeout().connect(
            sigc::mem_fun(this, &CodeView::on_mouse_timer), 200);
    }
}
END_SLOT_(false)


////////////////////////////////////////////////////////////////
//
// Here's where we show the tooltip when the mouse hovers over
// a variable in the source code view
//
BEGIN_SLOT_(bool, CodeView::on_mouse_timer,())
{
    assert_ui_thread();
    if (!can_interact())
    {
    #if DEBUG
        clog << __func__ << ": busy" << endl;
    #endif
        return false;
    }
    int x = 0, y = 0;
    get_pointer(x, y);
    window_to_buffer_coords(TEXT_WINDOW_WIDGET, x, y, x, y);
    SourceBuffer::iterator iter = get_buffer()->end();
    get_iter_at_location(iter, x, y);
    const string word = get_word_at_iter(iter);
    if (!word.empty() && !isdigit(word[0]))
    {
        const size_t pos = iter.get_offset();
        rClick_.clear();
        run_on_main_thread(command(&CodeView::get_addrs_at_offs, this, pos));
        addr_t addr = rClick_.addrs().empty() ? 0 : rClick_.addrs().front();
        if (addr == 0)
        {
            addr = rClick_.program_count();
        }
        DebugSymbolList syms;
        query_symbols(word, addr, &syms, true /* read values */);

        ostringstream buf;
        size_t nVarCount = 0;

        for (DebugSymbolList::reverse_iterator i = syms.rbegin();
             i != syms.rend() && (nVarCount == 0);
             ++i)
        {
            RefPtr<DebugSymbol> sym = *i;

            filter.emit(&sym, sym->parent(), NULL);
            if (const char* tip = sym->tooltip())
            {
                buf << tip;
                ++nVarCount;
                break;
            }
            if (RefPtr<SharedString> val = sym->value())
            {
                buf << sym->name() << "=";
                if (is_ref(sym))
                {
                    sym = sym->nth_child(0);
                    read_symbol(sym, NULL);
                    if (sym->value())
                    {
                        val = sym->value();
                    }
                }
                buf << val->c_str();
                ++nVarCount;
            }
        }
        if (nVarCount)
        {
            tips_.reset(new Tooltips);
            tips_->enable();
            tips_->set_tip(*this, buf.str());
        }
    }
}
END_SLOT_(false)


////////////////////////////////////////////////////////////////
BEGIN_SLOT_(bool, CodeView::on_leave_notify_event,(GdkEventCrossing*))
{
    reset_tips();
}
END_SLOT_(false)


////////////////////////////////////////////////////////////////
void CodeView::reset_tips()
{
    if (timer_.connected())
    {
        timer_.disconnect();
    }
    if (tips_.get())
    {
        tips_->unset_tip(*this);
        tips_->disable();
        tips_.reset();
    }
#if HAVE_NEW_TOOLTIP_API
    set_has_tooltip(false);
#endif
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
