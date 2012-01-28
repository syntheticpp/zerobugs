#ifndef UNMANGLE_H__B2F5CA16_A0C9_4B4D_87F2_694D2BE7D531
#define UNMANGLE_H__B2F5CA16_A0C9_4B4D_87F2_694D2BE7D531
/*
 * $Id$
 */
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#define ANONYMOUS_NS_PREFIX "(anonymous namespace)"


enum UnmangleStatus
{
    UNMANGLE_STATUS_SUCCESS       = 0,
    UNMANGLE_STATUS_BAD_ALLOC     = -1,
    UNMANGLE_STATUS_INVALID_NAME  = -2,
    UNMANGLE_STATUS_INVALID_ARGS  = -3,
    UNMANGLE_STATUS_EXCEPTION     = -4,
    UNMANGLE_STATUS_UNKNOWN_EXCEPTION = -5,
};
enum
{
    UNMANGLE_DEFAULT        = 0,
    UNMANGLE_NOFUNARGS      = 1,
};

#ifdef __cplusplus
 extern "C"
#endif
/**
 * C API
 * @param mangledName input string to demangle
 * @param size may optionally contain the length of the string to
 *  demangle, is filled out upon return with the length of the
 *  demangled string
 * @param returns the operation's status, see UnmangleStatus
 *  (may be NULL)
 * @param optionFlags
 *
 * @note caller MUST free returned string
 */
char* unmangle( const char* mangledName,
                size_t*     size,
                int*        status,
                int         optionFlags);

#ifdef __cplusplus
/**
 * C++ API
 */
#include <string>


std::string inline
cplus_unmangle(const std::string& mangled, int opts = UNMANGLE_DEFAULT)
{
    std::string result;
    size_t len = mangled.length();
    if (char* str = unmangle(mangled.c_str(), &len, 0, opts))
    {
        assert(strlen(str) == len);
        result.assign(str, str + len);
        free(str);
    }
    else
    {
        result = mangled;
    }
    return result;
}


std::string inline
cplus_unmangle(const char* mangled, int opts = UNMANGLE_DEFAULT)
{
    std::string result;
    size_t len = 0;
    if (char* str = unmangle(mangled, &len, 0, opts))
    {
        assert(strlen(str) == len);
        result.assign(str, str + len);
        free(str);
    }
    else
    {
        result = mangled;
    }
    return result;
}


void cplus_unmangle_d();

#endif // C++
#endif /* UNMANGLE_H__B2F5CA16_A0C9_4B4D_87F2_694D2BE7D531 */
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
