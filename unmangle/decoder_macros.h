#ifndef DECODER_MACROS_H__0FCBF0C0_3E64_44C5_9B3E_D98E525B7E34
#define DECODER_MACROS_H__0FCBF0C0_3E64_44C5_9B3E_D98E525B7E34
//
// $Id: decoder_macros.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if defined(DEBUG_UNMANGLE)
 #define DECODER_STR(x) #x
 #if defined (CXXFILT)
  #define SHOW_ERROR_POSITION(x)
 #else
 // in debug mode, show the failed production and the location
 // in the input string where the error has occurred
  #define SHOW_ERROR_POSITION(x)                                    \
    std::clog << '(' << __FILE__ << ":" << __LINE__ << ") ";        \
    std::clog << __func__ << ": " << DECODER_STR(x) << std::endl;   \
    std::clog << name_ << std::endl;                                \
    std::clog << std::string(pos_, ' ') << "^\n";
 #endif
 #define DECODER_ERROR__(x) do {                                    \
    status_ = UNMANGLE_STATUS_##x;                                  \
    parse_error();                                                  \
    SHOW_ERROR_POSITION(x); } while(0)
 #define TODO std::clog << "***: " << name_ << std::endl;           \
    NUTS_ASSERT(!"not implemented"[0])
#else
 #define DECODER_ERROR__(x) status_ = UNMANGLE_STATUS_##x
 #define TODO do { status_ = UNMANGLE_STATUS_INVALID_NAME; } while(0)
#endif

#define BEGINS_WITH(cs) begins_with(cs, sizeof(cs)-1)

#endif // DECODER_MACROS_H__0FCBF0C0_3E64_44C5_9B3E_D98E525B7E34
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
