//
// $Id: fwdtype.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/shared_string_impl.h"
#include "public/fwdtype.h"
#if DEBUG
 #include <iostream>
 using namespace std;
#endif

namespace Stab
{

ForwardType::ForwardType(SharedString* name, const TypeID& id)
    : IndirectType(name, 0) // target type bitsize unknown here
    , observ_(create_observer_delegate(this))
    , typeID_(id)
{
}


bool ForwardType::link(DataType* type)
{
    if (!get_pointer(link_) && type && type != this)
    {
        if (ForwardType* fwd = interface_cast<ForwardType*>(type))
        {
            if (fwd == this)
            {
                return false;
            }
            if (DataType* tmp = fwd->link())
            {
                if (tmp == this)
                {
                    return false;
                }
                type = tmp;
            }
        }
        assert(type);
        assert(type->name());

        link_ = type;
        link_->attach_to_observer(observ_.get());

        set_name(*type->name());
        return true;
    }
    return false;
}



DataType* ForwardType::link() const
{
    RefPtr<DataType> link = link_.ref_ptr();
    if (RefPtr<ForwardType> fwd = interface_cast<ForwardType>(link))
    {
        assert(link->ref_count() > 2);
        assert(fwd.get() != this);
        return fwd->link();
    }
    return link.detach();
}


static const RefPtr<SharedString> opaque(shared_string("opaque"));

SharedString*
ForwardType::read(DebugSymbol* sym, DebugSymbolEvents* events) const
{
    // todo: thread-safety?
    //static const RefPtr<SharedString> opaque(shared_string("opaque"));

    RefPtr<SharedString> result;

    if (DataType* type = link())
    {
        result = type->read(sym, events);
    }
    else
    {
        result = opaque;
    }
    return result.detach();
}


bool ForwardType::is_fundamental() const
{
    if (RefPtr<DataType> link = link_.ref_ptr())
    {
        return link->is_fundamental();
    }
    return false;
}


bitsize_t ForwardType::bit_size() const
{
    if (RefPtr<DataType> link = link_.ref_ptr())
    {
        return link->bit_size();
    }
    return 0;
}


void ForwardType::describe(int fd) const
{
    if (RefPtr<DataType> link = link_.ref_ptr())
    {
        link->describe(fd);
    }
    else
    {
        BaseType::describe(fd);
    }
}


size_t ForwardType::parse(const char* value, Unknown2* unk) const
{
    if (RefPtr<DataType> link = link_.ref_ptr())
    {
        return link->parse(value, unk);
    }
    return 0;
}


void ForwardType::on_state_change(Subject* subject)
{
    assert(subject);

    assert(get_pointer(link_));
#ifdef DEBUG
    clog << __PRETTY_FUNCTION__ << ": " << link_->name() << endl;
#endif
}

} // namespace Stab
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
