//
// $Id: disasm.cpp 729 2010-10-31 07:00:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include "zdk/disasm.h"
#include "zdk/version_info_impl.h"
#include "dharma/environ.h"
#include "dharma/exec.h"
#include "dharma/exec_arg.h"
#include "dharma/pipe.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"
#include "elfz/public/binary.h"
#include "elfz/public/headers.h"
#include "generic/auto_file.h"
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <udis86.h>


using namespace std;


class Disasm86
    : public Disassembler
    , VersionInfoImpl<ZERO_API_MAJOR, ZERO_API_MINOR, 4>
{
    ud_t ud_;
    Syntax syntax_;

    void print_call_or_jmp(SymbolCallback*, addr_t, ostream&);

public:

BEGIN_INTERFACE_MAP(Disasm86)
    INTERFACE_ENTRY(Disassembler)
    INTERFACE_ENTRY(VersionInfo)
END_INTERFACE_MAP()

    Disasm86() : syntax_(ASM_ATT)
    {
        ud_init(&ud_);

        // check environment for default syntax
        if (env::get_string("ZERO_ASM_SYNTAX", "att") != "att")
        {
            syntax_ = ASM_INTEL;
        }
    }

    virtual ~Disasm86() throw() {}

    void release() { delete this; }

    Syntax syntax() const { return syntax_; }

    void set_syntax(Syntax syntax) { syntax_ = syntax; }

    void apply_syntax()
    {
        switch (syntax_)
        {
        case ASM_INTEL:
            ud_set_syntax(&ud_, UD_SYN_INTEL);
            break;

        case ASM_ATT:
            ud_set_syntax(&ud_, UD_SYN_ATT);
            break;

        case ASM_FIXED:
            break;
        }
    }

    bool uses_own_buffer() const { return false; }

    size_t disassemble(
        addr_t          startAddr,
        off_t           adjustment,
        const uint8_t*  memBuf,
        size_t          length,
        SharedString*   filename,
        OutputCallback* outputLineCB = 0,
        SymbolCallback* lookupSymbolCB = 0,
        SourceCallback* lookupSourceCB = 0);

    const char* description() const
    {
        return "udis86 Disassembler";
    }

    const char* copyright() const
    {
        return "Copyright (c) 2010 Cristian Vlasceanu\n"
"libudis86: Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008 vivek@sig9.com\n"
"All rights reserved.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND \n"
"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED \n"
"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE \n"
"DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR \n"
"ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES \n"
"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; \n"
"LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON \n"
"ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT \n"
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS \n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n";
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
        return new Disasm86;
    }
    return 0;
}


static string
hex_bytes(const char* bytes, bool addSpace = true)
{
    string result;

    for (size_t i = 0; bytes && *bytes; ++i)
    {
        result += *bytes++;
        if (addSpace && (i % 2))
        {
            result += ' ';
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
//
// Helpers used with exec-ing objdump
//
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
disasm_read(pid_t pid, int fd, Disassembler::OutputCallback* cb)
{
    while (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    {
        if (errno != EINTR) throw SystemError(errno);
    }

    addr_t addr = 0;

    char buf[1024];

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
                    if (!notify_disasm_output(pid, cb, addr, line, p))
                    {
                        return addr;
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
// end objdump helpers
//
////////////////////////////////////////////////////////////////

size_t Disasm86::disassemble (
    addr_t          startAddr,
    off_t           adjust,
    const uint8_t*  buff,
    size_t          length,
    SharedString*   filename,
    OutputCallback* outputLine,
    SymbolCallback* lookupSymbol,
    SourceCallback* lookupSource)
{
    if (!filename)
    {
        return 0;
    }

#ifdef DEBUG
    clog << "*** Disassembling " << length << " bytes from: ";
    clog << filename << ": 0x" << hex << startAddr << dec << endl;
#endif
    auto_file file;

    if (buff)
    {
        ud_set_input_buffer(&ud_, const_cast<uint8_t*>(buff), length);
    }
    else
    {
#if 0 // Does not work, libudis86 will try to disassemble the
      // entire file.
      // One way to go is to inspect the ELF headers and read in
      // a "window" from the file and then disassemble that buffer.

        file.reset (fopen(filename->c_str(), "r"));
        if (!file)
        {
            string err("could not open: ");
            throw SystemError(err + filename->c_str());
        }
        ud_set_input_file(&ud_, file.get());
#else
      // ... But I just had THIS code sitting 'round :)
        Pipe pipe(false);

        const addr_t addr = startAddr;
        ostringstream cmd;

        cmd << "objdump -";
        cmd << (lookupSource ? "S" : "d");
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

        ExecArg arg(cmd.str());

        pid_t pid = ::exec("objdump", arg.strings(), pipe.input());
        const addr_t last = disasm_read(pid, pipe.output(), outputLine);

        const size_t length = labs(last - startAddr);
        return length;
#endif
    }

    ud_set_pc(&ud_, startAddr);
    int mode = (ELF::Binary(filename->c_str()).header().klass() == ELFCLASS64)
        ? 64 : 32;
    ud_set_mode(&ud_, mode);

    size_t total = 0;

    apply_syntax();

    while (unsigned len = ud_disassemble(&ud_))
    {
        total += len;

        if (outputLine)
        {
            addr_t addr = ud_insn_off(&ud_);

            if (lookupSource)
            {
                // display the source code associated with this assembly code
                if (const vector<string>* src = lookupSource->notify(addr, len))
                {
                    vector<string>::const_iterator i = src->begin();
                    for (; i != src->end(); ++i)
                    {
                        outputLine->notify(addr, i->c_str(), i->size());
                    }
                }
            }
            size_t tabs[2] = { 0, 0 };
            outputLine->tabstops(&tabs[0], &tabs[1]);
            //
            // format output
            //
            ostringstream line;
            line << hex << setw(2 * sizeof(addr_t)) << setfill('0');
            line << addr << ":\t";

            const string hexBytes = hex_bytes(ud_insn_hex(&ud_));
            line << hexBytes;

            if (const size_t padding = tabs[1] - tabs[0])
            {
                for (unsigned n = hexBytes.size(); n < padding; ++n)
                line << ' ';
            }
            line << '\t';
            switch (ud_.mnemonic)
            {
            case UD_Icall:
                print_call_or_jmp(lookupSymbol, addr + len, line);
                break;

            default:
                //if ((ud_.mnemonic >= UD_Ijcxz) && (ud_.mnemonic <= UD_Ijnle))
                if ((ud_.mnemonic >= UD_Ijcxz) && (ud_.mnemonic <= UD_Ijg))
                {
                    print_call_or_jmp(lookupSymbol, addr + len, line);
                }
                else
                {
                    line << ud_insn_asm(&ud_);
                }
                break;
            }

            if (!outputLine->notify(addr, line.str().c_str(), line.str().size()))
            {
                break;
            }
        }
        else if (lookupSource)
        {
            addr_t addr = ud_insn_off(&ud_);
            lookupSource->notify(addr, len);
        }
    }
    return total;
}



void Disasm86::print_call_or_jmp(SymbolCallback* symLookup, addr_t addr, ostream& out)
{
    out << ud_insn_asm(&ud_);

    if (symLookup)
    {
        addr += static_cast<int>(ud_.operand[0].lval.udword);

        /* if (ud_.operand[0].type == UD_OP_REG)
        {
            addr = ud_.operand[0].base;
        } */

        if (const Symbol* sym = symLookup->notify(addr, false))
        {
            out << " <" << sym->demangled_name(false) << "+0x" << sym->offset() << ">";
        }
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
