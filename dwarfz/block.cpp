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

#include <dwarf.h>
#include "error.h"
#include "location.h"
#include "utils.h"
#include "variable.h"
#include "impl.h"
#include "inlined_instance.h"
#include "private/generic_attr.h"

using namespace std;
using namespace Dwarf;


Block::Block(Dwarf_Debug dbg, Dwarf_Die die) : Die(dbg, die)
{
}


Dwarf_Addr Block::low_pc() const
{
    Dwarf_Addr  pc = 0;
    Dwarf_Error err = 0;

    if (Utils::has_attr(dbg(), die(), DW_AT_low_pc))
    {
        if (dwarf_lowpc(die(), &pc, &err) == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg(), err);
        }
    }
    return pc;
}


Dwarf_Addr Block::high_pc() const
{
    Dwarf_Addr  pc = 0;
    Dwarf_Error err = 0;

    if (Utils::has_attr(dbg(), die(), DW_AT_high_pc))
    {
        if (dwarf_highpc(die(), &pc, &err) == DW_DLV_ERROR)
        {
            THROW_ERROR(dbg(), err);
        }
    }

  // TODO: this is just a hack until DW_AT_ranges
  // and .debug_ranges are fully supported
    else if (Utils::has_attr(dbg(), die(), DW_AT_ranges))
    {
        pc = (Dwarf_Addr)-1;
    }
    return pc;
}




const InlinedBlocks& Block::inlined_blocks() const
{
    if (inlinedBlocks_.get() == 0)
    {
        inlinedBlocks_.reset(new InlinedBlocks);

        List<InlinedInstance> blocks =
            List<InlinedInstance>(dbg(), die());

        List<InlinedInstance>::const_iterator i = blocks.begin(),
                                            end = blocks.end();
        for (; i != end; ++i)
        {
            inlinedBlocks_->push_back(i);
        }
    }
    return *inlinedBlocks_;
}


List<Block> Block::blocks() const
{
    return List<Block>(this->dbg(), this->die());
}


std::shared_ptr<Block>
IterationTraits<Block>::first(Dwarf_Debug dbg, Dwarf_Die die)
{
    std::shared_ptr<Block> p =
        IterationTraits<LexicalBlock>::first(dbg, die);

    if (!p)
    {
        p = IterationTraits<TryBlock>::first(dbg, die);
    }
    return p;
}


/**
 * Get the sibling of same type for a given element
 */
void
IterationTraits<Block>::next(std::shared_ptr<Block>& elem)
{
    assert(elem);
    std::shared_ptr<Block> tmp(elem);
    IterationTraits<LexicalBlock>::next(elem);
    if (!elem)
    {
        elem = tmp;
        IterationTraits<TryBlock>::next(elem);
    }
    if (!elem)
    {
        elem = tmp;
        IterationTraits<CatchBlock>::next(elem);
    }
}


const Dwarf::VarList& Block::variables() const
{
    if (vars_.get() == 0)
    {
        vars_.reset(new VarList);

        List<VariableT<Block> > v(dbg(), die());
        List<VariableT<Block> >::const_iterator i = v.begin(),
                                              end = v.end();
        for (; i != end; ++i)
        {
            vars_->push_back(i);
        }
    }
    return *vars_;
}


Dwarf::VarList Block::all_variables() const
{
    Dwarf::VarList vars = variables();

    List<Block> nestedBlocks = this->blocks();
    List<Block>::const_iterator i = nestedBlocks.begin();
    List<Block>::const_iterator end = nestedBlocks.end();
    for (; i != end; ++i)
    {
        VarList tmp = (*i).all_variables();
        vars.insert(vars.end(), tmp.begin(), tmp.end());
    }
    return vars;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
