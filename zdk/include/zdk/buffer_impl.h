#ifndef BUFFER_IMPL_H__5335ABC3_23EA_4379_A599_A565C1573A3F
#define BUFFER_IMPL_H__5335ABC3_23EA_4379_A599_A565C1573A3F
//
// $Id: buffer_impl.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "buffer.h"
#include "zdk/zobject_impl.h"


class ZDK_LOCAL BufferImpl : public ZObjectImpl<Buffer>
{
public:
    BufferImpl();

    explicit BufferImpl(size_t);

    virtual ~BufferImpl() throw();

    void resize(size_t);

    size_t size() const;

    const char* data() const;

    void put(const void*, size_t, size_t = 0);

private:
    std::vector<char> data_;
};
#endif // BUFFER_IMPL_H__5335ABC3_23EA_4379_A599_A565C1573A3F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
