#ifndef PREDICATES_H__7EECA098_4A6D_4420_8A2F_1098B58917A2
#define PREDICATES_H__7EECA098_4A6D_4420_8A2F_1098B58917A2
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
#include "zdk/platform.h"
#include "zdk/key_ptr.h"
#include "zdk/symbol.h"
#include "zdk/string.h"

using Platform::addr_t;

typedef RefPtr<Symbol> SymPtr;
typedef KeyPtr<Symbol, addr_t> SymAddrPtr;

namespace Predicate
{
    /**
     * Helper predicate, compares symbols by their addresses
     */
    struct ZDK_LOCAL CompareAddr
    {
        bool operator()(const SymAddrPtr& lhs, addr_t rhs) const
        {
            return lhs.key() < rhs;
        }
        bool operator()(addr_t lhs, const SymAddrPtr& rhs) const
        {
            return lhs < rhs.key();
        }
        bool operator()(const SymAddrPtr& lhs, const SymAddrPtr& rhs) const
        {
            return lhs.key() < rhs.key();
        }
    };


    template<bool IgnoreSpace>
    struct ZDK_LOCAL SharedStringLess
    {
        bool is_less(const SharedString* lname,
                     const SharedString* rname) const
        {
            //assert(lname);
            //assert(rname);

            if ((lname == rname) || (lname->hash() == rname->hash()))
            {
                return false;
            }

            const char* lstr = lname->c_str();
            return is_less(lstr, rname);
        }
        bool is_less(const SharedString* lname, const char* rstr) const
        {
            //assert(lname);

            const char* lstr = lname->c_str();

            if (IgnoreSpace)
            {
                return strcmp_ignore_space(lstr, rstr) < 0;
            }
            return strcmp(lstr, rstr) < 0;
        }
        bool is_less(const char* lstr, const SharedString* rname) const
        {
            //assert(rname);
            const char* rstr = rname->c_str();

            if (IgnoreSpace)
            {
                return strcmp_ignore_space(lstr, rstr) < 0;
            }
            return strcmp(lstr, rstr) < 0;
        }
    };


    template<bool IgnoreSpace>
    struct ZDK_LOCAL SharedStringEqual
    {
        bool is_equal(const SharedString* lname,
                      const SharedString* rname) const
        {
            if (lname == rname)
            {
                return true;
            }
            return lname ? lname->is_equal2(rname, IgnoreSpace) : false;
        }
        bool is_equal(const SharedString* lname, const char* rstr) const
        {
            return lname ? lname->is_equal(rstr, IgnoreSpace)
                         : (rstr == NULL);
        }
    };


    /**
     * Compare symbols by name
     */
    struct ZDK_LOCAL CompareName : private SharedStringLess<false>
    {
        /**
         * used for sorting
         */
        bool operator()(const SymPtr& lhs, const SymPtr& rhs) const
        {
            return is_less(lhs->name(), rhs->name());
        }
        /**
         * used for lookups
         */
        bool operator()(const SymPtr& lhs, const char* rhs) const
        {
            return is_less(lhs->name(), rhs);
        }
        bool operator()(const char* lhs, const SymPtr& rhs) const
        {
            return is_less(lhs, rhs->name());
        }
        /**
         * utility
         */
        static const char* symbol_name(const SymPtr& sym)
        {
            if (SharedString* name = sym->name())
            {
                return name->c_str();
            }
            return "";
        }
    };


    struct ZDK_LOCAL NameEqual : private SharedStringEqual<true>
    {
        bool operator()(const RefPtr<SharedString>& lhs,
                        const RefPtr<SharedString>& rhs) const
        {
            return is_equal(lhs.get(), rhs.get());
        }

        bool operator()(const RefPtr<SharedString>& lhs, const char* rhs) const
        {
            return is_equal(lhs.get(), rhs);
        }
        bool operator()(const char* lhs, const RefPtr<SharedString>& rhs) const
        {
            return is_equal(rhs.get(), lhs);
        }
    };

    struct ZDK_LOCAL CompareDemangled : private SharedStringLess<true>
    {
        /**
         * used for sorting
         */
        bool operator()(const SymPtr& lhs, const SymPtr& rhs) const
        {
            return is_less(lhs->demangled_name(false), rhs->demangled_name(false));
        }
        /**
         * used for lookups
         */
        bool operator()(const SymPtr& lhs, const char* rhs) const
        {
            return is_less(lhs->demangled_name(false), rhs);
        }
        bool operator()(const char* lhs, const SymPtr& rhs) const
        {
            return is_less(lhs, rhs->demangled_name(false));
        }
        /**
         * utility
         */
        static const char* symbol_name(const SymPtr& sym)
        {
            if (SharedString* name = sym->demangled_name(false))
            {
                return name->c_str();
            }
            return "";
        }
    };
}
#endif // PREDICATES_H__7EECA098_4A6D_4420_8A2F_1098B58917A2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
