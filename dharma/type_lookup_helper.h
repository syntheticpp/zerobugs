#ifndef TYPE_LOOKUP_HELPER_H__D70D1BFC_5C2A_414D_8C0D_0796165BAE90
#define TYPE_LOOKUP_HELPER_H__D70D1BFC_5C2A_414D_8C0D_0796165BAE90
//
// $Id: type_lookup_helper.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/data_type.h"
#include "zdk/debug_sym.h"
#include "zdk/zero.h" // for DebuggerPlugin
#include "dharma/object_manager.h"



class TypeLookupHelper : public EnumCallback<DebuggerPlugin*>
{
public:
    virtual ~TypeLookupHelper() throw() {}

    TypeLookupHelper
    (
        Thread& thread,
        const std::string& name,
        addr_t addr,
        LookupScope scope = LOOKUP_ALL
    )
      : thread_(&thread)
      , name_(name)
      , addr_(addr)
      , scope_(scope)
    {}

    void notify(DebuggerPlugin* p)
    {
        if (type_.get()) return;

        if (DebugInfoReader* debugInfo = interface_cast<DebugInfoReader*>(p))
        {
            Thread* thread = thread_.get();
            const char* name = name_.c_str();
            type_ = debugInfo->lookup_type(thread, name, addr_, scope_);
        }
    }

    RefPtr<DataType> type() const { return type_; }

private:
    RefPtr<Thread>      thread_;
    std::string         name_;
    addr_t              addr_;
    RefPtr<DataType>    type_; // result
    LookupScope         scope_;
};

#endif // TYPE_LOOKUP_HELPER_H__D70D1BFC_5C2A_414D_8C0D_0796165BAE90
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
