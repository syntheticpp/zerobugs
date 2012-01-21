//
// $Id: unwind.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <dwarf.h>
#include <string.h>
#include "error.h"
#include "abi.h"
#include "location.h"
#include "log.h"
#include "unwind.h"
#if DEBUG
  #define IF_DEBUG_FH(x) if (debug_fh()) { x; } else { }
#else
  #define IF_DEBUG_FH(x)
#endif

using namespace std;
using namespace Dwarf;


static inline bool debug_fh()
{
    static const bool debug = getenv("ZERO_DEBUG_FH");
    return debug;
}



/**
 * Print frame state
 */
static void print(ostream& os, const RegTable& table)
{
    IF_DEBUG_FH(
        os << "----- Frame Table -----\n";
        static const size_t n = ABI::user_reg_count();
        for (size_t i = 0; i != n; ++i)
        {
            os << i << ' ' << (i ? ABI::user_reg_name(i) : "CFA");
            os << ": " << table.regs[i].state << " ";
            os << hex << table.regs[i].value << dec << endl;
        }
    )
}


RegTable::RegTable()
{
    memset(regs, 0, sizeof regs);
}


static size_t rule_table_size()
{
    return max<size_t>(ABI::user_reg_count(), DW_REG_TABLE_SIZE);
}


Unwind::Unwind(Dwarf_Debug dbg)
    : dbg_(dbg)
    , cie_(0)
    , cieElemCount_(0)
    , fde_(0)
    , fdeElemCount_(0)
{
    Dwarf_Error err = 0;

    dwarf_set_frame_rule_table_size(dbg_, rule_table_size());

    // try the .debug_frame section first;
    if (dwarf_get_fde_list( dbg_,
                            &cie_,
                            &cieElemCount_,
                            &fde_,
                            &fdeElemCount_,
                            &err) == DW_DLV_ERROR)
    {
        cerr << Error::Message(dbg_, err) << " (.debug_frame)" << endl;
    }

    // then try the .eh_frame section
    if (!fde_)
    {
        assert(!cie_);
        assert(cieElemCount_ == 0);
        assert(fdeElemCount_ == 0);

        if (dwarf_get_fde_list_eh(dbg_,
                                  &cie_,
                                  &cieElemCount_,
                                  &fde_,
                                  &fdeElemCount_,
                                  &err) == DW_DLV_ERROR)
        {
            cerr << Error::Message(dbg_, err) << " (.eh_frame)" << endl;
        }

        IF_DEBUG_FH(
            clog << ".eh_frame: CIE elem count=" << cieElemCount_ << endl;
            clog << ".eh_frame: FDE elem count=" << fdeElemCount_ << endl;
        )
    }
}


Unwind::~Unwind() throw()
{
    dwarf_fde_cie_list_dealloc(dbg_, cie_, cieElemCount_, fde_, fdeElemCount_);
}



static Dwarf_Addr
get_reg(const RegTable& frame, AddrOps& ops, Dwarf_Half n)
{
    Dwarf_Addr addr = 0;
    if (frame.regs[n].state == RegTable::REG_READY)
    {
        addr = frame.regs[n].value;
    }
    else
    {
        addr = ops.read_cpu_reg(ABI::user_reg_index(n));
    }
    return addr;
}



////////////////////////////////////////////////////////////////
static Dwarf_Addr eval_loc(

    Dwarf_Debug             dbg,
    Dwarf_Addr              base,
    const Dwarf_Locdesc&    desc) throw()

{
    try
    {
        bool isVal = false;

        // okay to pass pc=0 (only needed for cfa computation
        // currently, which in this context would be circular
        // anyway)
        Dwarf_Addr addr(Location::eval(dbg, 0, base, 0, &desc, isVal));

        // do not expect values within frame expressions
        assert(!isVal);

        return addr;
    }
    catch (const exception& e)
    {
        clog << __func__ << ": " << e.what() << endl;
    }
    return 0;
}


////////////////////////////////////////////////////////////////
static Dwarf_Addr
eval_frame_expr(Dwarf_Debug dbg,
                Dwarf_Addr base,
                Dwarf_Ptr ptr,
                Dwarf_Unsigned len)
{
    Dwarf_Error err = NULL;
    Dwarf_Locdesc* desc = NULL;
    Dwarf_Signed count = 0;

    int r = dwarf_loclist_from_expr(dbg,
                                    ptr,
                                    len,
                                    &desc,
                                    &count,
                                    &err);

    if (r != DW_DLV_OK)
    {
        THROW_ERROR(dbg, err);
    }

    Dwarf_Addr result = 0;

    if (desc)
    {
        result = eval_loc(dbg, base, *desc);

        dwarf_dealloc(dbg, desc->ld_s, DW_DLA_LOC_BLOCK);
        dwarf_dealloc(dbg, desc, DW_DLA_LOCDESC);
    }
    return result;
}


/**
 * Debugging utility.
 */
static void
print(ostream& os, Dwarf_Addr pc, const Dwarf_Regtable3& table)
{
    IF_DEBUG_FH(
        os << "##### Dwarf_RegTable " << hex << pc << dec << " #####\n";

        for (size_t i = 0; i != ABI::user_reg_count(); ++i)
        {
            os << i << ' ' << (i ? ABI::user_reg_name(i) : "CFA") << ' ';

            os << (int)table.rt3_rules[i].dw_offset_relevant << ' ';
            os << table.rt3_rules[i].dw_regnum << ' ';
            os << (long)table.rt3_rules[i].dw_offset_or_block_len << endl;
        }
        os << flush;
    )
}


/**
 * Compute register number N based on registers tables
 * derived from either .frame_debug or .eh_frame
 * @note the semantics of N are platform-dependent.
 */
static Dwarf_Addr
get_reg(Dwarf_Debug             dbg,
        const Dwarf_Regtable3&  regs,
        Dwarf_Addr              base,
        RegTable&               frame,
        Dwarf_Half              n,
        AddrOps&                ops)
{
    assert(n != DW_FRAME_CFA_COL3);

    if (n >= ABI::user_reg_count())
    {
        ostringstream msg;
        msg << __func__ << ": regnum out of range: " << n;
        throw invalid_argument(msg.str());
    }
    Dwarf_Addr addr = 0;
    Dwarf_Regtable_Entry3& entry = regs.rt3_rules[n];
    Dwarf_Half regnum = entry.dw_regnum;
    IF_DEBUG_FH(
        clog << __func__ << " " << ABI::user_reg_name(n);
        clog << ": " << regnum << endl;
    )

    if (entry.dw_block_ptr)
    {
        regnum = 0; // use the expression block rather than regnum
    }

    switch (regnum)
    {
    case DW_FRAME_SAME_VAL:
        if (frame.regs[n].state == RegTable::REG_READY)
        {
            addr = frame.regs[n].value;
        }
        else
        {
            addr = ops.read_cpu_reg(ABI::user_reg_index(n));
        }
        break;

    case DW_FRAME_UNDEFINED_VAL:
        break;

    default:
        switch (entry.dw_value_type)
        {
        case DW_EXPR_OFFSET:
            if (entry.dw_offset_relevant)
            {
                const Dwarf_Addr offs = entry.dw_offset_or_block_len;

                IF_DEBUG_FH(
                    clog << __func__ << ": [" << hex << base << dec;
                    clog << " + " << static_cast<long>(offs) << "]" << endl;
                )
                addr = ops.read_mem(base + offs);
            }
            else
            {
                addr = get_reg(frame, ops, regnum);
            }
            break;

        case DW_EXPR_VAL_OFFSET:
            throw runtime_error("DW_EXPR_VAL_OFFSET: not implemented");
            break;

        case DW_EXPR_EXPRESSION:
            assert(get_addr_operations());
            addr = eval_frame_expr(dbg,
                                   base,
                                   entry.dw_block_ptr,
                                   entry.dw_offset_or_block_len);
            addr = ops.read_mem(addr);
            break;

        case DW_EXPR_VAL_EXPRESSION:
            assert(get_addr_operations());
            addr = eval_frame_expr(dbg,
                                   base,
                                   entry.dw_block_ptr,
                                   entry.dw_offset_or_block_len);
            break;

        default:
            {
                ostringstream msg;
                msg << "invalid dw_value_type: " << entry.dw_value_type;
                throw runtime_error(msg.str());
            }
        }
        break;
    }
    frame.regs[n].state = RegTable::REG_READY;
    return (frame.regs[n].value = addr);
}



/**
 * Read the reg table at the given program counter.
 * @return true on success, false if there's no FDE or table
 * that corresponds to given PC
 */
static bool
get_dwarf_regtable(Dwarf_Debug      dbg,
                   Dwarf_Fde*       fdes,
                   Dwarf_Addr       pc,
                   Dwarf_Regtable3& regs)
{
    if (!fdes)
    {
        return false; // no Frame Descriptors available
    }
    Dwarf_Error err = 0;
    Dwarf_Addr lopc = 0, hipc = 0;
    Dwarf_Fde fde = 0;

    int r = dwarf_get_fde_at_pc(fdes, pc, &fde, &lopc, &hipc, &err);
    if (r == DW_DLV_NO_ENTRY)
    {
        IF_DEBUG_FH(clog << __func__ << " pc=" << hex << pc << dec
                         << ": " << Error::Message(dbg, err) << endl);
        return false;
    }
    if (r != DW_DLV_OK)
    {
        assert(r == DW_DLV_ERROR);
        THROW_ERROR(dbg, err);
    }
    r = dwarf_get_fde_info_for_all_regs3(fde, pc, &regs, NULL, &err);

    if (r != DW_DLV_OK)
    {
        assert(r == DW_DLV_ERROR);
        THROW_ERROR(dbg, err);
    }
    return true;
}


static Dwarf_Addr
compute_cfa(const RegTable& frame,
            const Dwarf_Regtable3& regs,
            AddrOps& addrOps)
{
    // calculate the Canonical Frame Address
    const Dwarf_Half regnum = regs.rt3_cfa_rule.dw_regnum;

    IF_DEBUG_FH( clog << __func__ << ": " << "cfa_rule.dw_regnum=" << regnum << endl; )
    if (regnum == DW_FRAME_SAME_VAL) // initialization issues?
    {
        return 0;
    }
    assert(regnum < regs.rt3_reg_table_size);
    assert(regnum < ABI::user_reg_count());

    Dwarf_Addr cfa = 0;
    // get the saved registers from the input frame, if ready
    if (frame.regs[regnum].state == RegTable::REG_READY)
    {
        cfa = frame.regs[regnum].value;
    }
    else
    {   // otherwise read it from the current thread's state
        cfa = addrOps.read_cpu_reg(ABI::user_reg_index(regnum));
        IF_DEBUG_FH( clog << __func__ << ": " << hex << cfa << dec << endl; )
    }
    if (regs.rt3_cfa_rule.dw_offset_relevant)
    {
        const Dwarf_Unsigned len = regs.rt3_cfa_rule.dw_offset_or_block_len;
        cfa += len;

        IF_DEBUG_FH(
            clog << __func__ << ": dw_offset_or_block_len=" << len;
            clog << ", cfa=" << hex << cfa << dec << endl;
        )
    }
    return cfa;
}


/**
 * Compute the CFA at the local program count.
 * For supporting DW_OP_call_frame_cfa:
 */
Dwarf_Addr Unwind::compute_cfa(Dwarf_Addr pc, AddrOps& ops)
{
    Dwarf_Addr cfa = 0;

    if (fde_)
    {
        Dwarf_Regtable3 regs;
        memset(&regs, 0, sizeof regs);

        regs.rt3_reg_table_size = rule_table_size();

        Dwarf_Regtable_Entry3 rules[regs.rt3_reg_table_size];
        regs.rt3_rules = rules;

        if (get_dwarf_regtable(dbg_, fde_, pc, regs))
        {
            RegTable frame; // discard
            cfa = ::compute_cfa(frame, regs, ops);
        }
    }
    return cfa;
}


/* Algorithm:
 1) calculate the Canonical Frame Address, and
 2) based on the CFA, calculate the return address (i.e.
    the program counter that was saved on the stack);
 3) compute the stack pointer and frame pointer of the
    previous frame.
 */
Dwarf_Addr
Unwind::step(Dwarf_Addr pc, AddrOps& addrOps, RegTable& frame)
{
    if (!fde_)
    {
        return 0;
    }

    Dwarf_Regtable3 regs;
    memset(&regs, 0, sizeof regs);

    regs.rt3_reg_table_size = rule_table_size();

    Dwarf_Regtable_Entry3 rules[regs.rt3_reg_table_size];
    regs.rt3_rules = rules;

    if (!get_dwarf_regtable(dbg_, fde_, pc, regs))
    {
        return 0;
    }
    print(clog, pc, regs);
    print(clog, frame);

    // calculate the Canonical Frame Address
    Dwarf_Addr cfa = ::compute_cfa(frame, regs, addrOps);

    // get the return address, based on the CFA
    Dwarf_Addr retAddr = get_reg(dbg_,
                                 regs,
                                 cfa,
                                 frame,
                                 ABI::user_reg_pc(),
                                 addrOps);
    if (retAddr)
    {
        IF_DEBUG_FH(
            clog << __func__ << ": ret_addr=" << hex << retAddr << dec << endl
        )
        // calculate the frame pointer at the previous call site
        if (regs.rt3_rules[ABI::user_reg_fp()].dw_regnum != DW_FRAME_SAME_VAL)
        {
            get_reg(dbg_, regs, cfa, frame, ABI::user_reg_fp(), addrOps);
        }
        // calculate the stack pointer at the previous call site
        if (regs.rt3_rules[ABI::user_reg_sp()].dw_regnum == DW_FRAME_SAME_VAL)
        {
            frame.regs[ABI::user_reg_sp()].value = cfa;
        }
        else
        {
            get_reg(dbg_, regs, cfa, frame, ABI::user_reg_sp(), addrOps);
        }
    }
    if (retAddr == pc) // make sure we don't loop indefinitely
    {
        IF_DEBUG_FH(
            clog << __func__ << ": retAddr == pc\n"
        )
        retAddr = 0;
    }
    return retAddr;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
