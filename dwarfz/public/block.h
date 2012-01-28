#ifndef BLOCK_H__322EDDB7_0AE5_4951_9E36_B15A92622BAF
#define BLOCK_H__322EDDB7_0AE5_4951_9E36_B15A92622BAF
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
#include <memory>
#include <vector>
#include "interface.h"
#include "child.h"
#include "die.h"
#include "list.h"
#include "variable.h"

namespace Dwarf
{
    template<Dwarf_Half tag> class BlockT;

    typedef BlockT<DW_TAG_lexical_block> LexicalBlock;
    typedef BlockT<DW_TAG_try_block> TryBlock;
    typedef BlockT<DW_TAG_catch_block> CatchBlock;

    class InlinedInstance;

    typedef std::vector< boost::shared_ptr<InlinedInstance> > InlinedBlocks;

    /**
     * Base wrapper for:
     * DW_TAG_catch_block,
     * DW_TAG_lexical_block,
     * DW_TAG_try_block and
     * DW_TAG_subprogram
     */
    CLASS Block : public Die, public Child<Block>
    {
    public:
        friend class IterationTraits<LexicalBlock>;
        friend class IterationTraits<TryBlock>;
        friend class IterationTraits<CatchBlock>;

        virtual ~Block() throw() {}

        /**
         * @return the lowest program counter value
         * associated with this block
         * @todo add ranges support
         */
        virtual Dwarf_Addr low_pc() const;

        /**
         * @return the highest program counter value
         * associated with this block
         * @todo add ranges support
         */
        virtual Dwarf_Addr high_pc() const;

        /*
        virtual Dwarf_Addr frame_base(
            Dwarf_Addr moduleBase,
            Dwarf_Addr frameBase,
            Dwarf_Addr pc) const;
        */
        /**
         * @return a list of variables at the
         * topmost scope within this block
         * @todo rename to toplevel_variables?
         */
        const VarList& variables() const;

        /**
         * recursively dive into nested blocks, and return
         * all variables encompassed by this block.
         */
        VarList all_variables() const;

        /**
         * @return a list of nested blocks
         */
        List<Block> blocks() const;

        const InlinedBlocks& inlined_blocks() const;

    protected:
        Block(Dwarf_Debug, Dwarf_Die);

    private:
        mutable std::auto_ptr<InlinedBlocks> inlinedBlocks_;
        mutable std::auto_ptr<VarList> vars_;
    };


    template<> struct IterationTraits<Block>
    {
        typedef boost::shared_ptr<Block> ptr_type;

        /**
         * Obtain the first element in the list
         */
        static ptr_type first(Dwarf_Debug dbg, Dwarf_Die die);

        /**
         * Get the sibling of same type for a given element
         */
        static void next(ptr_type& elem);
    };


    template<Dwarf_Half tag>
    CLASS BlockT : public Block
    {
    public:
        enum { TAG = tag };

        friend class IterationTraits<Block>;
        friend class IterationTraits<BlockT>;

    protected:
        BlockT(Dwarf_Debug dbg, Dwarf_Die die) : Block(dbg, die)
        { }
    };
}
#endif // BLOCK_H__322EDDB7_0AE5_4951_9E36_B15A92622BAF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
