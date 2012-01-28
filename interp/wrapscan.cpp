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
#if __GNUC__ == 2
    // Hack around bug in flex 2.5.4
    // The skeleton includes the forward decl: class istream;
    // instead of compiling scan.cpp directly, wrap it here.
    // This takes advantage of the fact that the only time the
    // `class' keyword is used is in the buggy forward decl.
    //
    #include <iostream>
    #include <FlexLexer.h>

    #define class using std::

#endif // __GNUC__ == 2

#include "scan.cpp"

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
