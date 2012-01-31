//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "code_view.h"
#include "menu.h"
#include "user_interface.h"
#include "dharma/system_error.h"
#include <iostream>
#include <string>
#include <pthread.h>


////////////////////////////////////////////////////////////////
class ZDK_LOCAL StateImpl : public ui::State
{
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
    virtual void add_to(ui::Layout&) { }
    virtual void update(const ui::State&) { }
    // Layout interface
    virtual void show(ui::View&, bool) { }
};


////////////////////////////////////////////////////////////////
class ZDK_LOCAL CommandError : public ui::Command
{
    ui::Controller& controller_;
    std::string     error_;

public:
    CommandError( ui::Controller& c, const char* e)
        : controller_(c)
        , error_(e)
    { }

    void exec_on_main_thread() { }

    void exec_on_ui_thread() 
    {
        controller_.error_message(error_);
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
    , layout_(nullptr)
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

    layout_.reset(init_layout());

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
        v->add_to(*layout_);
    }
}


class Quit : public ui::Command
{
    Debugger* debugger_;

    void exec_on_main_thread() { debugger_->quit(); }
    void exec_on_ui_thread() { } 

public:
    explicit Quit(Debugger* debugger) : debugger_(debugger)
    { }
};

////////////////////////////////////////////////////////////////
void ui::Controller::build_menu()
{
    assert(menu_);

    menu_->add(RefPtr<MenuItem>(new MenuItem("File/Quit")));
}


////////////////////////////////////////////////////////////////
void ui::Controller::error_message(const std::string&) const
{
}


////////////////////////////////////////////////////////////////
void ui::Controller::run()
{
    lock();
    build();

    while (wait_for_event() > 0)
    {
        if (command_) try
        {
            command_->exec_on_ui_thread();
        }
        catch (const std::exception& e)
        {
            error_message(e.what());
        }

        command_.reset();
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


////////////////////////////////////////////////////////////////
/**
 * Called from the main debugger thread.
 */
bool ui::Controller::on_event(

    Thread*     thread,
    EventType   eventType )

{
    {   LockedScope lock(*this);

        state_->update(thread, eventType);
        
        if (command_) try
        {
            command_->exec_on_main_thread();
        }
        catch (const std::exception& e)
        {
            command_.reset( new CommandError(*this, e.what()) );
        }
    }
    notify_ui_thread();

    return false;
}


////////////////////////////////////////////////////////////////
static void* ui_main(void* p)
{
    auto controller = reinterpret_cast<ui::Controller*>(p);
    try
    {
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
    int r = pthread_create(&uiThreadId_, nullptr, ui_main, this);

    if (r < 0)
    {
        throw SystemError(__func__, r);
    }
}


////////////////////////////////////////////////////////////////
void ui::Controller::shutdown()
{
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

