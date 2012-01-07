//
// $Id: program_view.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <iostream>
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "generic/temporary.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/color.h"
#include "gtkmm/connect.h"
#include "gtkmm/events.h"
#include "gtkmm/eventbox.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "zdk/check_ptr.h"
#include "zdk/history.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/utility.h"
#include "zdk/zero.h"
#include "code_view.h"
#include "command.h"
#include "fixed_font.h"
#include "memory_req.h"
#include "program_view.h"
#include "right_menu.h"
#include "slot_macros.h"
#include "src_tabs.h"
#include "stack_view.h"
#include "utils.h"
#include "icons/close.xpm"


using namespace std;



ProgramView::ProgramView(Debugger& debugger)
    : debugger_(debugger)
    , current_(0)
    , disableSwitchPageCallback_(false)
    , hasViews_(false)
{
    set_scrollable(true);
    set_show_tabs(true);
    set_show_border(true);

    popup_enable();

    Properties* prop = debugger.properties();
    if (prop)
    {
        fontName_ = prop->get_string("cfont", fixed_font().c_str());
    }
    init(prop);
    hasViews_ = false;
}


ProgramView::~ProgramView() throw()
{
    clear(false);
}


void ProgramView::init(Properties* prop)
{
    create_code_view();
    add_tab_label(0, "(untitled)");

    if (menu_empty())
    {
        register_menu(new MenuInsertBreakpoint);
        register_menu(new MenuRunToCursor);
        register_menu(new MenuSetProgramCounter);
        register_menu(new MenuShowNextStatement);
        register_menu(new MenuShowFunctionStart);
        register_menu(new MenuOpenFolder);
    }
}


/**
 * Brings symbol and thread in view, optionally
 * hilites the symbols, and updates history for Back/Forward navigation.
 * @note May scroll if necessary, to bring the symbol in view.
 */
void ProgramView::set_current
(
    RefPtr<Symbol>   sym,
    RefPtr<Thread>   thread,
    RefPtr<Frame>    frame,
    bool             hilite,
    ViewType         viewType
)
{
    history_save();
    set_current_view(sym, thread, frame, hilite, viewType);
}


/**
 * Brings symbol and thread in view, optionally
 * hilites the symbols, and updates history for Back/Forward navigation.
 *
 * Does NOT save navigation history.
 */
void ProgramView::set_current_view
(
    const RefPtr<Symbol>&   sym,
    const RefPtr<Thread>&   thread,
    const RefPtr<Frame>&    frame,
    bool                    hilite,
    ViewType                viewType
)
{
    size_t n = 0;
    if (ViewPtr view = find_page(sym, n))
    {
        view->set_elements_in_view(sym, thread, frame, hilite, viewType);
    }
    else
    {
        CodeView* view = 0;
        if (current_view().file())
        {
            view = create_code_view().get();
        }
        else
        {
            view = &current_view();
        }
        view->set_elements_in_view(sym, thread, frame, hilite, viewType);

        assert(views_.size());
        n = views_.size() - 1;
    }
    set_current_page(n);
    current_ = n; // work-around for gtk 1.2
    hasViews_ = current_view().file();
    views_changed(); // this updates the UI state
}


void
ProgramView::set_current(RefPtr<Thread> thread, const string& filename)
{
    size_t pageIndex = 0;

    if (find_page_by_name(filename, pageIndex))
    {
        set_current_page(pageIndex);

        hasViews_ = true;
        views_changed();
        assert(current_view().thread());
    }
    else
    {
        //
        // read the source file
        //
        ViewPtr view = create_code_view();
        const size_t pageIndex  = views_.size() - 1;

        if (!thread)
        {
            thread = debugger_.get_thread(DEFAULT_THREAD);
        }
        // note: set the thread before reading the source file,
        // so that the breakpoint query inside read_source_file
        // works correctly

        view->set_thread(thread.get());

        if (view->read_source_file(shared_string(filename)))
        {
            set_current_page(pageIndex);

            assert(&current_view() == view.get());
            assert(view->thread());
        }
        else
        {
            close_tab(pageIndex);
        }
    }
}



bool
ProgramView::find_page_by_name(const string& fname, size_t& index) const
{
    return find_page(shared_string(fname), &index);
}


void ProgramView::show_breakpoint(RefPtr<BreakPoint> bpnt)
{
    set_current(
        CHKPTR(bpnt)->symbol(),
        CHKPTR(bpnt->thread()),
        thread_current_frame(CHKPTR(bpnt->thread())),
        true);
}


/**
 * Same as set_current(), only it does not update history.
 */
void
ProgramView::show_function( const RefPtr<Symbol>& fun,
                            const RefPtr<Thread>& thread)
{
    if (fun && thread)
    {
        RefPtr<Frame> frame = thread_current_frame(thread.get());
        current_view().set_elements_in_view(fun, thread, frame, true);
        //set_current_view(fun, thread, frame, true, VIEW_DEFAULT);
    }
}


void ProgramView::redraw()
{
    current_view().redraw();
}



/**
 * get the current symbol in view
 */
RefPtr<Symbol> ProgramView::symbol() const
{
    return current_view().symbol();
}


RefPtr<Thread> ProgramView::thread() const
{
    return current_view().thread();
}


void ProgramView::set_thread(const RefPtr<Thread>& thread)
{
    current_view().set_thread(thread.get());
}


/**
 * @return the name of the file currently in view
 */
RefPtr<SharedString> ProgramView::file() const
{
    RefPtr<SharedString> filename = current_view().file();
    return filename;
}


void ProgramView::clear(bool reinit)
{
    Temporary<bool> setFlag(disableSwitchPageCallback_, true);
    history_clear();
    pages().clear();
    views_.clear();
    current_ = 0;

    if (reinit)
    {
        init();
    }
    assert(get_current_page() <= 0);
    hasViews_ = false;
    if (reinit)
    {
        views_changed();
    }
}


void ProgramView::close_tab(size_t pageIndex)
{
    assert(pages().size() == views_.size());

    {// prevent on_switch_page from kicking in
        Temporary<bool> setFlag(disableSwitchPageCallback_, true);

        remove_page(pageIndex);
        views_.erase(views_.begin() + pageIndex);
    }
    if (views_.empty())
    {
        init(); // always show one page
        // should only happen when last page is closed
        assert(pageIndex == 0);

        hasViews_ = false;
        views_changed();
    }

    assert(views_.size() == pages().size());

    int current = get_current_page();
    if (current < 0)
    {
        current = 0;
        // should only happen when last page is closed
        assert(pageIndex == 0);
    }
    on_switch_page(NULL, current);

    assert(current_ < views_.size());
}


void ProgramView::set_view_type(ViewType type)
{
    assert(type != VIEW_DEFAULT);
    current_view().set_view_type(type);
}


/**
 * @return the current view type, can be one of
 * VIEW_SOURCE, VIEW_DISASSEMBLED or VIEW_MIXED
 */
ViewType ProgramView::view_type() const
{
    return current_view().view_type();
}


/**
 * True if the ViewType is either VIEW_DISASSEMBLED or
 * VIEW_MIXED, and the disassembly was successful.
 */
bool ProgramView::have_disassembly() const
{
    return current_view().have_disassembly();
}


void ProgramView::force_mixed_view()
{
    CodeView& view = current_view();

    if (view.view_type() == VIEW_SOURCE)
    {
        RefPtr<Symbol> symbol = view.symbol();
        RefPtr<Frame> frame = view.frame();
        RefPtr<Thread> thread = view.thread();

        view.set_view_type(VIEW_MIXED);
        view.disasm_fallback();

        if (!view.symbol())
        {
        #if DEBUG
            clog << __func__ << ": refreshing...\n";
        #endif
            view.set_elements_in_view(symbol, thread, frame, true);
        }
    }
}


/**
 * A hack for speeding up the lookup of the next address
 * to be executed. Rather than using the symbol tables,
 * use the info cached by this widget.
 * @todo: document param, return
 *
 */
addr_t ProgramView::next_addr(addr_t addr) const
{
    addr_t next = current_view().next_addr(addr);
    return next;
}


/**
 * @return the current line
 * @note this may not be equal to the current symbol's line,
 * if we're showing disassembled code
 */
size_t ProgramView::current_line() const
{
    return current_view().current_line();
}



void ProgramView::hilite_line(size_t line,
                              bool hilite,
                              BreakPointState state)
{
    CodeView& view = current_view();

    if (state == B_AUTO)
    {
        if (line)
        {
            state = view.query_breakpoint_at_line(line);
            view.hilite_line(line, hilite, state);
        }
    }
    else
    {
        view.hilite_line(line, hilite, state);
    }
}


bool ProgramView::is_line_visible(size_t line) const
{
    return current_view().is_line_visible(line);
}


BreakPointState
ProgramView::query_breakpoint_at_line(size_t line) const
{
    return current_view().query_breakpoint_at_line(line);
}


std::string ProgramView::selection() const
{
    return current_view().selection();
}


void ProgramView::search(const std::string& str)
{
    current_view().search(str);
}


void ProgramView::search_again()
{
    current_view().search_again();
}


void ProgramView::refresh_line(size_t line)
{
    current_view().refresh_line(line);
}


/**
 * Notification that the breakpoint at the given address
 * needs to be redrawn (if in current view) because its
 * enabled/disable state has changed.
 */
void ProgramView::breakpoint_state_changed(addr_t addr)
{
    current_view().breakpoint_state_changed(addr);
}


/**
 * @return true if breakpoint is new -- for example,
 * if a disabled breakpoint is enabled, this function
 * returns false.
 */
bool
ProgramView::on_insert_breakpoint(const RefPtr<Symbol>& sym, bool isDeferred)
{
    bool result = false;

    size_t pageIndex = 0;

    if (ViewPtr v = find_page(sym, pageIndex))
    {
        result = v->on_insert_breakpoint(sym, isDeferred);
    }
    return result;
}


void
ProgramView::on_remove_breakpoint(const RefPtr<Symbol>& sym, bool isDeferred)
{
    size_t pageIndex = 0;

    if (ViewPtr v = find_page(sym, pageIndex))
    {
        v->on_remove_breakpoint(sym, isDeferred);
    }
}


/**
 * called when read_memory_async completes
 */
void ProgramView::on_read_done(RefPtr<MemoryRequest> req)
{
    if (ViewPtr view = find_page(req->file))
    {
        view->on_read_done(req);
    }
    else if (req->file)
    {
    #ifdef DEBUG
        clog << req->file->c_str() << ": not found, using active view\n";
    #endif
        current_view().on_read_done(req);
    }
}


template<typename T> static void
save_current(T& trace, CodeView& view, Debugger& debugger)
{
    if (RefPtr<Symbol> sym = view.symbol())
    {
        if (trace.empty() || (sym->addr() != trace.back().sym->addr()))
        {
            ProgramView::Snapshot entry = {
                sym,
                view.thread(),
                view.frame(),
                view.view_type()
            };
            if (!entry.thread || !thread_is_attached(*entry.thread))
            {
                entry.thread = debugger.get_thread(DEFAULT_THREAD);
            }
            trace.push_back(entry);
        }
        else if (trace.back().thread != view.thread())
        {
            trace.back().thread = view.thread();
        }
    }
}


BEGIN_SLOT(ProgramView::history_back,())
{
    navigate(traceFwd_, traceBack_, debugger_);
    history_can_back(traceBack_.size());
    history_can_forward(traceFwd_.size());
}
END_SLOT()



BEGIN_SLOT(ProgramView::history_forward,())
{
    navigate(traceBack_, traceFwd_, debugger_);
    history_can_back(traceBack_.size());
    history_can_forward(traceFwd_.size());
}
END_SLOT()



BEGIN_SLOT(ProgramView::history_save,())
{
    save_current(traceBack_, current_view(), debugger_);
    history_can_back(traceBack_.size());
}
END_SLOT()



void ProgramView::history_clear()
{
    traceBack_.clear();
    traceFwd_.clear();

    history_can_back(0);
    history_can_forward(0);
}


CodeView& ProgramView::current_view()
{
    assert(!views_.empty());
    assert(current_ < views_.size());
    return *views_[current_];
}


const CodeView& ProgramView::current_view() const
{
    assert(!views_.empty());
    assert(current_ < views_.size());

    return *views_[current_];
}


Frame* ProgramView::frame() const
{
    return current_view().frame();
}


ProgramView::ViewPtr ProgramView::create_code_view()
{
    ViewPtr view(new CodeView(debugger_, *this, fontName_));
    views_.push_back(view);

    Widget* sw = manage(view->add_scrolled_window());
    add(*sw);
    sw->show_all();

    view->set_breakpoints.connect(set_breakpoints.slot());
    view->delete_breakpoint.connect(delete_breakpoint.slot());
    view->disable_breakpoint.connect(disable_breakpoint.slot());
    view->enable_breakpoint.connect(enable_breakpoint.slot());
    view->set_program_count.connect(set_program_count.slot());
    view->show_next_line.connect(show_next_line.slot());
    view->string_not_found.connect(string_not_found.slot());
    view->read_memory_async.connect(read_memory_async.slot());
    view->read_symbol.connect(read_symbol.slot());
    view->can_interact.connect(can_interact.slot());
    view->symbol_changed.connect(symbol_changed.slot());
    view->query_symbols.connect(query_symbols.slot());
    view->evaluate.connect(evaluate.slot());
    view->status_message.connect(status_message.slot());
    view->file_changed.connect(Gtk_SLOT(this, &ProgramView::on_file_changed));
    view->run_on_main_thread.connect(run_on_main_thread.slot());
    view->step_over_func.connect(step_over_func.slot());
    view->step_over_file.connect(step_over_file.slot());
    view->step_over_dir.connect(step_over_dir.slot());
    view->step_over_reset.connect(step_over_reset.slot());
    view->step_over_manage.connect(step_over_manage.slot());
    view->accept_invalid_utf8.connect(accept_invalid_utf8.slot());
    view->filter.connect(filter.slot());

    hasViews_ = true;
    views_changed();
    return view;
}


ProgramView::ViewPtr
ProgramView::find_page(const RefPtr<Symbol>& sym, size_t& n) const
{
    return find_page(CHKPTR(sym)->file(), &n);
}


ProgramView::ViewPtr
ProgramView::find_page(RefPtr<SharedString> file, size_t* index) const
{
    Views::const_iterator i = views_.begin();

    for (size_t n = 0; i != views_.end(); ++i, ++n)
    {
        boost::shared_ptr<CodeView> v = CHKPTR(*i);
        RefPtr<SharedString> vfile = v->file();
        if (!vfile)
        {
            continue;
        }
        if (vfile->is_equal2(file.get()))
        {
            if (index)
            {
                *index = n;
            }
            return v;
        }
    }
    return ViewPtr();
}



void ProgramView::on_file_changed(CodeView* view)
{
    Views::iterator i = views_.begin();
    for (size_t n = 0; i != views_.end(); ++i, ++n)
    {
        if (i->get() == view)
        {
            set_tab_name(n, CHKPTR(view->file()));
            break;
        }
    }
}


void
ProgramView::set_tab_name(size_t i, const RefPtr<SharedString>& file)
{
    // show the short name only (no path)
    const char* fname = CHKPTR(file)->c_str();

    if (const char* p = strrchr(fname, '/'))
    {
        fname = ++p;
    }

    add_tab_label(i, fname);
}


void
ProgramView::on_switch_page(Gtk_NOTEBOOK_PAGE* page, guint pageNum)
{
    if (page)
    {
        Notebook_Adapt::on_switch_page(page, pageNum);
    }
    if (!disableSwitchPageCallback_)
    {
        const bool pageChanged = (current_ != pageNum);
        current_ = pageNum;

        if (pageChanged && !views_.empty())
        {
            CodeView& view = current_view();
            Frame* frame = view.frame();
            symbol_changed.emit(view.thread(), view.symbol(), frame);
        }
    }
}


bool ProgramView::apply_font(const string& name)
{
    if (name == fixed_font())
    {
        return false;
    }
    fontName_ = name;
    CHKPTR(debugger_.properties())->set_string("cfont", name.c_str());

    Gdk_Font font(name);

    Views::iterator v = views_.begin();
    for (; v != views_.end(); ++v)
    {
        (*v)->apply_font(font);
    }
    return true;
}


void ProgramView::add_tab_label(size_t pageIndex, const char* text)
{
    get_page(pageIndex).set_menu_label_text(text);

    if (!close_) // create the close pixmap if have not done so already
    {
        close_.reset(new Gtk::Pixmap(close_xpm));
    }
    Gtk::Button* xbtn = manage(new Gtk::Button());

    if (close_) // have pixmap?
    {
        Gtk_add_pixmap(*xbtn, *close_);
        xbtn->set_relief(Gtk_FLAG(RELIEF_NONE));
    }
    else
    {
        xbtn->add_label("x");
    }
    Gtk::Box* box = manage(new Gtk::HBox);
    Gtk::Label* label = manage(new Gtk::Label(text, .0, .5));

    Gtk::Label* filler = manage(new Gtk::Label("    "));
    box->pack_start(*filler, false, false);
    box->pack_start(*label, false, false);
    box->pack_end(*xbtn, false, false);

    box->show_all();
    get_page(pageIndex).set_tab_label(*box);
    Gtk_CONNECT_1(  xbtn,
                    clicked,
                    this,
                    &ProgramView::close_page,
                    get_nth_page(pageIndex));
}



void ProgramView::save_open_tabs(Thread& thread)
{
    if (debugger_.properties()->get_word("atexit.savetabs", 0) == 0)
    {
        return;
    }
    if (RefPtr<Properties> props = get_history_properties(thread))
    {
        RefPtr<SourceTabs> srcTabs(new SourceTabs(thread.filename()));
        const size_t ntabs = views_.size();
    #if DEBUG
        clog << __func__ << ": " << ntabs << endl;
    #endif
        for (size_t i = 0, n = 0 ; i != ntabs; ++i, ++n)
        {
            if (RefPtr<SharedString> file = views_[i]->file())
            {
                // for now, save only source files (no ASM)

                if (views_[i]->view_type() == VIEW_SOURCE)
                {
                    srcTabs->add_file_name(views_[i]->file());
                }
            }
        }
        props->set_object("tabs", srcTabs.get());
    }
}


void ProgramView::restore_open_tabs(Thread& thread)
{
    if (RefPtr<Properties> props = get_history_properties(thread))
    {
        if (RefPtr<Process> process = thread.process())
        {
            if (process->enum_threads() > 1)
            {
                return; // do it only for the first thread
            }
        }
        SourceTabs* tabs = interface_cast<SourceTabs*>(props->get_object("tabs"));

        if (tabs)
        {
            SourceTabs::const_iterator i = tabs->begin();
            for (; i != tabs->end(); ++i)
            {
                ViewPtr view = create_code_view();
                view->read_source_file(*i);
                view->set_thread(&thread);
            }
            close_tab(0); // close the default tab
        }
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
