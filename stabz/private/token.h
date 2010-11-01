#ifndef TOKEN_H__35E3A060_F084_4C0D_98A0_99620501BDD1
#define TOKEN_H__35E3A060_F084_4C0D_98A0_99620501BDD1
//
// $Id: token.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

enum Token
{
    T_ERROR         = -1,
    T_END           = 0,

    T_MEM_TYPE      = '@',
    T_INHERITANCE   = '!',
    T_PERCENT       = '%',
    T_COMMA         = ',',
    T_COLON         = ':',
    T_SEMICOLON     = ';',
    T_POINTER       = '*',
    T_REFERENCE     = '&',
    T_MEM_FUN       = '#',
    T_EQUALS        = '=',

    T_ENUM          = 'e',
    T_FUN           = 'f',
    T_CONST_QUAL    = 'k',
    T_PARAM         = 'p',
    T_RANGE         = 'r',
    T_STRUCT        = 's',
    T_UNION         = 'u',

    T_TYPE_DEF      = 't',
    T_TYPE_TAG      = 'T',

    T_VOLATILE_QUAL = 'B',
    T_BUILTIN_FP    = 'R',

//  T_IGNORE        = 256,
    T_IDENT         = 257,
    T_TYPE_KEY      = 258,
    T_ARRAY         = 259,
    T_NUMBER        = 260,

    T_TYPE_ATTR     = 261,
};


/*  Extracts next token and moves pointers. */
// Token next_token(const char*& begin, const char*& end);

#include "token.cpp"
#endif // TOKEN_H__35E3A060_F084_4C0D_98A0_99620501BDD1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
