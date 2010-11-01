#ifndef ZOBJECT_ADAPT_H__900D8B3B_165F_4A70_9A42_FE95452CD8C7
#define ZOBJECT_ADAPT_H__900D8B3B_165F_4A70_9A42_FE95452CD8C7
//
// $Id: zobject_adapt.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zobject_impl.h"


class ZObjectAdapter : public ZObjectImpl<>
{
public:

BEGIN_INTERFACE_MAP(ZObjectAdapter)
    INTERFACE_ENTRY(ZObject)
    INTERFACE_ENTRY_DELEGATE(unk_)
END_INTERFACE_MAP()

    explicit ZObjectAdapter(Unknown2* unk) : unk_(unk)
    { }

    virtual ~ZObjectAdapter() throw ()
    { }

    void clear() { unk_ = 0; }

private:
    Unknown2* unk_;
};


#endif // ZOBJECT_ADAPT_H__900D8B3B_165F_4A70_9A42_FE95452CD8C7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
