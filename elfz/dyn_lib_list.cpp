//
// $Id: dyn_lib_list.cpp 711 2010-10-16 07:09:23Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#ifdef HAVE_SYS_STAT_H
 #include <sys/stat.h>
#else
 #include <unistd.h>
#endif
#include <iostream>
#include <boost/tokenizer.hpp>
#include "dharma/canonical_path.h"
#include "dharma/dynamic_lib.h"
#include "dharma/syscall_wrap.h"
#include "elfz/public/binary.h"
#include "elfz/public/dyn_lib_list.h"
#include "elfz/public/headers.h"
#include "elfz/public/link.h"
#include "elfz/public/section.h"
#include "elfz/private/ldsoconf.h"

using namespace std;


static vector<string> tokenize(const string& pathlist)
{
    typedef  boost::char_separator<char> Delim;
    typedef boost::tokenizer<Delim> Tokenizer;

    Tokenizer tok(pathlist, Delim(":;"));

    vector<string> v(tok.begin(), tok.end());
    return v;
}


static bool
is_valid_elf_file(const string& path, const ELF::Binary& elf) throw()
{
    bool result = false;

    struct stat stbuf;
    if (stat(path.c_str(), &stbuf) == 0)
    {
        try
        {
            ELF::Binary binary(path.c_str());

            result = binary.check_format()
                  && binary.abi() == elf.abi()
                  // && binary.abi_version() == elf.abi_version()
                  && binary.header().klass() == elf.header().klass()
                  && binary.header().machine() == elf.header().machine();
        }
        catch (const exception& e)
        {
            clog << __func__ << "(" << path << "): " << e.what() << endl;
        }
    }
    return result;
}


void DynLibList::get_dl_search_paths()
{
    static const char* defaultLibs[] =
    {
        "/lib",
        "/usr/lib",
        "/lib64",
        "/usr/lib64",
        "/lib32",
        "/usr/lib32",
    #if __i386__ || __x86_64__
        "/lib/i686",
    #endif
    };
    static const size_t libCount = sizeof defaultLibs / sizeof defaultLibs[0];

    if (paths_.empty())
    {
        if (sys::uses_nptl())
        {
            paths_.push_back("/lib/tls");
            paths_.push_back("/lib64/tls");
        }
        paths_.insert(paths_.end(), defaultLibs, defaultLibs + libCount);
        string config("/etc/ld.so.conf");
        map_path(config);
        ldsoconf_parse(config.c_str(), paths_);

        for (vector<string>::iterator i = paths_.begin();
             i != paths_.end();
              ++i)
        {
            map_path(*i);
        }
    }
}


DynLibList::DynLibList(const char* filename,
                       const char* const* environ,
                       PathMapper* mapper)
    : environ_(environ)
    , elf_(filename)
    , mapper_(mapper)
{
    collect_needed(elf_, elf_.header());
}


void DynLibList::collect_needed(const char* filename)
{
    ELF::Binary binary(filename);
    ELF::Binary::Header hdr = binary.header();

    collect_needed(binary, hdr);
}


void
DynLibList::collect_needed(const string& pathlist, list_type& libs)
{
    if (pathlist.empty())
    {
        return;
    }
    const list_type paths = tokenize(pathlist);
    collect_needed(paths, libs);
}


void
DynLibList::collect_needed(const list_type& paths, list_type& libs)
{
    list_type::iterator i = libs.begin();

    for (; i != libs.end(); )
    {
        if (libs_.find(*i) != libs_.end())
        {
            i = libs.erase(i);
        }
        else if (try_paths(paths, *i))
        {
            i = libs.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

bool
DynLibList::try_paths(const list_type& paths, const string& name)
{
    list_type::const_iterator i = paths.begin();
    for (; i != paths.end(); ++i)
    {
        string tmp;
        if (name[0] == '/')
        {
            tmp = name;
        }
        else
        {
            tmp = *i;
            if (size_t n = tmp.size())
            {
                if (tmp[n - 1] != '/')
                {
                    tmp += '/';
                }
            }
            tmp += name;
        }
        const string libpath = canonical_path(tmp.c_str());
        if (!is_valid_elf_file(libpath, elf_))
        {
            continue;
        }
        // insert the canonicalized path
        if (libs_.insert(libpath).second)
        {
            collect_needed(libpath.c_str());
        }
        return true;
    }
    return false;
}



void DynLibList::collect_needed(const ELF::Binary& elf, const ELF::ElfHdr& hdr)
{
    list_type libs; // build a list of libs from DT_NEEDED
    string rpath;

    get_library_path(); // get LD_LIBRARY_PATH

    ELF::List<ELF::Section>::const_iterator i = elf.sections().begin();
    for (; i != elf.sections().end(); ++i)
    {
        if (i->header().type() == SHT_DYNAMIC)
        {
            const Elf_Data& d = i->data();
            const size_t count = d.d_size / sizeof(ElfW(Dyn));

            for (size_t n = 0; n != count; ++n)
            {
                const ElfW(Dyn)& dyn = reinterpret_cast<ElfW(Dyn)*>(d.d_buf)[n];

                ElfW(Word) strindx = i->header().link();
                ElfW(Word) offs = dyn.d_un.d_val;

                if (dyn.d_tag == DT_NEEDED)
                {
                    const char* lib = elf_strptr(elf.elf(), strindx, offs);
                    libs.push_back(lib);
                }
                else if (dyn.d_tag == DT_RPATH || dyn.d_tag == DT_RUNPATH)
                {
                    if (!rpath.empty())
                    {
                        rpath += ":";
                    }
                    rpath += elf_strptr(elf.elf(), strindx, offs);
                }
            }
        }
    }
    //copy(libs.begin(), libs.end(), ostream_iterator<string>(out, "\n"));
    collect_needed(rpath, libs);

    // TODO: skip files with the setuid and/or setgid bit set
    if (!libraryPath_.empty())
    {
        collect_needed(libraryPath_, libs);
    }
    // TODO: if FLAG_1 is set, ignore default libs

    get_dl_search_paths();
    collect_needed(paths_, libs);
}


const string& DynLibList::get_library_path() const
{
    if (libraryPath_.empty() && environ_)
    {
        for (const char* const* env = environ_; env && *env; ++env)
        {
            static const size_t len = sizeof ("LD_LIBRARY_PATH") - 1;
            if (strncmp("LD_LIBRARY_PATH=", *env, len) == 0)
            {
                libraryPath_ = *env + len + 1;

                break;
            }
        }
    }
    return libraryPath_;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
