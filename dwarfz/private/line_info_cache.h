#ifndef LINE_INFO_CACHE_H__E66E9648_7CBB_455C_9BDA_B9CD3A1D3EB0
#define LINE_INFO_CACHE_H__E66E9648_7CBB_455C_9BDA_B9CD3A1D3EB0
//
// $Id: line_info_cache.h 715 2010-10-17 21:43:59Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sys/stat.h>
#include <cstdlib>
#include <stdexcept>
#include <boost/utility.hpp>
// use an associative vector instead of a map, to
// minimize memory overhead
#include "generic/AssocVector.h"
#include "generic/nearest.h"
#include "interface.h"
#include "utils.h"
#include "zdk/shared_string_impl.h"
#include "zdk/string_cache.h"
#include "zdk/weak_ptr.h"
#include "dharma/canonical_path.h"
#include "dharma/environ.h"


static bool use_gcc_workarounds()
{
    static bool flag = env::get_bool("ZERO_ENABLE_GCC_WORKAROUNDS", false);
    return flag;
}


static bool use_nearest_match()
{
    static bool flag = env::get_bool("ZERO_LINE_NEAREST_MATCH", true);
    return flag;
}

/**
 * Nested helper class for accessing line number
 * information for objects within a compile-unit.
 */
CLASS CompileUnit::LineInfoCache
{
    //
    // for mapping addresses to begin-statement lines
    //
    typedef Loki::AssocVector<Dwarf_Addr, Dwarf_Line> map_type;

    void add_line_info()
    {
        assert(map_.empty());
        map_.reserve(size_);

        for (Dwarf_Signed i = 0; i != size_; ++i)
        {
            add_line_info(line_[i]);
        }
    }

public:
    LineInfoCache(const CompileUnit&, Dwarf_Debug, Dwarf_Die);
    ~LineInfoCache();

    /**
     * Lookup line by address
     */
    size_t addr_to_line(
        Dwarf_Addr      addr,
        Dwarf_Addr*     nearest,
        SrcLineEvents*  events) const;

    Dwarf_Addr next_line(
        const char* file,
        size_t      line,
        Dwarf_Addr  addr,
        size_t*     next) const;

    bool export_line_info(SrcLineEvents*) const;

protected:
    void add_line_info(Dwarf_Line line);

private:
    const CompileUnit& unit_;
    RefPtr<StringCache> stringCache_;

    Dwarf_Debug     dbg_;
    Dwarf_Line*     line_;
    Dwarf_Signed    size_;
    map_type        map_;
};


CompileUnit::LineInfoCache::LineInfoCache
(
    const CompileUnit&  unit,
    Dwarf_Debug         dbg,
    Dwarf_Die           die
)
    : unit_(unit)
    , stringCache_(unit.string_cache())
    , dbg_(dbg)
    , line_(0)
    , size_(0)
{
    assert(dbg);
    assert(die);
    assert(unit.die() == die);

    Dwarf_Error err = 0;

    // retrieve info about all source lines in this unit
    if (dwarf_srclines(die, &line_, &size_, &err) == DW_DLV_ERROR)
    {
        throw Error(dbg, err);
    }
    add_line_info();
}



CompileUnit::LineInfoCache::~LineInfoCache()
{
    if (line_)
    {
        assert(size_ >= 0);
        dwarf_srclines_dealloc(dbg_, line_, size_);
    }
}



template<typename T>
inline ZDK_LOCAL RefPtr<SharedString> make_path(const T& str, bool makeAbsPath)
{
    if (makeAbsPath)
    {
        return SharedStringImpl::take_ownership(abspath(str));
    }
    else
    {
        return shared_string(str);
    }
}


inline RefPtr<SharedString>  ZDK_LOCAL
full_path(const CompileUnit& unit,
          const char* src,
          const char* dir,
          bool makeAbsPath)
{
    assert(src);
    RefPtr<SharedString> path; // string to return

    if (use_gcc_workarounds()) // workaround bug in gcc 2.95
    {
        struct stat buf = { };

        if (stat(src, &buf) == 0)
        {
            path = SharedStringImpl::take_ownership(abspath(src));
        }
        else
        {
            // could not stat, does not exist?
            const char* delim1 = strrchr(src, '/');
            const char* delim2 = strrchr(unit.short_path(), '/');

            if (delim1 && delim2)
            {
                string tmp = unit.build_path()->c_str();
                tmp.append(unit.short_path(), delim2);
                tmp.append(delim1);

                path = make_path(tmp, makeAbsPath);
            }
            else
            {
                path = make_path(src, makeAbsPath);
            }
        }
    }
    else
    {
        if (src[0] == '/') // absolute path?
        {
            // canonicalize it (by calling abspath)
            path = SharedStringImpl::take_ownership(abspath(src));
        }
        else if (!makeAbsPath)
        {
            const size_t n = unit.build_path()->length();
            const size_t m = strlen(src);
            char* buf = reinterpret_cast<char*>(malloc(n + m + 1));
            if (!buf)
                throw std::bad_alloc();

            strcpy(buf, unit.build_path()->c_str());
            strcpy(buf + n, src);
            buf[m + n] = 0;

            path = SharedStringImpl::take_ownership(buf);
        }
        else    // construct the full path
        {
            char tmp[4096] = { 0 };

            if (!dir || dir[0] != '/')
            {
                // assume path is relative to the compilation directory
                strncpy(tmp, unit.build_path()->c_str(), 2048);
            }
            if (dir)
            {
                strncat(tmp, dir, 1024);
                strcat(tmp, "/");
            }
            strncat(tmp, src, 1024);
            path = make_path(tmp, makeAbsPath);
        }
    }
  /*
    if (path)
    {
        if (RefPtr<StringCache> cache = unit.string_cache())
        {
            path = cache->get_string(path.get());
        }
    }
  */
    return path;
}



namespace
{
    /**
     * Extracts the source filename part from Dwarf_Line
     */
    CLASS FileNameExtractor : boost::noncopyable
    {
        Dwarf_Debug dbg_;
        Dwarf_Line line_;
        RefPtr<SharedString> path_;
        const CompileUnit& unit_;
        RefPtr<StringCache> stringCache_;

    public:
        FileNameExtractor(Dwarf_Debug dbg,
                          const CompileUnit& u,
                          RefPtr<StringCache> stringCache)
            : dbg_(dbg), line_(NULL), unit_(u), stringCache_(stringCache)
        { }

        SharedString* get(Dwarf_Line line)
        {
            if (!path_ || (line_ != line))
            {
                path_.reset();
                line_ = line;

                Dwarf_Error err = 0;
                char* fileName = 0;

                typedef Dwarf::Utils::AutoDealloc<char, DW_DLA_STRING> AutoFree;
                AutoFree autoFree(dbg_, fileName);

                if (dwarf_linesrc(line, &fileName, &err) == DW_DLV_ERROR)
                {
                    throw Error("dwarf_linesrc", dbg_, err);
                }
                if (fileName)
                {
                    path_ = full_path(unit_, fileName, NULL, true);
                }
                else
                {
                    path_ = shared_string((const char*)fileName);
                }
            }
            if (path_)
            {
                if (const RefPtr<StringCache>& cache = unit_.string_cache())
                {
                    path_ = cache->get_string(path_.get());
                }
            }
            return path_.get();
        }
    };
}


inline bool ZDK_LOCAL
advertise_line(
    FileNameExtractor&  fnameExtractor,
    SrcLineEvents*      events,
    Dwarf_Debug         dbg,
    Dwarf_Addr          addr,
    Dwarf_Line          line,
    Dwarf_Unsigned      lineNum = 0)
{
    bool result = true;
    if (events)
    {
        if (!lineNum && line)
        {
            Dwarf_Error err = 0;

            if (dwarf_lineno(line, &lineNum, &err) == DW_DLV_ERROR)
            {
                throw Error("dwarf_lineno", dbg, err);
            }
        }
        SharedString* fname = fnameExtractor.get(line);
        if (events)
        {
            result = events->on_srcline(fname, lineNum, addr);
        }
        Dwarf::log<debug>(1) << " -- " << fname << ":" << lineNum << "\n";
    }
    return result;
}



inline bool ZDK_LOCAL
is_block_start(
    const CompileUnit& unit,
    Dwarf_Debug dbg,
    Dwarf_Addr addr,
    Dwarf_Line line)
{
    Dwarf_Bool isBlock = 0;
    Dwarf_Error err;

    if (dwarf_lineblock(line, &isBlock, &err) != DW_DLV_OK)
    {
        throw Error("is_block_start: dwarf_lineblock", dbg, err);
    }
    else if (isBlock)
    {
        return true;
    }

    shared_ptr<Function> fun = unit.lookup_function(addr);

    if (fun && fun->low_pc() == addr)
    {
        return true;
    }
    // NOTE: we could also descend into the blocks, but that is expensive
    return false;
}


size_t
CompileUnit::LineInfoCache::addr_to_line(
    Dwarf_Addr      addr,
    Dwarf_Addr*     nearest,
    SrcLineEvents*  events) const
{
    size_t count = 0;

    // if nearest is NULL, do an exact lookup
    if (!nearest)
    {
        map_type::const_iterator i = map_.find(addr);
        FileNameExtractor extractor(dbg_, unit_, stringCache_);

        for (; i != map_.end() && i->first == addr; ++i, ++count)
        {
            if (!advertise_line(extractor, events, dbg_, addr, i->second))
            {
                break;
            }
        }
        return count;
    }
    //
    // ... otherwise lookup the nearest line information
    //
    assert(nearest);

    map_type::const_iterator i = map_.lower_bound(addr);

    Dwarf_Addr nearestAddr = 0;

    if (i == map_.end() ||
        (i->first != addr && !is_block_start(unit_, dbg_, addr, i->second)))
    {
        if (i == map_.begin())
        {
            return 0;
        }
        map_type::const_iterator j = i;
        --j;
        if (!use_nearest_match() || compare_nearest(addr, j->first, i->first))
        {
            --i;
        }
        nearestAddr = i->first;

        if (compare_nearest(addr, *nearest, nearestAddr))
        {
            return 0; // *nearest is a closer match
        }
        *nearest = nearestAddr;
        assert(*nearest);
    }
    else
    {
        assert(i != map_.end());

        *nearest = nearestAddr = i->first;
    }
    addr = i->first;

    FileNameExtractor fnameExtractor(dbg_, unit_, stringCache_);
    for (; (i->first == addr) && (i != map_.end()); ++i, ++count)
    {
        if (!advertise_line(fnameExtractor, events, dbg_, addr, i->second))
        {
            break;
        }
    }
    assert(count);
    assert(!nearest || *nearest);

    return count;
}



Dwarf_Addr
CompileUnit::LineInfoCache::next_line(
    const char* file,
    size_t      line,
    Dwarf_Addr  addr,
    size_t*     next) const
{
    Dwarf_Addr result = 0;

    map_type::const_iterator i = map_.upper_bound(addr);

    if (i != map_.end())
    {
        Dwarf_Error err = 0;

        if (dwarf_lineaddr(i->second, &result, &err) == DW_DLV_ERROR)
        {
            throw Error("next_line: dwarf_lineaddr", dbg_, err);
        }
        assert(i->first == result);

    /*
        if (shared_ptr<Function> fun = unit_.lookup_function(addr))
        {
            if (result > fun->high_pc())
            {
                result = fun->high_pc();
                i = map_.upper_bound(result);
            }
        } */

        Dwarf_Unsigned lineNum = 0;
        for (; i != map_.end(); ++i)
        {
            if (dwarf_lineno(i->second, &lineNum, &err) == DW_DLV_ERROR)
            {
                throw Error("next_line: dwarf_lineno", dbg_, err);
            }

            if (lineNum != line)
            {
                break;
            }
        }
        if (next)
        {
            *next = lineNum;
        }
    }
    return result;
}



void
CompileUnit::LineInfoCache::add_line_info(Dwarf_Line line)
{
    Dwarf_Error err = 0;
    Dwarf_Bool endSeq = 0;

    if (dwarf_lineendsequence(line, &endSeq, &err) == DW_DLV_ERROR)
    {
        throw Error("add_line_info: dwarf_lineendsequence", dbg_, err);
    }
    // do not add to cahce if it is end-of-sequence marker
    if (endSeq)
    {
        return;
    }
    Dwarf_Addr addr = 0;
    if (dwarf_lineaddr(line, &addr, &err) == DW_DLV_ERROR)
    {
        throw Error("add_line_info: dwarf_lineaddr", dbg_, err);
    }

    if (addr)
    {
        if (unit_.low_pc() && (addr < unit_.low_pc()))
        {
            return;
        }
        if (unit_.high_pc() && (addr > unit_.high_pc()))
        {
            return;
        }
        map_.insert(make_pair(addr, line));
    }
}


bool
CompileUnit::LineInfoCache::export_line_info(SrcLineEvents* events) const
{
    bool result = true;

    for (map_type::const_iterator i = map_.begin();
         i != map_.end() && result;
         ++i)
    {
        FileNameExtractor fname(dbg_, unit_, stringCache_);
        result = advertise_line(fname, events, dbg_, i->first, i->second);
    }
    return result;
}

#endif // LINE_INFO_CACHE_H__E66E9648_7CBB_455C_9BDA_B9CD3A1D3EB0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
