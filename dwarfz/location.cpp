//
// $Id: location.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <dwarf.h>
#include <pthread.h>
#include "public/error.h"
#include "public/addr_ops.h"
#include "public/abi.h"
#include "private/log.h"
#include "public/impl.h"
#include "public/location.h"
#include "generic/lock.h"

#ifdef __func__
 #undef __func__
#endif
#define DBG Dwarf::log<debug>(1)

using namespace std;
using namespace Dwarf;
using namespace Dwarf::ABI;


namespace
{
    /**
     * Stack for evaluating location operations
     */
    CLASS Stack : public stack<Dwarf_Addr>
    {
    public:
        void push(Dwarf_Addr addr)
        {
            stack<Dwarf_Addr>::push(addr);
            DBG << "PUSH " << hex << addr << dec << "\n";
        }

        void pop()
        {
            stack<Dwarf_Addr>::pop();
            DBG << "POP\n";
        }
    };
}

static pthread_key_t addrOpsKey;

static void delete_key(void*)
{
    clog << __func__ << endl;
    pthread_key_delete(addrOpsKey);
}

static int init = pthread_key_create(&addrOpsKey, delete_key);


/**
 * note: the client must ensure thread-safety
 */
AddrOps* Dwarf::set_addr_operations(AddrOps* ops)
{
    AddrOps* prev = (AddrOps*)pthread_getspecific(addrOpsKey);
    pthread_setspecific(addrOpsKey, ops);
    return prev;
}


AddrOps* Dwarf::get_addr_operations()
{
    AddrOps* addrOps = (AddrOps*)pthread_getspecific(addrOpsKey);
    assert(addrOps);

    return addrOps;
}


Location::Location(Dwarf_Debug dbg, Dwarf_Attribute attr)
    : dbg_(dbg), list_(0), size_(0)
{
    assert(dbg);
    Dwarf_Error err = 0;

    if (dwarf_loclist_n(attr, &list_, &size_, &err) == DW_DLV_ERROR)
    {
        THROW_ERROR(dbg, err);
    }
}


Location::~Location()
{
    if (list_)
    {
        for (Dwarf_Signed i(0); i != size_; ++i)
        {
            dwarf_dealloc(dbg_, list_[i]->ld_s, DW_DLA_LOC_BLOCK);
            dwarf_dealloc(dbg_, list_[i], DW_DLA_LOCDESC);
        }
        dwarf_dealloc(dbg_, list_, DW_DLA_LIST);
    }
}


bool Location::is_register(Dwarf_Addr pc, Dwarf_Addr base) const
{
    bool result = false;
    if (list_)
    {
        assert(size_  > 0);

        for (Dwarf_Signed j = 0; (j < size_) && !result; ++j)
        {
            const Dwarf_Locdesc* desc = list_[j];
            if (pc < desc->ld_lopc + base)
            {
                continue;
            }
            // -1 means unlimited to the right
            if ((desc->ld_hipc != Dwarf_Addr(-1)) && (pc >= desc->ld_hipc + base))
            {
                continue;
            }
            if (desc->ld_cents == 1)
            {
                const Dwarf_Loc& loc = desc->ld_s[0];

                if (loc.lr_atom == DW_OP_regx)
                {
                    result = true;
                }
                else if (loc.lr_atom >= DW_OP_reg0 && loc.lr_atom < DW_OP_reg31)
                {
                    //const size_t nr = loc.lr_atom - DW_OP_reg0;
                    //if (nr < ABI::user_reg_count())
                    {
                        result = true;
                    }
                }
            }
        }
    }
    return result;
}


Dwarf_Addr Location::eval(Dwarf_Addr frame,
                          Dwarf_Addr moduleBase,
                          Dwarf_Addr unitBase,
                          Dwarf_Addr pc) const
{
    DBG << __func__ << " unit base=" << hex << unitBase << dec << "\n";

    Dwarf_Addr result = 0;

    if (list_)
    {
        assert(size_ > 0);
        for (Dwarf_Signed i = 0; i != size_; ++i)
        {
            const Dwarf_Locdesc* loc = list_[i];
            Dwarf_Addr lopc = loc->ld_lopc;
            if (loc->ld_lopc)
            {
                lopc += moduleBase + unitBase;
            }
            Dwarf_Addr hipc = loc->ld_hipc;
            if (hipc != static_cast<Dwarf_Addr>(-1))
            {
                hipc += moduleBase + unitBase;
            }

            DBG << "loc[" << i << "]=" << hex << lopc /* loc->ld_lopc */
                << "-" << hipc << " pc=" << pc << dec << "\n";

            if ((pc >= lopc) && (pc < hipc))
            {
                DBG << "evaluating loc[" << i << "]\n";
                result = eval(frame, moduleBase, loc);
                DBG << __func__ << "=" << hex << result << dec << "\n" ;
                break;
            }
        }
    }
    return result;
}


static void handle_empty_stack(const string& func)
{
    Dwarf::log<warn>() << func << ": stack empty" << "\n";
    if (/* const char* var = */ getenv("ZERO_DWARF_ABORT_EMPTY_STACK"))
    {
        abort();
    }
    throw runtime_error(func + ": state machine stack empty");
}


/**
 * The DW_OP_deref operation pops the top stack entry and
 * treats it as an address. The value retrieved from that
 * address is pushed
 */
static void op_deref(Stack& stack)
{
    AddrOps* addrOps = get_addr_operations();
    if (!addrOps)
    {
        //Dwarf::log<warn>() << __func__ << ": addrOps not set\n";
        throw runtime_error("op_deref: memory operations not set");
    }
    else
    {
        if (stack.empty())
        {
            handle_empty_stack(__func__);
        }
        else
        {
            DBG << "deref: " << hex << stack.top() << dec << "\n";
            Dwarf_Addr addr = addrOps->read_mem(stack.top());

            stack.pop();
            stack.push(addr);
        }
    }
}


/**
 * The DW_OP_plus_uconst operation pops the top stack entry, adds
 * it to the unsigned LEB128 constant operand and pushes the result
 */
static void op_plus_uconst(Stack& stack, Dwarf_Unsigned operand)
{
    if (stack.empty())
    {
        handle_empty_stack(__func__);
    }
    else
    {
        Dwarf_Addr addr = stack.top();
        DBG << hex << addr << "+=" << operand << dec << "\n";

        addr += operand;

        stack.pop();
        stack.push(addr);
    }
}


/**
 * The DW_OP_minus operations pops the top two stack values,
 * subtracts the former top of the stack from the former second
 * entry, and pushes the result.
 */
static void op_minus(Stack& stack)
{
    if (stack.size() < 2)
    {
        Dwarf::log<error>() << __func__ << ": not enough operands\n";
    }
    else
    {
        Dwarf_Addr op2 = stack.top();
        stack.pop();

        Dwarf_Addr op1 = stack.top();
        stack.pop();

        const Dwarf_Addr res = op1 - op2;

        DBG << hex << op1 << '-' << op2 << '=' << res << dec << "\n";
        stack.push(res);
    }
}


static void op_plus(Stack& stack)
{
    if (stack.size() < 2)
    {
        Dwarf::log<error>() << __func__ << ": not enough operands\n";
    }
    else
    {
        Dwarf_Addr op2 = stack.top();
        stack.pop();

        Dwarf_Addr op1 = stack.top();
        stack.pop();

        Dwarf_Addr result = op1 + op2;

        DBG << hex << op1 << "+" << op2
            << "=" << result << dec << "\n";
        stack.push(result);
    }
}


/**
 * Evaluate an address using a simple stack machine
 */
Dwarf_Addr
Location::eval(Dwarf_Addr frameBase,
               Dwarf_Addr moduleBase,
               const Dwarf_Locdesc* desc)
{
    AddrOps* addrOps = get_addr_operations();
    if (!addrOps)
    {
        throw runtime_error("eval: memory operations not set");
    }
    assert(desc);
    Stack stack;

    DBG << "moduleBase=" << hex << moduleBase << dec << "\n";
    if (frameBase)
    {
        DBG << "frameBase=" << hex << frameBase << dec << "\n";
        stack.push(frameBase);
    }

    for (size_t i(0); i != desc->ld_cents; ++i)
    {
        const Dwarf_Loc& loc = desc->ld_s[i];
        const int atom = loc.lr_atom;

        DBG << "opcode=0x" << hex << atom << dec << "\n";

        switch (atom)
        {
        case DW_OP_addr:
            DBG << "addr " << hex << loc.lr_number
                << " + " << moduleBase << dec << "\n";

            // adjust the operand to the address where
            // the module is loaded in memory

            stack.push(loc.lr_number + moduleBase);
            break;

        case DW_OP_lit0:
        case DW_OP_lit1:
        case DW_OP_lit2:
        case DW_OP_lit3:
        case DW_OP_lit4:
        case DW_OP_lit5:
        case DW_OP_lit6:
        case DW_OP_lit7:
        case DW_OP_lit8:
        case DW_OP_lit9:
        case DW_OP_lit10:
        case DW_OP_lit11:
        case DW_OP_lit12:
        case DW_OP_lit13:
        case DW_OP_lit14:
        case DW_OP_lit15:
        case DW_OP_lit16:
        case DW_OP_lit17:
        case DW_OP_lit18:
        case DW_OP_lit19:
        case DW_OP_lit20:
        case DW_OP_lit21:
        case DW_OP_lit22:
        case DW_OP_lit23:
        case DW_OP_lit24:
        case DW_OP_lit25:
        case DW_OP_lit26:
        case DW_OP_lit27:
        case DW_OP_lit28:
        case DW_OP_lit29:
        case DW_OP_lit30:
        case DW_OP_lit31:
            DBG << "literal: " << atom - DW_OP_lit0 << "\n";
            //
            // note: assume that literals from 0 thru 31 are
            // defined in contiguous, increasing order
            //
            stack.push(atom - DW_OP_lit0);
            break;

        case DW_OP_deref:
            op_deref(stack);
            break;

        case DW_OP_dup:
            DBG << "dup\n";
            //
            // DW_OP_dup duplicates the value at the top of the stack
            //
            if (stack.empty())
            {
                handle_empty_stack("DW_OP_dup");
            }
            else
            {
                stack.push(stack.top());
            }
            break;

        case DW_OP_reg0:
        case DW_OP_reg1:
        case DW_OP_reg2:
        case DW_OP_reg3:
        case DW_OP_reg4:
        case DW_OP_reg5:
        case DW_OP_reg6:
        case DW_OP_reg7:
        case DW_OP_reg8:
        case DW_OP_reg9:
        case DW_OP_reg10:
        case DW_OP_reg11:
        case DW_OP_reg12:
        case DW_OP_reg13:
        case DW_OP_reg14:
        case DW_OP_reg15:
        case DW_OP_reg16:
        case DW_OP_reg17:
        case DW_OP_reg18:
        case DW_OP_reg19:
        case DW_OP_reg20:
        case DW_OP_reg21:
        case DW_OP_reg22:
        case DW_OP_reg23:
        case DW_OP_reg24:
        case DW_OP_reg25:
        case DW_OP_reg26:
        case DW_OP_reg27:
        case DW_OP_reg28:
        case DW_OP_reg29:
        case DW_OP_reg30:
        case DW_OP_reg31:
            {
                const size_t nreg = atom - DW_OP_reg0;
                const int ureg = user_reg_index(nreg);
                DBG << "read_cpu_reg " << nreg << " " << ureg
                    << " (" << user_reg_name(nreg) << ")\n";

                const Dwarf_Addr addr = addrOps->read_cpu_reg(ureg);
                DBG << "read_cpu_reg=" << hex << addr << dec << "\n";
                stack.push(addr);
            }
            break;

        case DW_OP_regx:
            {
                const int ureg = user_reg_index(loc.lr_number);
                const Dwarf_Addr addr = addrOps->read_cpu_reg(ureg);
                DBG << "read_cpu_regx=" << hex << addr << dec << "\n";
                stack.push(addr);
            }
            break;

        case DW_OP_fbreg:
            //
            // The DW_OP_fbreg operation provides a signed offset from the
            // address specified by the location description in the
            // DW_AT_frame_base attribute of the current function.
            //
            DBG << "DW_OP_fbreg: " << hex << loc.lr_number << dec << "\n";

            stack.push(frameBase + loc.lr_number);
            break;

        case DW_OP_plus_uconst:
            op_plus_uconst(stack, loc.lr_number);
            break;

        case DW_OP_minus: op_minus(stack); break;
        case DW_OP_plus: op_plus(stack); break;

        case DW_OP_breg0:
        case DW_OP_breg1:
        case DW_OP_breg2:
        case DW_OP_breg3:
        case DW_OP_breg4:
        case DW_OP_breg5:
        case DW_OP_breg6:
        case DW_OP_breg7:
        case DW_OP_breg8:
        case DW_OP_breg9:
        case DW_OP_breg10:
        case DW_OP_breg11:
        case DW_OP_breg12:
        case DW_OP_breg13:
        case DW_OP_breg14:
        case DW_OP_breg15:
        case DW_OP_breg16:
        case DW_OP_breg17:
        case DW_OP_breg18:
        case DW_OP_breg19:
        case DW_OP_breg20:
        case DW_OP_breg21:
        case DW_OP_breg22:
        case DW_OP_breg23:
        case DW_OP_breg24:
        case DW_OP_breg25:
        case DW_OP_breg26:
        case DW_OP_breg27:
        case DW_OP_breg28:
        case DW_OP_breg29:
        case DW_OP_breg30:
        case DW_OP_breg31:
            {
                const size_t nreg = atom - DW_OP_breg0;
                const int ureg = user_reg_index(nreg);
                DBG << "DW_OP_breg" << nreg << "\n";
                DBG << "Read_cpu_reg " << nreg << " " << ureg
                    << " (" << user_reg_name(nreg) << ")\n";

                const Dwarf_Addr addr = addrOps->read_cpu_reg(ureg);
                DBG << "Read_cpu_reg=" << hex << addr << dec << "\n";

                DBG << "loc.lr_num=" << hex<< loc.lr_number << dec << "\n";
                stack.push(addr + loc.lr_number);
            }
            break;

        case DW_OP_piece:
            log<warn>() << __func__ << ": opcode=0x"
                        << hex << (int)loc.lr_atom << dec
                        << " operand=" << loc.lr_number << "\n";

            // todo: figure out how to handle this correctly
            if (loc.lr_number < sizeof (Dwarf_Addr))
            {
                assert(!stack.empty());
                Dwarf_Addr v = stack.top();
                clog << "stack_top=" << hex << v << dec << endl;
            }
            break;

        case DW_OP_const1u:
        case DW_OP_const2u:
        case DW_OP_const4u:
        case DW_OP_const8u:
        case DW_OP_constu:
            DBG << "const: " << hex << loc.lr_number << dec << "\n";
            stack.push(loc.lr_number);
            break;
        case DW_OP_const1s:
        case DW_OP_const2s:
        case DW_OP_const4s:
        case DW_OP_const8s:
        case DW_OP_consts:
            DBG << "const: " << (Dwarf_Signed)loc.lr_number << "\n";
            stack.push(loc.lr_number);
            break;

        default:
            log<warn>() << __func__ << ": unhandled opcode=0x"
                        << hex << (int)loc.lr_atom << dec << "\n";
            break;
        }
    }
    const Dwarf_Addr res = stack.empty() ? 0 : stack.top();
    DBG << __func__ << "=" << hex << res << dec << "\n";
    return res;
}


VTableElemLocation::VTableElemLocation(Dwarf_Debug dbg, Dwarf_Attribute attr)
    : Location(dbg, attr)
{
}


/**
 * GCC emits the index in the vtable.
 * Other compilers emit the absolute address in the vtable; in the latter
 * case, adapt the result to a GCC-like, index in the vtable. Assume that
 * 'base' is where the object begins, and that is where the .vptr
 */
Dwarf_Addr
VTableElemLocation::eval( Dwarf_Addr base,
                          Dwarf_Addr modBase,
                          Dwarf_Addr unitBase,
                          Dwarf_Addr pc
                        ) const
{
    Dwarf_Addr addr = Location::eval(base, modBase, unitBase, pc);

    if (addr >= base)
    {
        if (AddrOps* addrOps = get_addr_operations())
        {
            addr = addrOps->read_mem(base);
            addr = Location::eval(addr, modBase, unitBase, pc) - addr;
            addr /= sizeof(void*);
        }
        else
        {
            throw runtime_error("VTableElemLocation: memory operations not set");
        }
    }
    return addr;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
