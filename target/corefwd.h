#ifndef COREFWD_H__BB940A22_5874_11DA_A633_000C29CB02FA
#define COREFWD_H__BB940A22_5874_11DA_A633_000C29CB02FA
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

#include <boost/shared_ptr.hpp>

namespace ELF
{
    class CoreFile;
}

typedef boost::shared_ptr<ELF::CoreFile> CorePtr;


#endif // COREFWD_H__BB940A22_5874_11DA_A633_000C29CB02FA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
