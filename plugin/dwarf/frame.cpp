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
#include <iostream>
#include <stdexcept>
#include "frame.h"



Symbol* Dwarf::FrameImpl::function(Symbol*) const
{
#if DEBUG
    std::cout << "WARNING: null function symbol\n";
#endif
    return 0;
}


void Dwarf::FrameImpl::set_user_object(const char* key, ZObject* obj)
{
/*
    if (!key || strcmp(key, "inlined") != 0)
    {
        throw std::logic_error("unsupported user object");
    }
    userObj_ = obj;
*/
}


ZObject* Dwarf::FrameImpl::get_user_object(const char* key) const
{
/*
    if (!key || strcmp(key, "inlined") != 0)
    {
        throw std::logic_error("unsupported user object");
    }
    return userObj_.get();
*/
    return NULL;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
