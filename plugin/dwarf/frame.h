#ifndef FRAME_H__AF2067DC_E74E_46AA_AF31_6D1C60861FC7
#define FRAME_H__AF2067DC_E74E_46AA_AF31_6D1C60861FC7
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

#include "zdk/stack.h"
#include "zdk/zobject_impl.h"


namespace Dwarf
{
    using Platform::addr_t;

    /**
     * A simple implementation of the the Frame interface.
     * @see zdk/stack.h
     * Used for generating frames when unwinding the stack
     * using DWARF info.
     * @note Not using the same impl as the engine, in order
     * to keep this plugin and the engine decoupled.
     */
    class ZDK_LOCAL FrameImpl : public ZObjectImpl<Frame>
    {
    public:
        DECLARE_UUID("4f0d458d-5df5-4448-8c99-2cb08f3a6eea")
        BEGIN_INTERFACE_MAP(FrameImpl)
            INTERFACE_ENTRY(FrameImpl)
            INTERFACE_ENTRY(Frame)
        END_INTERFACE_MAP()

        FrameImpl() : pc_(0), sp_(0), fp_(0), realPC_(0) { }

        virtual ~FrameImpl() throw() { }

        virtual addr_t program_count() const { return pc_; }

        virtual addr_t stack_pointer() const { return sp_; }

        virtual addr_t frame_pointer() const { return fp_; }

        // not used
        virtual Symbol* function(Symbol* = NULL) const;

        // not used
        virtual size_t index() const { return 0; }

        /**
         * The Frame interface exposes a mechanism for passing
         * arbitrary data around, as opaque ZObject instances.
         */
        virtual ZObject* get_user_object(const char* key) const;
        virtual void set_user_object(const char* key, ZObject*);

        void set_program_count(addr_t pc)
        {
            pc_ = pc;
            if (!realPC_)
            {
                realPC_ = pc;
            }
        }

        void set_stack_pointer(addr_t sp) { sp_ = sp; }

        void set_frame_pointer(addr_t fp) { fp_ = fp; }

        void set_real_program_count(addr_t pc) { realPC_ = pc; }

        addr_t real_program_count() const { return realPC_; }

        // not used
        bool is_signal_handler() const { return false; }

        bool is_from_frame_handler() const
        { return true; }

    private:
        addr_t pc_;
        addr_t sp_;
        addr_t fp_;
        addr_t realPC_;
        // RefPtr<ZObject> userObj_;
    };
}
#endif // FRAME_H__AF2067DC_E74E_46AA_AF31_6D1C60861FC7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
