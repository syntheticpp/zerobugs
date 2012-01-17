#ifndef APP_SLOTS_H__35B9FEEE_F780_4C6C_864F_973252ECD90C
#define APP_SLOTS_H__35B9FEEE_F780_4C6C_864F_973252ECD90C
//
// $Id: app_slots.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/stdexcept.h"
#include "zdk/debug_symbol_list.h"
#include "zdk/expr.h"
#include "zdk/mutex.h"
#include "zdk/weak_ptr.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "assert_thread.h"
#include "auto_condition.h"
#include "thread_marshaller.h"

class InterThreadCommand;
class PopupList;
class TextEntry;

using Platform::addr_t;


/**
 * Provides the slots where most view widgets connect
 * their signals.
 *
 * @note refactoring is in progress, some methods of
 * the MainWindow class will move here over time
 */
class ZDK_LOCAL AppSlots : public ThreadMarshaller
{
public:
    void read_symbol(RefPtr<DebugSymbol>, DebugSymbolEvents*);

    void step_over_func(RefPtr<Symbol>);
    void step_over_file(RefPtr<Symbol>);
    void step_over_dir(RefPtr<Symbol>);
    void step_over_reset(RefPtr<Symbol>);
    void step_over_delete(RefPtr<SharedString>, long);

    bool on_auto_complete (TextEntry*, std::string, bool);
    void on_menu_delete_breakpoint(addr_t, size_t);

    void hide_auto_completion_window() { popupList_.reset(); }

    void update_stack_on_main_thread();

    bool run_on_main_thread(RefPtr<InterThreadCommand> cmd) throw()
    {
        try
        {
            if (::is_ui_thread())
            {
                call_on_main_thread(&AppSlots::run_on_main_thread, cmd);
            }
            else
            {
                MainThreadScope scope(mainThreadMutex_, mainThreadReady_);
                cmd->execute();
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << __func__ << ": " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool run_on_ui_thread(RefPtr<InterThreadCommand> cmd)
    {
        static Condition cond;
        static bool result = false;

        if (!is_ui_thread())
        {
            TRY_LOCK(lock, mainThreadMutex_, "UI");
            post_request(command(&AppSlots::run_on_ui_thread, this, cmd));
            cond.wait(lock); // wait for UI thread to complete
        }
        else
        {
            assert_ui_thread();
            Lock<Mutex> lock(mainThreadMutex_);
            AutoCondition signal_when_done(cond);
            result = cmd->execute();
        }
        return result;
    }

    bool on_apply_filter
      (
        RefPtr<DebugSymbol>*,
        RefPtr<DebugSymbol>,
        DebugSymbolEvents*
      );

    virtual void on_help(const char*) = 0;
    virtual void update_stack_view() = 0;
    virtual void update_breakpoint_view(addr_t, size_t line) = 0;

    void register_object(const char* name, ZObject* obj)
    {
        objMap_.insert(std::make_pair(name, obj));
    }

    RefPtr<ZObject> get_object(const char* name) const
    {
        ObjMap::const_iterator i = objMap_.find(name);
        if (i != objMap_.end())
        {
            return i->second.lock();
        }
        return NULL;
    }

    bool on_evaluate
      (
        RefPtr<Thread>,
        std::string expr,
        addr_t frameBase,
        ExprEvents*,
        int numericBase
      );

protected:
    class MainThreadScope : boost::noncopyable
    {
        Lock<Mutex> lock_;
        AutoCondition autoBroadcast_;

    public:
        MainThreadScope(Mutex& mutex, Condition& cond)
            : lock_(mutex), autoBroadcast_(cond)
        { }
    };

    AppSlots(Debugger&);

    void save_stack(std::ostream*);

    RefPtr<DataType> on_query_type(RefPtr<Thread>, std::string, addr_t);

    void on_query_symbols
      (
        RefPtr<Thread>,
        std::string name,
        addr_t addr,
        DebugSymbolList* syms,
        bool readValues
      );

    template<typename R, typename T>
        void
        call_on_main_thread(R (T::*mfun)())
        {
           TRY_LOCK(lock, mainThreadMutex_);
           this->set_service_call(true);
           CALL_MAIN_THREAD_(command(mfun, static_cast<T*>(this)));
           wait_for_main_thread(lock);
        }
    template<typename A0, typename R, typename T>
        void
        call_on_main_thread(R (T::*mfun)(A0), A0 a0)
        {
           TRY_LOCK(lock, mainThreadMutex_);
           this->set_service_call(true);
           CALL_MAIN_THREAD_(command(mfun, static_cast<T*>(this), a0));
           wait_for_main_thread(lock);
        }
    template<typename A0, typename A1, typename R, typename T>
        void
        call_on_main_thread
        (
            R (T::*mfun)(A0, A1),
            A0 a0,
            A1 a1
        )
        {
           TRY_LOCK(lock, mainThreadMutex_);
           this->set_service_call(true);
           CALL_MAIN_THREAD_(command(mfun, static_cast<T*>(this), a0, a1));
           wait_for_main_thread(lock);
        }
    template<typename A0, typename A1, typename A2, typename R, typename T>
        void
        call_on_main_thread
        (
            R (T::*mfun)(A0, A1, A2),
            A0 a0,
            A1 a1,
            A2 a2
        )
        {
           TRY_LOCK(lock, mainThreadMutex_);
           this->set_service_call(true);
           CALL_MAIN_THREAD_(command(mfun, static_cast<T*>(this), a0, a1, a2));
           wait_for_main_thread(lock);
        }
    template<typename A0, typename A1, typename A2, typename A3,
             typename R, typename T>
        void
        call_on_main_thread
        (
            R (T::*mfun)(A0, A1, A2, A3),
            A0 a0,
            A1 a1,
            A2 a2,
            A3 a3
        )
        {
           TRY_LOCK(lock, mainThreadMutex_);
           this->set_service_call(true);

           CALL_MAIN_THREAD_(command(mfun, static_cast<T*>(this), a0, a1, a2, a3));
           wait_for_main_thread(lock);
        }
    template<typename A0, typename A1, typename A2, typename A3, typename A4,
             typename R, typename T>
        void
        call_on_main_thread
        (
            R (T::*mfun)(A0, A1, A2, A3, A4),
            A0 a0,
            A1 a1,
            A2 a2,
            A3 a3,
            A4 a4
        )
        {
           TRY_LOCK(lock, mainThreadMutex_);
           this->set_service_call(true);
           CALL_MAIN_THREAD_(command(boost::bind(mfun, static_cast<T*>(this), a0, a1, a2, a3, a4)));
           wait_for_main_thread(lock);
        }

protected:
    mutable Mutex mainThreadMutex_;   // for synchronous slots

private:
    inline void wait_for_main_thread(Lock<Mutex>& lock)
    {
        mainThreadReady_.wait(lock);
    }

    typedef ext::hash_map<std::string, WeakPtr<ZObject> > ObjMap;
    
    Condition                       mainThreadReady_;
    boost::shared_ptr<PopupList>    popupList_;
    ObjMap                          objMap_;
};

#endif // APP_SLOTS_H__35B9FEEE_F780_4C6C_864F_973252ECD90C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
