#ifndef LINKER_EVENTS_H__DF9422B4_E907_4742_941C_2E7CD074B8AD
#define LINKER_EVENTS_H__DF9422B4_E907_4742_941C_2E7CD074B8AD
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
#include "zdk/arch.h"
#include "zdk/log.h"
#include "zdk/thread_util.h"
#include "elfz/public/binary.h"
#include "elfz/public/link.h"
#include "symbolz/private/link_data.h"

#include <iostream>


/**
 * Intercept dynamic linker events in the debug target
 * by setting an internal breakpoint
 */
template<typename T>
class ZDK_LOCAL LinkerEventObserver : public T
{
private:
    /**
     * Wrap the update_symbol_map method into
     * a BreakPointAction
     */
    class ZDK_LOCAL Action : public ZObjectImpl<BreakPointAction>
    {
        LinkerEventObserver<T>* leo_;

        const char* name() const { return "LinkerEvent"; }

        word_t cookie() const { return 0x1984; }

    public:
        bool execute(Thread* thread, BreakPoint*)
        {
            if (thread)
            {
                assert(&leo_->debugger() == thread->debugger());
                leo_->update_symbol_map(*thread);
            }
            return true;
        }

        explicit Action(LinkerEventObserver* leo) : leo_(leo)
        { }
    }; // end Action

private:
    addr_t addr_;
    bool inited_;

    friend bool Action::execute(Thread*, BreakPoint*);

    /**
     * Determine the address where the breakpoint should be set.
     * Called by init_linker_events
     */
    addr_t get_breakpoint_addr(Thread& thread)
    {
        addr_t addr = 0;
        if (addr_)
        {
            size_t nread = 0;
            thread_read(thread, this->addr_, addr, &nread);

            if (nread && addr)
            {
                r_debug_<> rd;
                thread_readx(thread, addr, rd, &nread);

                if (nread == 0)
                {
                    addr = 0;
                }
                else
                {
                    addr_ = addr;
                    addr = ADDR_CAST(rd.r_brk);
                }
            }
        }
        dbgout(2) << __func__ << ": addr=" << (void*)addr << std::endl;
        return addr;
    }

    void detect_multithread_target()
    {
        static const bool initThreadAgent = true;

        if (!this->is_multithread()
          && this->is_multithread(initThreadAgent))
        {
            std::cout << "Detected multithreaded target" << std::endl;
        }
    }

protected:
    template<typename U>
    explicit LinkerEventObserver(U& arg)
        : T(arg), addr_(0), inited_(false) { }

    virtual ~LinkerEventObserver() throw () { }

    void uninitialize_linker_events()
    {
        inited_ = false, addr_ = 0;
    }

    void init_linker_events(Thread& thread)
    {
        if (inited_)
        {
            return;
        }
        if (!addr_)
        {
            ELF::Binary bin(CHKPTR(this->process_name())->c_str());

            if ((addr_ = ELF::get_linker_debug_struct_addr(bin)) == addr_t(-1))
            {
                dbgout(0) << __func__ << ": static target" << std::endl;
                inited_ = true; // static binary
                addr_ = 0;
                detect_multithread_target();
            }
        }
        // adjust addr_ by the base address where process is loaded
        if (this->process())
        {
            if (RefPtr<SymbolMap> symbols = this->process()->symbols())
            {
                SymbolTable* table =  symbols->symbol_table_list(this->process_name()->c_str());
                if (table && addr_ < table->addr())
                {
                    addr_ += table->addr();
                }
            }
        }

        if (addr_t addr = get_breakpoint_addr(thread))
        {
            if (BreakPointManager* mgr = this->debugger().breakpoint_manager())
            {
                RefPtr<Action> action = new Action(this);

                if (mgr->set_breakpoint(get_runnable(&thread),
                                        BreakPoint::SOFTWARE,
                                        addr,
                                        action.get()))
                {
                    inited_ = true;

                    dbgout(0) << __func__ << " successful" << std::endl;
                }
                else
                {
                    dbgout(0) << __func__ << ": failed to set breakpoint" << std::endl;
                }
            }
        }
    }

public:
    bool has_linker_events() const 
    {
        dbgout(1) << __func__ << ": " << T::process_name()->c_str() 
                      << "=" << inited_ << std::endl;
        return inited_;
    }

    /**
     * The breakpoint action fires this method every time
     * the dynamic linker/loader maps a shared object into
     * the debug target's memory. The method updates the
     * target' s symbol tables.
     */
    void update_symbol_map(Thread& thread)
    {
        if (addr_ == 0)
        {
            return;
        }
        r_debug_<> rd;
        thread_readx(thread, addr_, rd);

        if (rd.r_state != RT_CONSISTENT)
        {
            return;
        }
        inited_ = true;

        // read the linked list of link_map objects
        RefPtr<LinkDataImpl> head;
        RefPtr<LinkDataImpl> tail;

        while (rd.r_map)
        {
            link_map_<> m;
            const addr_t maddr = rd.r_map;

            char buf[PATH_MAX + 1] = { 0 };
            thread_readx(thread, maddr, m);

            if (m.l_name)
            {
                memset(buf, 0, sizeof buf);
                size_t nread = 0;
                thread_read(thread, static_cast<addr_t>(m.l_name), buf, &nread);
                assert(nread <= PATH_MAX);
                dbgout(1) << __func__ << ": " << std::hex << m.l_addr 
                              << std::dec << ": " << buf << std::endl;

                RefPtr<LinkDataImpl> ldata(new LinkDataImpl(m.l_addr, buf));

                if (!head)
                {
                    head = ldata;
                }
                if (tail)
                {
                    tail->set_next(ldata);
                }
                tail = ldata;
            }
            else
            {
                dbgout(0) << __func__ << "??? " << m.l_addr << std::endl;
            }
            rd.r_map = m.l_next;
        }
        dbgout(1) << "Updating symbol map..." << std::endl;
        CHKPTR(this->symbols())->update(head.get());

        detect_multithread_target();
    }
};

#endif // LINKER_EVENTS_H__DF9422B4_E907_4742_941C_2E7CD074B8AD
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
