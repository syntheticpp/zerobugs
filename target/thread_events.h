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
#include <boost/utility.hpp> // for noncopyable
#include "thread_agent.h"
#include "thread.h"
#include "debugger_base.h"

using Platform::addr_t;


/**
 * Receives notifications when threads are being created
 * or destroyed in the target.
 */
template<typename T>
class ThreadEventObserver : public ThreadAgentWrapper<T>
                          , private boost::noncopyable
{
    ThreadEventObserver* self_; // hack around const

    class ZDK_LOCAL Action : public ZObjectImpl<BreakPointAction>
    {
        ThreadEventObserver* teo_;

        const char* name() const { return "ThreadEvent"; }
        word_t cookie() const { return reinterpret_cast<word_t>(teo_); }

    public:
        bool execute(Thread* thread, BreakPoint* bp)
        {
            teo_->on_thread_breakpoint(thread, bp);
            return true;
        }

        explicit Action(ThreadEventObserver* teo) : teo_(teo) { }
    }; // Action

    friend bool Action::execute(Thread*, BreakPoint*);


    void on_thread_breakpoint(Thread* thread, BreakPoint*)
    {
        assert(thread);
        assert(this->get_agent());

        td_event_msg_t msg;

        // extract all available messages
        for (int err = 0;;)
        {
            memset(&msg, 0, sizeof msg);

            err = td_ta_event_getmsg(this->get_agent(), &msg);

            if (err == TD_NOMSG)
            {
                break;
            }
            else if (err)
            {
                thread_db_error e(__FILE__, __LINE__, err);
                dbgout(0) << thread->lwpid() << ": " << e.what() << std::endl;

                break;
            }
            dbgout(0) << __func__ << ": " << thread->lwpid()
                          << ": msg.event="   << msg.event << std::endl;

            td_thrinfo_t info = { 0 };

            switch (msg.event)
            {
            case TD_CREATE:
                if (msg.th_p)
                {
                #ifdef __FreeBSD__
                    this->on_thread_handle((td_thrhandle_t *)msg.th_p);
                #else
                    this->on_thread_handle(msg.th_p);
                #endif
                }
                break;

            case TD_DEATH:
            #ifdef __FreeBSD__
                TD_ENFORCE(td_thr_get_info((td_thrhandle_t *)msg.th_p, &info));
            #else
                TD_ENFORCE(td_thr_get_info(msg.th_p, &info));
            #endif
                dbgout(0) << "TD_DEATH: info.ti_lid=" << info.ti_lid
                          << " info.ti_tid=" << info.ti_tid
                          << " (reported on thread " << thread->lwpid()
                    #ifdef __FreeBSD__
                          << ") msg.data=" << msg.data << std::endl;
                    #else
                          << ") msg.data=" << msg.msg.data << std::endl;
                    #endif
              /*
                My understanding is that the breakpoint should occur on
                the thread that is about to exit;  but I noticed this
                TD_DEATH notification coming in on the main thread rather
                than on the about-to-die thread (2.6.18 kernel with glib2.4)

                I have to investigate this further, for now I will just
                ignore the event.


                if ((thread == NULL) || (info.ti_lid != thread->lwpid()))
                {
                    thread = this->get_thread(info.ti_lid, info.ti_tid);
                }
                if (thread)
                {
                    assert(info.ti_lid == thread->lwpid());
                    interface_cast<ThreadImpl&>(*thread).set_exiting();
                }
              */
                break;

            default: // silence off compiler warning about unhandled cases
                break;
            }
        }
    }

    /**
     * Install breakpoint action for the specified event.
     */
    bool set_breakpoint( const RefPtr<BreakPointAction>& action,
                         const RefPtr<Thread>& thread,
                         BreakPointManager& bpMgr,
                         td_event_e event // TD_CREATE, TD_DEATH, etc.
                       ) const
    {
        if (addr_t addr = this->get_event_addr(event))
        {
            return bpMgr.set_breakpoint(get_runnable(thread.get()),
                                 BreakPoint::SOFTWARE,
                                 addr,
                                 action.get());
        }
        return false;
    }

    /**
     * Install breakpoints that intercept the creation and
     * destruction of threads -- at this point, the target
     * should already have one thread.
     */
    bool set_event_breakpoints() const
    {
        bool result = false;

        assert(!this->empty()); // need at least one thread

        Debugger& dbg = T::debugger();
        if (BreakPointManager* mgr = dbg.breakpoint_manager())
        {
            RefPtr<Thread> thread = *self_->threads_begin();
            RefPtr<BreakPointAction> action(new Action(this->self_));

            result |= set_breakpoint(action, thread, *mgr, TD_CREATE);
            result &= set_breakpoint(action, thread, *mgr, TD_DEATH);
        }
        return result;
    }

protected:
    virtual ~ThreadEventObserver() throw() { }

    ThreadEventObserver(debugger_type& arg)
        : ThreadAgentWrapper<T>(arg), self_(this)
    { }

    virtual bool init_thread_agent(bool force = false)
    {
        bool result = ThreadAgentWrapper<T>::init_thread_agent(force);
        if (result)
        {
            try
            {
                result &= set_event_breakpoints();
            }
            catch (const std::exception& e)
            {
                result = false;
                dbgout(0) << __func__ << ": " << e.what() << std::endl;
            }
        }

        return result;
    }
};
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
