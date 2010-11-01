//
// $Id: right_click_context.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "code_view.h"
#include "right_click_context.h"

typedef Platform::addr_t addr_t;


RightClickContext::RightClickContext(CodeView& view) : view_(view)
{
}


Thread* RightClickContext::thread() const
{
    return view_.thread().get();
}



size_t RightClickContext::position() const
{
    return view_.right_click().position();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
