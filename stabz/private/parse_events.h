#ifndef PARSE_EVENTS_H__F72A3528_EAD6_4E7E_8B9F_B3BBF17E7D5E
#define PARSE_EVENTS_H__F72A3528_EAD6_4E7E_8B9F_B3BBF17E7D5E
//
// $Id: parse_events.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <deque>
#include <iosfwd>
#include "public/descriptor.h"
#include "public/type_id.h"
#include "private/parse_state.h"

class ClassTypeImpl;
class FunType;
class TypeSystem;


namespace Stab
{
    typedef std::pair<int64_t, uint64_t> IntRange;

    class Block;
    class Function;
    class Parameter;
    class Variable;

    /**
     * on_stab() and on_section() event handler.
     * on_section() is currently a no-op.
     * on_stab() parses LSYM, PSYM, RSYM and other stab
     * type entries and builds the type tables associated
     * with the compilation units.
     * @see Stab::InitEvents
     * @see Stab::Descriptor::parse
     * @see Stab::Descriptor::init_and_parse
     */
    class ParseEvents : public Events
    {
    public:
        ParseEvents(TypeSystem&, Descriptor&);

        ~ParseEvents() throw();

    private:
        //
        // Events interface
        //
        void on_section(const char*);
        void on_begin(SharedString&, const char*, size_t);
        bool on_stab(size_t, const stab_t&, const char*, size_t);
        void on_done(size_t);

        //
        // Implementation details
        //
        CompileUnit* current_unit() const
        {
            register CompileUnit* unit = unit_.get();
            assert(unit);
            return unit;
        }

        /* Called by on_stab for entries of the N_FUN type. */
        void on_func(size_t, const stab_t&, const char*, size_t);

        void finish_func();

        /* Dump parser state stack for diagnostics & debugging */
        std::ostream& dump_stack(std::ostream&) const;

        /**
         * Helper function used internally in debug macros:
         * pops the state on the top of the stack, if the
         * value is true, return the value.
         */
        bool pop_state(bool value);

        bool parse(const stab_t&, const char*, size_t);

        bool parse_type(const char*&, TypeID&, SharedString* = 0);
        bool parse_type(const char*&, TypeID&, RefPtr<DataType>&);

        bool parse_array(const char*&, const TypeID&, SharedString*);

        /* Called by parse_class */
        bool parse_base_classes(
                const char*&, const TypeID&, SharedString*);

        bool parse_class(
                const char*&,
                const TypeID&,
                SharedString*,
                bool isUnion);

        bool parse_constant(const char*&, SharedString*);

        /**
         * Parse C++ special members such as the .vptr and .vtbl
         */
        /*
        bool parse_cplus(
                const char*&,
                const TypeID&,
                TypeID&,
                Platform::bitsize_t&,
                const RefPtr<SharedString>&); */
        bool parse_enum(
                const char*&,
                const TypeID&,
                unsigned int nbytes,
                SharedString*);

        bool parse_global(
                const stab_t&,
                const char*&,
                TypeID&,
                SharedString*);

        /**
         * Parse a struct, union, or class member data,
         * or member method. Updates position in string,
         * and fills out isFunction.
         */
        bool parse_member(
                const char*&,
                const TypeID& classTypeID,
                bool& isFunction);

        bool parse_mem_fun(
            const char*&, TypeID&, SharedString*);

        bool parse_pointer(
                const char*,
                const char*&,
                const TypeID&,
                SharedString*);

        bool parse_fun_type(
                const char*&,
                const TypeID&,
                SharedString*,
                bool  isStatic = true);

        bool parse_fwd_type(
            const char*, const char*&, const TypeID&);

        void parse_negative_types(
                const char*,
                const TypeID&,
                SharedString*,
                size_t);

        /**
         * Parse (Sun extension) builtin floating
         * point types.
         */
        bool parse_builtin_fp_type(
                const char*&,
                const TypeID&,
                SharedString*);

        bool parse_qualified_type(
                const char*&,
                TypeID&,
                SharedString*,
                int);

        bool parse_range_type(
                const char*&,
                const TypeID&,
                SharedString*);

        bool parse_range(const char*&, IntRange& result);

        void add_local_var(
                const stab_t&,
                const TypeID&,
                SharedString&);

        /**
         * Construct a function type and add it to the
         * compilation unit type tables. If a class debug
         * info is in process, then also add a member function
         * to the class.
         */
        RefPtr<FunType> add_fun_type(
                const TypeID*           typeID,
                SharedString*           name,
                const char*             linkageName,
                const RefPtr<DataType>& retType,
                const ParamTypes*       paramTypes,
                Access              = ACCESS_PUBLIC,
                bool isVirtual      = false,
                Qualifier           = QUALIFIER_NONE,
                long virtFunIndex   = 0);

    private:
        typedef std::vector<ParseState> StateStack;
        typedef std::vector<RefPtr<Variable> > VarList;
        typedef std::deque<RefPtr<Block> > Blocks;

        typedef std::vector<RefPtr<ClassTypeImpl> > ClassStack;

        TypeSystem&         types_;
        Descriptor&         desc_;
        StateStack          stack_; // for debugging

        RefPtr<CompileUnit> unit_;  // Current compilation unit

        RefPtr<Function>    func_;  // Current function

        Blocks              blocks_;// lexical blocks

        VarList             param_;

        /**
         * pending variables -- N_LSYM etc. appear before
         * the N_LBRAC of their scope block
         */
        VarList             vars_;

        RefPtr<ClassTypeImpl> klass_; // current class, if any
        ClassStack          klassStack_;
    };
} // namespace Stab

#endif // PARSE_EVENTS_H__F72A3528_EAD6_4E7E_8B9F_B3BBF17E7D5E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
