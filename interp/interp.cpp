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

#include <cassert>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include "dharma/environ.h"
#include "zdk/expr.h"
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "zdk/type_system.h"
#include "zdk/variant_util.h"
#include "zdk/zero.h"
#include "abstract_decl.h"
#include "constant.h"
#include "cpu_state_saver.h"
#include "debug_out.h"
#include "errors.h"
#include "context.h"
#include "ident.h"
#include "interp.h"
#include "grammar.h"
#include "type_spec.h"
#include "unary_expr.h"
#include "typez/public/debug_symbol_impl.h"
#include "variant_impl.h"

using namespace std;


Interp::Interp(Context& context, istream& input)
    : context_(&context)
    , lexer_(new Lexer(*this, input, output_))
    , numericBase_(10)
    , lookupTypes_(true)
{
}


Interp::~Interp() throw()
{
}


Context& Interp::context()
{
    assert(context_.get());
    return *context_;
}


RefPtr<Interp> Interp::create(Context& context, istream& input)
{
    return new Interp(context, input);
}


RefPtr<Variant> Interp::eval(RefPtr<Expr> expr)
{
    //DEBUG_OUT << this << ": " << expr->_name() << endl;
    if (!expr)
    {
        throw logic_error("interpreter got null expression");
    }

    RefPtr<Variant> result = expr->eval(*context_);
    if (!result)
    {
        //DEBUG_OUT << expr->_name() << "=null\n";
        return result;
    }
    ostringstream ss;

    variant_print(ss, *result, numericBase_);
    output_ << endl << ss.str();

    /*
    DEBUG_OUT << expr->_name() << ": type_tag="
              << result->type_tag() << ": encoding="
              << result->encoding() << endl;
    DEBUG_OUT << "result=" << ss.str() << endl; */

    return ensure_symbol(*expr, *result, ss.str());
}


bool Interp::parse()
{
    try
    {
        int rc = yyparse();

        lexer_.reset();

        if (rc)
        {
            return false; // parse error
        }
        if (!yylval_)
        {
            error("empty expression?"); // should not happen
            return false;
        }
        return true; // success
    }
    catch (const exception& e)
    {
        error(e.what());
    }
    catch (...)
    {
        error("Unknown exception");
    }
    return false;
}


Interp::Result Interp::run(ExprEvents* events, int numericBase)
{
    context_->reset_frame_setup();
    // expect a clean stack
    //assert(context_->reserved_stack() == 0);
    assert(!context_->top());

    context_->set_events(events);

    if (numericBase)
    {
        numericBase_ = numericBase;
    }
    if (lexer_.get() && !parse())
    {
        return EVAL_ERROR;
    }
    context_->set_macro_helper(NULL);
    try
    {
        // push expression on the context stack
        context_->push(yylval_);

        /***** and this is where the actual work happens *****/
        return resume();
        /*****************************************************/
    }
    catch (const logic_error& e)
    {
        string msg("INTERPRETER ERROR: ");
        msg += e.what();
        error(msg.c_str());
    }
    catch (const exception& e)
    {
        error(e.what());
    }
    return EVAL_ERROR;
}


RefPtr<Variant>
Interp::ensure_symbol(Expr& expr, Variant& var, const string& val)
{
    RefPtr<Variant> result(&var);

    if (var.debug_symbol())
    {
        if (RefPtr<DebugSymbolImpl> impl =
                interface_cast<DebugSymbolImpl*>(var.debug_symbol()))
        {
            impl->set_value(shared_string(val));
        }
    }
    else
    {
        RefPtr<DataType> type = expr.type();
        if (!type)
        {
            throw logic_error("expression with null type");
        }

        if (RefPtr<DebugSymbol> sym = context_->new_const(*type, val))
        {
            // DEBUG_OUT << sym->name() << "=" << sym->value() << endl;
            // sym->read(context_.get());
            result = new VariantImpl(*sym);
        }
        else
        {
    #ifdef DEBUG
            throw logic_error("null constant symbol");
    #endif
            cerr << "*** Warning: null constant symbol\n";
        }
    }
    return result;
}


Interp::Result
Interp::resume(addr_t addr, CPUStateSaver* state, bool* interact)
{
    //note: this is a stack of interpreters, not to be
    //mistaken with the Context stack of pending expressions
    if (!stack_.empty())
    {
        DEBUG_OUT << "resuming interpreter from stack\n";
        return stack_.back()->resume(addr, state, interact);
    }
    if (addr)
    {
        context_->notify_function_return_event(addr);
    }
    RefPtr<Variant> v;
    std::string error;

    for (; error.empty();)
    {
        RefPtr<Expr> expr = context_->top();
        if (!expr)
        {
            break;
        }
        context_->pop();

        try
        {
            v = eval(expr);
            assert(v.get());
        }
        catch (const CallPending&)
        {
            // A function is to be called inside the
            // debugged program; we need to yield control
            // back to the main event loop in DebuggerBase,
            // so that the debuggee gets a chance to run the
            // function. At this point, we have arranged
            // for either a breakpoint to be hit, or a signal
            // to be delivered when the function returns;
            // the interpreter should resume when that happens.
            return EVAL_AGAIN;
        }
        catch (const exception& e)
        {
            error = e.what();
        }
    }
    if (state)
    {
        state->restore_state();
    }
    if (!error.empty())
    {
        context_->notify_error_event(error.c_str());
        return EVAL_ERROR;
    }
    const bool interactive =
        context_->notify_completion_event(v.get(), interact);
    if (interact)
    {
        *interact = interactive;
    }
    return EVAL_DONE;
}


void Interp::error(const char* errorMsg)
{
    context_->notify_error_event(errorMsg, &output_);
}


YYSTYPE Interp::yylval() const
{
    return yylval_;
}


void Interp::set_yylval(YYSTYPE yylval)
{
    yylval_ = yylval;
}


int Interp::yylex()
{
    int result = lexer().yylex();

    switch (result)
    {
    // for fully-qualified type lookups
    case IDENTIFIER: ident_ += yytext(); break;
    case TYPE_NAME: ident_ += yytext(); break;
    case DOUBLE_COLON: ident_ += yytext(); break;
    default: ident_.clear(); break;
    }
    if ((result == '.') || (result == PTR_OP))
    {
        // the name of a member datum or function may
        // follow, disable type-lookups in check_type()
        lookupTypes_ = false;
    }
    else
    {
        lookupTypes_ = true;
    }
    return result;
}


int yylex(YYSTYPE* yylval, void* param)
{
    assert(yylval);
    assert(param);

    Interp* interp = reinterpret_cast<Interp*>(param);

    int result = interp->yylex();
    *yylval = interp->yylval();

    return result;
}


Lexer& Interp::lexer()
{
    assert(lexer_.get());
    return *lexer_;
}


/**
 * Helper class, determines whether the passed name
 * denotes a CPU register
 */
class ZDK_LOCAL IsRegister : private EnumCallback<Register*>
{
public:
    virtual ~IsRegister() throw ()
    { }
    IsRegister(const Context& context, const char* name)
        : thread_(context.thread())
        , name_(name)
    {
        if (*name_ == '%')
        {
            ++name_;
        }
        if (thread_.get())
        {
            thread_->enum_cpu_regs(this);
        }
    }
    void notify(Register* reg)
    {
        assert(reg);
        if (!reg_ && (strcmp(reg->name(), name_) == 0))
        {
            reg_ = reg;
        }
    }
    bool operator ()(RefPtr<Register>& reg) const
    {
        reg = reg_;
        return reg;
    }
private:
    IsRegister(const IsRegister&);
    IsRegister& operator=(const IsRegister&);

    // forbid free-store allocation
    void* operator new(size_t);

    RefPtr<Thread> thread_;
    const char* name_;
    RefPtr<Register> reg_;
};


/**
 * check for "true" or "false";
 * the reason for checking for these here (and not in scan.l)
 * is for compatibilty with C programs
 */
bool Interp::is_bool(const char* name)
{
    const char* value = NULL;
    if (strcmp(name, "false") == 0)
    {
        value = "0";
    }
    else if (strcmp(name, "true") == 0)
    {
        value = "1";
    }
    if (value)
    {
        RefPtr<DataType> type(context_->type_system().get_bool_type(8));
        yylval_ = new Constant(this, *context_->new_const(*type, value));
        return true;
    }
    return false;
}


/**
 * helper called from Interp::type_or_ident
 */
static bool is_namespace(const Context& ctxt, const char* name)
{
    // hack: hard-wire the standard stuff, useful when
    // the debug format does not support namespaces
    if (!strcmp(name, "std") || !strcmp(name, "boost"))
    {
        return true;
    }

    if (Thread* thread = ctxt.thread())
    {
        if (Debugger* debugger = thread->debugger())
        {
            if (Frame* f = thread_current_frame(thread))
            {
                const addr_t pc = f->program_count();
                RefPtr<TranslationUnit> unit =
                    debugger->lookup_unit_by_addr(thread->process(), pc);

                if (unit)
                {
                    bool result = unit->enum_ns(name, NULL);
                    DEBUG_OUT << name << "=" << result << endl;
                    return result;
                }
            }
        }
    }
    return false;
}


#define RETURN_IDENT(n,s) \
    DEBUG_OUT << "IDENT: " << (s)->name() << endl; \
    yylval_ = new Ident(this, (n), (s)); \
    return IDENTIFIER;

#define RETURN_TYPE_NAME(type,id) \
    DEBUG_OUT << "TYPE_NAME: " << (id) << endl; \
    yylval_ = new TypeName(this, (type)); \
    return TYPE_NAME;

#define TRY_AS_TYPE(name) \
    const string id = ident_ + name; \
    if (RefPtr<DataType> type = context_->lookup_type(id.c_str())) { \
        RETURN_TYPE_NAME(type, id); \
    } \
    else { \
        DEBUG_OUT << "lookup_type failed: " << id << endl; \
    }


int Interp::template_type(const char* name)
{
    DEBUG_OUT << name  << endl;
    yylval_ = new Ident(this, name);
    return IDENTIFIER;
}


/**
 * This function is called from the lexer to determine if the
 * token is an identifier or a type name
 */
int Interp::type_or_ident(const char* name)
{
    assert(context_);

    if ((yylval_ = context_->lookup_macro(this, name)).get())
    {
        return IDENTIFIER;
    }
    RefPtr<Register> reg;

    // try symbols first, since looking up types may be more expensive
    if (RefPtr<DebugSymbol> sym = context_->lookup_debug_symbol(name))
    {
        if (!CHKPTR(sym->name())->is_equal(name)
            && strstr(sym->name()->c_str(), "::"))
        {
            // looked for "A" and the symbol name is "A::A?"
            DEBUG_OUT << "\"" << sym->name() << "\""
                      << " != " << "\"" << name << "\"\n";
            TRY_AS_TYPE(name);
        }
        RETURN_IDENT(name, sym);
    }
    else if (IsRegister(*context_, name)(reg))
    {
        yylval_ = new Ident(this, name, false, reg.get());
        return IDENTIFIER;
    }
    else if (is_bool(name))
    {
        return CONSTANT;
    }

    RefPtr<DebugSymbol> sym;
    if (!ident_.empty())
    {
        sym = context_->lookup_debug_symbol((ident_ + name).c_str());
    }
    if (sym)
    {
        string id = ident_ + name;
        RETURN_IDENT(id.c_str(), sym);
    }
    else if (lookupTypes_
        && (name[0] != '%') // misspelled CPU register?
        && !is_namespace(context(), name))
    {
        TRY_AS_TYPE(name);
    }
    // finally, fallback to assuming a debug symbol:
    yylval_ = new Ident(this, name);
    return IDENTIFIER;
}


void Interp::set_string_literal(const char* yytext)
{
    yylval_ = new StringLiteralConstant(this, yytext);
}


bool Interp::debug_enabled()
{
    static bool flag = env::get_bool("ZERO_DEBUG_INTERP");
    return flag;
}


void Interp::switch_to(const RefPtr<Interp>& temp)
{
    if (temp.get())
    {
        stack_.push_back(temp);
    }
    else //if (!stack_.empty())
    {
        assert(!stack_.empty());
        stack_.pop_back();
    }
}


/**
 * spawn alternative interpreter, used when parsing macros
 */
RefPtr<Interp> Interp::alt_interp(istream& input)
{
    RefPtr<Context> tmp(context_->spawn());
    context_->set_macro_helper(NULL);

    RefPtr<Interp> interp(Interp::create(*tmp, input));

    // use same numeric base as current interp
    interp->set_numeric_base(this->numeric_base());
    switch_to(interp);
    return interp;
}


bool Interp::is_32_bit() const
{
    if (context_)
    {
        if (RefPtr<Thread> thread = context_->thread())
        {
            return thread->is_32_bit();
        }
    }
    return false;
}

// Copyright (c) 2004, 2006 Cristian L. Vlasceanu
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
