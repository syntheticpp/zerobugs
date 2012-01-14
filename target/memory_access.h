#ifndef MEMORY_ACCESS_H__02E48702_AD27_4F24_9878_E9D81A27C682
#define MEMORY_ACCESS_H__02E48702_AD27_4F24_9878_E9D81A27C682
//
// $Id: memory_access.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <sys/fcntl.h>
#include <string>
#include <sstream>
#include "generic/auto_file.h"
#include "zdk/arch.h"
#include "zdk/export.h"
#include "target/target.h"


typedef Target::SegmentType SegmentType;


template<int WordSize>
struct ZDK_LOCAL MemoryBase
{
    typedef typename Arch<WordSize>::Long Long;

    /**
     * read debuggee's memory using the /proc filesystem
     */
    static bool read_using_proc (
        pid_t       pid,
        SegmentType seg,
        addr_t      addr,
        Long*       buf,
        size_t      buflen, // length of buffer in machine words
        size_t*     readlen,
        const std::string& procfs
    )
    {
        std::ostringstream mem;

        mem << procfs << pid << "/mem";

        auto_fd fd(open(mem.str().c_str(), O_RDONLY));

        if (fd.get())
        {
            off_t n = lseek(fd.get(), addr, SEEK_SET);

            if (n != -1)
            {
                n = ::read(fd.get(), buf, buflen * sizeof(Long));

                if (n != -1)
                {
                    if (readlen)
                    {
                        *readlen = n / sizeof(Long);
                    }
                    return true;
                }
            }
        }
        else
        {
            std::cerr << mem.str() << ", " << strerror(errno) << std::endl;
        }
        return false;
    }


    static void read_using_ptrace (
        pid_t       pid,
        SegmentType seg,
        addr_t      addr,
        Long*       buf,
        size_t      buflen, // length of buffer in machine words
        size_t*     readlen
    )
    {
        const __ptrace_request req =
            seg == Target::CODE_SEGMENT ? PT_READ_I : PT_READ_D;

        size_t i = 0;
        try
        {
            for (; i != buflen; ++i, addr += sizeof(Long))
            {
                buf[i] = sys::ptrace(req, pid, addr, 0);
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << __func__ << ": " << e.what() << std::endl;

            if (!readlen) 
            {
                throw;
            }
        }
        if (readlen)
        {
            *readlen = i;
        }
    }


    static void read (
        pid_t       pid,
        SegmentType seg,
        addr_t      addr,
        Long*       buf,
        size_t      len,
        size_t*     readlen,
        const std::string& proc
    )
    {
        if (!addr && readlen)
        {
            *readlen = 0;
            // if the readlen pointer is NULL, then go ahead and
            // attempt to read, so that an exception is thrown
        }
        else
        {
            if (len > 2)
            {
                // more than 2 words required -- it is as cheaper, from
                // a number of system calls perspective, to attempt reading
                // the debuggee's memory using the /proc interface;
                // requires 3 system calls: open(), lseek() and read()
                if (read_using_proc(pid, seg, addr, buf, len, readlen, proc))
                {
                    return;
                }
            }
            read_using_ptrace(pid, seg, addr, buf, len, readlen);
        }
    }


    static void write (
        pid_t       pid,
        SegmentType seg,
        addr_t      addr,
        const Long* buf,
        size_t      buflen
    )
    {
        const __ptrace_request req =
            seg == Target::CODE_SEGMENT ? PT_WRITE_I : PT_WRITE_D;

        for (size_t i = 0; i != buflen; ++i, addr += sizeof(Long))
        {
            //std::clog << "*** POKE [" << (void*)addr << "]: ";
            //std::clog << (void*)buf[i] << "\n";

            sys::ptrace(req, pid, addr, buf[i]);
        }
    }
};


template<int HostWord = __WORDSIZE, int TargetWord = __WORDSIZE>
struct ZDK_LOCAL Memory : public MemoryBase<TargetWord>
{
};


/*
template<>
struct Memory<64, 32> : public MemoryBase<32>
{
    typedef Arch<32>::Long TargetLong;

    static void read
    (
        pid_t       pid,
        SegmentType seg,
        addr_t      addr,
        long*       buf,
        size_t      len,
        size_t*     nRead
    )
    {
        MemoryBase<32>::read(pid, seg, addr, (TargetLong*)buf, len * 2, nRead);
        if (nRead)
        {
            *nRead /= 2;
        }
    }


    static void write
    (
        pid_t       pid,
        SegmentType seg,
        addr_t      addr,
        const long* buf,
        size_t      len
    )
    {
        MemoryBase<32>::write(pid, seg, addr, (const TargetLong*)buf, len);
    }
};
*/

#endif // MEMORY_ACCESS_H__02E48702_AD27_4F24_9878_E9D81A27C682
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
