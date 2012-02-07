#ifndef DEBUG_INFO_ENTRY_H__8E387EEA_1754_4AF5_A2F9_1F99E450F5F3
#define DEBUG_INFO_ENTRY_H__8E387EEA_1754_4AF5_A2F9_1F99E450F5F3
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

#include <memory>
#include <string>
#include <libdwarf.h>
#include "access.h"
#include "interface.h"


namespace Dwarf
{
    class Debug;
    class Utils;

    /**
     * C++ wrapper for Dwarf_Die
     */
    CLASS Die
    {
        template<typename T> friend class Wrapper;

        friend class Utils;

        Die(const Die&);    // non-copyable
        const Die& operator=(const Die&);

    public:
        virtual ~Die() throw();

        const char* name() const;

        static std::string name(Dwarf_Debug, Dwarf_Die);

        Dwarf_Half get_tag() const;

        static Dwarf_Off offset(Dwarf_Debug, Dwarf_Die);

        Dwarf_Off offset() const { return offset(dbg(), die_); }

        Dwarf_Off cu_offset() const;

        bool is_artificial() const;

        const Debug& owner() const;
        Debug& owner();

        /// @todo document
        std::shared_ptr<Die> check_indirect(bool spec = true) const;

        /**
         * @return the die referenced by ithe DW_AT_import attribute
         */
        std::shared_ptr<Die> import() const;

        /**
         * @return the Dwarf_Debug handle
         */
        Dwarf_Debug dbg() const { return dbg_; }

        /**
         * @return the Dward_Die handle
         */
        Dwarf_Die die() const { return die_; }

        Access access() const;

        /**
         * @return line number where symbol is declared, or zero
         */
        size_t decl_line() const;

        /**
         * @return the index of the source filename where symbol
         * is declared, or zero
         */
        size_t decl_file() const;

    protected:
        /**
         * takes ownership of the Dwarf_Die
         */
        Die(Dwarf_Debug, Dwarf_Die);

        virtual char* name_impl() const;

        void set_name(const char*);

        static void set_name(Die& die, const char* name)
        {
            die.set_name(name);
        }

    private:
        Dwarf_Debug     dbg_;
        Dwarf_Die       die_;
        mutable char*   name_;

        // abstract_origin or specification
        mutable std::shared_ptr<Die> indirect_;
    };
};

#undef USING_ALLOC_POOL

#endif // DEBUG_INFO_ENTRY_H__8E387EEA_1754_4AF5_A2F9_1F99E450F5F3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
