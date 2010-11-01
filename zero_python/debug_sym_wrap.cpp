//
// $Id: debug_sym_wrap.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <boost/bind.hpp>
#include <boost/python/extract.hpp>
#include "zdk/check_ptr.h"
#include "zdk/data_type.h"
#include "zdk/debug_sym.h"
#include "zdk/shared_string_impl.h"
#include "zdk/type_system.h"
#include "typez/public/debug_symbol.h"
#include "debug_sym_wrap.h"
#include "marshaller.h"

using namespace std;
using namespace boost;
using namespace boost::python;



DebugSymbolWrap::~DebugSymbolWrap() throw()
{
}


const char* DebugSymbolWrap::name() const
{
    RefPtr<SharedString> v;
    if (sym_)
    {
        v = sym_->name();
    }
    return v ? CHKPTR(v->c_str()) : "";
}


const char* DebugSymbolWrap::value() const
{
    SharedString* v = NULL;
    if (sym_)
    {
        v = sym_->value();
    }
    return v ? CHKPTR(v->c_str()) : "";
}


const char* DebugSymbolWrap::type_name() const
{
    SharedString* v = NULL;
    if (sym_)
    {
        v = sym_->type_name();
    }
    return v ? CHKPTR(v->c_str()) : "";
}


boost::python::object DebugSymbolWrap::type() const
{
    RefPtr<DataType> dataType;
    if (sym_)
    {
        dataType = sym_->type();
    }
    if (RefPtr<ClassType> classType = interface_cast<ClassType>(dataType))
    {
        return object(classType);
    }

    return object(dataType);
}


object DebugSymbolWrap::thread() const
{
    RefPtr<Thread> thread;
    if (sym_)
    {
        thread = sym_->thread();
    }
    return object(thread);
}


object DebugSymbolWrap::process() const
{
    RefPtr<Process> process;
    if (sym_)
    {
        if (RefPtr<Thread> thread = sym_->thread())
        {
            process = thread->process();
        }
    }
    return object(process);
}


object DebugSymbolWrap::type_system() const
{
    RefPtr<TypeSystem> typesys;
    if (sym_)
    {
        if (RefPtr<Thread> thread = sym_->thread())
        {
            typesys = interface_cast<TypeSystem*>(thread->process());
        }
    }
    return object(typesys);
}


bool DebugSymbolWrap::has_children() const
{
    return sym_ ? sym_->enum_children(events()) : false;
}


namespace
{
    class DebugSymbolObserver : public DebugSymbolCallback
    {
        mutable Mutex mutex_;
        boost::python::list list_;
        DebugSymbolEvents* events_;

        bool notify(DebugSymbol* sym)
        {
            Lock<Mutex> lock(mutex_);
            sym->read(events_);

            list_.append(RefPtr<DebugSymbolWrap>(new DebugSymbolWrap(sym, events_)));
            return false;
        }

    BEGIN_INTERFACE_MAP(DebugSymbolObserver)
        INTERFACE_ENTRY(DebugSymbolCallback)
    END_INTERFACE_MAP()

    public:
        explicit DebugSymbolObserver(DebugSymbolEvents* events)
            : events_(events)
        {
        }

        boost::python::list list() const
        {
            Lock<Mutex> lock(mutex_);
            return list_;
        }
    };
}


static void
enum_children(RefPtr<DebugSymbol> sym, DebugSymbolCallback* cb)
{
    sym->enum_children(cb);
}


boost::python::list DebugSymbolWrap::children() const
{
    if (!childrenRead_)
    {
        DebugSymbolObserver obs(events());

        ThreadMarshaller::instance().send_command(
                bind(enum_children, sym_, &obs),
                __func__);

        children_ = obs.list();
        childrenRead_ = true;
    }
    return children_;
}


static void add(RefPtr<DebugSymbol> sym, RefPtr<DebugSymbol> child)
{
    sym->add_child(child.get());
}


void DebugSymbolWrap::add_child(RefPtr<DebugSymbolWrap> child)
{
    if (child)
    {
        ThreadMarshaller::instance().send_command(bind(add, sym_, child->sym_), __func__);
    }
}


void DebugSymbolWrap::read_on_main_thread()
{
    if (sym_)
    {
        sym_->read(events());

        // Explicitly limit the ExprEvents object's life time;
        // the DebuggerEngine keeps track of all live ExprEvents
        // instances, and they should not live beyond expression
        // evaluation, otherwise the engine may get confused.
        //
        // Resetting (null-ing) the object completely would not work
        // (because we may need to re-evaluate this DebugSymbol).

        if (context_)
        {
            context_->clone_expr_events();
        }

    #if 0
        clog << __func__ << " " << sym_->name();
        clog << ": events=" << events_;
        if (events_)
        {
            clog << " (" << events_->_name() << ")";
        }
        clog << endl;

        if (sym_->value())
        {
            clog << __func__ << ": " << sym_->value()->c_str() << endl;
        }
    #endif
    }
}


void DebugSymbolWrap::read()
{
    Marshaller& marshal = ThreadMarshaller::instance();
    if (marshal.is_main_thread())
    {
        read_on_main_thread();
    }
    else
    {
        marshal.send_command(bind(&DebugSymbolWrap::read_on_main_thread, this), __func__);
    }
}


void DebugSymbolWrap::set_type_name(DataType* type)
{
    if (DebugSymbolImpl* impl = interface_cast<DebugSymbolImpl*>(sym_.get()))
    {
        impl->set_type_name(type->name());
    }
    else
    {
        throw runtime_error("DebugSymbol::set_type_name(): unknown impl");
    }
}


/**
 * Create a brand new debug symbol on the main debugger thread
 */
static void
create_symbol(RefPtr<DebugSymbol> tmpl,
              RefPtr<DataType> type,
              const string name, // by value on purpose
              addr_t addr,
              RefPtr<DebugSymbol>& sym)
{
    if (RefPtr<Thread> thread = tmpl->thread())
    {
        if (RefPtr<TypeSystem> typesys =
                interface_cast<TypeSystem*>(thread->process()))
        {
            sym = typesys->create_debug_symbol(
                    NULL, // no reader, since we make up the sym
                    thread.get(),
                    type.get(),
                    shared_string(name).get(),
                    addr,
                    tmpl->decl_file(),
                    tmpl->decl_line(),
                    tmpl->is_return_value());
        }
    }
}


RefPtr<DebugSymbolWrap>
DebugSymbolWrap::create(const char* name, DataType* type, addr_t addr)
{
    assert(name);
    RefPtr<DebugSymbol> sym;

    if (sym_)
    {
        ThreadMarshaller::instance().send_command(
            bind(create_symbol, sym_, type, name, addr, ref(sym)),
            __func__);
    }
    return new DebugSymbolWrap(sym, events());
}


void DebugSymbolWrap::set_constant()
{
    if (DebugSymbolImpl* impl = interface_cast<DebugSymbolImpl*>(sym_.get()))
    {
        impl->set_constant();
    }
    else
    {
        throw runtime_error("DebugSymbol::set_constant(): unknown impl");
    }
}


void DebugSymbolWrap::set_value(const char* value)
{
    if (DebugSymbolImpl* impl = interface_cast<DebugSymbolImpl*>(sym_.get()))
    {
        impl->set_value(shared_string(value));
    }
    else
    {
        throw runtime_error("DebugSymbol::set_value(): unknown impl");
    }
}


const char* DebugSymbolWrap::tooltip() const
{
    const char* tip = NULL;

    if (sym_.get())
    {
        tip = sym_->tooltip();
    }
    return tip ? tip : "";
}


void DebugSymbolWrap::set_tooltip(const char* tip)
{
    if (DebugSymbolImpl* impl = interface_cast<DebugSymbolImpl*>(sym_.get()))
    {
        if (tip && !tip[0])
        {
            tip = NULL;
        }
        impl->set_tooltip(tip);
    }
    else
    {
        throw runtime_error("DebugSymbol::set_tooltip(): unknown impl");
    }
}


void DebugSymbolWrap::set_numeric_base(int numericBase)
{
    const int oldBase = events_.numeric_base(sym_.get());
    events_.set_numeric_base(numericBase);
    if (oldBase != events_.numeric_base(sym_.get()))
    {
        set_value(NULL);
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
