//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/auto_condition.h"
#include "code_view.h"
#include "controller.h"
#include "menu.h"
#include <FL/Enumerations.H>
#include "dharma/system_error.h"
#include <iostream>
#include <string>
#include <pthread.h>



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

    virtual Symbol* current_symbol() const
    {
        return currentSymbol_.get();
    }

    virtual Thread* current_thread() const
    {
        return currentThread_.get();
    }
};


////////////////////////////////////////////////////////////////
//
// null object pattern
//
class ZDK_LOCAL NullLayout : public ui::Layout
{
    // View interface
    virtual void added_to(const ui::Layout&) { }
    virtual void update(const ui::State&) { }
    // Layout interface
    virtual void add(ui::View&) { }
    virtual void show(ui::View&, bool) { }
};


////////////////////////////////////////////////////////////////
class ZDK_LOCAL CommandError : public ui::Command
{
    std::string msg_;

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
ui::State* ui::Controller::init_state( )
{
    return new StateImpl();
}


////////////////////////////////////////////////////////////////
void ui::Controller::init_main_window()
{
}


////////////////////////////////////////////////////////////////
ui::CodeView* ui::Controller::init_code_view()
{
    return nullptr;
}


////////////////////////////////////////////////////////////////
ui::CompositeMenu* ui::Controller::init_menu()
{
    return nullptr;
}


////////////////////////////////////////////////////////////////
ui::Layout* ui::Controller::init_layout( )
{
    return new NullLayout();
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
    
    if (CodeView* v = init_code_view())
    {
        layout_->add(*v);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::build_menu()
{
    assert(menu_);

    menu_->add(new SimpleCommandMenu<>("File/Quit", FL_ALT + 'q', [this]() {
            debugger_->quit();
        }));
}


////////////////////////////////////////////////////////////////
void ui::Controller::error_message(const std::string&) const
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
        catch (const std::exception& e)
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
        {   
            Lock<Mutex> lock(mutex_);
            cancelled_ = true;
        }
        cond_.broadcast();
    }

protected:
    ~WaitCommand() throw() { }

public:
    WaitCommand() : cancelled_(false) { }
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
    catch (const std::exception& e)
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
    catch (const std::exception& e)
    {
        std::cerr << __func__ << ": " << e.what() << std::endl;
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

