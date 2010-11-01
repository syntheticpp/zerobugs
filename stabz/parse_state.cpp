// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: parse_state.cpp 714 2010-10-17 10:03:52Z root $
//
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include "private/parse_state.h"

using namespace std;


ParseState::ParseState(const string& name, const char*& pos)
    : name_(name)
    , begin_(pos)
    , end_(pos)
    , pos_(pos)
{
}


void ParseState::freeze()
{
    if (end_ == begin_)
    {
        end_ = pos_;
    }
}


ostream& ParseState::dump(ostream& outs) const
{
    outs << "  " << name_ << ": ";
    copy(begin_, end_, ostream_iterator<char>(outs));

    return outs;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
