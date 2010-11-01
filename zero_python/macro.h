#ifndef MACRO_H__6A9BB131_EAAD_4EE8_B91C_2E01B487D6F3
#define MACRO_H__6A9BB131_EAAD_4EE8_B91C_2E01B487D6F3
//
// $Id: macro.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <boost/python.hpp>
#include <string>
#include "zdk/command.h"
#include "zdk/properties.h"
#include "zdk/zobject_impl.h"

/**
 * Models a user-defined command
 */
class Macro : public ZObjectImpl<DebuggerCommand>
{
public:
    Macro(const char* name, PyObject*, PyObject*);
    virtual ~Macro() throw();

BEGIN_INTERFACE_MAP(Macro)
    INTERFACE_ENTRY(DebuggerCommand)
    INTERFACE_ENTRY_DELEGATE(prop_)
END_INTERFACE_MAP()

    virtual const char* name() const;
    virtual const char* help() const;

    virtual void auto_complete(
        const char*,
        EnumCallback<const char*>*) const;

    virtual bool execute(
        Thread*,
        const char* const* argv, // NULL-ended list of args
        Unknown2* = 0);

private:
    std::string name_;
    PyObject* callable_;
    RefPtr<Properties> prop_;
    boost::python::dict dict_;
};


void export_macro();


#endif // MACRO_H__6A9BB131_EAAD_4EE8_B91C_2E01B487D6F3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
