#ifndef DESCRIPTOR_H__CA386345_851A_4ECC_8326_ED5400ED87FC
#define DESCRIPTOR_H__CA386345_851A_4ECC_8326_ED5400ED87FC
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
//
#include <map>
#include <string>
#include <vector>
#include <libelf.h>
#include "zdk/platform.h"
#include "zdk/ref_counted_impl.h"
#include "zdk/ref_ptr.h"
#include "zdk/weak_ptr.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "stabz/public/object.h"
#include "stabz/public/stab.h"
#include "stabz/public/type_table.h"

using Platform::word_t;

class TypeSystem;

namespace ELF
{
    class Binary;
}

namespace Stab
{
    class CompileUnit;
    class Descriptor;
    class ForwardType;

    struct Events;

    /**
     * Descriptor for a file containing debug info
     * in the STABS format.
     * @note The implementation assumes STABs are
     * organized in sections (as in ELF -- actually, ELF
     * is the only format I support as of now ;-)).
     */
    class Descriptor : public RefCountedImpl<Object>
    {
    public:
        /* A vector of compilation units */
        typedef std::vector<RefPtr<CompileUnit> > UnitList;

        typedef UnitList::const_iterator iterator;
        typedef UnitList::const_iterator const_iterator;

        /* Map header file names to their type tables. */
        typedef ext::hash_map<
            RefPtr<SharedString>, TypeTablePtr> HeaderMap;

        /* Header maps, indexed by a hashed key. In the N_EXCL
           stab entries, the stab value is a hashed key used to
           find the matching N_BINCL. */
        typedef ext::hash_map<word_t, HeaderMap> HeaderHash;

        /* Maintain one HeaderHash per section. */
        typedef ext::hash_map<
            RefPtr<SharedString>, HeaderHash> HeadersBySection;

        /* index forward types by name */
        typedef ext::hash_map<
            RefPtr<SharedString>, RefPtr<ForwardType> > FwdTypeMap;

        /* Index types by name */
        typedef ext::hash_map<
            RefPtr<SharedString>, WeakDataTypePtr> TypeMap;

    public:
        explicit Descriptor(SharedString&);
        explicit Descriptor(const char* fileName);

        virtual ~Descriptor() throw();

        /**
         * Perform a light processing of the stab entries:
         * record beginning and ending of compilation units
         * and functions, but do not parse any type information.
         * Further parsing can be done with CompileUnit::parse_stabs.
         */
        void init();

        /**
         * Parse stab entries. init() must be called before
         * invoking this function.
         */
        void parse(TypeSystem&);

        /**
         * Thoroughly process and parse STAB entries.
         */
        void init_and_parse(TypeSystem&);

        /**
         * @return the filename to which the descriptor refers to.
         */
        SharedString& name() const;

        /**
         * Lookup compile unit by address using a
         * lower_bound algorithm; return the
         * unit where symbol closes to the given
         * address is defined, return a null ptr if
         * not found.
         * @param addr lookup address
         * @param index if specified, the index in
         * the list of stabs entries for the unit
         * needs to match the given index; otherwise
         * the first non-empty unit that matches the
         * lower_bound criteria is returned.
         */
        CompileUnit* get_compile_unit(
            addr_t addr,
            size_t index = 0) const;

        //
        //  For iterating thru the compilation units
        //
        const_iterator begin() const { return unit_list().begin(); }
        const_iterator end() const   { return unit_list().end(); }

        iterator begin() { return unit_list().begin(); }
        iterator end() { return unit_list().end(); }

        /**
         * Open the associated file, iterate thru sections,
         * and call the provided Stab::Events callbacks.
         * @note the function uses a stack-lived ELF::Binary
         */
        void for_each_stab_section(Events**, size_t numEvents);

        /**
         * For each section with the given name, iterate
         * from `first' to `last' stab indices, and call
         * the events callbacks.
         * @note the function uses a stack-lived ELF::Binary
         */
        void for_each_stab(
                SharedString& section,
                size_t first,
                size_t last,
                Events**,
                size_t numEvents,
                SharedString* msg = NULL);

        void for_each_stab(
                ELF::Binary&,
                SharedString& section,
                size_t first,
                size_t last,
                Events**,
                size_t numEvents,
                SharedString* msg = NULL);

        /**
         * Begin a new compilation unit.
         */
        CompileUnit& new_compile_unit(
                SharedString& section,
                const stab_t&,
                SharedString* buildPath,
                const char* fileName,
                size_t stabOffset);

        /**
         * Set the end address and end offset, and add
         * current unit to the unit map.
         */
        void finish_compile_unit(
                CompileUnit&,
                addr_t endAddr,
                size_t endOffset);

        TypeTablePtr add_type_table(
                const RefPtr<SharedString>& section,
                const RefPtr<SharedString>& headerFileName,
                const stab_t&);

        void add_source_line(
                const RefPtr<CompileUnit>&,
                const RefPtr<SharedString>&,
                size_t lineNum,
                addr_t addr);

        /**
         * For a given source filename and line number
         * return the corresponding addresses.
         */
        std::vector<addr_t>
        line_to_addr(RefPtr<SharedString> sourceFileName,
                     size_t line);

        /**
         * Given an address, return the nearest line number that
         * generated the instruction at that address, and
         * fill out the source file name; return zero if not found.
         */
        /* size_t nearest_line(addr_t, RefPtr<SharedString>&) const; */

        /**
         * @return the address of the next line, or zero,
         *  if it could not be determined.
         * @param sourceFile the name of the source file
         * @param lineNum the line number in the file
         * @param addrHint the address that corresponds to
         *  the given line, if available
         * @param nextLineNum if not NULL, is filled out
         *  with the number of the next line.
         */
        addr_t next_line(
                RefPtr<SharedString> sourceFile,
                size_t  lineNum,
                addr_t  addrHint = 0,
                size_t* nextLineNum = 0) const;

        void set_observer_events(Events* events);

        /**
         * Indexing types by name is useful when a reference-to-typename
         * (or forward-decl) type appears after the module the fully
         * declares it.
         * @note the map may only be available during the parse phase.
         * After that, memory may be reclaimed.
         */
        TypeMap& type_map() { return typeMap_; }

        /**
         * The ForwardTypes map is used to memorize types that
         * are forward-declared until we see the full definition.
         */
        FwdTypeMap& forward_types() { return fwdTypeMap_; }

    private:

        /* There might be some units of zero size; we do
           keep them around for the types declared inside them;
           hence the use of multimap instead of just a map. */
        typedef std::multimap<addr_t, RefPtr<CompileUnit> > UnitMap;

        /*  Map line numbers to their corresponding addresses
            in the code; a line in C or C++ may translate into
            several instructions, hence the multi map. */
        typedef std::multimap<size_t, addr_t> AddrByLine;

        /* map source file names to addr info */
        //typedef std::map<RefPtr<SharedString>, AddrByLine> AddrByFile;
        typedef ext::hash_map<RefPtr<SharedString>, AddrByLine> AddrByFile;

        /* map addresses to a (source, line) pair */
        /* typedef std::map<
                addr_t,
                std::pair<RefPtr<SharedString>, size_t> > LineByAddr; */

    private:
        void init_hash_maps();
        void end_parse();

        void for_each_stab_section(const ELF::Binary&, Events**, size_t);

        /**
         * Call the given decoder routine on each stab in
         * the section's data; also pass the data for the
         * associated string section.
         */
        void for_each_stab(
            const Elf_Data&,
            const Elf_Data&,
            Events**,
            size_t numEvents);

        UnitList& unit_list();
        const UnitList& unit_list() const;

        bool process_stab(
            size_t          index,
            const stab_t&   stab,
            const Elf_Data& strData,
            std::string&    buf,
            Stab::Events**  events,
            size_t          numEvents);

        void on_section(const char*);
        bool on_stab(size_t, const stab_t&, const char*, size_t);

        void on_begin(const char* sectionName,
                     size_t numberOfEntries,
                     size_t startIndex = 0,
                     SharedString* msg = NULL);

        void on_done(size_t);

    private:
        Descriptor&             self_;
        RefPtr<SharedString>    fileName_;
        mutable bool            initialized_;
        mutable bool            parsed_;
        size_t                  startIndex_;
        UnitList                unitList_;
        UnitMap                 unitMap_;
        HeadersBySection        headers_;
        AddrByFile              addrByFile_;
        FwdTypeMap              fwdTypeMap_;
        TypeMap                 typeMap_;
        Events*                 observer_;
    };


    /**
     * Interface that receives notifications in
     * the process of iterating over a section
     * containing debug info in the STABS format.
     * Typically, a binary file contains one stabs
     * section; we support an arbitrary number of
     * sections per file.
     */
    struct Events
    {
        virtual ~Events() {}

        /* Invoked when a new section is encountered. */
        virtual void on_section(const char* name) = 0;

        virtual void on_begin(
                        SharedString& fileName,
                        const char* sectionName,
                        size_t numEntries) = 0;

        virtual bool on_stab(
                        size_t index,
                        const stab_t&,
                        const char* str,
                        size_t strLength) = 0;

        virtual void on_done(size_t howMany) = 0;
    };


}
#endif // DESCRIPTOR_H__CA386345_851A_4ECC_8326_ED5400ED87FC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
