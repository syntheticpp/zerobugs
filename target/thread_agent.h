#ifndef THREAD_AGENT_H__8892B9FE_7AAE_4BB9_9DCE_6543274269D5
#define THREAD_AGENT_H__8892B9FE_7AAE_4BB9_9DCE_6543274269D5
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
// Wrapper for thread_db library calls for enumerating
// threads in the target, initializing, etc.
//
#include "zdk/export.h"
#include "thread_db_error.h"
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_const.hpp>



template<typename T>
class ZDK_LOCAL ThreadAgentWrapper : public T
{
    // non-copyable, non-assignable
    ThreadAgentWrapper(const ThreadAgentWrapper&);
    ThreadAgentWrapper& operator=(const ThreadAgentWrapper&);

    struct AgentDeleter // for use with boost::shared_ptr<>
    {
        void operator()(td_thragent* ta) { td_ta_delete(ta); }
    };

    /**
     * Ensure that thread_db library is initialized
     */
    bool init_thread_db()
    {
        static bool inited = false;
        if (!inited)
        {
            int res = td_init();
            if (res == TD_OK)
            {
                inited = true;
            }
            else
            {
                dbgout(0) << "td_init=" << res << " ("
                          << thread_db_error(__FILE__, __LINE__, res).what()
                          << ")" << endl;
            }
        }
        return inited;
    }

protected:
    mutable td_thr_events_t mask_;
    mutable boost::shared_ptr<td_thragent_t> ta_;

    const td_thragent_t* get_agent() const { return ta_.get(); }

    /**
     * Callback for iterate_threads, see below.
     */
    template<typename U>
    struct ZDK_LOCAL Iteration
    {
        static int step(const td_thrhandle_t* thr, void* fn) throw()
        {
            try
            {
                assert(fn);
                return static_cast<U*>(fn)->on_thread_handle(thr);
            }
            catch (const std::exception& e)
            {
                std::cerr << __func__ << ": " << e.what() << std::endl;
            }
            return TD_ERR;
        }
    };


    void enable_event_reporting(const td_thrhandle_t* thr)
    {
        assert(td_eventismember(&mask_, TD_CREATE));
        // not interested in TD_DEATH event, for now
        //assert(td_eventismember(&mask_, TD_DEATH));

        TD_ENFORCE(td_thr_event_enable(thr, 1));
        TD_ENFORCE(td_thr_set_event(thr, &mask_));
    }


    void disable_events()
    {
        if (ta_)
        {
            td_event_emptyset(&mask_);
            TD_ENFORCE(td_ta_set_event(ta_.get(), &mask_));
        }
    }

    virtual void
    on_thread(const td_thrhandle_t* thr, const td_thrinfo_t&)
    {
    }

    virtual bool init_thread_agent(bool force = false)
    {
        if (force)
        {
            ta_.reset();
        }

        if (!ta_ && init_thread_db())
        {
            td_thragent_t* ta = NULL;
            ps_prochandle* ps = this;

            int res = td_ta_new(ps, &ta);

            // td_ta_new may fail for stripped executable
            if (res != TD_OK)
            {
                dbgout(1) << "td_ta_new=" << res << " ("
                          << thread_db_error(__FILE__, __LINE__, res).what()
                          << ")" << endl;
            }
            else
            {
            #ifdef DEBUG
                std::clog << __func__ << ": ok\n";
            #endif

                assert(ta);
                ta_ = boost::shared_ptr<td_thragent_t>(ta, AgentDeleter());

                td_event_addset(&mask_, TD_EVENTS_ENABLE);
                td_event_addset(&mask_, TD_CREATE);

                // not interested in this event for now
                //td_event_addset(&mask_, TD_DEATH);

                TD_ENFORCE(td_ta_set_event(ta, &mask_));

                iterate_threads(*this);
            }
        }
        return get_agent() != NULL;
    }

    /**
     * Template method pattern: on_thread is virtual
     */
    int on_thread_handle(const td_thrhandle_t* thr)
    {
        if (thr && td_thr_validate(thr) == TD_OK)
        {
            td_thrinfo_t info;
            memset(&info, 0, sizeof info);

            TD_ENFORCE(td_thr_get_info(thr, &info));

            enable_event_reporting(thr);
            on_thread(thr, info);
        }
        return TD_OK;
    }

    bool is_multithread(bool initAgent = false)
    {
        if (initAgent)
        {
            init_thread_agent();
        }
        return get_agent() != NULL;
    }

    template<typename U>
    void iterate_threads(U& callback)
    {
        typedef typename boost::remove_const<U>::type V;

        if (const td_thragent_t* agent = get_agent())
        {
            td_ta_thr_iter(agent,
                           &Iteration<V>::step,
                           &callback,
                           TD_THR_ANY_STATE,
                           TD_THR_LOWEST_PRIORITY,
                           TD_SIGNO_MASK,
                           TD_THR_ANY_USER_FLAGS);
        }
        else
        {
            dbgout(1) << __func__ << ": no thread agent" << endl;
        }
    }

    addr_t get_event_addr(td_event_e event) const
    {
        if (!td_eventismember(&mask_, event))
        {
            return 0;
        }

        if (const td_thragent_t* agent = get_agent())
        {
            td_notify_t notify;

            //td_event_addset(&mask_, event);
            //TD_ENFORCE(td_ta_set_event(agent, &mask_));

            TD_ENFORCE(td_ta_event_addr(agent, event, &notify));

            if (notify.type == NOTIFY_BPT)
            {
                // on FreeBSD psaddr_t is nicely defined as an unsigned,
                // on Linux it is defined as void* -- hence the cast
                return addr_t(notify.u.bptaddr);
            }
        }
        return 0;
    }

    unsigned long lwpid_to_tid(pid_t pid) const
    {
        if (const td_thragent_t* agent = get_agent())
        {
            td_thrhandle_t handle = { 0 };

            if (td_ta_map_lwp2thr (agent, pid, &handle) == TD_OK)
            {
                td_thrinfo_t info = { 0 };
                if (td_thr_get_info(&handle, &info) == 0)
                {
                    return info.ti_tid;
                }
            }
        }
        return 0;
    }

    ThreadAgentWrapper(debugger_type& arg) : T(arg)
    {
        td_event_emptyset(&mask_);
    }

    virtual ~ThreadAgentWrapper() throw() { }
};

#endif // THREAD_AGENT_H__8892B9FE_7AAE_4BB9_9DCE_6543274269D5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
