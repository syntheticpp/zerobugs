#ifndef CORE_H__07A8D5C6_DF54_4FBE_A8E0_98A9B5A76985
#define CORE_H__07A8D5C6_DF54_4FBE_A8E0_98A9B5A76985
//
// $Id: core_file.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
#include "zdk/config.h"
#include <map>
#include <vector>
#include <string>
#include <boost/cstdint.hpp>
#ifdef HAVE_SYS_PROCFS_H
 #include <sys/procfs.h>
#endif
#ifdef HAVE_SYS_USER_H
 #include <sys/user.h>
#endif
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "dharma/sarray.h"
#include "elfz/public/binary.h"
#include "elfz/public/core.h"
#include "elfz/public/headers.h"


namespace ELF
{
    /**
     * Minimal segment mapping information
     */
    struct Segment
    {
        off_t       offs;   // offset in the core file
        size_t      msize;  // size mapped in memory
        size_t      fsize;  // size in file
        long        flags;

        std::string filename;
    };


    class Note;
    class ProgramHeader;


    /**
     * Support for reading ELF core dumps
     */
    class CoreFile : public Binary
    {
    public:
        typedef std::map<ElfW(Addr), Segment> SegmentMap;
        typedef SegmentMap::const_iterator const_segment_iterator;
        typedef elf_prstatus_<__WORDSIZE> prstatus_t;
        typedef elf_prpsinfo_<__WORDSIZE> prpsinfo_t;

        typedef ext::hash_map<pid_t, prstatus_t> PrStatusMap;
        typedef std::vector<prpsinfo_t> Info;
        typedef std::vector<user_fpxregs_struct>  FPX;

        typedef PrStatusMap::const_iterator const_prstatus_iterator;

        typedef Info::const_iterator const_info_iterator;
        typedef FPX::const_iterator const_fpxregs_iterator;

        typedef std::vector<std::pair<long, long> > aux_vector;

        /**
         * Constructor, takes corefile name and [optionally]
         * the name of the progam that produced the core
         * as parameters.
         */
        explicit CoreFile(const char* corefile, const char* prog = 0);

        template<int W, template<int> class T>
            void xread_(ElfW(Addr) addr, T<__WORDSIZE>& val) const
            {
                if (W == __WORDSIZE)
                {
                    readval(addr, val);
                }
                else
                {
                    if (W == 32)
                    {
                        addr &= 0xffffffff;
                    }
                    T<W> tmp;
                    readval(addr, tmp);
                    val = tmp;
                }
            }
        template<typename A, template<int> class T>
            void xread(A ptr, T<__WORDSIZE>& val) const
            {
                if ((__WORDSIZE != 32) && header().klass() == ELFCLASS32)
                {
                    xread_<32>((ElfW(Addr))ptr, val);
                }
                else
                {
                    xread_<__WORDSIZE>((ElfW(Addr))ptr, val);
                }
            }

        /**
         * @return the name of the process that produced this core
         */
        const std::string& process_name() const
        {
            return procname_;
        }
        virtual const std::string& executable_name() const
        {
            return procname_;
        }

        /**
         * @return the task id of the process (main thread)
         */
        pid_t pid() const;

        const_segment_iterator seg_begin() const
        { return seg_.begin(); }

        const_segment_iterator seg_end() const
        { return seg_.end(); }

        // prstatus
        const_prstatus_iterator prstatus_find(pid_t lwpid) const
        {
            return prstatus_.find(lwpid);
        }

        const_prstatus_iterator prstatus_begin() const
        { return prstatus_.begin(); }

        const_prstatus_iterator prstatus_end() const
        { return prstatus_.end(); }

        // prpsinfo
        const_info_iterator info_begin() const
        { return info_.begin(); }

        const_info_iterator info_end() const
        { return info_.end(); }

        const_fpxregs_iterator fpxregs_begin() const
        { return fpxregs_.begin(); }

        const_fpxregs_iterator fpxregs_end() const
        { return fpxregs_.end(); }

        const user_fpxregs_struct& fpxregs(size_t n) const
        {
            assert(fpxregs_.size() > n);
            return fpxregs_[n];
        }

        /**
         * If the requested length is larger than the size on
         * disk, but smaller than the size in memory, the buffer
         * is padded with zeroes up to the requested length.
         *
         * If for some reason this function cannot read exactly
         * len bytes, it throws a runtime_ error
         */
        size_t readbuf(ElfW(Addr), char* buf, size_t len) const;

        size_t readmem(ElfW(Addr) addr, char* buf, size_t len) const
        {
            return readbuf_(addr, buf, len);
        }

        const aux_vector& auxv() const { return auxv_; }

        void get_environment(SArray&);
        void get_environment();

        Segment* find_segment(ElfW(Addr), ElfW(Addr)&);
        const Segment* find_segment(ElfW(Addr), ElfW(Addr)&) const;

    private:
        /**
         * Translates the given address to a core file offset,
         * and reads up to `len' bytes, stopping at segment boudary.
         * @return the actual number of bytes read.
         * @note does not throw if the specified number of bytes
         * could not be read
         */
        size_t readbuf_(ElfW(Addr), char* buf, size_t len) const;

        /*** functions called from ctor ***/
        void read_segment(const ProgramHeader&);

        void read_notes(const ProgramHeader&);
        void read_prpsinfo(const ELF::Note&);
        void read_prstatus(const ELF::Note&);
        void read_fpxregs(const ELF::Note&);
        void read_fpregs(const ELF::Note&);
        void read_auxv(const ELF::Note&);

        void read_debug_struct(ElfW(Addr));
        bool program_matches(const char* progfile) const;

        void set_segment_filename(ElfW(Addr), const char*);

    private:
        SegmentMap  seg_;
        std::string procname_; // process name
        std::string shortname_;

        PrStatusMap prstatus_;
        Info        info_;
        FPX         fpxregs_;
        aux_vector  auxv_;
        SArray      env_;
    };
}

#endif // CORE_H__07A8D5C6_DF54_4FBE_A8E0_98A9B5A76985
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
