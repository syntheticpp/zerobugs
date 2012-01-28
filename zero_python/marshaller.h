#ifndef MARSHALLER_H__7CDBA4E5_E190_447B_8754_EE2F90842BC8
#define MARSHALLER_H__7CDBA4E5_E190_447B_8754_EE2F90842BC8
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Only one thread (the main debugger thread) can manipulate
// the debuggee via ptrace calls. Other threads that need to
// access debugger functions need to marshal the calls to the
// main thread.
//
// Some calls that initiate stepping through code, such as
// Thread::step_until_current_func_returns() may be asynchronous.
//
// Running a UI with its own event loop thread is supported.
//
#include <pthread.h>
#include <deque>
#include <iostream>
#include "dharma/pipe.h"
#include "generic/lock.h"
#include "generic/singleton.h"
#include "zdk/weak_ptr.h"
#include "zdk/zero.h"
#include "zdk/zobject_impl.h"
#include "event_queue.h"
#include "message.h"
#include "python_mutex.h"
#include "utility.h"


/**
 * Receives notifications from the Marshaller
 */
DECLARE_ZDK_INTERFACE_(MarshallerCallbacks, Unknown)
{
    /// @note: called on Python thread
    virtual void on_error(const std::string&) = 0;

    virtual bool on_command_loop_state(bool active) = 0;
};


/**
 * Routes commands between the main debugger thread and
 * other threads
 */
class ZDK_LOCAL Marshaller
{
    typedef blocking_queue<RefPtr<Message> > CommandQueue;
    typedef thread_safe_queue<RefPtr<Message> > SchedQueue;

    typedef event_queue<RefPtr<Message> > EventQueueBase;
    /**
     * Raises a notification every time an event message is
     * pushed into the queue
     */
    class EventQueue : public EventQueueBase
    {
        Marshaller& m_;

    public:
        explicit EventQueue(Marshaller& m) : m_(m) { }

    private:
        void raise_event(bool f) volatile { m_.raise_event(f); }

    }; // EventQueue

    typedef MarshallerCallbacks Callbacks;

    EventQueue      events_;
    CommandQueue    commands_;
    SchedQueue      sched_;
    Condition       event_;
    bool            shutdown_,
                    debugCommands_,
                    debugEvents_,
                    usePipe_,
                    inCommandLoop_,
                    wakeup_,
                    eventsActive_;  // event_pipe or event_loop

    Pipe            eventPipe_;     // for use with gtk
    mutable Mutex   mutex_;
    Condition       eventStart_;
    Condition       cmdLoopActive_;
    pthread_t       self_;
    Callbacks*      callbacks_;
    size_t          eventSequence_;

protected:
    void error(std::string);

    void wait_for_event(Lock<Mutex>&);

    bool send_event_(const RefPtr<Message>&, bool = false) volatile;

    bool in_command_loop() const;

    //
    // methods prefixed with post_ are asynchronous, i.e. they
    // stick the message into a queue and return without waiting
    // for the message to be dispatched
    //
    void post_command_(const RefPtr<Message>&);
    void post_event_(const RefPtr<Message>&) volatile;

    /**
     * Extract command from commands queue and dispatch it.
     *
     * A global python_mutex is held while the command is being
     * dispatched.
     *
     * @note runs on the main thread.
     */
    bool dispatch_command() volatile;

    void signal_command_loop_active() volatile
    {
        cmdLoopActive_.broadcast();
    }

public:
    Marshaller();
    virtual ~Marshaller();

    void error_message(const std::string&) volatile;

    bool is_main_thread() const;

    /**
     * @note this is meant to be called from a thread different
     * than the main thread
     */
    void break_command_loop(size_t sequence = 0);

    void command_loop() volatile;

    void raise_event(bool) volatile;

    void unblock_waiting_threads() volatile;

    void event_loop();

    bool event_iteration(bool readEventPipe = false);

    void wait_for_main_thread();

    bool extract_events(std::deque<RefPtr<Message> >&, bool);

    void process_events(std::deque<RefPtr<Message> >&);

    void shutdown() volatile;

    template<typename T>
    bool send_event(const T& t, const char* name) volatile
    {
        return send_event_(new MessageAdapter<T>(t, name));
    }
    template<typename T>
    bool send_event(const T& t, const char* name, bool keepCmdLoop) volatile
    {
        return send_event_(new MessageAdapter<T>(t, name), keepCmdLoop);
    }
    template<typename T>
    void post_event(const T& t, const char* name) volatile
    {
        post_event_(new MessageAdapter<T>(t, name));
    }

    template<typename T>
    bool send_command(const T& t, const char* name)
    {
        bool ret = false;

        if (!shutdown_)
        {
            RefPtr<SyncMessageAdapter<T> > msg(new SyncMessageAdapter<T>(t, name));

            if (is_main_thread())
            {
                msg->dispatch();
            }
            else
            {
                post_command_(msg);
                msg->wait();
            }
            ret = msg->result();
            Lock<Mutex> lock(commands_.mutex());
            msg.reset();
        }
        return ret;
    }

    /**
     * schedule command to execute after next debug event
     */
    template<typename T>
    void schedule(const T& t, const char* name)
    {
        if (!shutdown_)
        {
            RefPtr<MessageAdapter<T> > msg(new MessageAdapter<T>(t, name));

            if (is_main_thread())
            {
                msg->dispatch();
            }
            else
            {
                sched_.push(msg);
            }
        }
    }

    /**
     * Try executing the command on the current thread.
     * If it fails, try running it in the main thread.
     */
    template<typename T>
    bool exec_command(const T& t, const char* name)
    {
        try
        {
            Lock<Mutex> lock(python_mutex());
            return MessageAdapter<T>(t, name).dispatch();
        }
        catch (const std::exception& e)
        {
            if (is_main_thread())
            {
                throw e;
            }
        #ifdef DEBUG
            std::clog << __func__ << ": " << e.what();
            std::clog << " (re-trying on main thread)\n";
        #endif
        }
        send_command(t, __func__);
        return false;
    }

    void set_debug_commands() { debugCommands_ = true; }
    void set_debug_events() { debugEvents_ = true; }

    int event_pipe();

    void activate_events();

    /**
     * Wait for either the event_loop to start or event_pipe
     * to be called.
     */
    void wait_for_event_activation();

    void wait_for_command_loop_activation();

    void set_callbacks(MarshallerCallbacks* callbacks)
    {
        callbacks_ = callbacks;
    }

    bool is_command_loop_active() const
    {
        Lock<Mutex> lock(mutex_);
        return inCommandLoop_;
    }

    bool is_gtk_mode_active() const { return usePipe_; }

    /**
     * Run the command loop if the commands_ queue is not empty.
     *
     * @return true if any processing occurred
     */
    bool process_pending_commands();

    size_t event_sequence() const { return eventSequence_; }
};



class ThreadMarshaller : public Singleton<Marshaller>
{
public:
    ~ThreadMarshaller() throw() { }
    static void shutdown() { instance().shutdown(); }
};


#endif // MARSHALLER_H__7CDBA4E5_E190_447B_8754_EE2F90842BC8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
