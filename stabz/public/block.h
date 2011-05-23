#ifndef BLOCK_H__F76096AD_417A_419A_AC35_21B1D50E97AE
#define BLOCK_H__F76096AD_417A_419A_AC35_21B1D50E97AE
//
// $Id: block.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <algorithm>
#include <vector>
#include "zdk/platform.h"
#include "zdk/shared_string.h"
#include "zdk/zobject_impl.h"

#include "stabz/public/object.h"

using Platform::addr_t;

namespace Stab
{
    class Variable;

    /**
     * Represents a block of code in the source.
     * The beginning of a block is marked by a N_LBRAC
     * stab, end the end by a N_RBRAC. The values of
     * the stabs are the beginning and ending addresses.
     * For stabs in sections, they are relative to the
     * function in which they occur.
     */
    class Block : public ZObjectImpl<Object>
    {
    public:
        /* List of variables defined within the
           scope of this block. */
        typedef std::vector<RefPtr<Variable> > VarList;

        /* List of children, nested blocks */
        typedef std::vector<RefPtr<Block> > BlockList;

        Block(addr_t beginAddr, size_t beginIndex);

        Block(addr_t beginAddr, size_t beginIndex, VarList&);

        virtual ~Block() throw();

        addr_t begin_addr() const;
        addr_t end_addr() const;

        void set_end_addr(addr_t);

        size_t begin_index() const;
        size_t end_index() const;

        void set_end_index(size_t);

        /* Assign a list of variables to this block.*/
        virtual void assign_variables(const VarList&);

        void add_child(const RefPtr<Block>&);

        const VarList& variables() const { return vars_; }

        const BlockList& blocks() const { return blocks_; }

        template<typename T> T&
        for_each_block(T& pred) const
        {
            for (BlockList::const_iterator i = blocks_.begin(); i != blocks_.end(); ++i)
            {
                pred(*i);
            }
            return pred;
        }

        template<typename T> T&
        for_each_var(T& pred) const
        {
            for (VarList::const_iterator i = vars_.begin(); i != vars_.end(); ++i)
            {
                pred(*i);
            }
            return pred;
        }

    private:
        const addr_t    beginAddr_;
        addr_t          endAddr_;

        /*  Index of stab where this block begins
            and ends, respectively. */
        const size_t    beginIndex_;
        size_t          endIndex_;

        BlockList       blocks_;
        VarList         vars_;
    };
}

#endif // BLOCK_H__F76096AD_417A_419A_AC35_21B1D50E97AE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
