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

#include "unit.h"
#include "dwarfz/public/compile_unit.h"
#include "dwarfz/public/namespace.h"
#include "zdk/shared_string_impl.h"

using namespace Dwarf;


Unit::Unit
(
    const boost::shared_ptr<Debug>& dbg,
    const boost::shared_ptr<CompileUnit>& unit,
    SharedString* moduleName
)
: dbg_(dbg), unit_(unit), moduleName_(moduleName)
{
    assert(unit_.get());
}


Unit::~Unit() throw()
{
}


const char* Unit::filename() const
{
    return unit_ ? unit_->full_path()->c_str() : NULL;
}


SharedString* Unit::module_name() const
{
    return moduleName_.get();
}


Platform::addr_t Unit::lower_addr() const
{
    return unit_ ? unit_->low_pc() : 0;
}


Platform::addr_t Unit::upper_addr() const
{
    return unit_ ? unit_->high_pc() : 0;
}


const char* Unit::producer() const
{
    return unit_ ? unit_->producer() : "";
}


size_t
Unit::enum_ns(const char* name, EnumCallback<const char*>* cb) const
{
    size_t count = 0;
    if (unit_)
    {
        List<Namespace> ns = unit_->namespaces();

        List<Namespace>::const_iterator i = ns.begin(),
                                        end = ns.end();
        for (; i != end; ++i)
        {
            if (!name || strcmp(name, i->name()) == 0)
            {
                ++count;
                if (cb)
                {
                    cb->notify(i->name());
                }
            }
        }
    }
    return count;
}


int Unit::language() const
{
    return unit_ ? unit_->language() : -1;
}


size_t Unit::enum_sources(EnumCallback<SharedString*, bool>* callback)
{
    size_t count = 0;

    if (unit_)
    {
        for (size_t i = 0; i < unit_->source_files_count(); ++i, ++count)
        {
            if (callback)
            {
                RefPtr<SharedString> path = unit_->filename_by_index(i);

                if (!callback->notify(path.get()))
                {
                    break;
                }
            }
        }
    }
    return count;
}


SharedString* Unit::declared_module() const
{
    return unit_ ? unit_->module() : 0;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
