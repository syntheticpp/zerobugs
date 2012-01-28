#ifndef DEBUG_H__54B787C0_C170_4A94_AD84_6C93ED26BEA4
#define DEBUG_H__54B787C0_C170_4A94_AD84_6C93ED26BEA4
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
// NOTE: "debug" is somewhat of a a misnomer -- what it
// actually means, in this context, is a "handle"; I kept
// the name for consistency: Dwarf_Debug --> Dwarf::Debug
//
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include "generic/auto_file.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "zdk/mutex.h"
#include "zdk/weak_ptr.h"
#include "addr_ops.h"
#include "funfwd.h"
#include "list.h"
#include "interface.h"


class StringCache;

namespace Dwarf
{
    class CompileUnit;
    class Datum;
    class Die;
    class Function;
    class SrcLineEvents;
    class Unwind;
    struct RegTable;

    template<typename T> class Cache;


    CLASS DebugBase : boost::noncopyable
    {
    protected:
        explicit DebugBase(const char* filename);
        virtual ~DebugBase() throw();

        auto_fd fd_;    /* associated file descriptor */
        Dwarf_Debug dbg_;
    };


    typedef bool (*ProgressCallback)(void*, const char*);

    struct ZDK_LOCAL UnitHeadersCallback
    {
        virtual void query_unit_headers(
            const std::string& modulePath,
            vector<Dwarf_Unsigned>&) const = 0;
    };

    /**
     * Wrapper for Dwarf_Debug handles
     */
    CLASS Debug : public DebugBase
    {
        struct RangeEntry
        {
            Dwarf_Addr upper_;
            boost::shared_ptr<CompileUnit> unit_;

            RangeEntry() : upper_(0) { }
            RangeEntry(Dwarf_Addr upper, boost::shared_ptr<CompileUnit> unit)
                : upper_(upper)
                , unit_(unit)
            { }
        };

        typedef std::multimap<Dwarf_Addr, RangeEntry> UnitsByRange;

    public:
        typedef std::vector<boost::shared_ptr<CompileUnit> > UnitList;
        typedef std::vector<boost::shared_ptr<Datum> > Data;

    #ifdef HAVE_HASH_MAP
        typedef ext::hash_map<Dwarf_Off,
            boost::shared_ptr<Die>,
            ext::identity<Dwarf_Off> > DieCache;
    #else
    #error no hash_map!
        typedef std::map<Dwarf_Off, boost::shared_ptr<Die> > DieCache;
    #endif

    public:
        explicit Debug(const char* filename, const UnitHeadersCallback* = NULL);

        ~Debug() throw();

        void enter() { mutex_.enter(); }
        void leave() { mutex_.leave(); }

        /**
         * Return the Debug C++ wrapper class that corresponds
         * to the given Dwarf_Debug handle, if any exists, or
         * a null pointer otherwise.
         */
        static Debug* get_wrapper(Dwarf_Debug);

        bool empty() const;

        bool is_null() const { return dbg_ == 0; }

        ino_t inode() const;

        /**
         * Get the list of compilation units
         */
        const UnitList& units() const;

        /**
         * Lookup compile unit by address
         */
        boost::shared_ptr<CompileUnit> lookup_unit(Dwarf_Addr) const;

        /**
         * Lookup compilation unit by file name
         */
        boost::shared_ptr<CompileUnit> lookup_unit(const char*) const;

        /**
         * Lookup data in .debug_pubnames
         */
        Data lookup_global_data(const char*) const;

        /**
         * Lookup functions in .debug_pubnames
         */
        FunList lookup_global_funcs(const char*) const;

        const FunList& global_funcs() const;

        /**
         * Lookup function by address, and, optionally,
         * by linkage name
         */
        boost::shared_ptr<Function> lookup_function(
            Dwarf_Addr,
            const char* linkage = NULL) const;

        boost::shared_ptr<Type> lookup_type(const char*, int level = 0) const;
        boost::shared_ptr<Type> lookup_type_by_decl(const Type&) const;

        /**
         * Hack: attempt to compensate for GCC not generating debug_pubtypes
         * sections, by looking up the debug_pubnames and peeking at the
         * function parameters
         */
        boost::shared_ptr<Type> lookup_type_by_ctor(const char*) const;

        static std::string infer_ctor_name(const char*);
        boost::shared_ptr<Type> lookup_type_by_ctor(const std::string&) const;

        boost::shared_ptr<Die> get_object(
            Dwarf_Off,
            bool use_DW_AT_specification = false,
            bool check_indirect = true) const;

        void cache_object(Dwarf_Die) const;

        Dwarf_Addr next_line(
            const char*,
            size_t,
            Dwarf_Addr,
            size_t* = 0) const;

        /**
         * Given a line of C/C++ source code (specified by its source
         * file and line number within that file), find all addresses
         * in the code that correspond to said line. For each match,
         * call the SrcLineEvents observer.
         * @note the source file name must be specified in its fully
         * qualified, canonical form.
         * @return true if the operation completed successfully, or
         * false if it got interrupted.
         */
        bool line_to_addr(SharedString*, size_t, SrcLineEvents&) const;

        boost::shared_ptr<CompileUnit> get_compile_unit(Dwarf_Die,
                                                        Dwarf_Off nextUnitHdr
                                                       )const;

        Dwarf_Addr unwind_step(Dwarf_Addr pc, AddrOps&, RegTable&);

        void set_string_cache(StringCache*);
        WeakPtr<StringCache> string_cache() const { return strCache_; }

        const std::string& filename() const { return filename_; }

    private:
        bool validate_line_map(const char*) const;
        bool units_from_unit_headers() const;

        boost::shared_ptr<CompileUnit> lookup_unit_by_arange(Dwarf_Addr) const;

        class SrcLineMap;
        class UnitMap;  /* maps Dwarf_Offsets to compilation units */

        mutable Cache<Dwarf_Global>* globalCache_;
    /* todo: */
    /*  mutable Cache<Dwarf_Var>* varCache_; */

        mutable SrcLineMap* srcLineMap_;
        mutable UnitMap* unitMap_;
        //mutable std::auto_ptr<UnitsByRange> unitsByRange_;

        mutable std::auto_ptr<UnitList> units_;

        mutable ino_t inode_; /* inode of associated file */

        std::string filename_;

        mutable std::auto_ptr<FunList> funcs_; /* global functions */

        mutable DieCache dieCache_;

        mutable boost::shared_ptr<Unwind> unwind_; /* frame unwind info */
        mutable Mutex mutex_;
        WeakPtr<StringCache> strCache_;
        const UnitHeadersCallback* unitHeadersCallback_;
    };


    /**
     * Helper function that looks up a type among the arguments of
     * the functions in the given list.
     */
    boost::shared_ptr<Type> ZDK_LOCAL
        lookup_type(const FunList&, const char*, bool = false);
}
// namespace

#endif // DEBUG_H__54B787C0_C170_4A94_AD84_6C93ED26BEA4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
