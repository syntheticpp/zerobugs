// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if !defined(__i386__) && !defined(__x86_64__)
 #error wrong platform
#endif

#ifdef DEBUG
 #include <iostream>
 using namespace std;
#endif
#include "zdk/thread_util.h"
#include "zdk/zero.h"
#include "zdk/zobject_scope.h"
#include "elfz/public/binary.h"
#include "elfz/public/headers.h"
#include "jump.h"

/**
 * Check for PLT jmp opcodes, return destination address in DEST
 */
static bool is_jump32(Thread& thread, addr_t pc, addr_t& dest)
{
    word_t w = 0;
    size_t n = 0; // words read

    thread.read_code(pc, &w, 1, &n);

    if ((w & 0xffff) == 0x25FF)
    {
        thread.read_code(pc + 2, &w, 1, &n);
        dest = (w & 0xffffffff);

        return true;
    }

    return false;
}


/**
 * Check for PLT jmp opcodes, return destination address in DEST
 */
static bool is_jump64(Thread& thread, addr_t pc, addr_t& dest)
{
    word_t w = 0;
    size_t n = 0; // words read

    thread.read_code(pc, &w, 1, &n);

    if ((w & 0xffff) == 0x25FF)  // jmpq, RIP relative
    {
        dest = ((w >> 16) & 0xffffffff) + pc;
        return true;
    }

 /* this jmp opcode does not seem to be used by GCC PLTs
    if ((w & 0xff) == 0xe9) // JMP rel16
    {
        dest = ((w >> 8) & 0xffffffff) + pc;
        return true;
    } */
    return false;
}



/**
 * is instruction at current program counter a PLT/GOT jump?
 */
bool is_plt_jump(Thread& thread, const Symbol* sym, addr_t pc)
{
    ZObjectScope scope;
    if (!sym)
    {
    #if DEBUG
        clog << __func__ <<": null symbol at " << (void*)pc << endl;
    #endif
    }
    else if (SymbolTable* table = sym->table(&scope))
    {
        if (SharedString* filename = table->filename())
        {
            addr_t addr = 0; // addr of jump destination
            ELF::Binary bin(filename->c_str());

            switch (bin.header().machine())
            {
            case EM_386:
                if (!is_jump32(thread, pc, addr))
                {
                    return false;
                }
                break;

            case EM_X86_64:
                if (!is_jump64(thread, pc, addr))
                {
                    return false;
                }
                break;

            default:
                assert(false);
                return false;  // unknown machine type
            }

            bool result = false;
            size_t size = 0;

            addr_t plt = bin.get_plt_addr(&size) + table->adjustment();

            thread_read(thread, plt, plt);
            plt += table->adjustment();

        #if DEBUG
            //clog << __func__ << ": plt reloc=" << (void*)plt << " end=";
            //clog << (void*)(plt + size) << " addr=" << (void*)addr << endl;
        #endif

            // is the address inside the PLT relocs?
            if ((addr >= plt) && (addr < plt + size))
            {
                result = true;
            }
            return result;
        }
    }
    return false;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
