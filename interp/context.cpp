//
//
// $Id: context.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/expr.h"
#include "zdk/type_system_util.h"
#include "zdk/variant_util.h"
#include "context.h"
#include "debug_out.h"
#include "expr.h"
#ifdef DEBUG
 #include "zdk/symbol.h"
 #include "interp.h" // for debug_out
#endif

using namespace std;


////////////////////////////////////////////////////////////////
Context::Context()
    : conversions_(0)
    , macroHelper_(NULL)
{
}


////////////////////////////////////////////////////////////////
Context::~Context()
{
}


////////////////////////////////////////////////////////////////
void Context::push(const RefPtr<Expr>& expr)
{
    stack_.push(expr);
}


////////////////////////////////////////////////////////////////
void Context::pop()
{
    assert(!stack_.empty());
    stack_.pop();
}


////////////////////////////////////////////////////////////////
RefPtr<Expr> Context::top() const
{
    if (stack_.empty())
    {
        return RefPtr<Expr>();
    }
    return stack_.top();
}


////////////////////////////////////////////////////////////////
RefPtr<DataType> Context::get_int_type(bool isSigned) const
{
    if (isSigned)
    {
        return GET_INT_TYPE(type_system(), int);
    }
    else
    {
        return GET_INT_TYPE(type_system(), unsigned int);
    }
}


////////////////////////////////////////////////////////////////
auto_ptr<Temporary<size_t> > Context::increment_conversion_count()
{
    auto_ptr<Temporary<size_t> >tmp(new Temporary<size_t>(conversions_));
    ++conversions_;

    return tmp;
}


////////////////////////////////////////////////////////////////
void Context::set_events(ExprEvents* events)
{
    events_ = events;
}


////////////////////////////////////////////////////////////////
void Context::notify_function_call_event(addr_t addr, Symbol* func)
{
    if (events_.get())
    {
    #ifdef DEBUG
        if (func)
            DEBUG_OUT << "---{ " << func->name() << endl;
        else
            DEBUG_OUT << "---}\n";
   #endif
        events_->on_call(addr /* return addr */, func);
    }
}


////////////////////////////////////////////////////////////////
void Context::notify_error_event(const char* errorMsg, ostream* os)
{
    if (events_.get())
    {
        events_->on_error(errorMsg);
    }
    else if (os)
    {
        (*os) << errorMsg;
    }
}


////////////////////////////////////////////////////////////////
void Context::notify_warning_event(const char* errorMsg, ostream* os)
{
    if (events_.get())
    {
        events_->on_warning(errorMsg);
    }
    else if (os)
    {
        (*os) << errorMsg;
    }
}


////////////////////////////////////////////////////////////////
bool
Context::notify_completion_event(Variant* variant, bool* interact)
{
    if (variant && events_.get())
    {
        return events_->on_done(variant, interact, this);
    }
    return false;
}


////////////////////////////////////////////////////////////////
void Context::set_macro_helper(MacroHelper* helper)
{
    macroMap_.clear();
    macroHelper_ = helper;
}


////////////////////////////////////////////////////////////////
void Context::map_arguments(const std::string& str)
{
    if (macroHelper_)
    {
        macroHelper_->map_arguments(str, macroMap_);
    }
}


////////////////////////////////////////////////////////////////
RefPtr<Expr>
Context::lookup_macro(Interp* interp, const string& name) const
{
    RefPtr<Expr> expr;

    MacroHelper::ArgMap::const_iterator i = macroMap_.find(name);
    if (i != macroMap_.end())
    {
        // return a deep copy, unevaluated -- this has the effect
        // of expressions being evaluated multiple times when
        // passed as arguments to macros
        expr = i->second->clone(interp);
        assert(strcmp(expr->_name(), i->second->_name()) == 0);
    }
    return expr;
}

////////////////////////////////////////////////////////////////
RefPtr<ExprEvents> Context::events() const
{
    return events_;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
