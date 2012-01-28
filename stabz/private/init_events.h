#ifndef INIT_EVENTS_H__0254E186_C1D9_4A11_ACD3_FF53FA103359
#define INIT_EVENTS_H__0254E186_C1D9_4A11_ACD3_FF53FA103359
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

#include <map>
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "public/descriptor.h"


namespace Stab
{
    class Function;


    /**
     * Scan the stabs entries for compile-units, functions and
     * source-line info. This functor is designed to work with
     * Stab::Descriptor::for_each_stab.
     * @see ParseEvents
     * @note the difference between "initializing" and "parsing"
     * is that the former gathers from the stabs only enough
     * information to locate compile-units and do line-to-address
     * and address-to-line lookups; the parse process goes into
     * further detail and looks at variables, function parameters,
     * etc., and is therefore slower.
     * @note ParseEvents assumes InitEvents were applied first
     * @note several Stab::Events functors can be combined together
     * and applied in the same for_each_stab pass (which is exactly
     * what Stab::Descriptor::init_and_parse does).
     */
    CLASS InitEvents : public Events
    {
    public:
        explicit InitEvents(Descriptor&);

        ~InitEvents() throw();

    private:
        //
        // Events interface
        //
        void on_section(const char*);
        void on_begin(SharedString&, const char*, size_t);
        bool on_stab(size_t, const stab_t&, const char*, size_t);
        void on_done(size_t) {};

        //
        // Internal methods called by on_stab()
        //
        void on_source(size_t, const stab_t&, const char*, size_t);
        void on_func(size_t, const stab_t&, const char*, size_t);
        void on_sline(size_t, const stab_t&, const char*, size_t);
        void on_sol(size_t, const stab_t&, const char*, size_t);

        /**
         * Get a string from the string pool if found, otherwise
         * create a new one and place it in the pool. Strings are
         * indexed by the stab.strindex
         */
        RefPtr<SharedString> string_from_pool(
            size_t index, const char* str, size_t length);

        void new_compile_unit(const stab_t&, const char*, size_t);
        void finish_compile_unit(const stab_t&, size_t);

    private:
        Descriptor&             desc_;
        RefPtr<SharedString>    section_;   // current section's name
        RefPtr<SharedString>    buildPath_; // current build path
        RefPtr<SharedString>    source_;    // current source name
        RefPtr<CompileUnit>     unit_;      // current unit
        RefPtr<Function>        func_;      // current function

        RefPtr<SharedString>    producer_;  // compiler version info

        // Duplicate strings share the same strindex in the
        // stabs section; "indexing" the shared strings here
        // reduces memory usage, at the cost of a map lookup
        typedef ext::hash_map<size_t, RefPtr<SharedString> > StringPool;

        StringPool stringPool_;
    };
}

#endif // INIT_EVENTS_H__0254E186_C1D9_4A11_ACD3_FF53FA103359
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
