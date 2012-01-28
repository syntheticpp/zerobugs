#ifndef INLINED_INSTANCE_H__9FD250AF_9081_47B1_B392_7C45CF2AF62C
#define INLINED_INSTANCE_H__9FD250AF_9081_47B1_B392_7C45CF2AF62C
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

#include "block.h"
#include "parameter.h"

namespace Dwarf
{
    class Function;

    /**
     * Represents an inline expansion of an inlineable subroutine --
     * with a set of concrete inlined instance entries that are its
     * children.
     * @todo rename InlinedSubroutine to be consistent with the DWARF
     * die that it wraps?
     */
    CLASS InlinedInstance : public Block
    {
    public:
        enum { TAG = DW_TAG_inlined_subroutine };

        typedef Block parent_type;
        typedef VariableT<InlinedInstance> Var;
        typedef List<Parameter> ParamList;
        typedef List<Var> VarList;

        friend class IterationTraits<InlinedInstance>;

        virtual ~InlinedInstance() throw() {}

        boost::shared_ptr<Function> function() const;

        List<Parameter> params() const;

        List<Var> variables() const;

        /**
         * @return the index of the source file of the call site.
         * Use CompileUnit::filename_by_index to translate it into
         * a string.
         */
        size_t call_file() const;

        /**
         * @return the source line number of the call site.
         */
        size_t call_line() const;

    protected:
        InlinedInstance(Dwarf_Debug, Dwarf_Die);
    };
}

#endif // INLINED_INSTANCE_H__9FD250AF_9081_47B1_B392_7C45CF2AF62C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
