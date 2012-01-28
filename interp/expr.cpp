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

#include <stdexcept>
#include "zdk/check_ptr.h"
#include "zdk/types.h"
#include "zdk/variant_util.h"
#include "alt_events.h"
#include "call_setup.h"
#include "context.h"
#include "debug_out.h"
#include "errors.h"
#include "expr.h"
#include "interp.h"
#include "on_call_return.h"
#include "variant_impl.h"



using namespace std;


Expr::Expr(Interp* interp)
    : interp_(interp)
    , strictType_(true)
{
}


Expr::~Expr() throw()
{
}


RefPtr<DataType> Expr::type() const
{
    return type_.ref_ptr();
}


void Expr::set_type(const RefPtr<DataType>& type)
{
    type_ = type;
}


RefPtr<Variant> Expr::eval(Context& context)
{
    if (!value_)
    {
        context.push(this);
        try
        {
            // do the actual evaluation work
            value_ = eval_impl(context);

            expand_macro(context);
        }
        catch (const CallPending&)
        {
            DEBUG_OUT << "expr=" << this << ": pending\n";

            // expression is left on the Context stack
            throw;
        }
        catch (const exception& e)
        {
            DEBUG_OUT << e.what() << endl;
            context.pop();
            throw;
        }
        context.pop();
    }

    return value_;
}


void Expr::set_result(const RefPtr<Variant>& var)
{
    if (var.get())
    {
        assert(var->type_tag() != Variant::VT_NONE);
        value_ = var;

        if (var->debug_symbol())
        {
            type_ = var->debug_symbol()->type();
        }
    }
}


RefPtr<BreakPointAction>
Expr::new_call_return_action(RefPtr<Symbol>  fun,
                             CallSetup&      setup,
                             Thread&         thread,
                             DebugInfoReader* dinfo)
{
    callSetup_ = &setup;
    return new OnCallReturn(this, fun, setup, thread, dinfo);
}


namespace
{
    class MacroEvents : public AltEvents
    {
        RefPtr<Variant>& result_;
        WeakPtr<DataType>& type_;

    public:
        MacroEvents(RefPtr<Variant>& res, WeakPtr<DataType>& type)
            : result_(res), type_(type)
        { }

        bool on_done(Variant* result,
                     bool* interactive,
                     DebugSymbolEvents* events)
        {
            if (result)
            {
                result_ = result;
                if (DebugSymbol* sym = result->debug_symbol())
                {
                    type_ = sym->type();
                }
            }
            return AltEvents::on_done(result, interactive, events);
        }

        void on_error(const char* msg)
        {
            AltEvents::on_error(msg);
            if (Interp::debug_enabled())
            {
                variant_print(cerr << msg << ": ", *result_) << endl;
            }
            if (expr_.get())
            {
                on_done(result_.get(), NULL, NULL);
            }
        }

        ExprEvents* clone() const
        {
            return new MacroEvents(*this);
        }
    };
}


RefPtr<Variant> Expr::expand_macro(Context& context)
{
    DebugSymbol* sym = value_.get() ? value_->debug_symbol() : 0;
    if (sym && interface_cast<MacroType*>(this->type().get()))
    {
        std::stringstream str;
        variant_print(str, *CHKPTR(value_));

        RefPtr<AltEvents> events(new MacroEvents(value_, type_));
        // use a new interpretor, in a temporary context,
        // to evaluate the string that the macro expands to
        RefPtr<Interp> macroInterp = interp()->alt_interp(str);

        if (const char* p = strchr(sym->name()->c_str(), '('))
        {
            macroInterp->context().map_arguments(p + 1);
        }

        if (macroInterp->run(events.get()) == Interp::EVAL_AGAIN)
        {
            events->expr_ = this;
            events->interp_ = interp();
            throw CallPending();
        }
        interp()->switch_to(NULL);
    }
    return value_;
}


RefPtr<CallSetup> Expr::call_setup() const
{
    return callSetup_;
}


RefPtr<Interp> Expr::interp() const
{
    return interp_.ref_ptr();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
