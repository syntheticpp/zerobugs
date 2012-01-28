#ifndef UNIT_H__637B7F5C_A58A_46E8_AC58_EA42DA9E3398
#define UNIT_H__637B7F5C_A58A_46E8_AC58_EA42DA9E3398
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
#include "zdk/translation_unit.h"
#include "zdk/zobject_impl.h"


namespace Dwarf
{
    class CompileUnit;
    class Debug;

    /**
     * Adapts Dwarf::CompileUnit objects into a common
     * TranslationUnit interface.
     */
    class ZDK_LOCAL Unit : public ZObjectImpl<TranslationUnit>
    {
    public:
    BEGIN_INTERFACE_MAP(Unit)
        INTERFACE_ENTRY(TranslationUnit)
    END_INTERFACE_MAP()

        Unit(const boost::shared_ptr<Debug>&,
             const boost::shared_ptr<CompileUnit>&,
             SharedString* moduleName);

        ~Unit() throw();

        const char* filename() const;

        Platform::addr_t lower_addr() const;

        Platform::addr_t upper_addr() const;

        const char* producer() const;

        size_t enum_ns(const char*, EnumCallback<const char*>*) const;

        int language() const;

        size_t enum_sources(EnumCallback<SharedString*, bool>*);

        SharedString* module_name() const;
        SharedString* declared_module() const;

        //size_t line_to_addr(size_t, EnumCallback<addr_t>*);

    private:
        boost::shared_ptr<Debug> dbg_; // keep alive
        boost::shared_ptr<CompileUnit> unit_;

        RefPtr<SharedString> moduleName_;
    };
}

#endif // UNIT_H__637B7F5C_A58A_46E8_AC58_EA42DA9E3398
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
