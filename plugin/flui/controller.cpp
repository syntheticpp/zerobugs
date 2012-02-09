//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/auto_condition.h"
#include "code_view.h"
#include "controller.h"
#include "locals_view.h"
#include "menu.h"
#include <FL/Enumerations.H>
#include "dharma/system_error.h"
#include <iostream>
#include <string>
#include <pthread.h>

using namespace std;



class ZDK_LOCAL StateImpl : public ui::State
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

    virtual void update(Thread* thread, EventType eventType)
    {
        currentThread_ = thread;
        currentEventType_ = eventType;
        
        isTargetStopped_ = true;

        if (thread)
        {
            addr_t pc = thread->program_count();
            assert(thread->symbols());

            currentSymbol_ = thread->symbols()->lookup_symbol(pc);
        }
    }
   
    virtual void set_target_stopped(bool stopped)
    {
        isTargetStopped_ = stopped;
    }

    virtual bool is_target_stopped() const
    {
        return isTargetStopped_;
    }

    virtual EventType current_event_type() const
    {
        return currentEventType_;
    }

    virtual RefPtr<Symbol> current_symbol() const
    {
        return currentSymbol_;
    }

    virtual RefPtr<Thread> current_thread() const
    {
        return currentThread_;
    }
};


////////////////////////////////////////////////////////////////
class ZDK_LOCAL CommandError : public ui::Command
{
    string msg_;

public:
    explicit CommandError(const char* m) : msg_(m)
    { }

    virtual ~CommandError() throw() { }

    void continue_on_ui_thread(ui::Controller& controller) 
    {
        controller.error_message(msg_);
    }
};


////////////////////////////////////////////////////////////////
//
// Controller implementation
// 
ui::Controller::Controller()
    : debugger_(nullptr)
    , uiThreadId_(0)
    , state_(init_state())
    , done_(false)
{
}


////////////////////////////////////////////////////////////////
ui::Controller::~Controller()
{
}


////////////////////////////////////////////////////////////////
unique_ptr<ui::State> ui::Controller::init_state( )
{
    return unique_ptr<ui::State>(new StateImpl());
}


////////////////////////////////////////////////////////////////
void ui::Controller::init_main_window()
{
}


////////////////////////////////////////////////////////////////
RefPtr<ui::CodeView> ui::Controller::init_code_view()
{
    return nullptr;
}


////////////////////////////////////////////////////////////////
RefPtr<ui::CompositeMenu> ui::Controller::init_menu()
{
    return nullptr;
}


////////////////////////////////////////////////////////////////
RefPtr<ui::VarView> ui::Controller::init_locals_view()
{
    return new LocalsView(*this);
}


////////////////////////////////////////////////////////////////
void ui::Controller::build()
{
    init_main_window();

    menu_ = init_menu();

    if (menu_)
    {
        build_menu();
    }

    layout_ = init_layout();

    if (layout_)
    {
        build_layout();
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::build_layout()
{
    assert(layout_);
    
    if (auto v = init_code_view())
    {
        layout_->add(*v);
    }

    if (auto  v = init_locals_view())
    {
        layout_->add(*v);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::build_menu()
{
    assert(menu_);

    menu_->add(simple_command_menu("File/Quit", FL_ALT + 'q', [this]()
    {
        debugger_->quit();
    }));
    
    menu_->add(simple_command_menu("Debug/Continue", FL_F + 5, [this]()
    {
        debugger_->resume();
    }));
    
    menu_->add(simple_command_menu("Debug/Next", FL_F + 10, [this]()
    {
        if (auto t = state_->current_thread())
        {
            debugger_->step(t.get(), Debugger::STEP_OVER_SOURCE_LINE);
            debugger_->resume();
        }
    }));
}


////////////////////////////////////////////////////////////////
void ui::Controller::error_message(const string&) const
{
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
            command_->continue_on_ui_thread(*this);
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
    return true;
}


class ZDK_LOCAL WaitCommand : public ui::Command
{
    Mutex       mutex_;
    Condition   cond_;
    bool        cancelled_;

    void execute_on_main_thread()
    {
        Lock<Mutex> lock(mutex_);
        while (!cancelled_) 
        {
            cond_.wait(lock);
        }
    }

    void cancel() 
    {
        set_cancel();
        cond_.broadcast();
    }

protected:
    ~WaitCommand() throw() { }

public:
    WaitCommand() : cancelled_(false) { }

    void set_cancel()
    {
        Lock<Mutex> lock(mutex_);
        cancelled_ = true;
    }
};


////////////////////////////////////////////////////////////////
RefPtr<ui::Command> ui::Controller::update(

    Thread*     thread,
    EventType   eventType )
{
    LockedScope lock(*this);
    state_->update(thread, eventType);

    // pass updated state info to UI elements
    layout_->update(*state_);
    menu_->update(*state_);

    if (command_ && command_->is_done())
    {
        command_.reset();
    }
        
    if (!command_)
    {
        command_ = new WaitCommand();
    }

    return command_;
}


/**
 * Called from the main debugger thread.
 */
bool ui::Controller::on_event(

    Thread*     thread,
    EventType   eventType )

{
    RefPtr<Command> c = update(thread, eventType);

    try
    {
        c->execute_on_main_thread();
    }
    catch (const exception& e)
    {
        c = new CommandError(e.what());
    }

    notify_ui_thread();

    return true;    // event handled
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
        cerr << __func__ << ": " << e.what() << endl;
    }
    catch (...)
    {
        assert(false);
    }
    return nullptr;
}


////////////////////////////////////////////////////////////////
void ui::Controller::start()
{
    int r = pthread_create(&uiThreadId_, nullptr, run, this);

    if (r < 0)
    {
        throw SystemError(__func__, r);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::shutdown()
{
    LockedScope lock(*this);

    done_ = true;
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
void ui::Controller::on_attach(Thread* thread)
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_detach(Thread* thread)
{
    // if (thread == 0) // detached from all threads?
    // {
    // } 
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_syscall(Thread*, int32_t)
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_program_resumed()
{
    state_->set_target_stopped( false );
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_insert_breakpoint(volatile BreakPoint*)
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::on_remove_breakpoint(volatile BreakPoint*)
{
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
void ui::Controller::call_async_on_main_thread(RefPtr<Command> c)
{
    if (c)
    {
        if (command_)
        {
            command_->cancel();
        }
        
        command_ = c;
    }
}

