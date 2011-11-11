//
// $Id: token.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cassert>
#include <cctype>
#include <cstring>


static inline Token
get_token(const char*& begin, const char*& end)
{
    if (isdigit(*begin))
    {
        return T_NUMBER;
    }
    register const char* p = begin + 1;

    if (end > p)
    {
        if (isdigit(*p))
        {
            switch (*begin)
            {
            case '-':
                return T_NUMBER;
            case 'u':
            case 's':
                if (*(begin - 1) == '=')
                {
                    end = begin + 1;
                    return (*begin == 's' ? T_STRUCT : T_UNION);
                }
                break;
            }
        }
        else if (*begin == '@')
        {
            return T_TYPE_ATTR;
        }
        else if (*begin == T_ENUM && (begin[-1] == '=' || begin[-1] == ';'))
        {
            p = end + 1;

            if ((*end == ':') && (isdigit(*p) || *p == '-'))
            {
                end = begin + 1;
                return T_ENUM;
            }
        }
    }
    return T_IDENT;
}


/**
 * Extract type descriptor token
 */
static inline Token
stab_descriptor(const char*& begin, const char*& end)
{
    switch (*begin)
    {
    case T_TYPE_TAG:
        if ((end == begin + 2) && (begin[1] == T_TYPE_DEF))
        {
            return T_TYPE_DEF; // Tt is okay
        }
        // fallthru

    case T_PARAM:

    case T_FUN:
    case T_MEM_FUN:
    case T_POINTER:
    case T_REFERENCE:
    case T_RANGE:
    case T_TYPE_DEF:

    case T_CONST_QUAL: // const qualifier (Sun)
    case T_VOLATILE_QUAL: // volatile qualifier (Sun)

        if (end == begin + 1)
        {
            return static_cast<Token>(*begin);
        }
        break;

    case 'a':
        if ((end > begin) && (end - begin == 2) && (begin[1] == 'r'))
        {
            return T_ARRAY;
        }
        break;

    case T_MEM_TYPE:
        // TODO: I should also check for digit, if I
        // ever want to support non-tuple type numbers
        if (/* begin[1] == '-' || */ begin[1] == '(')
        {
            return T_MEM_TYPE;
        }
        break;
    }

    return get_token(begin, end);
}


static inline bool is_operator(const char* ptr)
{
    // todo: use __WORDSIZE instead of cpu
#if (__i386__) || (__PPC__) // 32-bit?

    union keyword { char c[4]; int i; };

    static const keyword lo = { { 'o', 'p', 'e', 'r' } };
    static const keyword hi = { { 'a', 't', 'o', 'r' } };

    bool result = *(long*)(ptr) == lo.i && *(long*)(ptr + 4) == hi.i;

#elif (__x86_64__) || (__PPC64__)

    union keyword { char c[8]; long x; };

    static const keyword k = { { 'o', 'p', 'e', 'r', 'a', 't', 'o', 'r' } };

    bool result = *(long*)(ptr) == k.x;
#endif

    return result;
}



inline Token
next_token(const char*& begin, const char*& end)
{
    begin = end;
    assert(*begin != '\\');

    if ((*begin == '-') && isdigit(begin[1]))
    {
        for (end = begin + 1; isdigit(*end); ++end)
            ;
        return T_NUMBER;
    }

    for (const char* ptr = begin; *ptr; ++ptr)
    {
        switch (*ptr)
        {
        case '<':
            if ((ptr[1] == '<' || ptr[1] == ':') && ptr[2] == ':')
            {
                // operator<< or operator<
                break;
            }

            ++ptr;
            // skip C++ template brackets
            for (int n = 1; *ptr && ptr != end && n; ++ptr)
            {
                if (*ptr == '<')
                {
                    ++n;
                }
                else if (*ptr == '>')
                {
                    assert(n);
                    --n;
                }
            }
            --ptr;
            break;

        case ':':
            if (*(ptr + 1) == ':')
            {
                ++ptr;
                continue; // TODO: report ::: as T_ERROR?
            }
            // fallthru

DELIMITER:
        case '!':
        case ';':
        case '=':
        case '*':
        case '&':
        case '#':
            if (ptr == begin)
            {
                end = ptr + 1;
                return static_cast<Token>(*ptr);
            }

            if (*ptr != ':' && is_operator(begin))
            {
                continue;
            }

            end = ptr;
            return get_token(begin, end);

        case ',' :
            // enclosed in paranthesis?
            if (*begin != '(')
            {
                goto DELIMITER;
            }
            break;

        case '(':
            if (ptr != begin && *(ptr + 1) != ')')
            {
                end = ptr;
                return stab_descriptor(begin, end);
            }
            break;

        case ')':
            if ((*begin == '(') && (ptr > begin + 1))
            {
                end = ptr + 1;
                return T_TYPE_KEY;
            }
            break;

        case 0:
            end = ptr;
            break;

        case T_BUILTIN_FP:
            if (isdigit(ptr[1]) && ptr[2] == ';')
            {
                end = ptr + 1;
                return T_BUILTIN_FP;
            }
            break;

        default:
            if (strncmp(ptr, "decltype", 8) == 0)
            {
                ptr += 8;
                break;
            }
            // if (ptr > begin)
            {
                // transition from digit to non-digit?
                if (isdigit(*begin) && !isdigit(*ptr))
                {
                    end = ptr;
                    return get_token(begin, end);
                }
            }
            break;
        }
    }

    return T_END; // all chars exhausted
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
