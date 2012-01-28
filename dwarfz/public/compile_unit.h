#ifndef COMPILE_UNIT_H__E577C0D2_1040_4528_A770_3B714CA837BC
#define COMPILE_UNIT_H__E577C0D2_1040_4528_A770_3B714CA837BC
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
#include <boost/enable_shared_from_this.hpp>
#include "interface.h"
#include "child.h"
#include "comp_str.h"
#include "die.h"
#include "funfwd.h"
#include "line_events.h"
#include "list.h"
#include "variable.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string.h"
#include "zdk/string_cache.h"
#include "zdk/weak_ptr.h"


namespace Dwarf
{
    class Block;
    class Debug;
    class Namespace;

    template<typename T> class VariableT;

    /**
     * Function object interface for enumerating macro info
     */
    struct MacroEvents
        : public std::unary_function<Dwarf_Macro_Details*, bool>
    {
        virtual ~MacroEvents() = 0;

        /**
         * @return false to cancel enumeration
         */
        virtual bool on_macro(const Dwarf_Macro_Details&) = 0;
    };


    /**
     * Models a compilation unit (aka translation unit);
     * wraps a Dwarf_Die of type DW_TAG_compile_unit.
     */
    CLASS CompileUnit : public Die
                      , public Child<Debug>
                      , public boost::enable_shared_from_this<CompileUnit>
    {
        CompileUnit(const CompileUnit&); // non-copyable
        CompileUnit& operator=(const CompileUnit&); // non-assignable

    #if !defined(HAVE_HASH_MAP)
    #error no hash_map!
        // map types by offset in the .debug section
        typedef std::map<Dwarf_Off,
            boost::shared_ptr<Type> > TypeMapByOffset;
        typedef std::map<const char*,
            boost::shared_ptr<Type>, StrLess> TypeMapByName;
    #else
        typedef ext::hash_map<Dwarf_Off,
            boost::shared_ptr<Type>,
            ext::identity<Dwarf_Off> > TypeMapByOffset;

        typedef ext::hash_map<const char*,
            boost::shared_ptr<Type>,
            StrHash,
            StrEqual> TypeMapByName;
    #endif
    public:
        enum { TAG = DW_TAG_compile_unit };

        friend class Debug;
        friend class IterationTraits<CompileUnit>;

        ~CompileUnit() throw();

        List<Namespace> namespaces() const;

        /**
         * Get the functions defined in this translation unit
         */
        const FunList& functions() const;

        void uncache_functions() const { funcs_.clear(); }

        /**
         * Lookup a function by address, and optionally
         * by linkage name
         */
        boost::shared_ptr<Function>
            lookup_function(Dwarf_Addr, const char* linkage = NULL) const;

        /**
         * Get a list of variables at file scope
         */
        const VarList& variables() const;

        /**
         * Lookup a type definition by its forward decl
         */
        boost::shared_ptr<Type> lookup_type(const Type& decl) const;

        /**
         * Lookup a type by name
         */
        boost::shared_ptr<Type> lookup_type(const char* name) const;

        /**
         * Returns the concatenation of the compilation
         * directory and the Die::name()
         */
        RefPtr<SharedString> full_path() const;

        const char* short_path() const { return Die::name(); }

        RefPtr<SharedString> build_path() const;

        const char* name() const { return CHKPTR(full_path())->c_str(); }

        RefPtr<SharedString> filename_by_index(size_t) const;
        RefPtr<SharedString> filename(const char*) const;

        size_t source_files_count() const;

        /**
         * Find the line number in the source where
         * a symbol is first defined; return the number
         * of matching lines. For each matching source
         * file and line, call SrcLineEvents::on_srcline
         * if last parameter is not NULL
         * if NEAREST is not NULL, fill it out with the
         * nearest match for ADDR.
         * @return number of matches.
         */
        size_t addr_to_line(
            Dwarf_Addr addr,
            Dwarf_Addr* nearest,
            SrcLineEvents*) const;

        Dwarf_Addr next_line(
            const char* file,
            size_t      line,
            Dwarf_Addr  addr,
            size_t*     next) const;

        bool cache_srclines(SrcLineEvents*);

        /**
         * low pc, as in DW_AT_lowpc, or zero if not present
         */
        Dwarf_Addr base_pc() const;

        /**
         * @return the lowest address in the range convered
         * by this compilation unit; if DW_AT_lowpc is not present,
         * determine the low PC value from the functions in this unit
         */
        Dwarf_Addr low_pc() const;

        /**
         * @return the highest address in the range convered
         * by this compilation unit
         * @note is DW_AT_highpc attribute is not present,
         * determine the high PC value from the functions
         * defined in this unit.
         */
        Dwarf_Addr high_pc() const;

        bool is_populated() const { return populated_; }

        const char* producer() const;

        size_t enum_macros(MacroEvents*, size_t maxCount) const;

        int language() const;

        /**
         * for D programming language; may return NULL
         */
        SharedString* module() const { return module_.get(); }

        RefPtr<StringCache> string_cache() const { return strCache_.lock(); }
        Dwarf_Unsigned next_unit() const { return next_; }

        static boost::shared_ptr<CompileUnit>
            next_unit(Dwarf_Debug, Dwarf_Unsigned);


        CLASS TypeEnumeration
        {
            boost::shared_ptr<CompileUnit> unit_;
            TypeMapByOffset::const_iterator iterByOffs_;
            TypeMapByName::const_iterator iterByName_;

        public:
            explicit TypeEnumeration(const boost::shared_ptr<CompileUnit>&);
            boost::shared_ptr<Type> next();
        }; // TypeEnumeration

        TypeEnumeration get_types()
        {
            if (!populated_) read_children();
            return TypeEnumeration(shared_from_this());
        }

    private:
        CompileUnit(Dwarf_Debug, Dwarf_Die, Dwarf_Unsigned, WeakPtr<StringCache>);

        void get_range_from_funcs() const;

        /**
         * Read functions, types, global vars, namespaces, etc
         * in this compilation unit.
         */
        void read_children(
            Dwarf_Die,
            const char* prefix = 0,
            bool ignoreVars = false) const;

        void read_children() const
        {
            assert(!populated_);
            read_children(this->die());
        }

        /**
         * Called by read_children
         */
        boost::shared_ptr<Die> process_entry(
            Dwarf_Die,
            Dwarf_Half,
            const char* prefix,
            bool ignoreVars = false) const;

        boost::shared_ptr<Die> add_struct(Dwarf_Die, const char* prefix) const;

        class LineInfoCache; // opaque
        const LineInfoCache& line_info_cache() const;

        ////////////////////////////////////////////////////////
        const Dwarf_Unsigned next_;
        mutable RefPtr<SharedString> fullpath_;
        mutable RefPtr<SharedString> buildpath_;

        mutable LineInfoCache* lineInfoCache_;
        mutable FunList funcs_;
        mutable std::vector<Dwarf_Off> funcOffs_;
        mutable bool populated_;

        mutable std::string producer_; // compiler info

        mutable Dwarf_Addr base_;
        mutable Dwarf_Addr lowPC_;
        mutable Dwarf_Addr highPC_;

        mutable TypeMapByOffset tspecs_; // type specifications
        mutable TypeMapByName types_;
        mutable VarList vars_;  // global variables in this unit

        mutable std::vector<RefPtr<SharedString> > srcFiles_;
        mutable RefPtr<SharedString> module_;
        WeakPtr<StringCache> strCache_;
    };


    /**
     * Iteration traits specialization for CompileUnit
     */
    template<> struct IterationTraits<CompileUnit>
    {
        typedef boost::shared_ptr<CompileUnit> ptr_type;

        /**
         * Obtain the first element in the list
         */
        static ptr_type first(Dwarf_Debug dbg, Dwarf_Die die);

        /**
         * Get the sibling of same type for a given element
         */
        static void next(ptr_type& elem);
    };


    /**
     * Lookup a type by name in the given unit
     * @todo: revisit debug.cpp
     */
    boost::shared_ptr<Type> lookup_type(const CompileUnit& unit,
                                        const char* name,
                                        size_t nameLength);
}


#endif // COMPILE_UNIT_H__E577C0D2_1040_4528_A770_3B714CA837BC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
