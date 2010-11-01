#ifndef FILTER_H__077076B9_D870_4345_A200_E9AEF5F8972E
#define FILTER_H__077076B9_D870_4345_A200_E9AEF5F8972E
//
// $Id: filter.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/data_filter.h"
#include "zdk/plugin.h"

class Python;

CLASS UserDataFilter : public DataFilter
{
    DebugSymbol* transform(DebugSymbol*, DebugSymbol*, DebugSymbolEvents*) const;

    bool hide(const DebugSymbol*) const;

    void release() { delete this; }

    size_t enum_params(
        EnumCallback3<const char*,
                      const char*,
                      const Variant*,
                      bool>*) const;

    void on_param_change(Properties*, const char* name);

    //void set_param(const char*, const Variant*);
    //void get_param(const char*, Variant**) const;
    RefPtr<Python> interp_;

public:
    explicit UserDataFilter(WeakPtr<Python>&);
    virtual ~UserDataFilter() throw();

BEGIN_INTERFACE_MAP(UserDataFilter)
    INTERFACE_ENTRY(DataFilter)
END_INTERFACE_MAP()

};


#endif // FILTER_H__077076B9_D870_4345_A200_E9AEF5F8972E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
