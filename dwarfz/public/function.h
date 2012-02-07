#ifndef FUNCTION_H__71E651D5_A44A_4F71_B085_6CA7695873DF
#define FUNCTION_H__71E651D5_A44A_4F71_B085_6CA7695873DF
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
#include "access.h"
#include "block.h"
#include "list.h"
#include "parameter.h"
#include "funfwd.h"
#include "interface.h"

namespace Dwarf
{
    class CompileUnit;

    enum CallingConvention
    {
        CC_normal = DW_CC_normal,
        CC_program = DW_CC_program,
        CC_nocall = DW_CC_nocall,
    };


    /**
     * Wraps a Dwarf_Die of the DW_TAG_subprogram type
     */
    CLASS Function : public Block
    {
    public:
        enum { TAG = DW_TAG_subprogram };

        typedef CompileUnit parent_type;

        typedef std::vector<std::shared_ptr<Parameter> > ParamList;

        friend class IterationTraits<Function>;
        friend class CompileUnit;

        Function(
            Dwarf_Debug,
            Dwarf_Die,
            Dwarf_Addr lowPC = 0,
            Dwarf_Addr highPC = 0);

        ~Function() throw() {}

        RefPtr<SharedString> linkage_name() const;

        /**
         * Return the function's parameters
         */
        const ParamList& params() const;

        std::shared_ptr<Type> ret_type() const;

        const TypeList& param_types() const;

        /**
         * Get the compilation unit where this function lives
         */
        const CompileUnit* unit() const { return unit_; }

        /**
         * Return true if the other function has the same
         * return type and parameter types
         */
        bool compare_prototype(const Function&) const;

        bool has_variable_args() const;

        void set_range(Dwarf_Addr low, Dwarf_Addr high)
        {
            assert(lowPC_ == 0);
            assert(highPC_ == 0);
            lowPC_ = low, highPC_ = high;
        }

        virtual Dwarf_Addr low_pc() const;

        virtual Dwarf_Addr high_pc() const;

        virtual Dwarf_Addr frame_base(
            Dwarf_Addr moduleBase,
            Dwarf_Addr frameBase,
            Dwarf_Addr pc) const;

        bool inline_not_inlined() const;

        CallingConvention calling_convention() const;

    protected:
        virtual char* name_impl() const;

    private:
        void cache_params() const;

        const CompileUnit* unit_;

        mutable RefPtr<SharedString> linkageName_;
        mutable std::auto_ptr<TypeList> paramTypes_;
        mutable std::auto_ptr<ParamList> params_;
        mutable bool isMemFun_; // is member of a class?

    private:
        mutable Dwarf_Addr lowPC_;
        mutable Dwarf_Addr highPC_;
    };
}

#endif // FUNCTION_H__71E651D5_A44A_4F71_B085_6CA7695873DF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

