//
// $Id: class_type_proxy.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "dwarfz/public/class_type.h"
#include "dwarfz/public/debug.h"
#include "zdk/check_ptr.h"
#include "zdk/types.h"
#include "typez/public/debug_symbol_impl.h"
#include "typez/public/is_cv_qualified.h"
#include "addr_operations.h"
#include "dbgout.h"
#include "reader.h"
#include "type_adapter.h"
#include "class_type_proxy.h"


using namespace Dwarf;
using namespace std;
using namespace boost;


#define CHECK_SHALLOW(thread) check_shallow(__func__, thread)


////////////////////////////////////////////////////////////////
ClassTypeProxy::ClassTypeProxy
(
    Dwarf::Reader&          reader,
    const RefPtr<Thread>&   thread,
    const KlassType&        type,
    ClassType&              klass
)
  : BaseType(type)
  , reader_(&reader)
  , klass_(&klass)
  , shallow_(true)
  , addr_(0)
  , inode_(type.owner().inode())
  , offset_(type.offset())
  , declOffset_(type.decl_offset())
  , observ_(create_observer_delegate(this))
{
    assert(offset_);

    klass.attach_to_observer(observ_.get());

/* slow, and does not seem to buy much...
    if (type.is_incomplete())
    {
        TypeMap tmp; // todo: should use the reader's global map?

        TypeAdapter adapter(reader_, thread, 0, tmp);
        adapter.set_shallow(false);

        shared_ptr<Type> full = adapter.resolve(type);
        if (full)
        {
            inode_ = full->owner().inode();
            offset_ = full->offset();
            declOffset_ = full->decl_offset();
        }
    }
 */
}


////////////////////////////////////////////////////////////////
SharedString* ClassTypeProxy::name() const
{
    return klass_->name();
}


////////////////////////////////////////////////////////////////
size_t ClassTypeProxy::bit_size() const
{
    return klass_->bit_size();
}


////////////////////////////////////////////////////////////////
bool ClassTypeProxy::is_fundamental() const
{
    return false;
}


////////////////////////////////////////////////////////////////
bool ClassTypeProxy::is_equal(const DataType* that) const
{
    assert(that);
    if (!that)
    {
        return false;
    }
    if (this == that)
    {
        assert(klass_->is_equal(that));
        return true;
    }
    if (is_cv_qualified(that))
    {
        return false;
    }
    ClassTypeProxy* other = 0;
    if (((DataType*)that)->query_interface(_uuid(), (void**)&other))
    {
        if (inode_ == CHKPTR(other)->inode_)
        {
            if (offset_ == other->offset_)
            {
                return true;
            }
            if (declOffset_ == other->offset_
             || other->declOffset_ == offset_)
            {
                return true;
            }
        }
    }
    return klass_->is_equal(that);
}


////////////////////////////////////////////////////////////////
int
ClassTypeProxy::compare(const char* lhs, const char* rhs) const
{
    return klass_->compare(lhs, rhs);
}


////////////////////////////////////////////////////////////////
bool
ClassTypeProxy::is_adaptation_needed(const DebugSymbol& sym) const
{
    return klass_->virtual_base_count() && (sym.addr() != addr_);
}


////////////////////////////////////////////////////////////////
SharedString* ClassTypeProxy::read(
    DebugSymbol* sym,
    DebugSymbolEvents* events
    ) const
{
    if (!CHKPTR(sym))
    {
        return NULL;
    }
    assert(klass_.get());
    assert(interface_cast<ClassType*>(sym->type()));

    bool adapted = false;
    RefPtr<DebugSymbol> oldSym = sym->clone();

    if (shallow_ || is_adaptation_needed(*sym))
    {
        adapt_deep(CHKPTR(sym->thread()), sym->addr());
        adapted = true;
    }

    SharedString* result = klass_->read(sym, events);
    if (adapted)
    {
        if (events)
        {
            events->symbol_change(sym, oldSym.get());
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
SharedString* ClassTypeProxy::make_pointer_name (
    const char* ptr,
    RefTracker* trk
    ) const
{
    return klass_->make_pointer_name(ptr, trk);
}


////////////////////////////////////////////////////////////////
size_t ClassTypeProxy::parse(const char* value, Unknown2* unk) const
{
    return klass_->parse(value, unk);
}


////////////////////////////////////////////////////////////////
void ClassTypeProxy::describe(int fd) const
{
    klass_->describe(fd);
}


////////////////////////////////////////////////////////////////
void ClassTypeProxy::on_state_change(Subject* sub)
{
    this->notify_state_change();
}


////////////////////////////////////////////////////////////////
SharedString* ClassTypeProxy::unqualified_name() const
{
    return klass_->unqualified_name();
}


////////////////////////////////////////////////////////////////
bool ClassTypeProxy::has_vtable(Thread* thread) const
{
    return rtti(thread);
}

////////////////////////////////////////////////////////////////
bool ClassTypeProxy::is_union() const
{
    return klass_->is_union();
}


////////////////////////////////////////////////////////////////
size_t ClassTypeProxy::base_count() const
{
    return klass_->base_count();
}


////////////////////////////////////////////////////////////////
const BaseClass* ClassTypeProxy::base(size_t n) const
{
    return klass_->base(n);
}


////////////////////////////////////////////////////////////////
const BaseClass* ClassTypeProxy::lookup_base (
    const SharedString* name,
    off_t* offs,
    bool recursive
    ) const
{
    const BaseClass* base = klass_->lookup_base(name, offs, recursive);

    if (!base && shallow_ && reader_)
    {
        Thread* thread = reader_->debugger()->current_thread();
        CHECK_SHALLOW(thread);

        base = klass_->lookup_base(name, offs, recursive);
    }

    return base;
}


////////////////////////////////////////////////////////////////
size_t ClassTypeProxy::virtual_base_count() const
{
    return klass_->virtual_base_count();
}


////////////////////////////////////////////////////////////////
size_t ClassTypeProxy::member_count() const
{
    return klass_->member_count();
}


////////////////////////////////////////////////////////////////
const Member* ClassTypeProxy::member(size_t n) const
{
    return klass_->member(n);
}


////////////////////////////////////////////////////////////////
size_t ClassTypeProxy::method_count() const
{
    return klass_->method_count();
}


////////////////////////////////////////////////////////////////
const Method* ClassTypeProxy::method(size_t n) const
{
    return klass_->method(n);
}


////////////////////////////////////////////////////////////////
RTTI* ClassTypeProxy::rtti(Thread* thread) const
{
    CHECK_SHALLOW(thread);
    return klass_->rtti(thread);
}


////////////////////////////////////////////////////////////////
void ClassTypeProxy::adapt_deep(Thread* thread, addr_t addr)  const
{
    CHKPTR(reader_);
    CHKPTR(thread);

    // Step 1: find the dwarf type
    RefPtr<Process> process = thread->process();
    Handle dbg = reader_->get_debug_handle(inode_, process);
    if (!dbg)
    {
        throw logic_error("could not get handle by inode");
    }

    boost::shared_ptr<Die> die = dbg->get_object(offset_);
    boost::shared_ptr<Type> type = shared_dynamic_cast<Type>(die);
    if (!type)
    {
        throw logic_error("could not get type by offset");
    }
    dbgout(0) << __func__ << ": " << _name() << ": " << name() << endl;

    // Step 2: adapt it into a TypeSystem type
    AddrOperationsContext ctxt(thread);

    TypeMap tmp;
    TypeAdapter adapter(reader_, thread, addr, tmp);

    adapter.set_class_name(name());
    adapter.set_depth(ADAPT_FULL);
    adapter.apply(type);

    if (RefPtr<ClassType> klass = interface_cast<ClassType>(adapter.type()))
    {
        klass_ = klass;
    }
    else
    {
        cerr << __func__ << ": failed adapting class type\n";
        return;
    }
    dbgout(0) << __func__ << ": " << _name() << ": " << name() << endl;
    if (addr)
    {
        shallow_ = false;
    }
    addr_ = addr;
}


////////////////////////////////////////////////////////////////
void
ClassTypeProxy::check_shallow(const char* fun, Thread* thread) const
{
    if (!shallow_ || !thread)
    {
        return;
    }
    try
    {
        addr_t addr = addr_;

        if (!addr && thread->stack_trace_depth())
        {
            Frame* frame = CHKPTR(thread->stack_trace())->selection();
            addr = frame->frame_pointer();
        }
        adapt_deep(thread, addr);
    }
    catch (const std::exception& e)
    {
        dbgout(0) << __func__ << ": " << fun << ": " << e.what() << endl;
    }
}


////////////////////////////////////////////////////////////////
bool ClassTypeProxy::on_type_change(DebugSymbol* sym, DataType* type)
{
    return true;
}


////////////////////////////////////////////////////////////////
size_t ClassTypeProxy::enum_template_type_param(
    EnumCallback<TemplateTypeParam*>* callback
    ) const
{
    return klass_ ? klass_->enum_template_type_param(callback) : 0;
}


////////////////////////////////////////////////////////////////
size_t ClassTypeProxy::enum_template_value_param(
    EnumCallback<TemplateValueParam*>* callback
    ) const
{
    return klass_ ? klass_->enum_template_value_param(callback) : 0;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
