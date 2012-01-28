#ifndef PROGRAM_VIEW_H__A311440C_F695_447D_A8D3_4245ECD89D7E
#define PROGRAM_VIEW_H__A311440C_F695_447D_A8D3_4245ECD89D7E
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <deque>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "gtkmm/events.h"
#include "gtkmm/font.h"
#include "gtkmm/pixmap.h"
#include "gtkmm/notebook.h"
#include "code_view.h"
#include "menu_builder_impl.h"
#include "prog_view_signals.h"
#include "view_types.h"
#include "zdk/platform.h"
#include "zdk/ref_ptr.h"
#include "zdk/debug_symbol_list.h"

class BreakPoint;
class Debugger;
class MemoryRequest;
class Symbol;
class Thread;

class CodeView;


class ZDK_LOCAL ProgramView
    : public ProgramViewSignals
    , public Notebook_Adapt
    , public MenuBuilderImpl<CodeView>
{
public:
    typedef boost::shared_ptr<CodeView> ViewPtr;
    typedef std::vector<ViewPtr> Views;
    typedef Platform::addr_t addr_t;


    explicit ProgramView(Debugger&);
    ~ProgramView() throw();

    bool has_views() const { return hasViews_; }

    CodeView& current_view();
    const CodeView& current_view() const;

    const std::string& font_name() const { return fontName_; }

    bool apply_font(const std::string&);

    /**
     * Brings symbol and thread in view, optionally
     * hilites the symbols, and updates history
     * (for Back/Forward navigation).
     */
    void set_current
      (
        RefPtr<Symbol>,
        RefPtr<Thread>,
        RefPtr<Frame>,
        bool hilite,
        ViewType = VIEW_DEFAULT
      );

    void set_current(RefPtr<Thread>, const std::string& src);

    /**
     * @note param by value to required by Sigc++ 1.0
     */
    void show_breakpoint(RefPtr<BreakPoint>);

    void show_function(const RefPtr<Symbol>&, const RefPtr<Thread>&);

    void redraw();

    /**
     * get the current symbol in view
     */
    RefPtr<Symbol> symbol() const;

    /**
     * @return the current thread in view
     */
    RefPtr<Thread> thread() const;

    void set_thread(const RefPtr<Thread>&);

    /**
     * @return the name of the file currently in view
     */
    RefPtr<SharedString> file() const;

    /**
     * closes all pages
     */
    void clear(bool reinitialize = true);

    void close_tab(size_t pageIndex);

    void close_page(Gtk::Widget* page)
    { close_tab(page_num(*page)); }

    void close_current_tab() { close_tab(current_); }

    /**
     * Set the view type
     */
    void set_view_type(ViewType type);

    /**
     * @return the current view type, can be one of
     * VIEW_SOURCE, VIEW_DISASSEMBLED or VIEW_MIXED
     */
    ViewType view_type() const;

    /**
     * True if the ViewType is either VIEW_DISASSEMBLED or
     * VIEW_MIXED, and the disassembly was successful.
     */
    bool have_disassembly() const;

    /**
     * force source-disasm mixed view
     */
    void force_mixed_view();

    /**
     * A hack for speeding up the lookup of the next address
     * to be executed. Rather than using the symbol tables,
     * use the info cached by this widget.
     * @todo: document param, return
     */
    addr_t next_addr(addr_t = 0) const;

    /**
     * @return the current line
     * @note this may not be equal to the current symbol's line,
     * if we're showing disassembled code
     */
    size_t current_line() const;

    void hilite_line(size_t line, bool, BreakPointState = B_AUTO);

    bool is_line_visible(size_t line) const;

    BreakPointState query_breakpoint_at_line(size_t) const;

    std::string selection() const;

    /**
     * For compatibility with libsigc++ 1.x
     */
    std::string get_selection() { return selection(); }

    void search(const std::string&);
    void search_again();

    void refresh_line(size_t line);

    /**
     * Notification that the breakpoint at the given address
     * needs to be redrawn (if in current view) because its
     * enabled/disable state has changed.
     */
    void breakpoint_state_changed(addr_t);

    /**
     * @return true if breakpoint is new -- for example,
     * if a disabled breakpoint is enabled, this function
     * returns false.
     */
    bool on_insert_breakpoint(const RefPtr<Symbol>&, bool isDeferred);

    void on_remove_breakpoint(const RefPtr<Symbol>&, bool isDeferred);

    /**
     * called when read_memory_async completes
     */
    void on_read_done(RefPtr<MemoryRequest>);

    void restore_open_tabs(Thread&);
    void save_open_tabs(Thread&);

    // <files navigation>
    /**
     * Move back and forward in history
     */
    void history_back();
    void history_forward();

    void history_save();

    struct Snapshot
    {
        RefPtr<Symbol> sym;
        RefPtr<Thread> thread;
        RefPtr<Frame>  frame;
        ViewType       viewType;
    };
    // </files navigation>

private:
    typedef std::deque<Snapshot> ViewHistory;

    void set_current_view
      (
        const RefPtr<Symbol>&,
        const RefPtr<Thread>&,
        const RefPtr<Frame>&,
        bool hilite,
        ViewType
      );
    template<typename T> void
    navigate(T& traceFwd, T& traceBack, Debugger& dbg)
    {
        ProgramView::Snapshot entry;

        for (; !traceBack.empty(); )
        {
            entry = traceBack.back();
            traceBack.pop_back();

            if (symbol().is_null() ||
                symbol()->addr() != CHKPTR(entry.sym)->addr())
            {
                save_current(traceFwd, current_view(), dbg);
                set_current_view(entry.sym,
                                 entry.thread,
                                 entry.frame,
                                 true, //highlight
                                 entry.viewType);
                break;
            }
        }
    }

    void history_clear();
    Frame* frame() const;

    void init(Properties* = NULL);

    ViewPtr create_code_view();

    void on_file_changed(CodeView*);

    void set_tab_name(size_t, const RefPtr<SharedString>&);

    void on_switch_page(Gtk_NOTEBOOK_PAGE*, guint pageNum);

    /**
     * Find the page that contains given symbol,
     * @return NULL if none matches
     */
    ViewPtr find_page(const RefPtr<Symbol>&, size_t& pageIndex) const;

    ViewPtr find_page(RefPtr<SharedString>, size_t* pageIndex = NULL) const;

    bool find_page_by_name(const std::string&, size_t&) const;

    void add_tab_label(size_t pageIndex, const char* text);

private:
    Debugger&   debugger_;
    Views       views_;
    size_t      current_;
    bool        disableSwitchPageCallback_;
    bool        hasViews_;
    ViewHistory traceBack_;
    ViewHistory traceFwd_;
    std::string fontName_;

    boost::shared_ptr<Gtk::Pixmap> close_;
};

#endif // PROGRAM_VIEW_H__A311440C_F695_447D_A8D3_4245ECD89D7E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
