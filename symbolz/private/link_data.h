#ifndef LINK_DATA_H__3A891A11_032E_464C_8737_7B39E7640AD3
#define LINK_DATA_H__3A891A11_032E_464C_8737_7B39E7640AD3
//
// $Id: link_data.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/config.h"
#include "zdk/ref_counted_impl.h"
#include "zdk/symbol_map.h"

using Platform::addr_t;


CLASS LinkDataImpl : public RefCountedImpl<SymbolMap::LinkData>
{
public:
    explicit LinkDataImpl(const RefPtr<SymbolTable>& table);
    LinkDataImpl(addr_t addr, const char*);

    virtual ~LinkDataImpl() throw();

    SharedString* filename() const { return name_.get(); }
    void set_filename(const RefPtr<SharedString>& name)
    { name_ = name; }

    addr_t addr() const { return addr_; }

    off_t adjustment() const { return adjust_; }

    SymbolMap::LinkData* next() const { return next_.get(); }

    void set_next(RefPtr<LinkDataImpl>);

    SymbolTable* table() const { return table_.get(); }

private:
    RefPtr<LinkDataImpl> next_;

    RefPtr<SymbolTable> table_;

    addr_t addr_;

    off_t adjust_;

    RefPtr<SharedString> name_;
};

#endif // LINK_DATA_H__3A891A11_032E_464C_8737_7B39E7640AD3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
