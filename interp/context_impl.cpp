//
// $Id: context_impl.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include "zdk/config.h"
#include "context_impl.h"
#include "debug_out.h"
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "dharma/syscall_wrap.h"
#include "dharma/type_lookup_helper.h"
#include "generic/state_saver.h"
#include "zdk/align.h"
#include "zdk/check_ptr.h"
#include "zdk/data_filter.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "zdk/type_tags.h"
#include "zdk/variant_util.h"
#include "zdk/32_on_64.h"
#include "zdk/zobject_adapt.h"
#include "typez/public/debug_symbol_impl.h"
#include "typez/public/types.h"
#include "call_setup.h"
#include "collect_symbol.h"
#include "cpu_state_saver.h"
#include "errors.h"
#include "frame_setup.h"
#include "interp.h"
#include "variant_impl.h"

#include "target/config.h"


using namespace std;


static LookupScope symbol_lookup_scope()
{
    static bool lookupAll = env::get_bool("ZERO_GLOBAL_LOOKUP", false);
    return lookupAll ? LOOKUP_ALL : LOOKUP_MODULE;
}


static LookupScope type_lookup_scope()
{
    return symbol_lookup_scope();
}


/**
 * Initialize the object with the thread in which context
 * the expression evaluation(s) should be performed; also
 * initialize the function in whose scope the symbol lookups
 * are to be done
 */
ContextImpl::ContextImpl(Thread& thread, addr_t addr)
    : thread_(&thread)
    , collect_(false)
{
    if (addr)
    {
        function_ = CHKPTR(thread.symbols())->lookup_symbol(addr);
    }
}


////////////////////////////////////////////////////////////////
ContextImpl::~ContextImpl() throw()
{
}


////////////////////////////////////////////////////////////////
RefPtr<Context> ContextImpl::create(Thread& thread, addr_t addr)
{
    return new ContextImpl(thread, addr);
}


////////////////////////////////////////////////////////////////
RefPtr<Context> ContextImpl::spawn() const
{
    assert(thread_.get());
    RefPtr<ContextImpl> context(new ContextImpl(*thread_));
    context->macroHelper_ = macroHelper_;

    return context;
}


/**
 * In the event that there are several symbol matches, decide
 * which one to keep
 */
void ContextImpl::assign(const RefPtr<DebugSymbol>& sym)
{
    assert(debugSym_);

    DEBUG_OUT << debugSym_->decl_file() << ":"
              << debugSym_->decl_line() << endl;

    if ((debugSym_->decl_line() != sym->decl_line()) &&
        (debugSym_->decl_file() != sym->decl_file()))
    {
        debugSym_ = sym;
    }
}


/**
 * ContextImpl::lookup_debug_symbol() passes THIS as the
 * observer, this method is invoked for each match.
 */
bool ContextImpl::notify(DebugSymbol* sym)
{
    if (sym) try
    {
        DEBUG_OUT << sym->name() << ": "
                  << sym->type()->name() << endl;

        sym->read(this);

        if (collect_ && interface_cast<FunType*>(sym->type()) )
        {
            collect_symbol(type_system(), debugSym_, *sym);
        }
        else
        {
            RefPtr<DebugSymbol> xsym;
            if (thread_)
            {
                if (DataFilter* filt = interface_cast<DataFilter*>(thread_->debugger()))
                {
                    xsym = filt->transform(sym, NULL, this);
                    if (xsym)
                    {
                        sym = xsym.get();
                    }
                }
            }
            if (!debugSym_)
            {
                debugSym_ = sym;
            }
            else if (debugSym_)
            {
                assign(sym);
            }
        }
    }
    catch (const exception& e)
    {
        cerr << "Error reading symbol: " << sym->name();
        cerr << ": " << e.what() << endl;
    }
    return false;
}


////////////////////////////////////////////////////////////////
int ContextImpl::numeric_base(const DebugSymbol* sym) const
{
    if (sym)
    {
        DataType* type = sym->type();

        if (interface_cast<FunType*>(type))
        {
            return 0;
        }
        if (PointerType* pt = interface_cast<PointerType*>(type))
        {
            if (pt->is_cstring())
            {
                return 10;
            }
        }
    }
    return 16;
}


////////////////////////////////////////////////////////////////
bool ContextImpl::is_expanding(DebugSymbol* sym) const
{
    if (sym)
    {
        DataType* type = CHKPTR(sym->type());

        if (sym->depth() >= 1)
        {
            if (PointerType* pt = interface_cast<PointerType*>(type))
            {
                // DO not chase down pointers to objects; this
                // prevents infinite loops for objects that contain
                // pointers or references to self (directly or via
                // some loop such as: object points to parent, parent
                // contains pointers to children)

                if (interface_cast<ClassType*>(pt->pointed_type()))
                {
                    return false;
                }
            }
        }
    }
    static const bool debug = env::get_bool("ZERO_DEBUG_VAR", false);
    if (debug)
    {
        clog << sym->name() << ": " << sym->type()->_name() << endl;
    }
    return true;
}




////////////////////////////////////////////////////////////////
RefPtr<DataType> ContextImpl::lookup_type(const char* name)
{
    RefPtr<DataType> type;

    if (name && thread_ && thread_->debugger())
    {
        addr_t addr = function_.get() ? function_->addr() : 0;

        TypeLookupHelper helper(*thread_,
                                name,
                                addr,
                                type_lookup_scope());

        thread_->debugger()->enum_plugins(&helper);
        type = helper.type();
    }
    return type;
}


////////////////////////////////////////////////////////////////
RefPtr<DebugSymbol>
ContextImpl::lookup_debug_symbol(const char* name, LookupOpt opts)
{
    Temporary<bool> setFlag(collect_, (opts & LKUP_COLLECT));

    if (!thread_)
    {
        throw logic_error(string(__func__) + ": null thread");
    }
    debugSym_.reset();

    LookupScope lookupScope = symbol_lookup_scope();

    bool global = false;
    if (name)
    {
        if (strncmp(name, "std::", 5) == 0)
        {
            lookupScope = LOOKUP_ALL;
            global = true;
        }
        else if (name[0] == ':' && name[1] == ':')
        {
            name += 2;
            global = true;
        }
    }

    if (Debugger* debugger = thread_->debugger())
    {
        if (global)
        {
            debugger->enum_globals( thread_.get(),
                                    name,
                                    // NULL defaults to the
                                    // current function in scope
                                    NULL,
                                    this,
                                    lookupScope,
                                    (opts & LKUP_FUNCS));
        }
        else if (opts & LKUP_FUNCS_ONLY)
        {
            debugger->enum_globals(thread_.get(),
                                     name,
                                     NULL, // see comment above
                                     this,
                                     lookupScope,
                                     true);
        }
        else
        {
            debugger->enum_variables(thread_.get(),
                                     name,
                                     function_.get(),
                                     this,
                                     lookupScope,
                                     (opts & LKUP_FUNCS));
        }
    }
    return debugSym_;
}


////////////////////////////////////////////////////////////////
TypeSystem& ContextImpl::type_system() const
{
    TypeSystem* types = 0;
    if (!thread_)
    {
        throw logic_error(string(__func__) + ": null thread");
    }
    types = interface_cast<TypeSystem*>(thread_.get());
    if (!types)
    {
        throw runtime_error(string(__func__) + ": thread detached");
    }
    return *types;
}


////////////////////////////////////////////////////////////////
void ContextImpl::commit_value(DebugSymbol& sym, Variant& var)
{
    assert(sym.thread());

    if (!sym.thread()->is_live())
    {
        throw runtime_error("cannot modify variables in core file");
    }
    if (DataType* type = sym.type())
    {
        type->write(&sym, &interface_cast<Buffer&>(var));
        assert(var.type_tag() != Variant::VT_NONE);
        sym.read(this); // update
    }
    else assert(false);
}


////////////////////////////////////////////////////////////////
RefPtr<DebugSymbol>
ContextImpl::new_temp_ptr(PointerType& type, addr_t addr)
{
    // Note: not worrying about thread safety here, expression
    // evaluation works on the main debugger thread only.
    static RefPtr<SharedString> str(SharedStringImpl::create(""));

    RefPtr<DebugSymbolImpl> temp =
        DebugSymbolImpl::create( NULL, // no reader: synthesised sym
                                *thread_,
                                type,
                                *str,
                                addr,
                                NULL, // no decl_file, synthesised sym
                                0,    // ditto for decl_line
                                false);

    temp->set_constant();

    return temp;
}


////////////////////////////////////////////////////////////////
RefPtr<DebugSymbol>
ContextImpl::new_const(
    DataType&       type,
    const string&   val)
{
    RefPtr<SharedString> name(shared_string(val));
    return DebugSymbolImpl::create(*thread_, type, val, name.get());
}


////////////////////////////////////////////////////////////////
addr_t ContextImpl::get_stack_top() const
{
    addr_t sp = 0;

    if (frameSetup_.get())
    {
        sp = frameSetup_->stack_pointer();
    }
    if (!sp)
    {
        sp = thread_->stack_pointer();
    }
    return sp;
}


////////////////////////////////////////////////////////////////
void ContextImpl::ensure_frame_setup()
{
    if (!frameSetup_.get())
    {
        frameSetup_.reset(new FrameSetup);
    }
}


////////////////////////////////////////////////////////////////
addr_t ContextImpl::push_arg(const Variant& v, addr_t sp)
{
    if (sp == 0)
    {
        sp = get_stack_top();
    }

    ensure_frame_setup();

    sp = frameSetup_->push_arg(*CHKPTR(thread_), v, sp);

    DEBUG_OUT << "  [" << (void*)sp << "]\n";
    return sp;
}



/**
 * Check whether the function we're about to call returns by val.
 * @return the address on the stack for the returned object,
 *  or 0 if the function does not return an object by value.
 */
addr_t
ContextImpl::check_ret_by_value(CallSetup& setup, Symbol& func)
{
    addr_t retByValueAddr = 0;

    if (DebugInfoReader* debugInfo = setup.reader())
    {
        if (RefPtr<DebugSymbol> ret =
            debugInfo->get_return_symbol(thread_.get(), &func))
        {
            RefPtr<DataType> retType = CHKPTR(ret->type());
            if (interface_cast<ClassType>(retType))
            {
                // allocate an object on the stack
                const size_t size = retType->size();

                ensure_frame_setup();
                retByValueAddr = frameSetup_->reserve_stack(*thread_, size);

                setup.set_ret_addr(retByValueAddr);
                setup.set_ret_type(retType);
            }
        }
    }
    return retByValueAddr;
}


void ContextImpl::reset_frame_setup()
{
    frameSetup_.reset();
}



void ContextImpl::call_function(CallSetup& setup)
{
    assert(thread_.get());
    if (!thread_->is_live())
    {
        throw EvalError("cannot call functions in core files");
    }

    RefPtr<Symbol> func = get_func(setup);
    if (!func)
    {
        string fname = setup.fname()->c_str();
        throw EvalError("Function not found: " + fname);
    }
    frameSetup_.reset(new FrameSetup);

    addr_t retByValue = check_ret_by_value(setup, *func);

    // create an action that will get executed
    // when returning from the function call
    RefPtr<BreakPointAction> act =
        setup.expr()->new_call_return_action(
                                    func,
                                    setup,
                                    *thread_,
                                    setup.reader());

    const word_t pc = INVALID_PC_ADDR;

    vector<RefPtr<Variant> > args(eval_args(*this, setup));

#ifdef __x86_64__
    // on the AMD64 we need to pass in the address where to
    // return the object in register RDI
    // todo: hide this detail behind a good abstraction
    if (retByValue && !thread_->is_32_bit())
    {
        RefPtr<Variant> ret(new VariantImpl);
        put<addr_t>(ret.get(), retByValue);

        args.insert(args.begin(), ret);
    }
#endif // __x86_64__

    addr_t sp = setup.ret_addr();
    assert(sp); // eval_args() post-condition: ret_addr != 0

    // consume as many arguments as possible and pass them
    // via registers (architecture-dependent)
    pass_by_reg(args, sp);

    // push the arguments onto the debugged thread's stack;
    // assume C calling conventions, start at the rightmost arg
    vector<RefPtr<Variant> >::reverse_iterator i = args.rbegin();
    for (; i != args.rend(); ++i)
    {
        const RefPtr<Variant>& v = *i;
        assert(v); // checked by eval_args()
        sp = push_arg(*v, sp);
    }

    // push the address of the returned-by-value object
    if (retByValue)
    {
        Platform::dec_word_ptr(*CHKPTR(thread_), sp);
        thread_poke_word(*thread_, sp, (word_t)retByValue);
    }
    Runnable& task = interface_cast<Runnable&>(*thread_);

    if (RefPtr<Target> target = thread_->target())
    {
        sp = target->setup_caller_frame(*thread_, sp, pc);
    }
    else
    {
        throw logic_error("thread with null target");
    }

    // set program counter to the function we want to call
    task.set_program_count(func->addr());

    DEBUG_OUT << func->demangled_name() << endl;

    assert(thread_->program_count() == func->addr());
    assert(thread_->stack_pointer() == sp);

    Debugger* debugger = thread_->debugger();

    // todo: implement a mechanism for removing the Action
    // if the evaluation is being aborted
    CHKPTR(debugger)->set_sig_action(SIGSEGV, act.get());
    assert(act->ref_count() > 1);

    frameSetup_.reset();

    notify_function_call_event(pc, func.get());
    throw CallPending();
}


/**
 * @return true if the variant corresponds to a string
 * literal typed in by the user
 */
static bool is_string_literal(const Variant& var)
{
    if (var.type_tag() != Variant::VT_POINTER)
    {
        return false;
    }

    DebugSymbol* sym = var.debug_symbol();
    if (!sym || !sym->is_constant() || sym->addr())
    {
        return false;
    }

    if (PointerType* ptr = interface_cast<PointerType*>(sym->type()))
    {
        return ptr->is_cstring();
    }

    return false;
}


////////////////////////////////////////////////////////////////
ContextImpl::VariantList
ContextImpl::eval_args(Context& ctxt, CallSetup& setup)
{
    vector<RefPtr<Variant> > args; // result
    const addr_t sp = eval_args(ctxt, setup, args);
    setup.set_ret_addr(sp);
    return args;
}


////////////////////////////////////////////////////////////////
addr_t
ContextImpl::eval_args(Context& ctxt,
                       CallSetup& setup,
                       VariantList& args)
{
    addr_t sp = setup.ret_addr();
    if (sp == 0)
    {
        sp = get_stack_top();
    }

    ensure_frame_setup();
    frameSetup_->set_red_zone(*CHKPTR(thread_), sp);

    ExprList::iterator i = setup.args().begin();
    const ExprList::iterator end = setup.args().end();
    for (; i != end; ++i)
    {
        RefPtr<Variant> v = (*i)->eval(*this);
        if (!v)
        {
            ostringstream err;
            err << "could not evaluate argument ";
            err << distance(setup.args().begin(), i);
            throw logic_error(err.str());
        }
        // arguments of void type are forbidden
        assert(!interface_cast<VoidType>((*i)->type()));

        if (!is_string_literal(*v))
        {
            args.push_back(v);
        }
        else
        {
            assert(frameSetup_.get());

            RefPtr<Variant> var =
                frameSetup_->push_literal(*CHKPTR(thread_),
                                          v->debug_symbol(),
                                          sp);
            if (var)
            {
                args.push_back(var);
            }
            else
            {
                //todo
            }
        }
    }
    return sp;
}


////////////////////////////////////////////////////////////////
RefPtr<Symbol>
ContextImpl::get_func(const CallSetup& setup) const
{
    RefPtr<Symbol> func;
    RefPtr<SymbolMap> symbolMap = thread_->symbols();

    assert(symbolMap.get());

    if (setup.addr())
    {
        func = symbolMap->lookup_symbol(setup.addr());
    }
    else
    {
        if (!setup.fname())
        {
            throw logic_error("function call with no address nor name");
        }
        assert(setup.fname()->c_str());
        string fname = setup.fname()->c_str();

        FunctionEnum funcs;
        symbolMap->enum_symbols(fname.c_str(), &funcs);

        DEBUG_OUT << fname <<": " << funcs.size() << " matches\n";
        if (!funcs.empty())
        {
            func = funcs.front();
        }
    }
    return func;
}


/**
 * This function assumes that a C++ class instance lives at the
 * given address "addr", and reads the vtable pointer, vptr, from
 * that address. It then reads the address at "index" from the vtable
 * (i.e. the index-th value in the vtable), and uses it to lookup
 * a function, by address.
 */
RefPtr<DebugSymbol> ContextImpl::get_virt_func(addr_t addr, off_t off)
{
    RefPtr<DebugSymbol> result;
    if (thread_.get())
    {
        word_t vptr = 0, fptr = 0;

        thread_read(*CHKPTR(thread_), addr, vptr);

        Platform::inc_word_ptr(*thread_, vptr, off);

        thread_read(*CHKPTR(thread_), vptr, fptr);

        SymbolMap* symbols = CHKPTR(thread_->symbols());
        if (RefPtr<Symbol> sym = symbols->lookup_symbol(fptr))
        {
            DEBUG_OUT << sym->name() << endl;
            // todo: come up with a more effective way of
            // looking up the debug symbol that corresponds
            // to the function (rather than by name)
            RefPtr<SharedString> name = sym->name();
            result = lookup_debug_symbol(name->c_str(), LKUP_FUNCS_ONLY);
        }
    }
    return result;
}


bool
ContextImpl::pass_by_reg(vector<RefPtr<Variant> >& args, addr_t& sp)
{
    if (thread_)
    {
        if (RefPtr<Target> target = thread_->target())
        {
            sp = target->stack_align(*thread_, sp);

            if (target->pass_by_reg(*thread_, args))
            {
                return true;
            }
        }
    }
    return false;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
