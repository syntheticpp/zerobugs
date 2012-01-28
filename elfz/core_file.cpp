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

#include "zdk/config.h"
#include "zdk/platform.h"
#include "zdk/stdexcept.h"
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "generic/auto_file.h"
#include "generic/state_saver.h"
#include "public/binary.h"
#include "public/file.h"
#include "public/headers.h"
#include "public/core_file.h"
#include "public/link.h"
#include "public/list.h"
#include "public/note.h"
#include "public/program_header.h"
#include "public/section.h"
#include "dharma/canonical_path.h"
#include "dharma/environ.h"
#include "dharma/system_error.h"
#include "dharma/utility.h"

typedef user_fpregs_struct fpregset_t;
typedef user_fpxregs_struct fpxregset_t;


using namespace std;
using Platform::word_t;


ELF::CoreFile::CoreFile(const char* filename, const char* procname)
    : Binary(filename)
{
    if (header().type() != ET_CORE)
    {
        throw runtime_error(string(filename) + ": not a core file");
    }
    const List<ProgramHeader>& phdr = program_headers();

    List<ProgramHeader>::const_iterator i = phdr.begin();
    const List<ProgramHeader>::const_iterator end = phdr.end();

    for (; i != end; ++i)
    {
        switch (i->type())
        {
        case PT_LOAD:
            read_segment(*i);
            break;

        case PT_NOTE:
            read_notes(*i); // calls read_prpsinfo, which
                            //  may populate procname_
            break;
        }
    }
    if (program_matches(procname))
    {
        assert(procname);
        procname_ = procname;
    }
    try
    {
        // Open the associated file -- i.e. the program that
        // has generated the core dump
        ELF::Binary bin(procname_.c_str());

        if (ElfW(Addr) addr = get_linker_debug_struct_addr(bin))
        {
            read_debug_struct(addr);
        }
    }
    catch (const exception& e)
    {
        cerr << __func__ << "(" << procname_ << "): " << e.what() << endl;
    }
}


void ELF::CoreFile::read_segment(const ProgramHeader& phdr)
{
    assert(phdr.type() == PT_LOAD);

    if (phdr.memsz())
    {
        if (phdr.memsz() > numeric_limits<size_t>::max())
        {
            throw std::range_error("memsz out of range in corefile program header");
        }
        if (phdr.filesz() > numeric_limits<size_t>::max())
        {
            throw std::range_error("filesz out of range in corefile program header");
        }
        Segment seg =
        {
            phdr.offset(),
            size_t(phdr.memsz()),
            size_t(phdr.filesz()),
            phdr.flags()
        };
        ElfW(Addr) vaddr = phdr.vaddr();

        //assert(vaddr);
        //assert(seg_.find(vaddr) == seg_.end());

        seg_.insert(seg_.end(), make_pair(vaddr, seg));
    }
}


static inline void throw_seg_not_found(ElfW(Addr) addr)
{
/*
    ostringstream msg;
    msg << "core segment not found, addr=";
    msg << hex << showbase << addr;
    throw runtime_error(msg.str());
 */
}


static void
throw_not_enough_data(const char* func, size_t req, size_t read)
{
    assert(read < req);
    ostringstream msg;
    msg << __func__ << ": requested " << req << ", read " << read;
    throw runtime_error(msg.str());
}


size_t
ELF::CoreFile::readbuf(ElfW(Addr) addr, char* buf, size_t len) const
{
    const size_t bytesRead = readbuf_(addr, buf, len);
    if (bytesRead != len)
    {
        throw_not_enough_data(__func__, len, bytesRead);
    }
    return bytesRead;
}


size_t
ELF::CoreFile::readbuf_(ElfW(Addr) addr, char* buf, size_t len) const
{
    ElfW(Addr) segAddr = 0;
    const Segment* seg = find_segment(addr, segAddr);

    if (!seg)
    {
        throw_seg_not_found(addr);
        return 0;
    }
    const size_t offs = addr - segAddr;

    size_t result = 0;
    size_t bytesToRead = len;

    if (offs + len > seg->msize)
    {
        // requested chunk spans across segments, stop
        // reading at segment boundary
        if (seg->msize > offs)
        {
            bytesToRead = seg->msize - offs;
        }
        else
        {
            return 0;
        }
    }
    string filename = seg->filename;
    if (filename.empty())
    {
        filename = procname_;
    }
    // executable memory was locked when kernel dumped core?
    if (!seg->fsize && seg->msize && (seg->flags & PF_X))
    {
        //try getting it from the file image
        if (!filename.empty())
        {
            Binary elf(filename.c_str());
            result += elf.readbuf(offs, buf, bytesToRead);
        }
    }
    else
    {
        size_t readFromFile = bytesToRead;

        // fsize is the size of data in the file, msize is the
        // size in memory; msize can be larger than fsize, and
        // padded up with zeroes
        if (offs < seg->fsize)
        {
            readFromFile = min(bytesToRead, seg->fsize - offs);
            size_t tmp = File::readbuf(seg->offs + offs, buf, readFromFile);
            if (tmp != readFromFile)
            {
                throw_not_enough_data(__func__, readFromFile, tmp);
            }
            result += tmp;
        }
        else
        {
            readFromFile = 0;
        }
        if (bytesToRead > readFromFile)
        {
            if (!filename.empty())
            {
                Binary elf(filename.c_str());
                readFromFile = elf.readbuf(offs, buf, bytesToRead);
                result += readFromFile;
            }
            // pad with zeroes
            memset(buf + readFromFile, 0, bytesToRead - readFromFile);
            result += bytesToRead - readFromFile;
        }
    }
    return result;
}



/**
 * Find the segment that addr maps to
 */
ELF::Segment*
ELF::CoreFile::find_segment(ElfW(Addr) addr, ElfW(Addr)& segAddr)
{
    SegmentMap::iterator i = seg_.lower_bound(addr);
    if (i == seg_.end() || i->first != addr)
    {
        if (i == seg_.begin())
        {
            return NULL;
        }
        --i;
    }
    assert(addr >= i->first);
    segAddr = i->first;
    return &i->second;
}



const ELF::Segment*
ELF::CoreFile::find_segment(ElfW(Addr) addr, ElfW(Addr)& segAddr) const
{
    return const_cast<CoreFile*>(this)->find_segment(addr, segAddr);
}



void ELF::CoreFile::read_notes(const ELF::ProgramHeader& phdr)
{
    assert(phdr.filesz());

    vector<unsigned char> bytes(phdr.filesz());
    File::readbuf(phdr.offset(), (char*)&bytes[0], bytes.size());

    unsigned char* data = &bytes[0];

    for (;;)
    {
        size_t sz = bytes.size() - (data - &bytes[0]);
        if (sz < sizeof(ElfW(Nhdr)))
        {
            // bytes.resize(bytes.size() + sizeof(ElfW(Nhdr)) - sz);
            // data = &bytes[0];
    #ifdef DEBUG
            clog << "incomplete note? " << sz << " byte(s)\n";
            clog << hex << setfill('0');

            for (size_t i = 0; i != sz; ++i)
            {
                clog << setw(2) <<(int)data[i] << ' ';
            }
            clog << dec << endl;
    #endif
            break;
        }
        const ELF::Note note(*this, data);

        switch (note.type())
        {
        case NT_PRSTATUS:
            read_prstatus(note);
            break;

        case NT_PRPSINFO:
            read_prpsinfo(note);
            break;

        case NT_FPREGSET:
            read_fpregs(note);
            break;

    #ifdef HAVE_STRUCT_USER_FPXREGS_STRUCT
        // /usr/include/linux/elf.h copied this from
        // gdb5.1/include/elf/common.h
        case /* NT_PRXFPREG */ 0x46e62b7f: 

        case NT_PRFPXREG: // 20
            read_fpxregs(note);
            break;
    #endif
    #if __linux__
        case NT_AUXV:
            read_auxv(note);
            break;
    #endif
    #ifdef DEBUG
        case /* NT_TASKSTRUCT */ 4:
            clog << "task note size=" << note.data().size() << endl;
            break;

        default:
            clog << "note: " << note.type();
            clog << " size=" << note.data().size() << endl;
    #endif
        }
        assert(!note.is_null());

        data += note.total_rounded_size();

        const size_t size = distance(&bytes[0], data);

        if (size >= bytes.size())
        {
            break;
        }
    }
}


void ELF::CoreFile::read_prstatus(const ELF::Note& note)
{
    switch (header().klass())
    {
    case ELFCLASS32:
        assert(note.data().size() >= sizeof(elf_prstatus_<32>));

        if (elf_prstatus_<32>* ps = (elf_prstatus_<32>*)note.d_ptr())
        {
            prstatus_[ps->pr_pid] = *ps;
        }
        break;

    case ELFCLASS64:
        assert(note.data().size() == sizeof(elf_prstatus_<64>));

        if (elf_prstatus_<64>* ps = (elf_prstatus_<64>*)note.d_ptr())
        {
            prstatus_[ps->pr_pid] = *ps;
        }
        break;

    default:
        assert(false);
    }
}



void ELF::CoreFile::read_prpsinfo(const ELF::Note& note)
{
    switch (header().klass())
    {
    case ELFCLASS32:
        //assert(note.data().size() == sizeof(elf_prpsinfo_<32>));
        assert(note.data().size() >= sizeof(elf_prpsinfo_<32>));
        info_.push_back(*(const elf_prpsinfo_<32>*)(note.d_ptr()));
        break;

    case ELFCLASS64:
        assert(note.data().size() == sizeof(elf_prpsinfo_<64>));
        info_.push_back(*(const elf_prpsinfo_<64>*)(note.d_ptr()));
        break;

    default:
        assert(false);
    }
    const prpsinfo_t& prpsinfo = info_.back();

    if (procname_.empty())
    {
        procname_ = prpsinfo.pr_psargs;
        procname_ = procname_.substr(0, procname_.find(' '));
        shortname_ = procname_;
    }
    if (!procname_.empty() && procname_[0] != '/')
    {
        size_t n = this->name().rfind('/');
        if (n == string::npos)
        {
            procname_ = canonical_path(procname_.c_str());
        }
        else
        {
            string path = this->name().substr(0, n + 1);

            procname_.insert(0, path);
            procname_ = canonical_path(procname_.c_str());
        }
    }
}



void ELF::CoreFile::read_fpxregs(const ELF::Note& note)
{
    assert(note.data().size() == sizeof(fpxregset_t));
    const fpxregset_t& regs = *(const fpxregset_t*)(note.d_ptr());

    fpxregs_.push_back(regs);
}



void ELF::CoreFile::read_fpregs(const ELF::Note& note)
{
//todo:fixme 32 on 64
    if(note.data().size() != sizeof(fpregset_t))
    {
        return;
    }
//end todo
    assert(note.data().size() == sizeof(fpregset_t));
    const fpregset_t& regs = *(const fpregset_t*)(note.d_ptr());

#ifndef HAVE_STRUCT_USER_FPXREGS_STRUCT
    fpxregs_.push_back(regs);
#else
    fpxregset_t xregs;
    memset(&xregs, 0, sizeof xregs);

    xregs.cwd = regs.cwd;
    xregs.swd = regs.swd;
    xregs.twd = regs.twd;
    xregs.fip = regs.fip;
    xregs.fcs = regs.fcs;
    xregs.foo = regs.foo;
    xregs.fos = regs.fos;

    char* dst = reinterpret_cast<char*>(xregs.st_space);
    const char* src = reinterpret_cast<const char*>(regs.st_space);

    for (unsigned i = 0; i != 8; ++i)
    {
        memcpy(dst, src, 10);
        dst += 16;
        src += 10;
    }
    fpxregs_.push_back(xregs);

#endif // HAVE_STRUCT_USER_FPXREGS_STRUCT
}


template<typename T>
inline T read_as(const ELF::Note& note, size_t i)
{
    return *reinterpret_cast<const T*>(&note.data()[i]);
}


template<typename T>
inline size_t read_aux_vect_elem
(
    const ELF::Note& note,
    size_t i,
    size_t nMax,
    long& type,
    long& value
)
{
    type = read_as<T>(note, i);
    i += sizeof(T);

    if (!type)
    {
        i = nMax;
    }
    else
    {
        value = i < nMax ? read_as<T>(note, i) : T();
        i += sizeof (T);
    }
    return i;
}


void ELF::CoreFile::read_auxv(const ELF::Note& note)
{
    const bool dumpVect = env::get_bool("ZERO_DUMP_AUX_VECT");
    const size_t size = note.data().size();

    for (size_t i = 0; i < size; )
    {
        long type = 0, value = 0;

        if (header().klass() == ELFCLASS32)
        {
            i = read_aux_vect_elem<uint32_t>(note, i, size, type, value);
        }
        else
        {
            i = read_aux_vect_elem<word_t>(note, i, size, type, value);
        }

        if (dumpVect)
        {
            clog << "[" << type << "]=" << value << "\n";
        }
        auxv_.push_back(pair<long, long>(type, value));
    }
}



void ELF::CoreFile::read_debug_struct(ElfW(Addr) addr)
{
    r_debug_<>* dbgp = 0;
    readval(addr, dbgp);

    r_debug_<> dbg;
    xread(dbgp, dbg);

    link_map_<> lmap;

    for (ElfW(Addr) pmap = dbg.r_map; pmap; pmap = lmap.l_next)
    {
        xread(pmap, lmap);

        if (lmap.l_prev == 0) // skip the main module (first entry)
        {
            if (lmap.l_addr == get_load_addr())
            {
                set_segment_filename(lmap.l_addr, procname_.c_str());
            }
            continue;
        }

        char path[PATH_MAX];
        memset(path, 0, sizeof path);
        if (lmap.l_name)
        {
            readbuf_((ElfW(Addr))lmap.l_name, path, sizeof(path) - 1);
        }

        string fullpath;
        if (path[0])
        {
            fullpath.assign(path);
        }
        else
        {
            if (lmap.l_addr == dbg.r_ldbase)
            {
                Binary elf(procname_.c_str());
                fullpath = elf.dynamic_linker();
            }
            else
            {
                continue;
            }
        }
        try
        {
            get_environment();
            ensure_abs_lib_path(env_.cstrings(), fullpath);

            Binary elf(fullpath.c_str());
            lmap.l_addr += elf.get_load_addr();
        }
        catch (const exception& e)
        {
        #if DEBUG
            clog << __func__ << ": " << e.what() << endl;
        #endif
            continue;
        }
        set_segment_filename((ElfW(Addr))lmap.l_addr, fullpath.c_str());
    }
}



void ELF::CoreFile::set_segment_filename(ElfW(Addr) addr, const char* path)
{
    ElfW(Addr) segAddr = 0;
    if (Segment* seg = find_segment(addr, segAddr))
    {
        if (path && seg->filename.empty())
        {
            seg->filename = path;
            if (path[0] != '/')
            {
                get_environment();
                ensure_abs_lib_path(env_.cstrings(), seg->filename);
            }
        }
    }
    else
    {
#ifdef DEBUG
        clog << __func__ << ": not found ";
        clog << (void*)addr << " " << path << endl;
#endif
    }
}



pid_t ELF::CoreFile::pid() const
{
#ifdef HAVE_PRPSINFO_PID
    return info_.empty() ? 0 : info_.front().pr_pid;
#else
    return prstatus_.empty() ? 0 : prstatus_.begin()->second.pr_pid;
#endif
}


void ELF::CoreFile::get_environment(SArray& env)
{
    if (env_.empty())
    {
        get_environment();
    }
    env = env_;
}


/**
 * Get the environment of the crashed program from the core dump.
 */
void ELF::CoreFile::get_environment()
{
    // static const streampos step = 1024;

    if (!env_.empty())
    {
        return;
    }
    const List<ProgramHeader>& phdr = program_headers();

    List<ProgramHeader>::const_iterator i = phdr.begin(),
                                      end = phdr.end();
    off_t first = i->offset();
    // off_t last = first + i->filesz();
    //
    // locate the last loadable, non-executable segment
    //
    for (; i != end; ++i)
    {
        if ((i->type() == PT_LOAD) && (i->flags() & PF_X) == 0)
        {
            first = i->offset();
            // last = first + i->filesz();
        }
    }

    ifstream f(this->name().c_str(), ios::binary);
    if (!f)
    {
        throw SystemError(this->name());
    }

    f.seekg(first, ios::beg);

    unsigned char c = 0;
    string s;

    while (f >> noskipws >> c)
    {
        if (!c)
        {
            s.clear();
        }
        else
        {
            s.push_back(c);
            if (s == shortname_)
            {
                env::read(f, env_);
                if (!env_.empty())
                {
                    break;
                }
            }
        }
    }
}


/**
 * If progname given, make sure it matches the corefile
 */
bool ELF::CoreFile::program_matches(const char* progname) const
{
    if (progname && *progname)
    {
        if (!procname_.empty() && canonical_path(progname) != procname_)
        {
            clog << "*** Warning: " << procname_ << ": does not match ";
            clog << progname << endl;
        }
        else
        {
            return true;
        }
    }
    return false;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
