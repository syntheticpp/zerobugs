#ifndef ADDR_OPERATIONS_H__978E2FCA_6C8E_4695_9624_EF8BA36F4640
#define ADDR_OPERATIONS_H__978E2FCA_6C8E_4695_9624_EF8BA36F4640
//
// $Id: addr_operations.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "dwarfz/public/addr_ops.h"
#include "zdk/mutex.h"
#include "zdk/variant.h"
#include "zdk/weak_ptr.h"
#include "zdk/zero.h"
#include "generic/lock.h"
#include "dbgout.h"


namespace Dwarf
{
    Mutex& get_operations_mutex();


    /**
     * Implements the Dwarf::AddrOps interface,
     * needed by Dwarf::Location::eval(). Reads memory contents
     * and CPU registers in the context of given thread.
     */
    class ZDK_LOCAL AddrOpsImpl : public AddrOps, EnumCallback<Register*>
    {
    public:
        explicit AddrOpsImpl(const Thread* thread)
            : thread_(thread)
            , regnum_(0)
            , regval_(0)
        {
            assert(thread);
        }

        Dwarf_Addr read_mem(Dwarf_Addr addr)
        {
            long val = 0;

            try
            {
                thread_->read_data(addr, &val, 1);
                if (thread_->is_32_bit())
                {
                    val &= 0xffffffff;
                }
            }
            catch (const std::exception& e)
            {
                // benign, may be caused by erroneous user input
                std::cerr << __func__ << ": " << e.what() << std::endl;
            }
            return static_cast<Dwarf_Addr>(val);
        }

        /**
         * @note when useFrame is true, attempt to get the
         * value saved on the stack
         */
        Dwarf_Addr read_cpu_reg(Dwarf_Signed n, bool useFrame)
        {
            reg_t val = 0;
            if (RefPtr<Thread> thread = thread_.lock())
            {
                val = thread_->read_register(n, useFrame);

            #if (__WORDSIZE == 64)
                if (thread_->is_32_bit())
                {
                    val &= 0xffffffff;
                }
            #endif
            }
            return static_cast<Dwarf_Addr>(val);
        }

        bool is_32_bit() const { return thread_->is_32_bit(); }

    private:
        /**
         * implement the EnumCallback<Register*> interface,
         * to help out read_cpu_reg()
         */
        void notify(Register* reg)
        {
            if (regnum_)
            {
                if  (--regnum_ == 0)
                {
                    regval_ = reg->value()->uint64();
                }
            }
        }

    private:
        WeakPtr<Thread> thread_;
        Dwarf_Unsigned  regnum_; // read_cpu_reg
        Dwarf_Addr      regval_; // read_cpu_reg
    };


    /**
     * Set address operations in scope
     */
    class AddrOperationsContext : public Lock<Mutex>
    {
    public:
        explicit AddrOperationsContext(const Thread* thread)
            : Lock<Mutex>(get_operations_mutex())
            , ops_(thread)
            , oldOps_(0)
        {
            oldOps_ = set_addr_operations(&ops_);
        }

        /**
         * dtor restores old operations
         */
        ~AddrOperationsContext()
        {
            set_addr_operations(oldOps_);
        }

        AddrOpsImpl& operations() { return ops_; }

    private:
        // non-copyable, non-assignable
        AddrOperationsContext(const AddrOperationsContext&);
        AddrOperationsContext& operator=(const AddrOperationsContext&);

        AddrOpsImpl ops_;
        AddrOps*    oldOps_;
    };
}
#endif // ADDR_OPERATIONS_H__978E2FCA_6C8E_4695_9624_EF8BA36F4640
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
