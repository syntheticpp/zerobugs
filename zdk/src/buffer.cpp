// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <stdexcept>
#include <string>
#include "zdk/buffer_impl.h"



BufferImpl::BufferImpl()
{
}


BufferImpl::BufferImpl(size_t size) : data_(size)
{
}



BufferImpl::~BufferImpl() throw()
{
}


void BufferImpl::resize(size_t n)
{
    data_.resize(n);
}


size_t BufferImpl::size() const
{
    return data_.size();
}


const char* BufferImpl::data() const
{
    return &data_[0];
}


void BufferImpl::put(const void* data, size_t count, size_t offs)
{
// todo: should automatically resize?
    if (offs + count > data_.size())
    {
        throw std::out_of_range("Buffer::put");
    }
    memcpy(&data_[offs], data, count);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
