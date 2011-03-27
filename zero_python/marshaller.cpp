//
// $Id: marshaller.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <algorithm>
#include <deque>
#include <functional>
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include "zdk/export.h"
#include "dharma/config.h"
#include "dharma/system_error.h"
#include "generic/lock.h"
#include "generic/temporary.h"
#include "marshaller.h"
#include "python_mutex.h"
#include "utility.h"

using namespace std;
using namespace boost;
using namespace boost::python;



Marshaller::Marshaller()
    : events_(*this)
    , shutdown_(false)
    , debugCommands_(false)
    , debugEvents_(false)
    , usePipe_(false)
    , inCommandLoop_(false)
    , wakeup_(false)
    , eventsActive_(false)
    , eventPipe_()
    , self_(pthread_self())
    , callbacks_(NULL)
{
}



Marshaller::~Marshaller()
{
}


/**
 * Extract command from commands queue and dispatch it.
 *
 * A global python_mutex is held while the command is being
 * dispatched.
 */
bool Marshaller::dispatch_command() volatile
{
    bool result = false;

    assert(!commands_.empty());

    RefPtr<Message> cmd = commands_.front();
    commands_.pop();

    try
    {
        Lock<Mutex> lock(python_mutex());
        result = cmd->dispatch();
    }
    catch (const std::exception& e)
    {
        error_message(e.what());
    }
    catch (...)
    {
        error_message("Unknown exception in command_loop");
    }
    if (debugCommands_)
    {
        clog << __func__ << ": " << cmd->name() << "=" << result << endl;
    }
    {   Lock<Mutex> lock(commands_.mutex());
        cmd.reset();
    }
    return result;
}



void Marshaller::command_loop() volatile
{
    /**
     * Post notification events when entering / exiting command loop
     */
    class ZDK_LOCAL CommandLoopScope : noncopyable
    {
        bool debug_;
        Callbacks* cb_;

    public:
        CommandLoopScope(bool debug, MarshallerCallbacks* cb)
            : debug_(debug)
            , cb_(cb)
        {
            if (cb_)
            {
                ThreadMarshaller::instance().post_event(
                    bind(&Callbacks::on_command_loop_state, cb_, true),
                    "__command_loop_enter__");
            }
        }
        ~CommandLoopScope() throw()
        {
            if (cb_)
            {
                ThreadMarshaller::instance().post_event(
                    bind(&Callbacks::on_command_loop_state, cb_, false),
                    "__command_loop_done__");
            }
        }
    };
    /*******/
    Temporary<bool, Mutex> inLoop(inCommandLoop_, true, &mutex_);
    if (!inLoop())
    {
        signal_command_loop_active();
        CommandLoopScope scope(debugCommands_, callbacks_);

        for (bool done = false; !done && !shutdown_; )
        {
            CommandQueue::blocking_scope block(commands_);

            done = dispatch_command();
        }
    }
}



void Marshaller::error_message(const string& msg) volatile
{
    send_event(bind(&Marshaller::error, const_cast<Marshaller*>(this), msg),
               __func__);
}



void Marshaller::error(string msg)
{
    if (callbacks_)
    {
        callbacks_->on_error(msg);
    }
    else
    {
        cerr << "Error: " << msg << endl;
    }
}



static bool break_command(size_t sequence)
{
    // This assertion does not always hold, this call may also come
    // in on the UI thread.
    // assert (ThreadMarshaller::instance().is_main_thread());

    size_t eventSequence = ThreadMarshaller::instance().event_sequence();
    bool ret = (sequence == 0) || (eventSequence == sequence);
#if __DEBUG
    clog << __func__ << ": sequence=" << sequence << ", event=";
    clog << eventSequence << ", ret=" << ret << endl;
#endif
    return ret;
}


void Marshaller::break_command_loop(size_t sequence)
{
    assert(!is_main_thread());

    send_command(bind(break_command, sequence), __func__);
}


/**
 * For use with a UI main loop (eg. Gtk)
 *
 * Signals that an event has occurrd.
 */
void Marshaller::raise_event(bool writeEventPipe) volatile
{
    if (usePipe_ && writeEventPipe)
    {
        char dummy = 0;
        while (!eventPipe_.write(dummy, nothrow))
        {
            if (debugEvents_)
            {
                SystemError err(__func__);
                clog << err.what() << endl;
            }
        }
    }
    event_.broadcast();
}



static void wake_up(volatile bool& flag)
{
    flag = true;
}


void Marshaller::unblock_waiting_threads() volatile
{
    send_event(bind(wake_up, boost::ref(wakeup_)), "__wake_up__");
}

namespace
{
    /**
     * Send a break command when going out of scope
     */
    class ZDK_LOCAL AutoBreakCommandLoop : noncopyable
    {
        size_t sequence_;

    public:
        explicit AutoBreakCommandLoop(size_t sequence) : sequence_(sequence)
        { }
        ~AutoBreakCommandLoop()
        {
            ThreadMarshaller::instance().break_command_loop(sequence_);
        }
    };
    /**
     * Ensure that we don't loop forever processing commands.
     * Send a break command after dispatching the event. The command will
     * have no effect unless the sequence matches.
     *
     * @see Marshaller::break_command_loop
     */
    class ZDK_LOCAL EventWrapper : public ZObjectImpl<Message>
    {
    public:
        EventWrapper(const RefPtr<Message>& cmd, size_t sequence)
            : cmd_(cmd), sequence_(sequence)
        { }

        bool dispatch()
        {
            ThreadMarshaller::instance().wait_for_command_loop_activation();
            AutoBreakCommandLoop tmp(sequence_);

            return cmd_->dispatch();
        }

        const char* name() const { return cmd_->name(); }

        bool result() const { return cmd_ ? cmd_->result() : false; }

    private:
        RefPtr<Message> cmd_;
        size_t sequence_;
    };
}


/**
 * Send event from main debugger thread to Python thread
 */
bool
Marshaller::send_event_(const RefPtr<Message>& event, bool keepLoop) volatile
{
    {   Lock<Mutex> lock(mutex_);
        if (inCommandLoop_)
        {
            post_event_(event);
        }
        else if (!eventsActive_)
        {
            return false;
        }
    }
    const size_t sequence = keepLoop ? (eventSequence_ - 1) : ++eventSequence_;

    RefPtr<EventWrapper> wrap(new EventWrapper(event, sequence));

    post_event_(wrap);
    command_loop();
    return wrap->result();
}



void Marshaller::post_event_(const RefPtr<Message>& msg) volatile
{
    events_.push(msg);

    if (debugEvents_)
    {
        clog << "[to Python] <<< " << msg->name() << endl;
    }
}



void Marshaller::post_command_(const RefPtr<Message>& cmd)
{
    assert(!is_main_thread());
    commands_.push(cmd);

    if (debugCommands_)
    {
        clog << "[to main  ] >>> " << cmd->name() << endl;
    }
}


template<typename T>
static RefPtr<Message> wrap(const T& functor, const char* name)
{
    return new MessageAdapter<T>(functor, name);
}


///
/// @note runs on event thread
///
void Marshaller::wait_for_event(Lock<Mutex>& lock)
{
    while (events_.empty() && !shutdown_)
    {
        event_.wait(lock);
    }
}



void Marshaller::wait_for_main_thread()
{
    if (!is_main_thread())
    {
        wakeup_ = false;

        if (usePipe_)
        {
            Lock<Mutex> lock(events_.mutex());
            wait_for_event(lock);
        }
        do
        {
            event_iteration(usePipe_);
        } while (!wakeup_);
    }
}



void Marshaller::process_events(deque<RefPtr<Message> >& events)
{
    while (!events.empty())
    {
        string name = events.front()->name();
        string err;

        try
        {
            events.front()->dispatch();
        }
        catch (const error_already_set&)
        {
            err = name + ": " + python_get_error();
        }
        catch (const std::exception& e)
        {
            err = name + ": " + e.what();
        }
        catch (...)
        {
            err = name + ": unknown exception";
        }
        events.pop_front();

        if (!err.empty())
        {
            Lock<Mutex> lock(commands_.mutex());
            post_command_(wrap(bind(&Marshaller::error, this, err), __func__));
        }
    }
}



bool Marshaller::extract_events(deque<RefPtr<Message> >& events, bool readPipe)
{
    Lock<Mutex> lock(events_.mutex());
    if (usePipe_)
    {
        if (readPipe)
        {
            char dummy;
            eventPipe_.read(dummy);
        }
    }
    else
    {
        wait_for_event(lock);
    }
    events_.pop_all(lock, events);
    return !shutdown_;
}



bool Marshaller::event_iteration(bool readEventPipe)
{
    if (is_main_thread())
    {
        return true;
    }
    deque<RefPtr<Message> > events;

    bool keepRunning = extract_events(events, readEventPipe);
    if (keepRunning)
    {
        process_events(events);
    }
    return keepRunning;
}



void Marshaller::event_loop()
{
    if (usePipe_ && in_command_loop())
    {
        break_command_loop();
    }
    usePipe_ = false;
    activate_events();
    while (event_iteration())
    { }
}



void Marshaller::shutdown() volatile
{
    Lock<Mutex> lock(events_.mutex());
    if (!shutdown_)
    {
        shutdown_ = true;

        unblock_waiting_threads();

        // unblock threads waiting for command loop activation
        signal_command_loop_active();

        // drain command queue since there might be
        // threads waiting on a command to finish
        while (!commands_.empty())
        {
            dispatch_command();
        }
    }
}


int Marshaller::event_pipe()
{
    usePipe_ = true;
    int fd = eventPipe_.output();

    activate_events();
    return fd;
}



void Marshaller::wait_for_event_activation()
{
    Lock<Mutex> lock(mutex_);
    while (!eventsActive_ && !shutdown_)
    {
        eventStart_.wait(lock);
    }
}



void Marshaller::activate_events()
{
    Lock<Mutex> lock(mutex_);
    eventsActive_ = true;
    eventStart_.broadcast();
}



bool Marshaller::is_main_thread() const
{
    return self_ == pthread_self();
}



void Marshaller::wait_for_command_loop_activation()
{
    Lock<Mutex> lock(mutex_);
    while (!inCommandLoop_ && !shutdown_)
    {
        cmdLoopActive_.wait(lock);
    }
}


bool Marshaller::process_pending_commands()
{
    assert(is_main_thread());

    bool result = false;

    while (!result && !commands_.empty())
    {
        result = dispatch_command();
    }
    while (!sched_.empty())
    {
        RefPtr<Message> msg = sched_.front();
        sched_.pop();

        if (msg->dispatch())
        {
            result = true;
            break;
        }
    }
    return result;
}


bool Marshaller::in_command_loop() const
{
    Lock<Mutex> lock(mutex_);
    return inCommandLoop_;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
