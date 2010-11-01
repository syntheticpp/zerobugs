//
// $Id: disasm.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "zdk/disasm.h"
#include "zdk/shared_string.h"
#include "zdk/version_info_impl.h"
#include "dharma/environ.h"
#include "dharma/exec.h"
#include "dharma/exec_arg.h"
#include "dharma/pipe.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"

using namespace std;


/**
 * Implements a generic disassembler by executing the
 * objdump --disassemble program (part of binutils) and
 * by redirecting the output to a pipe.
 */
class ObjdumpWrapper
    : public Disassembler
    , VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR, 2>
{
    Syntax syntax_;

public:
    BEGIN_INTERFACE_MAP(ObjdumpWrapper)
        INTERFACE_ENTRY(Disassembler)
        INTERFACE_ENTRY(VersionInfo)
    END_INTERFACE_MAP()

    ObjdumpWrapper() : syntax_(ASM_ATT)
    {
        // check environment for default syntax
        if (env::get_string("ZERO_ASM_SYNTAX", "att") != "att")
        {
            syntax_ = ASM_INTEL;
        }
    }

    virtual ~ObjdumpWrapper() throw() {}

    void release() { delete this; }

    bool uses_own_buffer() const { return true; }

    size_t disassemble(
        addr_t          startAddr,
        off_t           adjustment,
        const uint8_t*  memBuf,
        size_t          length,
        SharedString*   filename,
        OutputCallback* outputLineCB = 0,
        SymbolCallback* lookupSymbolCB = 0,
        SourceCallback* lookupSourceCB = 0);

    Syntax syntax() const { return syntax_; }

    void set_syntax(Syntax syntax)
    {
        syntax_ = syntax;
    }

    const char* description() const
    {
        return "Generic Disassembler";
    }

    const char* copyright() const
    {
        return "Copyright (c) 2004 Cristian Vlasceanu";
    }
};


int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}


void query_plugin(InterfaceRegistry* reg)
{
    assert(reg);
    reg->update(Disassembler::_uuid());
}


Plugin* create_plugin(uuidref_t iid)
{
    if (uuid_equal(Disassembler::_uuid(), iid))
    {
        return new ObjdumpWrapper;
    }
    return 0;
}


static bool notify_disasm_output
(
    pid_t disasembler,
    Disassembler::OutputCallback* observer,
    addr_t addr,
    const string& line
)
{
    if (!observer->notify(addr, line.c_str(), line.length()))
    {
        sys::kill(disasembler, SIGTERM);
        sys::waitpid(disasembler, 0, 0);

        return false;
    }
    return true;
}


static bool notify_disasm_output
(
    pid_t disasembler,
    Disassembler::OutputCallback* observer,
    addr_t addr,
    const string& line,
    const char* text
)
{
    if (observer)
    {
        if (addr == 0)
        {
            return notify_disasm_output(disasembler, observer, addr, line);
        }
        else
        {
            //
            // format output line
            //
            ostringstream tmp;
            tmp << hex << setw(2 * sizeof(addr_t)) << setfill('0');
            tmp << addr;
            tmp << text;

            return notify_disasm_output(disasembler, observer, addr, tmp.str());
        }
    }
    else
    {
        cout << line << "\n";
    }
    return true;
}


/**
 * Reads the output of a disassembler (forked as a separate
 * process) from a pipe, and calls the enum callback sink,
 * one line at a time. Calls waitpid() on readers's pid to
 * determine that we are done.
 *
 * @return last disassembled address
 */
static addr_t
disasm_read(pid_t pid,
            addr_t prev,
            int fd,
            Disassembler::OutputCallback* outCB,
            Disassembler::SourceCallback* srcCB)
{
    while (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    {
        if (errno != EINTR) throw SystemError(errno);
    }

    addr_t addr = 0;

    char buf[1024]; // todo: use a vector<char> instead?

    string partialLine;
    size_t lineCount = 0;

    for (bool producerDone = false; ;)
    {
        const int bytesToRead = sizeof buf;
        int n = read(fd, buf, bytesToRead);

        if (n > 0)
        {
            int i = 0;
            for (int j = 0; j != n; ++j)
            {
                if (buf[j] != '\n')
                {
                    continue;
                }
                string line;
                if (!partialLine.empty())
                {
                    line = partialLine;
                    partialLine.erase();
                }
                line += string(buf + i, j - i);

                if (!line.empty() && line.find("Dis") != 0)
                {
                    // skip the first line of objdump disassembly output

                    if (++lineCount < 2)
                    {
                        i = j + 1;
                        continue;
                    }

                    char* p = 0;
                    addr = strtoul(line.c_str(), &p, 16);
                    if (!p || *p != ':')
                    {
                        addr = 0;
                    }
                    if (!notify_disasm_output(pid, outCB, addr, line, p))
                    {
                        return addr;
                    }
                    if ((srcCB != NULL) && (addr > prev))
                    {
                        srcCB->notify(0, addr - prev);
                        prev = addr;
                    }
                }
                i = j + 1;
            }
            if (i < n)
            {
                partialLine += string(buf + i, n - i);
            }
        }
        if (n < bytesToRead)
        {
            if (producerDone)
            {
                break;
            }

            static const int wfl = WNOHANG | WUNTRACED;
            int status = 0;

            if (waitpid(pid, &status, wfl) == pid)
            {
                producerDone = true;
            }
        }
    }
    return addr;
}



/**
 * Implements the disassemble method by invoking the
 * objdump program and redirecting its output to a pipe.
 */
size_t ObjdumpWrapper::disassemble (
    addr_t          startAddr,
    off_t           adjust,
    const uint8_t* /*  buff */,
    size_t          length,
    SharedString*   filename,
    OutputCallback* outputLineCB,
    SymbolCallback* lookupSymbolCB,
    SourceCallback* lookupSourceCB)
{
    assert(filename);
#ifdef DEBUG
   clog << "*** Disassembling " << length << " bytes from: ";
   clog << filename << ": 0x" << hex << startAddr << dec << endl;
#endif

    Pipe pipe(false);

    const addr_t addr = startAddr;
    ostringstream cmd;

    cmd << "objdump -";
    cmd << (lookupSourceCB ? "S" : "d");
    cmd << " --disassemble-all";
    cmd << " --demangle";
    cmd << " --start-address=0x" << hex << addr;
    cmd << " --stop-address=0x"  << hex << addr + length;

    switch (syntax_)
    {
    case ASM_FIXED:
        break;
    case ASM_ATT:
        cmd << " -M att";
        break;
    case ASM_INTEL:
        cmd << " -M intel";
        break;
    }

    if (adjust)
    {
        cmd << " --adjust-vma=0x" << hex << adjust;
    }
    cmd << ' ' << filename;
#ifdef DEBUG
    clog << cmd.str() << endl;
#endif
    ExecArg arg(cmd.str());

    pid_t pid = ::exec("objdump", arg.strings(), pipe.input());
    const addr_t last = disasm_read(pid,
                                    addr + adjust,
                                    pipe.output(),
                                    outputLineCB,
                                    lookupSourceCB);

    const size_t length = labs(last - startAddr);
#ifdef DEBUG
    clog << __func__ << ": " << length << endl;
#endif
    return length;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
