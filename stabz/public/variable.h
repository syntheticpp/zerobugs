#ifndef VARIABLE_H__435D5E8E_2749_44FF_B3C1_50D52C3D3FFC
#define VARIABLE_H__435D5E8E_2749_44FF_B3C1_50D52C3D3FFC
//
// $Id: variable.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "stabz/public/datum.h"

using Platform::addr_t;


namespace Stab
{
    class Variable : public Datum
    {
    public:
        DECLARE_UUID("dfbcd334-2247-4beb-8620-e4b3030b0396")

        Variable(SharedString&, DataType&, addr_t);

        addr_t offset() const { return offset_; }

        void set_offset(addr_t offset) { offset_ = offset; }

        /**
         * Compute the address, relative to the virtual memory
         * base (vm_area_struct) and a frame base.
         */
        virtual addr_t addr(addr_t vmBase, addr_t frame = 0) const;

        BEGIN_INTERFACE_MAP(Variable)
            INTERFACE_ENTRY(Variable)
        END_INTERFACE_MAP()

    private:
        addr_t offset_;
    };


    class Parameter : public Variable
    {
    public:
        DECLARE_UUID("33ffa561-03e0-41a9-9208-e4e7e1a69f32")

        BEGIN_INTERFACE_MAP(Parameter)
            INTERFACE_ENTRY(Parameter)
            INTERFACE_ENTRY(Variable)
        END_INTERFACE_MAP()

        Parameter(SharedString&, DataType&, addr_t);
    };


    class GlobalVariable : public Variable
    {
    public:
        GlobalVariable(SharedString&, DataType&, addr_t);

        virtual addr_t addr(addr_t, addr_t) const;

        BEGIN_INTERFACE_MAP(Variable)
            INTERFACE_ENTRY(Variable)
        END_INTERFACE_MAP()
    };
}

#endif // VARIABLE_H__435D5E8E_2749_44FF_B3C1_50D52C3D3FFC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
