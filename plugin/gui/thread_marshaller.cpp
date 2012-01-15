//
// $Id: thread_marshaller.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "gtkmm/connect.h"
#include "gtkmm/main.h"
#include "gui.h"
#include "slot_macros.h"
#include "thread_marshaller.h"

using namespace std;


ThreadMarshaller::ThreadMarshaller(Debugger& debugger)
    : debugger_(debugger)
    , percentagePipe_(false) // blocking I/O
    , isDebuggeeRunning_(false)
    , shutdown_(false)
{
}


ThreadMarshaller::~ThreadMarshaller()
{
}


void ThreadMarshaller::connect_event_pipes()
{
    GLIB_SIGNAL_IO.connect(
        Gtk_SLOT(this, &ThreadMarshaller::on_percent),
        percentagePipe_.output(), GDK_INPUT_READ);

    GLIB_SIGNAL_IO.connect(
        Gtk_SLOT(this, &ThreadMarshaller::on_request),
        request_pipe().output(), GDK_INPUT_READ);
}

#ifdef GTKMM_2


bool ThreadMarshaller::on_request(Glib::IOCondition)
{
    assert_ui_thread();
    process_request_queue();
    return true;
}



bool ThreadMarshaller::on_percent(Glib::IOCondition)
{
    assert_ui_thread();
    double perc = 0;
    size_t len = 0;

    vector<char> what;
    try
    {
        percentagePipe_.read(perc);
        assert(perc <= 1.0);
        percentagePipe_.read(len);
        what.resize(len);
        percentagePipe_.read(&what[0], len);
        if (len)
        {
            show_progress_indicator(&what[0], perc);
        }
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
    }
    return true;
}

#else


BEGIN_SLOT(ThreadMarshaller::on_request,(int fd, GdkInputCondition))
{
    assert_ui_thread();
    assert(fd == request_pipe().output());
    process_request_queue();
}
END_SLOT()



BEGIN_SLOT(ThreadMarshaller::on_percent,(int fd, GdkInputCondition))
{
    assert_ui_thread();
    assert(fd == percentagePipe_.output());

    double perc = 0;
    size_t len = 0;

    percentagePipe_.read(perc);
    percentagePipe_.read(len);
    vector<char> what(len);
    percentagePipe_.read(&what[0], len);

    dbgout(0) << __func__ << ": showing progress box\n";
    show_progress_indicator(&what[0], perc);
}
END_SLOT()
#endif



void ThreadMarshaller::process_request_queue()
{
    // move all requests from the inter-thread queue to a
    // "local" queue; pendingRequests_ are accessed by the
    // UI thread only
    PendingRequestQueue requests(pendingRequests_);
    requests_.pop_all(requests);

    process_pending_requests(requests);

    // just in case there are unprocessed requests,
    // this will save them till the next time
    pendingRequests_.swap(requests);
}



void
ThreadMarshaller::process_pending_requests(PendingRequestQueue& requests)
{
    for (CommandPtr prev; !requests.empty(); )
    {
        string errorMessage;

        CommandPtr cmd = requests.front();
        try
        {
            Lock<Mutex> lock(mutex_, TryLock());
            if (!lock && !cmd->is_safe())
            {
                // Give up, and hope we get a chance to handle
                // requests the next time a notification is sent
                // thru the pipe; this way, the main Gtk loop
                // can resume and handle other events.
                break;
            }
            requests.pop_front();

            // check for redundant commands in the queue
            if (prev && prev->is_equal(cmd.get()))
            {
                dbgout(2) << __func__ << ": " << cmd->name() << endl;
            }
            else
            {
                cmd->execute();
            }
            prev = cmd;
        }
        catch (exception& e)
        {
            errorMessage = e.what();
        }
        handle_error(__func__, errorMessage);
    }
}



void ThreadMarshaller::post_request(CommandPtr req) volatile
{
    assert_main_thread();
    requests_.push(req);
}



void ThreadMarshaller::post_response(CommandPtr req)
{
    assert_ui_thread();
    responses_.push(req);
}


void ThreadMarshaller::schedule_response(CommandPtr req) volatile
{
    const_cast<ResponseQueue&>(responses_).push(req);
}


bool ThreadMarshaller::write_percent(const char* msg, double percent) volatile
{
    size_t len = strlen(msg);
    if (len)
    {
        ++len; // include the ending NUL
    }
    try
    {
        percentagePipe_.write(percent);
        percentagePipe_.write(len);
        percentagePipe_.write(msg, len);
        usleep(1); // force thread context switch
    }
    catch (...)
    {
        return false;
    }
    return true;
}


/**
 * Process response messages from the UI thread.
 *
 * @note runs on main thread.
 */
bool ThreadMarshaller::process_responses() volatile
{
    bool resume = false;

    for (; !is_shutting_down(); )
    {
        ResponseQueue::blocking_scope block(responses_);

        while (!block.is_empty(responses_)) // non-locking
        {
            if (is_shutting_down())
            {
                break;
            }
            CommandPtr resp = responses_.front();
            responses_.pop();

            string error;

            try
            {
                resume |= resp->execute();
                dbgout(1) << __func__ << ": resume=" << resume << endl;
            }
            catch (const exception& e)
            {
                error = e.what();
            }
            catch (...)
            {
                error = "Unknown exception in main thread";
            }
            handle_error(__func__, error);
        }
        set_debuggee_running(false);

        if (is_service_call_pending())
        {
            dbgout(1) << __func__ << ": service call complete\n";
            set_service_call(false);

            // sync();

            const_cast<ThreadMarshaller*>(this)->update_after_responses();
            if (resume)
            {
                break;
            }
        }
        else if (responses_.empty())
        {
            break;
        }
    }

    return resume;
}


bool ThreadMarshaller::is_current_thread(pid_t lwpid, unsigned long id) const
{
    bool result = false;
    if (thread_)
    {
    #if HAVE_KSE_THREADS
        if (/* id != 0 && */ thread_->thread_id() == id)
        {
            result = true;
        }
    #else
        if (thread_->lwpid() == lwpid)
        {
            result = true;
        }
    #endif
    }
    return result;
}


RefPtr<Thread> ThreadMarshaller::current_thread() const volatile
{
    Lock<Mutex> lock(threadMutex_);
    return thread_;
}


void ThreadMarshaller::set_current_thread(const RefPtr<Thread>& thread)
{
    Lock<Mutex> lock(threadMutex_);
    thread_ = thread;
}


void ThreadMarshaller::reset_current_thread()
{
    Lock<Mutex> lock(threadMutex_);
    thread_.reset();
}


bool ThreadMarshaller::is_debuggee_running() const
{
    return compare_and_swap(&isDebuggeeRunning_, (AO_t)1, (AO_t)1);
}


void ThreadMarshaller::set_debuggee_running(AO_t running) volatile
{
    while (!compare_and_swap(&isDebuggeeRunning_, running, running))
    {
        assert((isDebuggeeRunning_ == 1) || (isDebuggeeRunning_ == 0));

        compare_and_swap(&isDebuggeeRunning_, (running ^ 1), running);
    }
}


class StackUpdater : public EnumCallback<Thread*>
{
    void notify(Thread* t)
    {
        t->stack_trace();
    }
};

void ThreadMarshaller::update_stack_traces()
{
    StackUpdater stackUpdater;
    debugger().enum_threads(&stackUpdater);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
