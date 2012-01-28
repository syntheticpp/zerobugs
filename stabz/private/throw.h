#ifndef THROW_H__800047D6_9653_4CC0_BDDD_70D343A03D40
#define THROW_H__800047D6_9653_4CC0_BDDD_70D343A03D40
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

#if STABZ_NO_THROW
 #include <cassert>
 #include <iostream>

 #define THROW(x) std::clog << (x).what() << std::endl
#if DEBUG
 assert(false);
#endif

#else
 #define THROW(x) throw((x))
#endif
#endif // THROW_H__800047D6_9653_4CC0_BDDD_70D343A03D40
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
