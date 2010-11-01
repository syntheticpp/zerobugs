//
// $Id: debug_symbol_impl.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <bitset>
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "generic/temporary.h"
#include "zdk/buffer_impl.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/type_system.h"
#include "public/debug_symbol_impl.h"
#include "public/debug_symbol_vector.h"
#include "public/value.h"
#include "public/types.h"


using namespace std;
using namespace Platform;


////////////////////////////////////////////////////////////////
void DebugSymbolImpl::ensure_children()
{
    if (children_.get() == 0)
    {
        children_.reset(new DebugSymbolVector);
    }
}

////////////////////////////////////////////////////////////////
DebugSymbolImpl::DebugSymbolImpl
(
    DebugInfoReader*    reader,
    Thread&             thread,
    DataType&           type,
    SharedString&       name,
    addr_t              addr,
    SharedString*       declFile,
    size_t              declLine,
    bool                isReturnValue
)
  : reader_(reader)
  , parent_(NULL)
  , thread_(&thread)
  , type_(&type)
  , name_(&name)
  , addr_(addr)
  , depth_(1)
  , retValue_(isReturnValue)
  , constant_(false)
  , declLine_(declLine)
{
    if (declFile)
    {
        TypeSystem& types = interface_cast<TypeSystem&>(thread);
        declFile_ = types.get_string(declFile);
    }
}


////////////////////////////////////////////////////////////////
DebugSymbolImpl::DebugSymbolImpl
(
    Thread&             thread,
    DataType&           type,
    const string&       value,
    RefPtr<SharedString>name,
    DebugInfoReader*    reader,
    Method*             method
)
  : reader_(reader)
  , parent_(NULL)
  , thread_(&thread)
  , type_(&type)
  , name_(name ? name : SharedStringImpl::create(""))
  , value_(shared_string(value))
  , addr_(0)
  , depth_(1)
  , retValue_(false)
  , constant_(true)
{
	if (method)
    {
        set_method(method);
        assert(this->method());
    }
}


////////////////////////////////////////////////////////////////
//
// copy ctor; used by clone()
//
DebugSymbolImpl::DebugSymbolImpl(const DebugSymbolImpl& other)
  : RefCountedImpl<DebugSymbol>()
  , reader_(other.reader_)
  , parent_(NULL)
  , thread_(other.thread_)
  , type_(other.type_)
  , name_(other.name_)
  , value_(other.value_)
  , addr_(other.addr_)
  , depth_(other.depth_)
  , retValue_(other.retValue_)
  , constant_(other.constant_)
{
    if (other.children_.get())
    {
        children_.reset(other.children_->clone());

        assert(children_->enumerate() == other.children_->enumerate());
        assert(children_->method() == other.children_->method());
    }
}


////////////////////////////////////////////////////////////////
DebugSymbolImpl::~DebugSymbolImpl() throw()
{
    assert(ref_count() == 0);
}


////////////////////////////////////////////////////////////////
RefPtr<DebugSymbolImpl> DebugSymbolImpl::create
(
    DebugInfoReader*    reader,
    Thread&             thread,
    DataType&           type,
    SharedString&       name,
    addr_t              addr,
    SharedString*       declFile,
    size_t              declLine,
    bool                retVal)
{
    return new DebugSymbolImpl(reader, thread, type, name,
                               addr, declFile, declLine, retVal);
}


////////////////////////////////////////////////////////////////
RefPtr<DebugSymbolImpl> DebugSymbolImpl::create
(
    Thread&             thread,
    DataType&           type,
    const string&       value,
    RefPtr<SharedString>name,
    DebugInfoReader*    reader,
    Method*             method
)
{
    return new DebugSymbolImpl(thread, type, value, name, reader, method);
}


////////////////////////////////////////////////////////////////
SharedString* DebugSymbolImpl::name() const
{
    assert(name_.get());
    return name_.get();
}


////////////////////////////////////////////////////////////////
void DebugSymbolImpl::set_name(SharedString* name)
{
    assert(name);
    name_ = name;
}


////////////////////////////////////////////////////////////////
DataType* DebugSymbolImpl::type() const
{
    return type_.ref_ptr().get();
}


////////////////////////////////////////////////////////////////
bool DebugSymbolImpl::set_type(DataType& type)
{
    if (TypeChangeObserver* observer =
        interface_cast<TypeChangeObserver*>(&type))
    {
        if (!observer->on_type_change(this, &type))
        {
            return false;
        }
    }
    type_ = &type;
    return true;
}


////////////////////////////////////////////////////////////////
SharedString* DebugSymbolImpl::value() const
{
    return value_.get();
}


////////////////////////////////////////////////////////////////
int DebugSymbolImpl::compare(const DebugSymbol* other) const
{
    if (!value_)
    {
        throw logic_error("debug symbol comparison before read");
    }
    if (other == NULL || other->value() == NULL)
    {
        return 1; // we're greater than the other
    }

    const DebugSymbolImpl& that =
        interface_cast<const DebugSymbolImpl&>(*other);

    assert(type());
    assert(that.type());

    assert (value());
    assert (other->value());

    if (!CHKPTR(type()->name())->is_equal2(that.type()->name()))
    {
        string msg("Comparing apples to oranges?\n");
        msg += type()->name()->c_str();
        msg += ' ';
        msg += value()->c_str();
        msg += "\nto\n";
        msg += that.type()->name()->c_str();
        msg += ' ';
        msg += that.value()->c_str();

    #ifdef DEBUG
        abort();
    #else
        cerr << "*** Warning: " << msg << endl;
        return strcmp(this->value()->c_str(), other->value()->c_str());
    #endif
    }

    return type()->compare(value()->c_str(), other->value()->c_str());
}


////////////////////////////////////////////////////////////////
Thread* DebugSymbolImpl::thread() const
{
    return CHKPTR(thread_.lock()).get();
}


////////////////////////////////////////////////////////////////
void DebugSymbolImpl::add_child_impl(DebugSymbolImpl* child)
{
    if (child)
    {
        assert(child != this);

    #ifdef DEBUG
        // check for cycles
        RefPtr<DebugSymbol> father = parent();
        for (; father; father = father->parent())
        {
            assert(child != father.get());
        }
    #endif
        ensure_children();

        children_->add(child);

        child->depth_ = depth() + 1;
        child->parent_ = this;
    }
}


////////////////////////////////////////////////////////////////
void DebugSymbolImpl::add_child(DebugSymbol* child)
{
    // verify that this sym is of an aggregate or pointer type
    /* assert(interface_cast<ClassType*>(type())
        || interface_cast<ArrayType*>(type())
        || interface_cast<PointerType*>(type())
        || interface_cast<MacroType*>(type())
        || interface_cast<FunType*>(type())); */

    DebugSymbolImpl* impl = 0;

    if (child->query_interface(DebugSymbolImpl::_uuid(), (void**)&impl))
    {
        add_child_impl(impl);
    }
}


////////////////////////////////////////////////////////////////
void DebugSymbolImpl::remove_all_children()
{
    // also check for addr_ here to avoid clearing the children
    // of containers that are populated "by hand" with add_child

    if (children_.get() && addr_)
    {
        children_->clear();
    }
}


////////////////////////////////////////////////////////////////
void DebugSymbolImpl::set_method(Method* method)
{
    if (method)
    {
        ensure_children();

        children_->set_method(method);
        assert(this->method());
    }
}


////////////////////////////////////////////////////////////////
RefPtr<Method> DebugSymbolImpl::method() const
{
    RefPtr<Method> m = children_.get() ? children_->method() : 0;

    return m;
}


/**
 * Read the data associated with this symbol; mask may be non-zero
 * if the symbol corresponds to a bit-field C/C++ structure member.
 */
void DebugSymbolImpl::read(DebugSymbolEvents* events, long bitmask)
{
    try
    {
        remove_all_children();

        if (RefPtr<DataType> type = type_.ref_ptr())
        {
            // delegate reading to the data type

            value_ = type->read(this, events);
        }
    #ifdef DEBUG
        // use the older implementation just to compare results
        if (bitmask)
        {
            RefPtr<SharedString> value = value_;
            read_bit_field(events, bitmask);

            assert(value->is_equal2(value_.get()));
        }
    #endif // DEBUG
    }
    catch (exception& e)
    {
        cerr << __func__ << '(' << name() << "): " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "Unknown exception in: " << __PRETTY_FUNCTION__ << endl;
    }
}


////////////////////////////////////////////////////////////////
#ifdef DEBUG

void DebugSymbolImpl::read_bit_field(DebugSymbolEvents* events, bitsize_t mask)
{
    assert(type_.ref_ptr());

    if (RefPtr<IntType> intType = interface_cast<IntType>(type_.ref_ptr()))
    {
        assert(intType->bit_size() <= long_size);

        Value<int> v;
        v.read(*this);
        //clog << bitset<sizeof(int)*4>(mask) << endl;

        int tmp = v.value() & mask;

        clog << hex << v.value() << "/" << tmp << endl;

        for (; (mask & 1) == 0; mask >>= 1, tmp >>= 1)
        { }

        clog << __func__ << ": " << tmp << dec << endl;

        ostringstream x;

        const int base = events ? events->numeric_base(this) : 0;

        switch (base)
        {
        default: assert(false); break;
        case 8:  x << oct << tmp; break;
        case 16: x << hex << tmp; break;

        case 0: // fallthru
        case 10:
            if (intType->is_signed())
            {
                unsigned long complement = mask + 1;
                if ((complement >> 1) & tmp)
                {
                    x << '-' << complement - tmp;
                }
            }
            else
            {
                x << tmp;
            }
            break;
        }
        x << showbase;
        value_ = shared_string(x.str());
    }
    else
    {
        string err(__func__);

        err += ": not an integral type: ";
        err += type_->name()->c_str();

        throw runtime_error(err);
    }
}
#endif // DEBUG


////////////////////////////////////////////////////////////////
static SharedString* no_type_name()
{
    static RefPtr<SharedString> name(SharedStringImpl::create("?"));
    return name.get();
}


////////////////////////////////////////////////////////////////
SharedString* DebugSymbolImpl::type_name() const
{
    if (typeName_)
    {
        return typeName_.get();
    }
    else if (RefPtr<DataType> type = type_.lock())
    {
        assert(type->name());
        return type->name();
    }
    return no_type_name();
}


////////////////////////////////////////////////////////////////
bool DebugSymbolImpl::is_fundamental_type() const
{
    if (RefPtr<DataType> type = type_.ref_ptr())
    {
        return type->is_fundamental();
    }
    return false;
}


////////////////////////////////////////////////////////////////
size_t DebugSymbolImpl::enum_children(DebugSymbolCallback* events) const
{
    if (children_.get())
    {
        return children_->enumerate(events);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
DebugSymbol* DebugSymbolImpl::nth_child(size_t n)
{
    if (children_.get())
    {
        return children_->nth_child(*this, n);
    }
    throw out_of_range(__func__ + string(": object has no children"));
    return 0; // avoid compiler warning
}


////////////////////////////////////////////////////////////////
size_t DebugSymbolImpl::write(const char* str)
{
    assert(str);

    if (constant_)
    {
        throw runtime_error("symbol is read-only");
    }
    if (RefPtr<DataType> type = type_.ref_ptr())
    {
        RefPtr<Buffer> buffer(new BufferImpl);
        const size_t nchars = type->parse(str, buffer.get());
        if (nchars && (nchars == strlen(str)))
        {
            type->write(this, buffer.get());
        }
        return nchars;
    }
    return 0;
}


////////////////////////////////////////////////////////////////
DebugSymbol* DebugSymbolImpl::clone (
    SharedString*   value,
    DataType*       type,
    bool            isConst
    ) const
{
    DebugSymbolImpl* dsym = new DebugSymbolImpl(*this);
    if (type)
    {
        dsym->type_ = type;
    }
    if (value)
    {
        dsym->set_addr(0); // todo: revisit the need for this
                           // (setting the address to zero prevents
                           // future reads)
        dsym->value_ = value;
        dsym->constant_ = isConst;

        /**
         * Hack used used by the expression interpreter:
         * (see on_call_return.cpp)
         * the value string may be the address where a function
         * returns an object by value
         */
        bool valueIsAddr = false;
        if (value)
        {
            if (interface_cast<ClassType*>(type))
            {
                valueIsAddr = true;
            }
            else if (PointerType* pt = interface_cast<PointerType*>(type))
            {
                valueIsAddr = !pt->is_cstring();
            }
        }
        if (valueIsAddr)
        {
            addr_t addr = strtoul(value->c_str(), 0, 0);
            dsym->set_addr(addr);
        }
        /*** end of hack ***/
    }
    return dsym;
}


void DebugSymbolImpl::set_children(auto_ptr<DebugSymbolCollection> children)
{
    children_ = children;
}


size_t DebugSymbolImpl::depth() const
{
    if (depth_)
    {
        return depth_;
    }

    // For the "tentative" parent use-case; sometimes it is useful
    // to know the depth before actually adding the child symbol to
    // a parent symbol.
    //
    // Example: we may want to apply a symbol transformation (using
    // the DataFilter interface). The implementation may want to do
    // different things based on the symbol's depth; and it may be
    // invoked before adding the symbol (because depending upon the
    // result of the transformation we may decide not to add it at all).
    size_t depth = 1;

    if (RefPtr<DebugSymbol> parent = parent_.ref_ptr())
    {
        depth += parent->depth();
    }
    return depth;
}


void DebugSymbolImpl::set_tooltip(const char* tip)
{
    if (tip)
    {
        tooltip_ = shared_string(tip);
    }
    else
    {
        tooltip_.reset();
    }
}


const char* DebugSymbolImpl::tooltip() const
{
    return tooltip_ ? tooltip_->c_str() : NULL;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
