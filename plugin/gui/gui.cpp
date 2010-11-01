//
// $Id: gui.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <glib.h>
#include <gtk/gtk.h>
#include "gtkmm/connect.h"
#include "gtkmm/main.h"
#include "zdk/argv_util.h"
#include "zdk/check_ptr.h"
#include "zdk/symbol_table.h"
#include "dharma/canonical_path.h"
#include "dharma/symbol_util.h"
#include "command.h"
#include "gui.h"
#include "main_window.h"
#include "message_box.h"
#include "text_entry.h"
#include "utils.h"
#include "src_tabs.h"


using namespace std;
using namespace SigC;
using namespace boost;


GUI*        GUI::theGUI_    = 0;
int         GUI::debugLevel_= 0;
pthread_t   GUI::tid_       = 0;

//static Mutex gdkMutex;
static Mutex mutex;
static Condition condStarted;
static bool started = false;


////////////////////////////////////////////////////////////////
int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}


////////////////////////////////////////////////////////////////
void query_plugin(InterfaceRegistry* registry)
{
    registry->update(DebuggerPlugin::_uuid());
}


////////////////////////////////////////////////////////////////
Plugin* create_plugin(uuidref_t iid)
{
    if (uuid_equal(DebuggerPlugin::_uuid(), iid))
    {
        return GUI::instance();
    }
    return 0;
}


////////////////////////////////////////////////////////////////
/*
static void enter_gdk_mutex()
{
    gdkMutex.enter();
}


static void leave_gdk_mutex()
{
    gdkMutex.leave(nothrow);
}
*/


////////////////////////////////////////////////////////////////
ZObject* GUI::new_drop_list(ObjectFactory*, const char* name)
{
    ZObject* obj = NULL;

    if (theGUI_ && theGUI_->mainWindow_)
    {
        // todo: should interface_cast<> it?
        obj = theGUI_->mainWindow_->get_object(name).get();
    }
    if (!obj)
    {
        obj = new DropList(name);
    }
    return obj;
}


////////////////////////////////////////////////////////////////
    static ZObject* new_src_tabs(ObjectFactory*, const char* name)
{
    return new SourceTabs(name);
}


////////////////////////////////////////////////////////////////
static void set_help_path(const char* filename)
{
    string path = canonical_path(filename);

    const size_t n = path.rfind('/');
    if (n != string::npos)
    {
        path.erase(n);
        path = canonical_path((path + "/../help").c_str());
    }
    setenv("ZERO_HELP_PATH", path.c_str(), false);
}


////////////////////////////////////////////////////////////////
GUI* GUI::instance()
{
    if (theGUI_ == 0)
    {
        theGUI_ = new GUI();
    }
    return theGUI_;
}


////////////////////////////////////////////////////////////////
GUI::GUI()
    : attached_(false)
    , disabled_(true)
    , factory_(0)
{
    assert(theGUI_ == 0);
    theGUI_ = this;
}


////////////////////////////////////////////////////////////////
GUI::~GUI() throw()
{
    disabled_ = true;

    if (factory_)
    {
        dbgout(0) << "Unregistering: " << DropList::_uuid() << endl;
        factory_->unregister_interface(DropList::_uuid());
    }
    if (tid_)
    {
        pthread_kill(tid_, SIGKILL);
    }

    delete Gtk::Main::instance();

    assert(theGUI_ == this);
    theGUI_ = 0;
    tid_ = 0;
}


////////////////////////////////////////////////////////////////
void GUI::release()
{
    if (debug_level())
    {
        clog << "UI::release()" << endl;
    }

    assert(pthread_self() != GUI::tid());
    delete theGUI_;
}


#if !defined (GTKMM_2)

static void pre_gtk_init()
{
}

static void post_gtk_init()
{
}

#else
//
// Gtkmm 2.x? add default resource file
//
static void pre_gtk_init()
{
    if (const char* rcPath = getenv("ZERO_PLUGIN_PATH"))
    {
        string rcFile = rcPath;
        if (!rcFile.empty())
        {
            if (rcFile[rcFile.size() - 1] != '/')
            {
                rcFile += '/';
            }
        }
        rcFile += "zero-gtkrc-2.0";
        Gtk::RC::add_default_file(rcFile);
    }
}

static void post_gtk_init()
{
    Glib::add_exception_handler(&GUI::handle_exception);
}
#endif // GTKMM_2


////////////////////////////////////////////////////////////////
bool GUI::initialize(Debugger* debugger, int* ac, char*** av)
{
    assert(Gtk::Main::instance() == 0);

    RefPtr<Properties> prop = debugger->properties();
    string strategy = prop->get_string("layout.strategy", "default");

    disabled_ = false;

BEGIN_ARG_PARSE(ac, av)
    ON_ARGV ("--ui-debug=", debugLevel_)
        {
        }
    ON_ARG ("--ui-disable")
        {
            disabled_ = true;
            return false;
        }
    ON_ARG ("--ui-no-busy-cursor")
        {
            setenv("ZERO_NO_BUSY_CURSOR", "1", true);
        }
    ON_ARGV("--ui-layout-", strategy)
        {
            prop->set_string("layout.strategy", strategy.c_str());
        }
END_ARG_PARSE

    set_help_path((*av)[0]);

    if (!disabled_)
    {
        //gdk_threads_set_lock_functions(enter_gdk_mutex, leave_gdk_mutex);
        pre_gtk_init();
        new Gtk::Main(*ac, *av);

        post_gtk_init();

        // pre-condition: main window not created
        assert(!mainWindow_);

        // create the main window (on the main thread)
        assert(debugger);
        mainWindow_.reset(new MainWindow(*debugger, strategy));
    }
    return true;
}



////////////////////////////////////////////////////////////////
void GUI::start()
{
    assert(!disabled_);

    ThreadsGuard guard;
    pthread_create(&tid_, 0, run, this);
}


////////////////////////////////////////////////////////////////
void GUI::shutdown()
{
    if (disabled_)
    {
        return;
    }
    pthread_t tid = tid_;
    if (tid)
    {
        TRY_LOCK(lock, mutex);
        while (!started)
        {
            // wait for the UI thread to start
            condStarted.wait(lock);
        }
        ThreadsGuard guard;
        Gtk::Main::quit();
    }
    if (tid && (pthread_self() != tid))
    {
        dbgout(0) << "waiting for UI thread to finish" << endl;
        pthread_join(tid, 0);
    }
    assert(tid_ == 0);
    disabled_ = true;
}


////////////////////////////////////////////////////////////////
void GUI::register_streamable_objects(ObjectFactory* factory)
{
    assert(factory_ == 0);
    factory_ = factory;
    // this is needed so that drop-lists can be depersisted
    // from the properties stream
    factory->register_interface(DropList::_uuid(), new_drop_list);
    factory->register_interface(SourceTabs::_uuid(), new_src_tabs);
}


////////////////////////////////////////////////////////////////
void GUI::on_table_init(SymbolTable* symtab)
{
    string msg = "Loading ";
    msg += CHKPTR(symtab)->filename()->c_str();

    mainWindow_->status_message(msg);
}


////////////////////////////////////////////////////////////////
void GUI::on_table_done(SymbolTable* symtab)
{
    if (CHKPTR(symtab)->is_loaded())
    {
        if (size_t size = symtab->size())
        {
            ostringstream msg;
            msg << CHKPTR(symtab->filename())->c_str();
            msg << ": " << symtab->size() << " symbol";
            if (size > 1)
            {
                msg << "s";
            }
            msg << " loaded";
            mainWindow_->status_message(msg.str());
        }
    }
}


////////////////////////////////////////////////////////////////
void GUI::on_attach (Thread* thread)
{
    assert(thread);
    if (disabled_)
    {
        return;
    }
    assert(mainWindow_);

    if (debug_level())
    {
        const pid_t pid = thread->lwpid();
        clog << "*** UI: attached, tid=" << pid << endl;
    }

    mainWindow_->set_debuggee_running(true);
    // If this is the first thread within the debuggee
    // that we are attaching to, set a one-time break point
    // at main()
    if (!attached_)
    {
        attached_ = true;
    }

    RefPtr<Thread> threadPtr(thread);

    // execute on UI thread
    post_request(&MainWindow::on_attached, mainWindow_, threadPtr);
}


////////////////////////////////////////////////////////////////
void GUI::on_detach (Thread* thread)
{
    if (!thread)
    {
        // when thread == 0, it means that the debugger
        // engine has detached from *ALL* debuggee threads
        attached_ = false;
    }
    if (disabled_)
    {
        return;
    }
    if (mainWindow_ /* && !mainWindow_->is_shutting_down() */)
    {
        RefPtr<Thread> tptr(thread);
        post_request(&MainWindow::on_detached, mainWindow_, tptr);
    }
    if (debug_level())
    {
        clog << "*** UI: detached ";
        if (thread)
        {
            clog << thread->lwpid();
        }
        clog << endl;
    }
}


////////////////////////////////////////////////////////////////
bool GUI::on_event(Thread* thread, EventType event)
{
    bool result = false;

    if (!disabled_ && mainWindow_)
    {
        result = mainWindow_->on_debug_event(thread, event);
    }
    return result;
}


namespace
{
    /**
     * Helper command object used by GUI::on_program_resumed.
     */
    class ZDK_LOCAL UpdateStateRunning
        : public RefCountedImpl<InterThreadCommand>
    {
    public:
        explicit UpdateStateRunning(MainWindow& w) : w_(&w) { }

        bool execute()
        {
            const_cast<MainWindow*>(w_)->update_running();
            return false;
        }
        bool is_equal(const InterThreadCommand* other) const
        {
            return other && strcmp(other->name(), this->name()) == 0;
        }

        const char* name() const { return "UpdateStateRunning"; }

    private:
        volatile MainWindow* w_;
    };
}

////////////////////////////////////////////////////////////////
void GUI::on_program_resumed()
{
    if (shared_ptr<MainWindow> w = mainWindow_)
    {
        w->set_debuggee_running(true);
        //
        // update the state of the UI, to reflect
        // that the debugged program is now running
        //
        w->post_request(new UpdateStateRunning(*w));
    }
}


////////////////////////////////////////////////////////////////
void GUI::on_insert_breakpoint(volatile BreakPoint* bpnt)
{
    if (disabled_)
    {
        return;
    }
    // pre-conditions
    assert(bpnt);
    assert(mainWindow_);

    if ((bpnt->type() != BreakPoint::HARDWARE)
      && bpnt->symbol()
      && bpnt->enum_actions("USER"))
    {
        RefPtr<Thread> thread = bpnt->thread();
        if (!thread->is_forked()
            && (bpnt->type() == BreakPoint::GLOBAL))
        {
            thread.reset(); // applies to ALL threads
        }

        if (is_ui_thread())
        {
            mainWindow_->on_insert_breakpoint(thread, bpnt->symbol(), bpnt->is_deferred());
        }
        else
        {
            post_command(&MainWindow::on_insert_breakpoint,
                         mainWindow_.get(),
                         thread,
                         RefPtr<Symbol>(bpnt->symbol()),
                         bpnt->is_deferred());
        }
    }
}


////////////////////////////////////////////////////////////////
void GUI::on_remove_breakpoint(volatile BreakPoint* bpnt)
{
    if (disabled_)
    {
        return;
    }
    // pre-conditions
    assert(bpnt);
    assert(mainWindow_);

    RefPtr<Symbol> symbol = bpnt->symbol();

    if ((bpnt->type() == BreakPoint::SOFTWARE) && symbol)
    {
        RefPtr<Thread> thread = bpnt->thread();
        if (!thread->is_forked())
        {
            // software breakpoints are global,
            // in the current implementation
            thread.reset();
        }

        post_command(&MainWindow::on_remove_breakpoint,
                     mainWindow_.get(),
                     thread,
                     symbol,
                     bpnt->is_deferred());
    }
}


////////////////////////////////////////////////////////////////
bool GUI::on_progress(const char* what, double perc, word_t w)
{
    if (disabled_)
    {
        return true;
    }
    return CHKPTR(mainWindow_)->on_progress(what, perc);
}


////////////////////////////////////////////////////////////////
bool GUI::on_message (
    const char* what,
    Debugger::MessageType type,
    Thread* thread,
    bool async)
{
    if (MainWindow* w = mainWindow_.get())
    {
        switch (type)
        {
        case Debugger::MSG_ERROR:
            if (async)
            {
                w->error_message_async(what);
            }
            else
            {
                w->set_debuggee_running(false);
                w->error_message(what);
            }
            return true;

        case Debugger::MSG_STATUS:
            w->status_message(what);
            break;

        case Debugger::MSG_INFO:
            if (async)
            {
                w->info_message_async(what);
            }
            else
            {
                w->info_message(what);
            }
            break;

        case Debugger::MSG_YESNO:
            {
                assert(!async);

                bool result = false;
                w->question_message(what, &result);

                return result;
            }
            break;

        case Debugger::MSG_HELP:
            w->help_search(what);
            break;

        case Debugger::MSG_UPDATE:
            if (thread) // hack:
            {
                RefPtr<Thread> threadPtr(thread);
                post_request(&MainWindow::on_attached, mainWindow_, threadPtr);
            }
            // w->update(thread);
            break;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
void GUI::add_command(DebuggerCommand* cmd)
{
    if (mainWindow_)
    {
        mainWindow_->add_command(cmd);
    }
}


////////////////////////////////////////////////////////////////
void GUI::enable_command(DebuggerCommand* cmd, bool enable)
{
    if (mainWindow_)
    {
        mainWindow_->enable_command(cmd, enable);
    }
}


/**
 * Announce that the UI thread has started.
 */
static int announce_started()
{
    Lock<Mutex> lock(mutex);
    started = true;
    condStarted.broadcast();
    return 0;
}


////////////////////////////////////////////////////////////////
//
// start UI thread
//
void* GUI::run(void* p)
{
    try
    {
        GLIB_SIGNAL_IDLE.connect(Gtk_PTR_FUN(&announce_started));
        //
        // main event loop
        //
        Gtk::Main::run();
    }
    catch (std::exception& e)
    {
        cerr << "Exception on UI thread: " << e.what() << endl;
    }
    dbgout(0) << "UI loop finished.\n";

    static_cast<GUI*>(p)->disabled_ = true;
    static_cast<GUI*>(p)->mainWindow_.reset();

    tid_ = 0;
    return 0;
}



////////////////////////////////////////////////////////////////
void GUI::handle_exception() throw()
{
    string msg;

    try
    {
    #ifdef DEBUG
        clog << __func__ << ": rethrowing exception" << endl;
    #endif
        throw;
    }
    catch (const std::exception& e)
    {
        msg = e.what();
    }
    catch (...)
    {
        msg = "Non-standard exception caught";
    }

    if (!theGUI_ || !theGUI_->mainWindow_)
    {
        cerr << __func__ << ": " << msg << endl;
    }
    else if (MainWindow* w = theGUI_->mainWindow_.get())
    {
        w->error_message(msg);
    }
}


////////////////////////////////////////////////////////////////
extern "C" void handle_ui_exception() throw()
{
#ifdef DEBUG
    clog << __func__ << endl;
#endif
    GUI::handle_exception();
}

bool is_ui_thread()
{
    return pthread_self() == GUI::tid();
}

void assert_ui_thread()
{
    assert(is_ui_thread());
}

void assert_main_thread()
{
    assert(pthread_self() != GUI::tid());
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
