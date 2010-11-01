// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: elf_header.cpp 714 2010-10-17 10:03:52Z root $
//
#include <iostream>
#include <boost/python.hpp>
#include <boost/type_traits.hpp>
#include "elfz/public/binary.h"
#include "elfz/public/headers.h"
#include "elf_header.h"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace ELF;


class Header : public ElfHdr
{
    shared_ptr<Binary> bin_;

public:
    Header(const ElfHdr& hdr, const shared_ptr<Binary>& bin)
        : ElfHdr(hdr)
        , bin_(bin)
    { }
};


Header get_elf_header(const char* file)
{
    shared_ptr<Binary> bin(new Binary(file));
//#if DEBUG
//    bin->header();
//    clog << __func__ << ": " << file << " ";
//    clog << elf_getident(bin->elf(), NULL) << endl;
//#endif
    return Header(bin->header(), bin);
}


enum Klass
{
   CLASS_NONE = ELFCLASSNONE,
   CLASS_32 = ELFCLASS32,
   CLASS_64 = ELFCLASS64
};

template<typename R, R (ElfHdr::*F)() const>
struct Convert
{
    union E
    {
        enum
        {
            e_max = UINT_MAX
        } e_;
        R r_;
        E(R r) : r_(r) {}
        operator long() const { return r_; }
    };
    typedef E result;
    static E fun(const Header& obj) { return static_cast<E>((obj.*F)()); }
};


void export_elf_header()
{
    def("get_elf_header", get_elf_header);

scope in_header =
    class_<Header>("Elf_Hdr", no_init)
        .def("klass", &Convert<Elf_Class, &ElfHdr::klass>::fun)
        .def("machine", &Convert<ElfW(Half), &ElfHdr::machine>::fun)
        .def("type", &Convert<ElfW(Half), &ElfHdr::type>::fun)
        ;

    enum_<Convert<Elf_Class, &ElfHdr::klass>::result>("Class")
        .value("None", CLASS_NONE)
        .value("32-bit", CLASS_32)
        .value("64-bit", CLASS_64)
        ;

    enum_<Convert<ElfW(Half), &ElfHdr::type>::result>("Type")
        .value("None", ET_NONE)
        .value("Relocatable", ET_REL)
        .value("Executable", ET_EXEC)
        .value("SharedObject", ET_DYN)
        .value("Core", ET_CORE)
        ;

    enum_<Convert<ElfW(Half), &ElfHdr::machine>::result>("Machine")
        .value("None", EM_NONE)
        .value("i386", EM_386)
        .value("x86-64", EM_X86_64)
        ;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
