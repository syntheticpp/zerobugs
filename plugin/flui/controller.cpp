// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/breakpoint_util.h"
#include "zdk/check_ptr.h"
#include "zdk/log.h"
#include "zdk/thread_util.h"
#include "generic/temporary.h"
#include "breakpoint_view.h"
#include "const.h"
#include "code_view.h"
#include "controller.h"
#include "dialog.h"
#include "locals_view.h"
#include "stack_view.h"
#include "toolbar.h"
#include "menu.h"
#include <FL/Enumerations.H>
#include "dharma/system_error.h"
#include <pthread.h>
#include <iostream>

// properties
#define WINDOW_X        "flui.window.x"
#define WINDOW_Y        "flui.window.y"
#define WINDOW_W        "flui.window.width"
#define WINDOW_H        "flui.window.height"

using namespace std;


/**
 * Pass debugger and debuggee state information
 * from main thread to UI thread.
 */
class ui::Controller::StateImpl : public ui::State
{
    RefPtr<Symbol>      currentSymbol_;
    RefPtr<Thread>      currentThread_;
    EventType           currentEventType_;
    size_t              attachedThreadCount_;
    bool                isTargetStopped_;

public:
    StateImpl( )
        : currentEventType_(E_NONE)
        , attachedThreadCount_(0)
        , isTargetStopped_(false)
    { }

    virtual void update(Thread*, EventType);

    virtual bool is_target_running() const {
        bool result = (currentEventType_ == E_TARGET_RESUMED);
        return result;
    }

    virtual bool is_target_stopped() const {
        return isTargetStopped_;
    }

    virtual EventType current_event() const {
        return currentEventType_;
    }

    virtual RefPtr<Symbol> current_symbol() const {
        return currentSymbol_;
    }

    virtual RefPtr<Thread> current_thread() const {
        return currentThread_;
    }
};


void ui::Controller::StateImpl::update(

    Thread*     currentThread,
    EventType   eventType )

{
    isTargetStopped_    = false;
    currentThread_      = currentThread;
    currentEventType_   = eventType;
    currentSymbol_.reset();

    if (currentThread && eventType != E_TARGET_RESUMED)
    {
        isTargetStopped_ = thread_stopped(*currentThread)
            // core dumps are neither "running" or "stopped"
            && currentThread->is_live();

        addr_t pc = currentThread->program_count();
        assert(currentThread->symbols());

        currentSymbol_ = currentThread->symbols()->lookup_symbol(pc);
    }
}


////////////////////////////////////////////////////////////////
class BreakPointUpdater : public EnumCallback<volatile BreakPoint*>
{
public:
    explicit BreakPointUpdater(ui::Controller& controller)
        : controller_(controller)
    { }

    // Add view to the internal list of views to be updated
    void push_back(RefPtr<ui::View> view)
    {
        if (view)
        {
            views_.push_back(view);
        }
    }

    bool empty() const {
        return views_.empty();
    }

    void notify(volatile BreakPoint* bp)
    {
        // we only care about user-defined breakpoints,
        // the internal breakpoints used by the engine
        // are not shown

        if (bp->enum_actions("USER"))
        {
            auto& breakPoint = const_cast<BreakPoint&>(*bp);

            if (auto d = controller_.current_dialog())
            {
                d->update_breakpoint(breakPoint);
            }

            for (auto v = begin(views_); v != end(views_); ++v)
            {
                (*v)->update_breakpoint(breakPoint);
            }
        }
    }

private:
    typedef vector<RefPtr<ui::View> > Views;

    ui::Controller& controller_;
    Views           views_;
};


////////////////////////////////////////////////////////////////
/**
 * An error may occur while executing a command on the main thread.
 * Should that happen, the controller replaces the command in the
 * mail-slot with this one, so that an error message is shown in
 * the UI thread.
 */
class CommandError : public ui::Command
{
    ui::Controller& controller_;
    string          msg_;

public:
    CommandError(ui::Controller& controller, const char* m)
        : controller_(controller), msg_(m)
    { }

    virtual ~CommandError() throw() { }

    void continue_on_ui_thread() {
        controller_.error_message(msg_);
    }
};


////////////////////////////////////////////////////////////////
ui::IdleCommand::IdleCommand() : cancelled_(false)
{
}


void ui::IdleCommand::execute_on_main_thread()
{
    Lock<Mutex> lock(mutex_);
    for (cancelled_ = false; !cancelled_; )
    {
        cond_.wait(lock);
    }
}


void ui::IdleCommand::cancel()
{
    set_cancel();
    cond_.broadcast();
}


void ui::IdleCommand::set_cancel()
{
    Lock<Mutex> lock(mutex_);
    cancelled_ = true;
}

////////////////////////////////////////////////////////////////
//
// Controller implementation
//
ui::Controller::Controller()
    : debugger_(nullptr)
    , threadId_(0)
    , state_(init_state())
    , done_(false)
    , probing_(false)
    , idle_(new IdleCommand)
{
}


////////////////////////////////////////////////////////////////
ui::Controller::~Controller()
{
}


////////////////////////////////////////////////////////////////
unique_ptr<ui::Controller::StateImpl> ui::Controller::init_state( )
{
    return unique_ptr<StateImpl>(new StateImpl());
}


////////////////////////////////////////////////////////////////
void ui::Controller::build()
{
    Properties& prop = *CHKPTR(debugger_)->properties();
    // get coordinates and dimensions from saved properties
    const word_t x = prop.get_word(WINDOW_X, 0);
    const word_t y = prop.get_word(WINDOW_Y, 0);
    const word_t h = prop.get_word(WINDOW_H, Const::default_window_height);
    const word_t w = prop.get_word(WINDOW_W, Const::default_window_width);

    init_main_window(x, y, w, h);

    build_menu();
    build_toolbar();
    build_layout();
}


////////////////////////////////////////////////////////////////
void ui::Controller::build_layout()
{
    layout_ = init_layout();

    code_ = init_code_view();
    if (code_)
    {
        layout_->add(*code_);
    }

    if (auto v = init_stack_view())
    {
        layout_->add(*v);
    }

    if (auto v = init_locals_view())
    {
        layout_->add(*v);
    }

    breakpoints_ = init_breakpoint_view();
    if (breakpoints_)
    {
        layout_->add(*breakpoints_);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::build_menu()
{
    menu_ = init_menu();

    menu_->add_item("&File/&Quit", FL_ALT + 'q', Enable_Always,
        [this]
        {
            CHKPTR(debugger_)->quit();
        });

    menu_->add_item("&Run/&Continue", FL_F + 5, Enable_IfStopped,
        [this]()
        {
            CHKPTR(debugger_)->resume();
        });

    menu_->add_item("&Run/&Next", FL_F + 10, Enable_IfStopped,
        [this]
        {
            if (auto t = state_->current_thread())
            {
                CHKPTR(debugger_)->step(t.get(), Debugger::STEP_OVER_SOURCE_LINE);
                CHKPTR(debugger_)->resume();
            }
        });

    menu_->add_item("&Run/&Step", FL_F + 11, Enable_IfStopped,
        [this]
        {
            if (auto t = state_->current_thread())
            {
                CHKPTR(debugger_)->step(t.get(), Debugger::STEP_SOURCE_LINE);
                CHKPTR(debugger_)->resume();
            }
        });

    menu_->add_item("&Run/&Break", FL_CTRL + 'c', Enable_IfRunning,
        [this]
        {   // nothing to do here, call_main_thread
            // will ensure the target breaks into the debugger
        });

    menu_->add_item("&Breakpoints/&Toggle", FL_F + 9, Enable_IfStopped,
        [this]
        {
            toggle_user_breakpoint();
        });

    menu_->add_ui_item("&Tools/E&valuate", FL_ALT + 'v', Enable_IfNotRunning,
        [this]
        {
            show_eval_dialog();
        });
}


////////////////////////////////////////////////////////////////
void ui::Controller::build_toolbar()
{
    toolbar_ = init_toolbar();
}


////////////////////////////////////////////////////////////////
/**
 * UI event loop
 */
void ui::Controller::run()
{
    while (wait_for_event() > 0)
    {
        if (command_) try
        {
            command_->continue_on_ui_thread();
        }
        catch (const exception& e)
        {
            error_message(e.what());
        }

        if (done_)
        {
            break;
        }
    }
}


////////////////////////////////////////////////////////////////
/**
 * Parse command line and other initializing stuff
 */
bool ui::Controller::initialize(

    Debugger*   debugger,
    int*        /* argc */,
    char***     /* argv */)
{
    debugger_ = debugger;

    if (probe_interactive_plugins())
    {
        return false;
    }
    return true;
}


////////////////////////////////////////////////////////////////
void ui::Controller::done()
{
    call_main_thread_async(new MainThreadCommand<>([this]() {
        CHKPTR(debugger_)->quit();
    }));

    unlock();
}


////////////////////////////////////////////////////////////////
void ui::Controller::update(

    LockedScope&    scope,
    Thread*         thread,
    EventType       eventType )

{
    state_->update(thread, eventType);

    // update modal dialog if any
    if (auto dialog = current_dialog())
    {
        dialog->update(*state_);
    }

    //
    // pass updated state info to UI elements
    //
    if (menu_)
    {
        menu_->update(*state_);
    }
    if (toolbar_)
    {
        toolbar_->update(*state_);
    }

    if (layout_)
    {
        layout_->update(*state_);
    }
    // @note: Updating the layout automatically updates code views
    // and we want to update breakpoints last;
    // DO NOT CHANGE this order of operations.

    if (thread->is_live())
    {
        BreakPointUpdater updater(*this);

        updater.push_back(breakpoints_);
        updater.push_back(code_);

        if (!updater.empty())
        {
            if (RefPtr<BreakPointManager> mgr = CHKPTR(debugger_)->breakpoint_manager())
            {
                mgr->enum_breakpoints(&updater);
            }
        }
    }

    if (state_->current_event() == E_TARGET_FINISHED)
    {
        status_message("Target disconnected");
    }
}


////////////////////////////////////////////////////////////////
RefPtr<ui::Command> ui::Controller::update(

    Thread*     thread,
    EventType   eventType )

{
    LockedScope lock(*this);
    update(lock, thread, eventType);

    if (command_ && command_->is_complete())
    {
        command_.reset();
    }

    if (!command_)
    {
        command_ = idle_;
    }

    return command_;
}


/**
 * Called from the main debugger thread.
 *
 * @return true to indicate that the event was handled.
 */
bool ui::Controller::on_event(

    Thread*     thread,
    EventType   eventType )

{
    if (eventType == E_PROBE_INTERACTIVE)
    {
        return !probing_;
    }

    RefPtr<Command> c = update(thread, eventType);
    try
    {
        c->execute_on_main_thread();
    }
    catch (const exception& e)
    {
        c = new CommandError(*this, e.what());
    }

    notify_ui_thread();
    return true;
}


////////////////////////////////////////////////////////////////
void* ui::Controller::run(void* p)
{
    auto controller = reinterpret_cast<ui::Controller*>(p);
    try
    {
        controller->lock();
        controller->build();

        controller->run();
    }
    catch (const exception& e)
    {
        dbgout(Log::ALWAYS) << __func__ << ": " << e.what() << endl;
    }
    catch (...)
    {
        assert(false);
    }
    controller->done();
    return nullptr;
}


////////////////////////////////////////////////////////////////
void ui::Controller::start()
{
    int r = pthread_create(&threadId_, nullptr, run, this);

    if (r < 0)
    {
        throw SystemError(__func__, r);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::shutdown()
{
    {
        LockedScope lock(*this);
        save_configuration();
        done_ = true;
    }
    pthread_join(threadId_, nullptr);
}


////////////////////////////////////////////////////////////////
void ui::Controller::register_streamable_objects(

    ObjectFactory* /* factory */)
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_table_init(SymbolTable*)
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_table_done(SymbolTable*)
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_attach(Thread*)
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_detach(Thread* t)
{
    if (t == 0) // detached from all threads?
    {
        LockedScope lock(*this);
        update(lock, nullptr, E_TARGET_FINISHED);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_syscall(Thread*, int32_t)
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_program_resumed()
{
    LockedScope lock(*this);
    update(lock, nullptr, E_TARGET_RESUMED);
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_insert_breakpoint(volatile BreakPoint* bpnt)
{
    if (bpnt->enum_actions("USER"))
    {
        LockedScope lock(*this);
        update(lock, state_->current_thread().get(), E_PROMPT);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_remove_breakpoint(volatile BreakPoint* bpnt)
{
    if (bpnt->enum_actions("USER"))
    {
        LockedScope lock(*this);
        update(lock, state_->current_thread().get(), E_PROMPT);
    }
}


////////////////////////////////////////////////////////////////
bool ui::Controller::on_progress(

    const char*     what,
    double          percent,
    word_t          cookie)
{
    return true;
}


////////////////////////////////////////////////////////////////
bool ui::Controller::on_message (
    const char*             what,
    Debugger::MessageType   type,
    Thread*                 thread,
    bool                    async)
{
    return false;
}


////////////////////////////////////////////////////////////////
void ui::Controller::call_main_thread_async(RefPtr<Command> c)
{
    if (c)
    {
        if (state_->is_target_running())
        {
            CHKPTR(debugger_)->stop();
        }
        else
        {
            awaken_main_thread();
        }

        command_ = c;
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::awaken_main_thread()
{
    idle_->cancel();
}


////////////////////////////////////////////////////////////////
void ui::Controller::save_configuration()
{
    if (!debugger_)
    {
        return;
    }
    Properties& prop = *debugger_->properties();

    prop.set_word(WINDOW_X, x());
    prop.set_word(WINDOW_Y, y());
    prop.set_word(WINDOW_H, h());
    prop.set_word(WINDOW_W, w());
}


////////////////////////////////////////////////////////////////
ui::State& ui::Controller::state()
{
    return *CHKPTR(state_.get());
}


////////////////////////////////////////////////////////////////
void ui::Controller::status_message(const std::string& msg)
{
    if (auto dialog = current_dialog())
    {
        if (dialog->status_message(msg.c_str()))
        {
            return;
        }
    }
    if (layout_)
    {
        layout_->status_message(msg);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::set_current_dialog(Dialog* dialog)
{
    if (dialog)
    {
        dialogStack_.push_back(dialog);
    }
    else
    {
        assert(!dialogStack_.empty());
        dialogStack_.pop_back();
    }
}


////////////////////////////////////////////////////////////////
bool ui::Controller::probe_interactive_plugins()
{
    Temporary<bool> temp(probing_, true);
    bool result = debugger()->publish_event(nullptr, E_PROBE_INTERACTIVE);
    return result;
}


////////////////////////////////////////////////////////////////
void ui::Controller::toggle_user_breakpoint(addr_t addr)
{
    if (auto t = state_->current_thread())
    {
        if (!CHKPTR(debugger_)->set_user_breakpoint(get_runnable(t.get()), addr))
        {
            // failed to set breakpoint? it means
            // that it exists already, so remove it
            CHKPTR(debugger_)->remove_user_breakpoint(0, 0, addr);
        }
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::toggle_user_breakpoint()
{
    if (code_)  // have a code view?
    {
        if (auto listing = code_->get_listing())
        {
            addr_t addr = listing->selected_addr();
            toggle_user_breakpoint(addr);
        }
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::enable_user_breakpoint(
    addr_t          addr,
    ui::EnableMode  mode )
{
    CHKPTR(debugger_);

    switch (mode)
    {
    case ui::Enable:
        enable_user_breakpoint_actions(*debugger_, addr);
        break;

    case ui::Disable:
        disable_user_breakpoint_actions(*debugger_, addr);
        break;

    case ui::Toggle:
        mode = has_enabled_user_breakpoint_actions(*debugger_, addr)
            ? ui::Disable : ui::Enable;

        enable_user_breakpoint( addr, mode );
        break;
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::set_temp_breakpoint(addr_t addr)
{
    if (auto r = get_runnable(state_->current_thread()))
    {
        CHKPTR(debugger_)->set_temp_breakpoint(r, addr);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::set_user_breakpoint(addr_t addr, bool set)
{
    if (auto r = get_runnable(state_->current_thread()))
    {
        if (set)
        {
            CHKPTR(debugger_)->set_user_breakpoint(r, addr);
        }
        else
        {
            CHKPTR(debugger_)->remove_user_breakpoint(0, 0, addr);
        }
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::show_edit_breakpoint_dialog(addr_t addr)
{
    UserBreakPoint ubp = breakpoints_->addr_to_breakpoint(addr);
    show_edit_breakpoint_dialog(ubp);
}

