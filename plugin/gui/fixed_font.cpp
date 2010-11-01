// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: fixed_font.cpp 714 2010-10-17 10:03:52Z root $
//
#include "dharma/environ.h"
#include "fixed_font.h"


std::string fixed_font()
{
    static const std::string ff =
        env::get_string("ZERO_FIXED_FONT", "fixed");

    return ff;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
