#ifndef UTILITY_H__32410679_5F5A_4A22_9FBA_3F9A55C77D85
#define UTILITY_H__32410679_5F5A_4A22_9FBA_3F9A55C77D85
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <errno.h>
#include <iostream>


template<typename T>
inline std::istream&
extract(std::istream& in, T& val, const char* name, size_t line)
{
    if (!(in >> val))
    {
        std::cerr << __FILE__ << ":" << line << ": Error reading ";
        std::cerr << name << " errno=" << errno << std::endl;
    }
    return in;
}
#endif // UTILITY_H__32410679_5F5A_4A22_9FBA_3F9A55C77D85
