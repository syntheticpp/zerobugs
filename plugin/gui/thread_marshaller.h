#ifndef THREAD_MARSHALLER_H__B3684CE4_DF14_4E54_B3B9_405CA8EBF9E0
#define THREAD_MARSHALLER_H__B3684CE4_DF14_4E54_B3B9_405CA8EBF9E0
//
// $Id: thread_marshaller.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/atomic.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "gtkmm/base.h"
#include "gtkmm/main.h"
#include "command.h"
#include <ext/functional> // identity

#define TRY_LOCK(lck, mx, ...) \
    Lock<Mutex> lck(mx, TryLock()); try_lock(__func__, lck, ##__VA_ARGS__)

template<typename M>
static void try_lock(const std::string& func,
                     Lock<M>& lock,
                     const char* name = "main")
{
    // not optimal to throw (since the user sees an error message box)
    // but at least we don't deadlock
    if (!lock)
    {
        throw std::logic_error(func + ": " + name + " thread busy");
    }
}


class Debugger;

/**
 * Manage marshalling calls between the UI thread and the main
 * thread (ptrace has a thread-affinity restriction, the UI cannot
 * call ptrace directly).
 */
class ZDK_LOCAL ThreadMarshaller : public Gtk::Base
{
public:
    virtual ~ThreadMarshaller();

    /**
     * The main thread posts a request for the UI thread,
     * and returns without waiting for completion.
     */
    void post_request(CommandPtr) volatile;

    /**
     * UI posts a response to the main thread
     */
    void post_response(CommandPtr);

    /**
     * post from any thread
     */
    void schedule_response(CommandPtr) volatile;

    //void signal_main_thread() { responses_.wake_up(); }
    void signal_main_thread()
    { post_response(wrap(boost::bind(ext::identity<bool>(), false))); }

    bool is_debuggee_running() const;

    void set_debuggee_running(AO_t running) volatile;

    RefPtr<Thread> current_thread() const volatile;

    void set_current_thread(const RefPtr<Thread>&);
    void reset_current_thread();

    bool is_shutting_down() const volatile { return shutdown_; }

    void update_stack_traces();

    Debugger& debugger() volatile { return debugger_; }

protected:
    explicit ThreadMarshaller(Debugger&);

    void set_shutdown(bool flag) { shutdown_ = flag; }

    /**
     * @return true if the UI thread has requested a "service"
     * from the main thread, i.e. has marshalled a function call,
     * and the result is pending.
     */
    bool is_service_call_pending() const volatile
    {
        return atomic_test(serviceCall_);
    }

    void set_service_call(bool flag) volatile
    {
        if (flag)
        {
            assert_ui_thread();
            atomic_inc(serviceCall_);
        }
        else
        {
            atomic_dec(serviceCall_);
            assert(atomic_read(serviceCall_) >= 0);
        }
    }

    void connect_event_pipes();

    Pipe& request_pipe() { return requests_.pipe(); }

    typedef std::deque<CommandPtr> PendingRequestQueue;

#if defined (GTKMM_2)
    bool on_request(Glib::IOCondition);

    bool on_percent(Glib::IOCondition);
#else
    // GTK-1.2
    /**
     * Called when there's a request from the main
     * debugger thread pending in the pipe.
     */
    void on_request(int, GdkInputCondition);

    void on_percent(int, GdkInputCondition);
#endif
    virtual void show_progress_indicator(std::string msg,
                                         double percent) = 0;
    virtual void handle_error(const char* func,
                              const std::string&) volatile = 0;

    virtual void update_after_responses() = 0;

    bool write_percent(const char* message, double percent) volatile;

    /**
     * Processes requests from the main thread
     */
    void process_request_queue();
    void process_pending_requests(PendingRequestQueue&);

    bool process_responses() volatile;

    bool is_current_thread(pid_t, unsigned long) const;

    volatile Mutex& mutex() const volatile { return mutex_; }

    bool is_response_queue_empty() const volatile
    {
        return responses_.empty();
    }

private:
    Debugger&           debugger_;
    RefPtr<Thread>      thread_; // current debuggee thread;
    mutable Mutex       mutex_;
    mutable Mutex       threadMutex_;
    RequestQueue        requests_;  // from main --> UI
    ResponseQueue       responses_; // from UI --> main
    Pipe                percentagePipe_; // for progress indication
    PendingRequestQueue pendingRequests_;
    mutable atomic_t    serviceCall_;

    mutable AO_t        isDebuggeeRunning_;
    bool                shutdown_;
};


#define TRY_MAIN(e) { TRY_LOCK(lck, mutex(), "main"); e; }

#if !defined( DEBUG_THREAD_MARSHALLING )
 #define CALL_MAIN_THREAD()     TRY_MAIN(signal_main_thread())
 #define CALL_MAIN_THREAD_(x)   TRY_MAIN(post_response(x))
#else

#define CALL_MAIN_THREAD() \
 std::clog << "CALL_MAIN_THREAD() "<<__FILE__<<':'<<__LINE__<<std::endl;\
 TRY_MAIN(signal_main_thread())

#define CALL_MAIN_THREAD_(x) \
 std::clog << "CALL_MAIN_THREAD(" << #x << ") "; \
 std::clog << __FILE__ << ':' << __LINE__ << std::endl;\
 TRY_MAIN(post_response(x))
#endif

#endif // THREAD_MARSHALLER_H__B3684CE4_DF14_4E54_B3B9_405CA8EBF9E0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
