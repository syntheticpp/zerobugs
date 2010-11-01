//
// $Id: block.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cassert>
#include <iostream>
#include "public/block.h"
#include "public/variable.h"


Stab::Block::Block(addr_t addr, size_t index)
    : beginAddr_(addr)
    , endAddr_(0)
    , beginIndex_(index)
    , endIndex_(0)
{
}


Stab::Block::Block(addr_t addr, size_t index, VarList& vars)
    : beginAddr_(addr)
    , endAddr_(0)
    , beginIndex_(index)
    , endIndex_(0)
{
    vars_.swap(vars);
/*
    VarList::const_iterator i = vars_.begin();
    for (; i != vars_.end(); ++i)
    {
        std::clog << __func__ << ": " << (*i)->name().c_str() << std::endl;
    }
 */
}


Stab::Block::~Block() throw()
{
}


addr_t Stab::Block::begin_addr() const
{
    return beginAddr_;
}


addr_t Stab::Block::end_addr() const
{
    return endAddr_;
}


void Stab::Block::set_end_addr(addr_t addr)
{
    assert(endAddr_ == 0);

    if (addr >= beginAddr_)
    {
        endAddr_ = addr;
    }
}


size_t Stab::Block::begin_index() const
{
    return beginIndex_;
}


size_t Stab::Block::end_index() const
{
    return endIndex_;
}


void Stab::Block::set_end_index(size_t index)
{
    assert(endIndex_ == 0);
    assert(beginIndex_ <= index);

    endIndex_ = index;
}


void Stab::Block::assign_variables(const VarList& vars)
{
    assert(vars_.empty());
    vars_ = vars;
/*
    // dump variables for debug purposes:
    VarList::const_iterator i = vars_.begin();
    for (; i != vars_.end(); ++i)
    {
        std::clog << __func__ << ": " << (*i)->name().c_str() << std::endl;
    }
 */
}


void Stab::Block::add_child(const RefPtr<Block>& block)
{
    assert(block.get() != this);
    blocks_.push_back(block);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
