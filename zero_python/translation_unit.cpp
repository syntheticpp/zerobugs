//
// $Id: translation_unit.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <boost/bind.hpp>
#include <boost/python.hpp>
#include "zdk/get_pointer.h"
#include "zdk/translation_unit.h"
#include "zdk/zero.h"
#include "lang.h"
#include "translation_unit.h"


using namespace std;
using namespace boost;
using namespace boost::python;


static Language language(const TranslationUnit* unit)
{
    return static_cast<Language>(unit->language());
}

static const char* declared_module(const TranslationUnit* unit)
{
    if (SharedString* mod = unit->declared_module())
    {
        return mod->c_str();
    }
    return "";
}


void export_translation_unit()
{
scope inUnit =
    class_<TranslationUnit, bases<>, boost::noncopyable>("TranslationUnit", no_init)
        .def("filename", &TranslationUnit::filename)
        .def("lower_addr", &TranslationUnit::lower_addr)
        .def("upper_addr", &TranslationUnit::upper_addr)
        .def("language", language)
        .def("producer", &TranslationUnit::producer)
        .def("module", declared_module)
        ;

    enum_<Language>("Language")
        .value("C89", LANG_C89)
        .value("C", LANG_C)
        .value("Ada83", LANG_Ada83)
        .value("C_plus_plus", LANG_C_plus_plus)
        .value("Cobol74", LANG_Cobol74)
        .value("Cobol85", LANG_Cobol85)
        .value("Fortran77", LANG_Fortran77)
        .value("Fortran90", LANG_Fortran90)
        .value("Pascal83", LANG_Pascal83)
        .value("Modula2", LANG_Modula2)
        .value("Java", LANG_Java)
        .value("C99", LANG_C99)
        .value("Ada95", LANG_Ada95)
        .value("Fortran95", LANG_Fortran95)
        .value("PLI", LANG_PLI)
        .value("ObjC", LANG_ObjC)
        .value("ObjC_plus_plus", LANG_ObjC_plus_plus)
        .value("UPC", LANG_UPC)
        .value("D", LANG_D)
        .value("lo_user", LANG_lo_user)
        .value("Assembler", LANG_Mips_Assembler)
        .value("Upc", LANG_Upc)
        .value("ALTIUM_Assembler", LANG_ALTIUM_Assembler)
        ;

    register_ptr_to_python<RefPtr<TranslationUnit> >();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
