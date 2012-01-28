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
#include <stab.h>
#include "zdk/shared_string_impl.h"
#include "public/compile_unit.h"
#include "public/function.h"
#include "private/init_events.h"
#include "private/util.h"

using namespace std;


Stab::InitEvents::InitEvents(Stab::Descriptor& descriptor)
    : desc_(descriptor)
{
}


Stab::InitEvents::~InitEvents() throw()
{
}


void Stab::InitEvents::on_section(const char* section)
{
    section_ = shared_string (section);
}


void Stab::InitEvents::on_begin(SharedString&, const char*, size_t)
{
}


bool Stab::InitEvents::on_stab(
    size_t          index,
    const stab_t&   stab,
    const char*     str,
    size_t          strLen)
{
    switch (stab.type())
    {
    case N_SO:
        on_source(index, stab, str, strLen);
        break;

    case N_FUN:
        /*  It is important to record the beginning and ending
            addresses of functions, because the line information
            in N_SLINE entries is relative to the current func's
            address. */

        on_func(index, stab, str, strLen);
        break;

    case N_BINCL:
    case N_EXCL:
        /*  Add the type tables here, since we already maintain
            a string pool; also, it balances the amount of work
            between initialization and type-parsing. */
        {
            RefPtr<SharedString> headerName =
                string_from_pool(stab.strindex(), str, strLen);

            TypeTablePtr table =
                desc_.add_type_table(section_, headerName, stab);

            unit_->add_type_table(table);
        }
        break;

    case N_SOL:
        if (strLen)
        {
            on_sol(index, stab, str, strLen);
        }
        break;

    case N_SLINE:
        if (!func_.is_null())
        {
            on_sline(index, stab, str, strLen);
        }
        break;
/*
    case N_OPT:
        if (strLen)
        {
            producer_ = string_from_pool(index, str, strLen);

            if (unit_.get())
            {
                unit_->set_producer(producer_);
            }
        }
        break;
*/
    }

    return true;
}


void Stab::InitEvents::on_sol(
    size_t          index,
    const stab_t&   stab,
    const char*     str,
    size_t          len)
{
    assert(len);

    if (unit_->name().is_equal(str))
    {
        source_ = &unit_->name();
    }
    else
    {
        source_ = string_from_pool(stab.strindex(), str, len);

        assert(!source_.is_null());
        if (source_.is_null() || *source_->c_str() != '/')
        {
            if (buildPath_.get())
            {
                source_ = buildPath_->append(str);
            }
        }
    }
}


void Stab::InitEvents::on_func(
    size_t          index,
    const stab_t&   stab,
    const char*     str,
    size_t          len)
{
    /*  GCC marks the end of the function with a
        a N_FUN stab with an empty string; the
        value is the (relative) ending address */

    if (!func_.is_null())
    {
        addr_t addr = stab.value();

        if (len == 0)
        {
            /*  Address is relative for the ending N_FUN.
                If not ending N_FUN is present, simply
                assume the end of the function is where
                the next function begins. */

            addr += func_->begin_addr();
        }
        func_->set_end_addr(addr);
        func_->set_end_index(index);

        unit_->add_function(func_);
        func_.reset();
    }

    if (len)
    {
        const char* p = strchr(str, ':');
        assert(p);
        RefPtr<SharedString> name(SharedStringImpl::create(str, p));

        func_.reset(new Function(stab.value(), index, *name));
        unit_->add_function(func_);
        // clog << __func__ << ": " << name->c_str() << endl;
    }
}


void Stab::InitEvents::on_source(
    size_t          index,
    const stab_t&   stab,
    const char*     str,
    size_t          len)
{
    if (len == 0)
    {
        /*  GCC emits a closing N_SO at the end of the unit,
            the value is the end address in the text section
            for the unit. */

        if (!unit_.is_null())
        {
            finish_compile_unit(stab, index + 1);
            unit_.reset();
        }

        buildPath_.reset();
    }
    else if (buildPath_.is_null())
    {
        /*  GCC emits a stab for the build path, and
            another one for the file name proper. */

        if (len && str[len - 1] == '/')
        {
            /*  Same build path as the previous unit? */

            if (!unit_.is_null()
              && unit_->build_path().is_equal(str))
            {
                /* then reuse the ref-counted string ptr */
                buildPath_ = &unit_->build_path();
            }
            else
            {
                buildPath_ = shared_string(str);
            }
        }
        else
        {
            new_compile_unit(stab, str, index);
        }
    }
    else
    {
        /*  Have seen the N_SO entry for the build path,
            this entry is for the source file name. */

        new_compile_unit(stab, str, index);
    }
}


void Stab::InitEvents::on_sline(
    size_t          index,
    const stab_t&   stab,
    const char*     str,
    size_t          len)
{
    assert(!func_.is_null());
    assert(!unit_.is_null());

    assert(!source_.is_null());

    /* addr is relative to the beginning of current func */
    const addr_t addr = stab.value() + func_->begin_addr();

    if (addr)
    {
        /* The descriptor keeps the (file, line) --> addr map */
        desc_.add_source_line(unit_, source_, stab.desc(), addr);

        /*  The compilation unit keeps the addr-->(file, line)
            mapping. Client code should lookup the unit by address,
            then lookup the (file, line) info inside the unit. */
        unit_->add_source_line(source_, stab.desc(), addr);
    }
}


RefPtr<SharedString>
Stab::InitEvents::string_from_pool(size_t      index,
                                   const char* str,
                                   size_t      length)
{
    StringPool::iterator i = stringPool_.find(index);

    if (i == stringPool_.end())
    {
        RefPtr<SharedString> shared = shared_string(str, length);

#ifdef HAVE_HASH_MAP
        i = stringPool_.insert(make_pair(index, shared)).first;
#else
        i = stringPool_.insert(i, make_pair(index, shared));
#endif
    }
    // else clog << i->second->c_str() << endl;

    return i->second;
}


void Stab::InitEvents::new_compile_unit(
        const stab_t&   stab,
        const char*     str,
        size_t          index)
{
    /* Finish the current unit, if necessary. */

    if (!unit_.is_null())
    {
        finish_compile_unit(stab, index);
    }

    assert(!section_.is_null());

    unit_ = &desc_.new_compile_unit(
                            *section_,
                            stab,
                            buildPath_.get(),
                            str,
                            index);

    // clog << "init: " << index << ' ' << unit_->name()->c_str();
    // clog << hex << ' ' << unit_->begin_addr() << dec << endl;

    source_ = &unit_->name();
}


void Stab::InitEvents::finish_compile_unit(
        const stab_t&   stab,
        size_t          index)
{
    assert(!unit_.is_null());

    if (unit_->end_index() == 0)
    {
        assert(unit_->end_addr() == 0);

        desc_.finish_compile_unit(*unit_, stab.value(), index);
    }

    /* end current function, if necessary */
    if (!func_.is_null())
    {
        assert(func_->end_addr() == 0);

        func_->set_end_addr(stab.value());
        func_->set_end_index(index);

        unit_->add_function(func_);

        func_.reset();
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
