#ifndef COMPILE_UNIT_H__6E5B99DA_572A_44B7_80A6_36B81526BFEF
#define COMPILE_UNIT_H__6E5B99DA_572A_44B7_80A6_36B81526BFEF
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

#include <assert.h>
#include <map>
#include <vector>
#include "zdk/platform.h"
#include "zdk/translation_unit.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "dharma/object_manager.h"
#include "stabz/public/descriptor.h"
#include "stabz/public/object.h"
#include "stabz/public/type_id.h"
#include "stabz/public/type_table.h"


using Platform::addr_t;

class MethodImpl;
class SharedStringImpl;
class TypeSystem;

namespace ELF
{
    class Binary;
}

namespace Stab
{
    class Function;
    class Variable;

    /**
     * Information about a compilation unit, obtained from
     * the .stabs section of a binary file.
     */
    class CompileUnit : public RefCountedImpl<TranslationUnit>
                      , public CountedInstance<CompileUnit>
    {
        /* Functions, "indexed" by their start address.
           Please NOTE this is the symbol table address,
           and not the virtual memory address. */
        typedef std::map<addr_t, RefPtr<Function> > FunctionMap;

        /* map addresses to a (source, line) pair */
        typedef std::multimap<
                addr_t,
                std::pair<RefPtr<SharedString>, size_t> > LineByAddr;

        /* map the mangled member function names to their classes */
        typedef ext::hash_map<RefPtr<SharedString>, RefPtr<MethodImpl> > MethodMap;

    public:
        typedef std::set<RefPtr<SharedString> > SourceSet;

        // global variables
        typedef ext::hash_multimap<RefPtr<SharedString>, RefPtr<Variable> > GlobalVarMap;

        typedef std::vector<RefPtr<Function> > FunList;

        typedef FunctionMap::const_iterator func_const_iterator;


        virtual ~CompileUnit() throw();

    BEGIN_INTERFACE_MAP(CompileUnit)
        INTERFACE_ENTRY(TranslationUnit)
    END_INTERFACE_MAP()

        CompileUnit(
            Descriptor&   desc,
            SharedString& section,
            SharedString* buildPath,
            const char*   fileName,
            addr_t        beginAddr,
            size_t        stabIndex);

        /**
         * @return a descriptor of the binary (executable,
         * shared object, etc) that this compile-unit is part of.
         */
        const Descriptor* descriptor() const { return desc_; }
        Descriptor* descriptor() { return desc_; }

        /*** TranslationUnit interface ***/
        virtual const char* filename() const { return name().c_str(); }

        virtual addr_t lower_addr() const { return begin_addr(); }

        virtual addr_t upper_addr() const { return end_addr(); }

        /* todo: infer compiler information from .stabs */
        virtual const char* producer() const { return ""; }

        /**
         * @return the name of the section in which the
         * stabs for the symbols in this compilation
         * unit are found.
         */
        SharedString& section() const;

        /**
         * @return the index of the stab, in the
         * stabs section, of this compilation unit.
         */
        size_t begin_index() const { return beginIndex_; }

        size_t end_index() const { return endIndex_; }

        void set_end_index(size_t offs);

        addr_t begin_addr() const { return beginAddr_; }

        addr_t end_addr() const { return endAddr_; }

        void set_end_addr(addr_t addr);

        size_t size() const;

        func_const_iterator func_begin() const
        {
            return functionMap_.begin();
        }

        func_const_iterator func_end() const
        {
            return functionMap_.end();
        }

        SharedString& build_path() const;

        /**
         * @return the file name of this compilation unit.
         */
        SharedString& name() const;

        size_t type_table_count() const
        {
            return typeTables_.size();
        }

        void add_type_table(TypeTablePtr table)
        {
            typeTables_.push_back(table);
        }

        void add_type(const TypeID&, DataType*);

        void add_type(const RefPtr<SharedString>& name,
                      const RefPtr<DataType>& type)
        {
            assert(!name.is_null());
            assert(!type.is_null());
            if (name->length() < 256)
            {
                Descriptor::TypeMap& tmap = desc_->type_map();
                tmap.insert(std::make_pair(name, type.get()));
            }
        }

        /**
         * Lookup type by id; if `follow' is true, and the
         * found type is a forward type, follow the links.
         */
        RefPtr<DataType>
        get_type(TypeSystem&, const TypeID&, bool follow = false);

        void add_function(const RefPtr<Function>&);

        /**
         * Lookup function by address.
         */
        Function* lookup_function(addr_t, bool strict) const;

        FunList functions() const;

        void add_source_line(
                const RefPtr<SharedString>& sourceFileName,
                size_t lineNumber,
                addr_t addr);

        size_t addr_to_line(
            addr_t,
            addr_t*,
            EnumCallback2<SharedString*, size_t>*) const;

        /**
         * @return the address of the next line, or zero
         * @param sourceFile the name of the source file;
         * on return it is set to the source file where the
         * next line is.
         * @param lineNum the line number in the file
         * @param addrHint the address that corresponds to
         * the given line, if available
         * @param nextLineNum if not NULL, is filled out
         * with the number of the next line.
         */
        addr_t next_line(
                RefPtr<SharedString>& sourceFile,
                size_t  lineNum,
                addr_t  addrHint,
                size_t* nextLineNum = 0) const;

        GlobalVarMap& globals() { return globals_; }

        RefPtr<DataType> lookup_type(SharedString&) const;

        void parse(TypeSystem&, ELF::Binary* = 0);

        void add_method(SharedString&, const RefPtr<MethodImpl>&);

        /**
         * Lookup method by mangled name
         */
        RefPtr<MethodImpl> lookup_method(SharedString&) const;

        size_t enum_ns(const char*, EnumCallback<const char*>*) const
        {
            return 0; /* namespaces are not supported in STAB */
        }

        int language() const { return -1; }

        // size_t source_files_count() const { return sources_.size(); }

        const SourceSet& sources() const { return sources_; }

        size_t enum_sources(EnumCallback<SharedString*, bool>*);

        // size_t line_to_addr(size_t, EnumCallback<addr_t>*);

        SharedString* module_name() const { return NULL; } // todo
        SharedString* declared_module() const { return NULL; } // todo

    private:
        Descriptor* desc_;  /* todo: use weak ptr? */

        RefPtr<SharedString> section_;
        RefPtr<SharedString> buildPath_;
        RefPtr<SharedString> name_;
        RefPtr<SharedString> shortName_;
        SourceSet sources_;

        /*  The start address of the portion of the
            the text section corresponding to this unit. */

        const addr_t beginAddr_;

        /*  The end address of the text section
            for this source file (compilation unit). */

        addr_t endAddr_;

        /*  Offset in stab section where stabs for this
            compilation begin. The purpose of beginIndex_
            and endIndex_ is to quickly locate the stabs for
            this unit within the stabs section. */

        const size_t beginIndex_;

        /*  Index of last stab defined for this unit, plus one */

        size_t endIndex_;

        LineByAddr  lineByAddr_;
        FunctionMap functionMap_;

        std::vector<TypeTablePtr> typeTables_;

        GlobalVarMap globals_; /* global variables */

        bool parsed_;

        MethodMap methodMap_;
    };
} // namespace Stab


inline void
Stab::CompileUnit::add_source_line
(
    const RefPtr<SharedString>& sourceFile,
    size_t lineNum,
    addr_t addr
)
{
    if (addr)
    {
        assert(!sourceFile.is_null());
        lineByAddr_.insert(
            std::make_pair(addr, std::make_pair(sourceFile, lineNum)));
    }
    sources_.insert(sourceFile);
}


inline SharedString& Stab::CompileUnit::name() const
{
    assert(!name_.is_null());
    return *name_.get();
}

// Copyright (c) 2004, 2005, 2006 Cristian L. Vlasceanu

#endif // COMPILE_UNIT_H__6E5B99DA_572A_44B7_80A6_36B81526BFEF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
