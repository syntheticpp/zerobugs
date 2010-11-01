#ifndef DYN_LIB_LIST_H__94C6EDBA_0D56_46F1_A2A9_3A4BCB9F7F18
#define DYN_LIB_LIST_H__94C6EDBA_0D56_46F1_A2A9_3A4BCB9F7F18
//
// $Id: dyn_lib_list.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <set>
#include <string>
#include <vector>
#include <memory>
#include <boost/utility.hpp>
#include "elfz/public/binary.h"

namespace ELF
{
    class ElfHdr;
}


struct PathMapper
{
    virtual ~PathMapper() { }
    virtual void apply(std::string&) const = 0;
};

/**
 * Builds a list of dependencies for an ELF executable, by
 * reading the DT_NEEDED entries of the dynamic sections, and
 * recursively examines each library for its own dependencies.
 */
class DynLibList : boost::noncopyable
{
public:
    typedef std::vector<std::string> list_type;
    typedef std::set<std::string> container_type;
    typedef container_type::iterator iterator;
    typedef container_type::const_iterator const_iterator;

    explicit DynLibList(const char* filename,
                        const char* const* environ = NULL,
                        PathMapper* = NULL);

    iterator begin() { return libs_.begin(); }
    iterator end() { return libs_.end(); }

    const_iterator begin() const { return libs_.begin(); }
    const_iterator end() const { return libs_.end(); }

private:
    /// collect all the dynamic dependencies for filename
    /// (i.e. add the needed libs to libs_)
    void collect_needed(const char* filename);

    void collect_needed(const ELF::Binary&, const ELF::ElfHdr&);

    /// given a colon-delimited list of lib paths, and a list
    /// of libraries, locate the libs and then collect their
    /// own dependencies
    void collect_needed(const std::string& paths, list_type& libs);

    void collect_needed(const list_type& paths, list_type& libs);

    bool try_paths(const list_type&, const std::string&);

    const std::string& get_library_path() const;

    void get_dl_search_paths();

    void map_path(std::string& path) const
    {
        if (PathMapper* mapper = mapper_.get()) mapper->apply(path);
    }

private:
    container_type      libs_;
    const char* const*  environ_;
    mutable std::string libraryPath_;
    ELF::Binary         elf_;
    std::auto_ptr<PathMapper> mapper_;
    std::vector<std::string> paths_;
};
#endif // DYN_LIB_LIST_H__94C6EDBA_0D56_46F1_A2A9_3A4BCB9F7F18
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
