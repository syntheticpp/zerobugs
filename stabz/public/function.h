#ifndef FUNCTION_H__F9C6FCB4_5B79_4F58_9FD6_C35C15218D9F
#define FUNCTION_H__F9C6FCB4_5B79_4F58_9FD6_C35C15218D9F
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

#include <memory>
#include "typez/public/types.h" // for ParamTypes
#include "block.h"


namespace Stab
{
    /**
     * Functions are represented by N_FUN stab entries.
     */
    class Function : public Block
    {
    public:
        DECLARE_UUID("6f06ebfc-e0f7-4b28-b5fe-5dcbdc84f1d2")
        BEGIN_INTERFACE_MAP(Function)
            INTERFACE_ENTRY(Function)
        END_INTERFACE_MAP()

        Function(addr_t, size_t index, SharedString&);

        ~Function() throw();

        SharedString& name() const;

        RefPtr<DataType> return_type() const;

        void set_return_type(const RefPtr<DataType>&);

        const ParamTypes& param_types() const;

        void assign_variables(const VarList&);

        const VarList& params() const;

    private:
        Function(const Function&);
        Function& operator=(const Function&);

        RefPtr<SharedString> name_;

        WeakDataTypePtr returnType_;

        mutable std::auto_ptr<ParamTypes> paramTypes_;
        mutable std::auto_ptr<VarList> params_;
    };
}
#endif // FUNCTION_H__F9C6FCB4_5B79_4F58_9FD6_C35C15218D9F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
