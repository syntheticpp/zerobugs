#ifndef CODE_VIEW_H__D71F3650_B6F7_42C8_8DF7_5380BA45795F
#define CODE_VIEW_H__D71F3650_B6F7_42C8_8DF7_5380BA45795F
//
// $Id: code_view.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <map>
#include <memory>
#include <set>
#include <string>
#include <boost/shared_ptr.hpp>
#include "gtkmm/color.h"
#include "gtkmm/events.h"
#include "gtkmm/font.h"
#include "gtkmm/stock.h"
#include "gtkmm/text.h"
#include "zdk/disasm.h"
#include "zdk/debug_symbol_list.h"
#include "zdk/enum.h"
#include "zdk/platform.h"
#include "code_view_signals.h"
#include "menu_builder.h"
#include "right_click_context.h"
#include "view_types.h"


// Forward declarations, minimize compile-time dependencies
class BreakPoint;
class DataType;
class Debugger;
class Frame;
class Symbol;
class Thread;

namespace Gtk
{
    class Menu;
    class MenuItem;
    class Tooltips;
}
typedef std::auto_ptr<Gtk::Tooltips> TipsPtr;


/**
 * Displays the source code of the debugged program. If source
 * is not available, call the Debugger::disassemble method and
 * show disassembled machine code.
 */
class ZDK_LOCAL CodeView
    : public Gtk::Text
    , public CodeViewSignals
    , Disassembler::OutputCallback
{
public:
    // map line numbers to text buffer offsets
    typedef std::map<size_t, size_t> LineMap;

    // map addresses to line numbers
    typedef std::multimap<size_t, size_t> AddrMap;

    SigC::Signal1<void, CodeView*> file_changed;
    SigC::Signal1<void, RefPtr<DataType> > what_is;

    CodeView(Debugger&, MenuBuilder<CodeView>&, const std::string& font);
    ~CodeView() throw();

    Gtk::Widget* add_scrolled_window();

    /**
     * Brings given symbol and thread in view, optionally
     * hilites the symbol, and updates history
     * (for Back/Forward navigation).
     * @note May scroll if necessary, to bring the symbol in view.
     */
    void set_elements_in_view
      (
        const RefPtr<Symbol>&,
        const RefPtr<Thread>&,
        const RefPtr<Frame>&,
        bool hilite,
        ViewType = VIEW_DEFAULT
      );

    /**
     * get the current symbol in view
     */
    RefPtr<Symbol> symbol() const { return symbol_; }

    RefPtr<Thread> thread() const { return thread_; }
    void set_thread(Thread*);

    /**
     * @return the name of the file currently in view
     */
    RefPtr<SharedString> file() const
    { Lock<Mutex> lock(mutex_); return file_; }

    void set_file(const RefPtr<SharedString>& file)
    { Lock<Mutex> lock(mutex_); file_ = file; }

    /**
     * Erases the entire text, and clears internal state
     */
    void clear();

    /**
     * Set the view type
     */
    void set_view_type(ViewType type, bool clear = true);

    /**
     * @return the current view type, can be one of
     * VIEW_SOURCE, VIEW_DISASSEMBLED or VIEW_MIXED
     */
    ViewType view_type() const { return type_; }

    /**
     * True if the ViewType is either VIEW_DISASSEMBLED or
     * VIEW_MIXED, and the disassembly was successful.
     */
    bool have_disassembly() const;

    /**
     * A hack for speeding up the lookup of the next address
     * to be executed. Rather than using the symbol tables,
     * use the info cached by this widget.
     * @todo: document param, return
     */
    addr_t next_addr(addr_t = 0) const;

    void search(const std::string&);
    void search_again();

    /**
     * Update the state to reflect the fact that a breakpoint
     * has been inserted at the specified symbol.
     * @return true if breakpoint is new -- for example,
     * if a disabled breakpoint is enabled, this function
     * returns false.
     */
    bool on_insert_breakpoint(const RefPtr<Symbol>&, bool);

    /**
     * Update the state to reflect the fact that a breakpoint
     * has been removed at the address corresponding to the
     * specified symbol.
     */
    void on_remove_breakpoint(const RefPtr<Symbol>&, bool);

    /**
     * Notification that the breakpoint at the given address
     * needs to be redrawn (if in current view) because its
     * enabled/disable state has changed.
     */
    void breakpoint_state_changed(addr_t);

    std::string selection() const;

    /**
     * called when read_memory_async completes
     */
    void on_read_done(RefPtr<MemoryRequest>);

    void refresh_line(size_t line)
    {
        hilite_line(line, line == curLine_);
    }

    void redraw();

    /**
     * @return the current line
     * @note this may not be equal to the current symbol's line,
     * if we're showing disassembled code
     */
    size_t current_line() const { return curLine_; }

    void hilite_line(size_t line, bool);

    void hilite_line(size_t line, bool, BreakPointState);

    bool is_line_visible(size_t line) const;

    /**
     * @return true if the source code is not available and
     * have automatically fellback to showing disassembly
     */
    bool disasm_fallen_back() const { return fallenBack_; }

    void disasm_fallback() { fallenBack_ = true; }

    Frame* frame() const { return frame_.get(); }

    void set_frame(const RefPtr<Frame>& frame) { frame_ = frame; }

    void apply_font(const Gdk_Font& font);

    /**
     * read a source file into the text widget.
     * @return true if successful.
     */
    bool read_source_file(const RefPtr<SharedString>&, addr_t = 0);

    /*
     * Contextual menu functions
     */
    void run_to_cursor();
    void insert_breakpoint();
    void insert_temp_breakpoint();
    void set_program_counter();
    void show_function_start();
    void show_next_statement();

    void step_over(SigC::Signal1<void, RefPtr<Symbol> >*);

    /**
     * Add breakpoint-related items to context menu
     * @return false if no breakpoint exists at given line.
     */
    bool context_menu_add_breakpoint_items(Gtk::Menu&);
    bool context_menu_add_breakpoint_items(Gtk::Menu&, size_t line);

    const RightClickInfo& right_click() const { return rClick_; }

    BreakPointState query_breakpoint_at_line(size_t) const;

    void goto_line(size_t);

private:
    static void customize_popup_menu(Gtk::Menu&);

    void init();

    void ensure_rclick_line();

    bool update_source(const RefPtr<Symbol>&,
                       const RefPtr<Thread>&,
                       const RefPtr<Frame>&,
                       bool hilite,
                       ViewType viewType,
                       size_t& line);

    event_result_t on_button_press_event(GdkEventButton*);

    std::string get_word_at_position(size_t currentPos) const;

    void on_populate_popup(Gtk::Menu*);

    /**
     * Pops up a contextual menu (on right mouse button clicked)
     */
    void popup_context_menu(GdkEventButton&);

    LineMap::const_iterator offs_to_line(size_t) const;
    void get_addrs_at_offs(size_t);

    /**
     * Fill out right click info
     *@return true if successful, false otherwise
     */
    void get_src_addr(size_t pos);
    void get_src_addr(size_t pos, const RefPtr<Thread>&);

    void get_asm_addr(size_t pos);

    /**
     * helper method, used for populating the popup menu
     */
    static void menu_add_breakpoint_item(
        Gtk::Menu&,
        const std::string& text,
        size_t line,
        const std::set<addr_t>&,
        SigC::Signal2<void, addr_t, size_t>&,
        const Gtk::BuiltinStockID* = NULL);

    void add_step_over_item(
        Gtk::Menu&,
        const std::string& text,
        SigC::Signal1<void, RefPtr<Symbol> >*,
        const Gtk::BuiltinStockID* = NULL);

    /**
     * When the token (word) under cursor looks like a function
     * name, this function adds a "GoTo" entry to the right-mouse menu;
     * if the clicked token looks like a variable, add a "Quick View"
     * (i.e. "Evaluate" dialog)
     */
    void context_menu_add_item(Gtk::Menu& popupMenu);

    /**
     * Helper called by CodeView::context_menu_add_item
     */
    bool context_menu_add_eval(Gtk::Menu&);

    /** internal slots for the contextual menu **/

    /**
     * Bring the next statement to be executed in view
     */
    void show_next_stat(addr_t);

    void show_cur_stat(addr_t);

    /**
     * Scroll to the line where the function encompassing
     * given address starts.
     */
    void show_func_start(addr_t);

    void scroll_to_position(size_t);

    void scroll_to_line(size_t);

    void mark_line(
        size_t lineNumber,
        const Gdk_Color& fore,
        const Gdk_Color& back);

    size_t line_count() const { return lineMap_.size(); }

    size_t get_line(Symbol&) const;

    /**
     * Calls the debugger engine to disassemble
     * the code starting at the given symbol.
     * @return true if the listing is ready available,
     * or false if an asynchronous reading of memory is
     * needed.
     */
    bool disasm_async(Thread&, Symbol&, bool hilite);

    int get_font_width();
    void set_asm_tabstops();
    void apply_tab_width();

    void check_before_insert_asm_line(size_t currentPos);

    /**
     * Implements the Disassembler::OuputCallback interface
     */
    bool notify(addr_t, const char*, size_t);
    bool tabstops(size_t*, size_t*) const;

    unsigned int line_number_width() const;

    size_t line_by_addr(addr_t) const;

    /**
     * Wrap the emission of change_source signal.
     * @return true if the signal was handled.
     */
    bool change_source_file(
                       const RefPtr<Symbol>&,
                       const RefPtr<Thread>&,
                       const RefPtr<Frame>&,
                       bool hilite,
                       ViewType viewType);

    int scroll_to_line_internal(size_t);

    void search_impl(size_t fromPosition);

    void search_forward(size_t fromPosition);

    void search_backward(size_t fromPosition);

    bool search_buf(size_t, const std::string&, bool = true);

    void reset_text();

    void set_symbol(const RefPtr<Symbol>&,
                    const RefPtr<Thread>&,
                    const RefPtr<Frame>&);

    bool on_map_event(GdkEventAny*);
    bool on_motion_notify_event(GdkEventMotion*);
    bool on_leave_notify_event(GdkEventCrossing*);

    bool on_mouse_timer();

    /**
     * Clears the text buffer but keeps the view type,
     * keeps thread and current symbol information
     */
    void reset();
    void reset_tips();

    bool is_different_file(const Symbol*) const;

    bool getline(std::istream& is, char* buf, size_t buflen);

    template<typename S>
    static S& ensure_new_line(S& line)
    {
        //deal with DOS text
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
           line.replace(line.size() - 1, 1, 1, '\n');
           assert(line[line.size() - 1] == '\n');
        }
        else
        {
            line += '\n';
        }
        return line;
    }

    void set_language_from_file_ext(const std::string&, addr_t);

    bool bring_in_view
      (
        const RefPtr<Symbol>&,
        const RefPtr<Thread>&,
        const RefPtr<Frame>&,
        bool hilite,
        ViewType
      );

private:
    typedef std::set<addr_t> BreakPointList;

    // Maps line numbers (in the Gtk::Text widget, which,
    // BTW, may differ from line numbers in the source code)
    // to a list of breakpoints
    typedef std::map<size_t, BreakPointList> BreakPointMap;

    const Gdk_Color foreGnd_;
    const Gdk_Color backGnd_;
    const Gdk_Color lineCol_;
    Gdk_Font        font_;

    Debugger&       debugger_;
    MenuBuilder<CodeView>& menuBuilder_;

    RefPtr<SharedString> file_; // name of current file
    // map line numbers to position (in chars) where they start
    LineMap         lineMap_;   // map line numbers to offsets
    AddrMap         addrMap_;   // map addrs to line numbers
    LineMap reverseLineMap_;    // offsets to line numbers
    size_t          curLine_;   // current high-lighted line
    BreakPointMap   breakpointMap_;

    addr_t          lower_;     // range of disassembled addresses
    addr_t          upper_;     //  when showing asm code

    ViewType        type_;
    RefPtr<Symbol>  symbol_;    // current symbol

    RefPtr<Thread>  thread_;    // thread in view
    RefPtr<Frame>   frame_;     // frame in view

    size_t          searchResult_;
    std::string     search_;    // search string

    ///// <popup menu>
    RightClickInfo  rClick_;
    std::string     wordAtCursor_;
    RightClickContext menuContext_;
    ///// </popup menu>

    bool            inChangeSource_;
    bool            fallenBack_;
    mutable Mutex   mutex_;
    SigC::Connection timer_;
    TipsPtr         tips_;
};

#endif // CODE_VIEW_H__D71F3650_B6F7_42C8_8DF7_5380BA45795F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
