//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "code_view.h"
#include "const.h"
#include "zdk/align.h"
#include "zdk/breakpoint_util.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/zero.h"
#include "command.h"
#include "controller.h"
#include "menu.h"
#include "flcode_table.h"
#include <fstream>
#include <sstream>

using namespace std;


////////////////////////////////////////////////////////////////
ui::CodeView::CodeView(ui::Controller& controller)
    : ui::View(controller)
    , parent_(nullptr)
{
}


ui::CodeView::~CodeView() throw()
{
}


void ui::CodeView::set_current_symbol(const RefPtr<Symbol>& sym)
{
    current_ = sym;
}


void ui::CodeView::show_contextual_menu(int x, int y)
{
    RefPtr<PopupMenu> menu(controller().init_contextual_menu());

    auto listing = get_listing();

    if (listing && controller().state().current_thread()
                && controller().state().current_thread()->is_live())
    {
        const addr_t addr = listing->selected_addr();
        auto& d = *CHKPTR(controller().debugger());

        // have breakpoint at current adddress?
        bool haveBreakPoint = false;

        if (has_enabled_user_breakpoint_actions(d, addr))
        {
            haveBreakPoint = true;

            menu->add_item("Disable breakpoint", 0, ui::Enable_IfStopped,
                [this, &d, addr] {
                disable_user_breakpoint_actions(d, addr);
            });
        }
        else if (has_disabled_user_breakpoint_actions(d, addr))
        {
            haveBreakPoint = true;

            menu->add_item("Enable breakpoint", 0, ui::Enable_IfStopped,
                [&d, addr] {
                enable_user_breakpoint_actions(d, addr);
            });
        }

        if (haveBreakPoint)
        {
            menu->add_item("Remove breakpoint", 0, ui::Enable_IfStopped,
                [this, addr] {
                controller().set_user_breakpoint(addr, false);
            });

            menu->add_ui_item("Edit breakpoint", 0, ui::Enable_IfStopped,
                [this, addr] {
                controller().show_edit_breakpoint_dialog(addr);
            },
            true /* append menu divider */);
        }
        else
        {
            menu->add_item("Insert breakpoint", 0, ui::Enable_IfStopped,
                [this, addr] {
                controller().set_user_breakpoint(addr, true);
            },
            true /* append menu divider */);
        }

        menu->add_item("Run to here", 0, ui::Enable_IfStopped,
            [this, addr] {
            controller().set_temp_breakpoint(addr);
            controller().debugger()->resume();
        });

    }

    menu->show(x, y);
}


////////////////////////////////////////////////////////////////
ui::SourceView::SourceView(Controller& controller)
    : ui::CodeView(controller)
{
}


ui::SourceView::~SourceView() throw()
{
}


const char* ui::SourceView::current_file() const
{
    return filename_.c_str();
}


size_t ui::SourceView::current_line() const
{
    return current_symbol() ? current_symbol()->line() : 0;
}


/**
 * Helper class for populating a LineAddrMap.
 */
class LineAddrMapper : public EnumCallback2<SymbolTable*, addr_t>
{
public:
    explicit LineAddrMapper(ui::LineAddrMap& m)
        : m_(m), lineNum_(0)
    { }

    void notify(SymbolTable*, addr_t addr) {
        m_.add(lineNum_, addr);
    }

    void set_line_num(int lineNum) {
        lineNum_ = lineNum;
    }

private:
    ui::LineAddrMap& m_;
    int lineNum_;
};


void ui::SourceView::read_file(Thread* t, const char* filename)
{
    assert(filename_ != filename);
    filename_ = filename;

    lines_.clear();

    // if source code lines exceed this size, though luck
    vector<char> buf(2048);

    //
    // read text lines into a vector of strings
    //
    ifstream fin(filename);
    if (fin)
    {
        filename_ = filename;

        while (fin.getline(&buf[0], buf.size()))
        {
            lines_.push_back(&buf[0]);
        }
    }

    //
    // map source lines to addresses
    //
    lineAddrMap_.clear();
    LineAddrMapper addrMapper(lineAddrMap_);

    SharedStringPtr fname(shared_string(filename));

    for (size_t i = 0; i != lines_.size(); ++i)
    {
        addrMapper.set_line_num(i + 1);
        controller().debugger()->line_to_addr(fname.get(), i + 1, &addrMapper, t);
    }
}


addr_t ui::SourceView::selected_addr() const
{
    return lineAddrMap_.line_to_addr(selected_line());
}


bool ui::SourceView::refresh(

    const RefPtr<Thread>& t,
    const RefPtr<Symbol>& sym )

{
    set_current_symbol(sym);

    const char* filename = sym->file()->c_str();
    if (filename_ == filename)
    {
        return false;
    }
    read_file(t.get(), filename);
    return true;
}


void ui::SourceView::show_contextual_menu(int x, int y)
{
    ui::CodeView::show_contextual_menu(x, y);
}


////////////////////////////////////////////////////////////////

ui::MultiCodeView::MultiCodeView(ui::Controller& controller)
    : ui::CodeView(controller)
{
}


ui::MultiCodeView::~MultiCodeView() throw()
{
}


const ui::CodeListing* ui::MultiCodeView::get_listing()
{
    return currentView_ ? currentView_->get_listing() : nullptr;
}


void ui::MultiCodeView::clear()
{
    views_.clear();
}


void ui::MultiCodeView::update(const ui::State& s)
{
    for (auto v = views_.begin(); v != views_.end(); ++v)
    {
        v->second->update(s);
    }

    if (s.current_event() == E_TARGET_FINISHED)
    {
        // target has detached, close all views
        clear();
        return;
    }

    RefPtr<Symbol> sym; // current symbol in view

    RefPtr<Thread> t = s.current_thread();
    if (t)
    {
        if (RefPtr<Frame> f = t->stack_trace()->selection())
        {
            sym = f->function();
        }
    }

    if (sym)
    {
        SharedStringPtr filename = sym->file();
        auto i = views_.find(filename);

        if (i == views_.end())
        {
            CodeViewPtr cv = make_view(*sym);
            i = views_.insert(make_pair(filename, cv)).first;

            Layout::CallbackPtr cb(make_callback());
            assert(cb);

            cv->insert_self(*cb);
            cv->set_parent(this);
        }
        i->second->show(t, sym);

        // If the event is caused by a change in the debug target
        // then update the current view; if the event is caused by
        // some user interaction (E_PROMPT) then leave it alone,
        // as the user may have explicitly changed it.

        if (s.current_event() != E_PROMPT || current_symbol() != sym)
        {
            currentView_ = i->second;
        }
        set_current_symbol(sym);
    }

    make_visible(currentView_);
}


void ui::MultiCodeView::update_breakpoint(BreakPoint& bpnt)
{
    if (RefPtr<Symbol> sym = bpnt.symbol())
    {
        SharedStringPtr filename = sym->file();

        auto i = views_.find(filename);
        if (i != views_.end())
        {
            i->second->update_breakpoint(bpnt);
        }
    }
}


////////////////////////////////////////////////////////////////
void ui::LineAddrMap::add(int lineNum, addr_t addr)
{
    addrToLine_[addr] = lineNum;
    lineToAddr_.insert(make_pair(lineNum, addr));
}


addr_t ui::LineAddrMap::line_to_addr(int lineNum) const
{
    auto i = lineToAddr_.find(lineNum);
    if (i == lineToAddr_.end())
    {
        ostringstream s;
        s << __func__ << ": line number " << lineNum << " is out of range";
        throw out_of_range(s.str());
    }
    return i->second;
}


int ui::LineAddrMap::addr_to_line(addr_t addr) const
{
    auto i = addrToLine_.find(addr);
    if (i == addrToLine_.end())
    {
        ostringstream s;
        s << __func__ << ": 0x" << hex << addr << dec << " is out of range";
        throw out_of_range(s.str());
    }
    return i->second;
}


////////////////////////////////////////////////////////////////
ui::AsmView::AsmView(ui::Controller& controller)
    : CodeView(controller)
{
}


ui::AsmView::~AsmView() throw()
{
}


void ui::AsmView::add_listing_line(

    addr_t          addr,
    const string&   line )

{
    int lineNum = lines_.size();

    lines_.push_back(line);
    set_row_count(lines_.size());

    lineAddrMap_.add(lineNum, addr);
}


const char* ui::AsmView::current_file() const
{
    return current_symbol() ? current_symbol()->file()->c_str() : "";
}


size_t ui::AsmView::current_line() const
{
    return current_symbol() ? current_symbol()->line() : 0;
}


const string& ui::AsmView::line(size_t n) const
{
    assert (n < lines_.size());

    return lines_[n];
}


addr_t ui::AsmView::selected_addr() const
{
    return lineAddrMap_.line_to_addr(selected_line());
}


/**
 * In order to disassemble a portion of the debug target
 * we need to read its memory -- which must happen on the
 * main debugger thread. This thread affinity restriction
 * comes from the fact that only the thread who called the
 * initial PTRACE_ATTACH operation can perform subsequent
 * ptrace calls successfully on that target.
 */
class DisasmCommand : public ui::Command, Disassembler::OutputCallback
{
public:
    DisasmCommand(RefPtr<ui::AsmView> view, RefPtr<Thread> t, RefPtr<Symbol> s)
        : view_(view)
        , thread_(t)
        , start_(s)
        , complete_(false)
    { }

protected:
    ~DisasmCommand() throw()
    { }

    void execute_on_main_thread()
    {
        const size_t nwords = round_to_word(ui::Const::asm_window_size);
        buffer_.resize(nwords * sizeof(word_t));
        word_t* wordBuf = reinterpret_cast<word_t*>(&buffer_[0]);
        thread_->read_code(start_->addr(), wordBuf, nwords);

        Debugger* debugger = CHKPTR(thread_->debugger());
        debugger->disassemble(
            thread_.get(),
            start_.get(),
            buffer_.size(),
            false,   /* include source code in listing? */
            &buffer_[0],
            this);
    }

    void continue_on_ui_thread() {
        complete_ = true;
    }

    bool is_complete() const {
        return complete_;
    }
    //
    // Disassembler::OutputCallback interface
    //
    bool notify(addr_t addr, const char* line, size_t lineLength)
    {
        view_->add_listing_line(addr, string(line, lineLength));
        return true;
    }

    bool tabstops(size_t*, size_t*) const
    {
        return true;
    }

private:
    RefPtr<ui::AsmView> view_;
    RefPtr<Thread>      thread_;
    RefPtr<Symbol>      start_;
    vector<uint8_t>     buffer_;
    bool                complete_;
};


int ui::AsmView::addr_to_line(addr_t addr) const
{
    return lineAddrMap_.addr_to_line(addr);
}


bool ui::AsmView::refresh(

    const RefPtr<Thread>&   t,
    const RefPtr<Symbol>&   s )

{
    if (s == current_symbol())
    {
        return false;
    }

    lines_.clear();         // reset listing lines
    lineAddrMap_.clear();

    set_current_symbol(s);

    addr_t addr = s->addr();

    // disassemble before the symbol in view
    // so that the user can scroll up and down
    addr -= ui::Const::asm_window_size / 2;

    RefPtr<SymbolMap> symbols = CHKPTR(t)->symbols();
    RefPtr<Symbol> start = CHKPTR(symbols)->lookup_symbol(addr);
    if (!start)
    {
        start = s;
    }

    RefPtr<ui::Command> disassemble = new DisasmCommand(this, t, start);
    disassemble->execute_on_main_thread();

    return true;
}


void ui::AsmView::show_contextual_menu(int x, int y)
{
    ui::CodeView::show_contextual_menu(x, y);
}

