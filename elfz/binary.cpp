//
// $Id: binary.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <errno.h>
#include <fcntl.h>  // O_RDONLY
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include "dharma/canonical_path.h"
#include "dharma/environ.h"
#include "dharma/path.h"
#include "dharma/system_error.h"
#include "public/binary.h"
#include "public/error.h"
#include "public/headers.h"
#include "public/link.h"
#include "public/program_header.h"
#include "public/section.h"


#ifndef __GNUC__
 #define __PRETTY_FUNCTION__ __func__
#endif

using namespace std;
using namespace ELF;

typedef boost::shared_ptr<Binary> BinaryPtr;


Binary::Binary(const char* filename)
    : File()
    , List<Binary>(*this)
    , List<ProgramHeader>(*this)
    , List<Section>(*this)
    , List<SymbolTable>(*this)
    , filename_(filename ? filename : "")
    , pltAddr_(0)
    , pltSize_(0)
    , abi_(0)
    , abiVersion_(0)
{
    assert(elf() == 0);
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        throw runtime_error("libelf is out of date");
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        assert(filename);
        throw SystemError(filename);
    }
    set_file_handle(fd);

    if (Elf* elf = elf_begin(fd, ELF_C_READ, 0))
    {
        set_elf(elf);
    }
    else
    {
        throw Error("elf_begin");
    }
}


Binary::Binary(Elf* elf) : File(elf)
    , List<Binary>(*this)
    , List<ProgramHeader>(*this)
    , List<Section>(*this)
    , List<SymbolTable>(*this)
    , pltAddr_(0)
    , pltSize_(0)
    , abi_(0)
    , abiVersion_(0)
{
    assert(this->elf() == elf);
}


Binary::~Binary()
{
    if (file_handle() != -1)
    {
        close(file_handle());
    }
}


bool Binary::check_format() const
{
    static const union
    {
        char c_[4];
        int32_t i_;
    }
    magic = { { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 } };

    const char* ident = elf_getident(const_cast<Elf*>(elf()), 0);
    bool result = ( ident && *(int32_t*)(ident) == magic.i_ );

    assert (result == (kind() != ELF_K_NONE));
    return result;
}


Binary::Header Binary::header() const
{
    return ElfHdr(const_cast<Elf*>(elf()));
}


bool Binary::is_null() const
{
    return !elf() || !check_format();
}


/**
 * Look for the first ELF program header of the PT_LOAD type.
 * Return its vaddr and optionally its memory size.
 *
 * @todo can we have more that one PT_LOAD header per executable?
 */
ElfW(Addr) Binary::get_load_addr(ElfW(Xword)* memsz) const
{
    ElfW(Addr) result = 0;

    List<ProgramHeader>::const_iterator i = program_headers().begin();

    for (; i != program_headers().end(); ++i)
    {
        if ((*i).type() == PT_LOAD)
        {
            result = (*i).vaddr();
            if (memsz)
            {
                *memsz = (*i).memsz();
            }
            break;
        }
    }
    return result;
}


const string& Binary::dynamic_linker() const
{
    if (interp_.empty())
    {
        const List<ProgramHeader>& hdrs = program_headers();
        List<ProgramHeader>::const_iterator i = hdrs.begin(),
            end = hdrs.end();
        for (; i != end; ++i)
        {
            if (i->type() == PT_INTERP)
            {
                char buf[PATH_MAX];
                memset(buf, 0, sizeof buf);
                try
                {
                    size_t n = min<size_t>(sizeof buf - 1, i->filesz());
                    readbuf(i->offset(), buf, n);
                }
                catch (...)
                {
                }
                if (buf[0])
                {
                    interp_ = canonical_path(buf);
                }
                break;
            }
        }
    }
    return interp_;
}


BinaryPtr Binary::next(File* arch) const
{
    assert(arch);

    BinaryPtr result;

    if (Elf* obj = const_cast<Elf*>(elf()))
    {
        Elf_Cmd cmd = elf_next(obj);
        obj = elf_begin(arch->file_handle(), cmd, arch->elf());
        if (obj)
        {
    #if defined(__GNUC__) && (__GNUC__ < 3)
            // hack to prevent g++ 2.95 from crashing, when
            // using -O2 -fsjlj-exceptions
            BinaryPtr(new Binary(obj)).swap(result);
    #else
            result.reset(new Binary(obj));
    #endif
        }
    }
    return result;
}


BinaryPtr IterationTraits<Binary>::first(File& arch)
{
    Elf* obj = elf_begin(arch.file_handle(), ELF_C_READ, arch.elf());
    return BinaryPtr(new Binary(obj));
}


BinaryPtr IterationTraits<Binary>::next(File* arch, const Binary& elf)
{
    assert(arch); return elf.next(arch);
}


template<typename T>
ElfW(Addr)
linker_debug_struct_addr(const Elf_Data& d, ElfW(Addr) addr)
{
    const size_t count = d.d_size / sizeof (T);

    for (size_t n = 0; n != count; ++n)
    {
        const T& dyn = reinterpret_cast<T*>(d.d_buf)[n];

        if (dyn.d_tag == DT_DEBUG)
        {
            return addr + ((char*)&dyn.d_un.d_ptr - (char*)d.d_buf);
        }
    }
    return 0;
}


/**
 * @note -1 is returned for binaries that do not
 * have a .dynamic section
 */
ElfW(Addr)
ELF::get_linker_debug_struct_addr(const ELF::Binary& bin)
{
    ElfW(Addr) addr = 0;
    bool haveDynamicSection = false;

    const Elf_Class klass = bin.header().klass();

    ELF::List<ELF::Section>::const_iterator i = bin.sections().begin();
    ELF::List<ELF::Section>::const_iterator end = bin.sections().end();

    for (; (i != end) && (addr == 0); ++i)
    {
        const ELF::Section::Header hdr = i->header();

        if (hdr.type() == SHT_DYNAMIC)
        {
            haveDynamicSection = true;
            assert(strcmp(hdr.name(), ".dynamic") == 0);

            const Elf_Data& d = i->data();

            switch (klass)
            {
            case ELFCLASS32:
                addr = linker_debug_struct_addr<Elf32_Dyn>(d, hdr.addr());
                break;

            case ELFCLASS64:
                addr = linker_debug_struct_addr<Elf64_Dyn>(d, hdr.addr());
                break;

            default:
                assert(false);
            }
            if (addr < hdr.addr())
            {
                addr = 0;
            }
        }
    }
    return haveDynamicSection ? addr : ElfW(Addr)(-1);
}


template<typename T>
static void _get_plt_addr(const Elf_Data& d, ElfW(Addr)& pltAddr, size_t& pltSize)
{
    const size_t count = d.d_size / sizeof(T);

    for (size_t n = 0; n != count; ++n)
    {
        const T& dyn = reinterpret_cast<T*>(d.d_buf)[n];

        switch (dyn.d_tag)
        {
        case DT_JMPREL: // address of PLT relocation entries
        //case DT_PLTGOT: // addr of the first entry in the global offset table
            assert(pltAddr == 0);
            pltAddr = dyn.d_un.d_ptr;
            break;

        case DT_PLTRELSZ: // Size in bytes of PLT relocs
            assert(pltSize == 0);
            pltSize = dyn.d_un.d_val;
            break;

        default:
            break;
        }
    }
}


ElfW(Addr) Binary::get_plt_addr(size_t* size) const
{
    if (!pltAddr_)
    {
        assert(!pltSize_);

        const Elf_Class klass = header().klass();

        ELF::List<ELF::Section>::const_iterator i = sections().begin();
        ELF::List<ELF::Section>::const_iterator end = sections().end();

        for (; i != end; ++i)
        {
            if (i->header().type() == SHT_DYNAMIC)
            {
                const Elf_Data& d = i->data();

                switch (klass)
                {
                case ELFCLASS32:
                    _get_plt_addr<Elf32_Dyn>(d, pltAddr_, pltSize_);
                    break;

                case ELFCLASS64:
                    _get_plt_addr<Elf64_Dyn>(d, pltAddr_, pltSize_);
                    break;

                default:
                    assert(false);
                    break;
                }
            }
        }
    }
    if (size)
    {
        *size = pltSize_;
    }
    return pltAddr_;
}


uint8_t Binary::abi() const
{
    if (abi_ == 0)
    {
        if (const char* ident = elf_getident(elf_, 0))
        {
            abi_ = ident[EI_OSABI];
            abiVersion_ = ident[EI_ABIVERSION];
        }
    }
    return abi_;
}


uint8_t Binary::abi_version() const
{
    abi();
    return abiVersion_;
}


static bool
file_exists(const string& dir, const char* file, string& path)
{
    path = dir;
    if (!path.empty() && path[path.size()] != '/')
    {
        path += '/';
    }
    path += file;

    struct stat statinfo = { 0 };
    while (::stat(path.c_str(), &statinfo) < 0)
    {
        if (errno != EINTR)
        {
            //SystemError e(__func__);
            //clog << __func__ << ": " << e.what() << endl;
            //throw e;
            return false;
        }
    }
    return true;
}


/**
 * Find the path to the file that contains external debug info.
 * Return empty string if not found.
 *
 * Such a file can be created with the following script:
 *
 * #! /bin/bash
 * DBGFILE=$1.dbg
 * if objcopy --only-keep-debug $1 $DBGFILE; then
 *	strip $1 # or strip -d $1
 *	objcopy --add-gnu-debuglink=$DBGFILE $1
 * fi
 */
string
ELF::debug_link(const Binary& bin, size_t* /* CRC, not implemented */)
{
#if __WORDSIZE == 64
    static const char defaultDir[] = "/usr/lib64/debug";
#else
    static const char defaultDir[] = "/usr/lib/debug";
#endif
    const string globalDir =
        env::get_string("ZERO_DEBUG_SYMBOLS_DIR", defaultDir);

    const string execDir =
        CanonicalPath(sys::dirname(bin.executable_name().c_str()));

    string path;

    ELF::List<ELF::Section>::const_iterator i = bin.sections().begin(),
                                          end = bin.sections().end();

    for (; i != end; ++i)
    {
        const ELF::Section::Header hdr = i->header();

        if (strcmp(hdr.name(), ".gnu_debuglink") == 0)
        {
            const Elf_Data& d = i->data();
            const char* filename = reinterpret_cast<char*>(d.d_buf);

            if (file_exists(execDir, filename, path)
             || file_exists(execDir + "/.debug", filename, path)
             || file_exists(globalDir + execDir, filename, path))
            {
                // todo: verify CRC
            }
            else
            {
                path.clear();
            }
            break;
        }
    }
    return path;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
