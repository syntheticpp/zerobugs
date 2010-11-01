//
//
// $Id: tests.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <stdio.h>

#include <iostream>
#include <memory>

#include "zdk/shared_string_impl.h"
#include "dharma/canonical_path.h"

#include "stabz/public/block.h"
#include "stabz/public/compile_unit.h"
#include "stabz/public/descriptor.h"
#include "stabz/public/function.h"
#include "stabz/public/variable.h"
#include "stabz/public/fwdtype.h"

#include "typez/public/types.h"
#include "typez/public/type_system.h"

#include "bench/test.h"

using namespace std;

// TODO: add tests for next_line

BEGIN_TEST(test_compile_unit_index, (const char* fileName))
{
    /**
     * Custom events implementation, verifies
     * stab index and compile-unit name.
     */
    class VerifyIndex : public Stab::Events
    {
    public:
        VerifyIndex(SharedString& name, size_t index)
            : name_(&name)
            , index_(index)
        {
            assert(!name_.is_null());
        }

    private:

        void on_section(const char*) {}
        void on_begin(SharedString&, const char*, size_t) {}
        void on_done(size_t) {}

        bool on_stab(
            size_t index,
            const Stab::stab_t& stab,
            const char* str,
            size_t /* strLength */)
        {
            assert(index == index_);
            // assert(name_->is_equal(str));
            // cout << name_->c_str() << endl;
            return true;
        }


    private:

        RefPtr<SharedString> name_;
        size_t index_;
    };


    Stab::Descriptor descriptor(fileName);
    descriptor.init();

    Stab::Descriptor::const_iterator i = descriptor.begin();

    cout << hex;
    for (; i != descriptor.end(); ++i)
    {
        RefPtr<Stab::CompileUnit> unit = *i;


        cout << "["  << unit->begin_addr()
             << ", " << unit->end_addr() << ") "
             << unit->name().c_str()
          // << " section: " << unit->section().c_str()
             << endl;

        VerifyIndex verify(unit->name(), unit->begin_index());
        Stab::Events* events = &verify;

        descriptor.for_each_stab(
                unit->section(),
                unit->begin_index(),
                unit->begin_index() + 1,
                &events, 1);
    }
    cout << dec;
}
END_TEST()



/*  Make sure that lookup by address lands on the
    correct compile unit. */
BEGIN_TEST(test_compile_unit_addr, (const char* fileName))
{
    Stab::Descriptor descriptor(fileName);
    descriptor.init();

    addr_t addr = reinterpret_cast<addr_t>(&test_compile_unit_addr);
    RefPtr<Stab::CompileUnit> unit = descriptor.get_compile_unit(addr);

    assert(!unit.is_null());

    // cout << hex << addr << dec << endl;
    cout << "UNIT NAME: " << unit->name().c_str() << endl;
    cout << "CANONICAL: " << canonical_path(__FILE__).c_str() << endl;
    //assert( unit->name().is_equal(canonical_path(__FILE__).c_str()) );

    /* additional test: CompileUnit::lookup_function(), strict  */
    RefPtr<Stab::Function> fun = unit->lookup_function(addr, true);
    assert(fun->begin_addr() == addr);

    /* void (Stab::CompileUnit::*pf)(void) =
        &Stab::CompileUnit::parse;
    Stab::CompileUnit* u = unit.get();
    addr = (addr_t)(u->*pf);
    unit = descriptor.get_compile_unit(addr);

    assert(!unit.is_null());
    cout << unit->name()->c_str() << endl; */
}
END_TEST()



BEGIN_TEST(test_line_to_addr, (const char* fileName))
{
    static const size_t line = __LINE__ - 2;

    Stab::Descriptor descriptor(fileName);
    descriptor.init();

    //RefPtr<SharedString> path(shared_string(canonical_path(__FILE__).c_str()));
    RefPtr<SharedString> path(shared_string(__FILE__));

    std::vector<addr_t> addrs = descriptor.line_to_addr(path, line);

    assert(!addrs.empty());

    /* std::vector<addr_t>::const_iterator i = addrs.begin();
    for (; i != addrs.end(); ++i)
    {
        cout << hex << *i << dec << endl;
    } */
    static const addr_t this_addr = (addr_t)&test_line_to_addr;
    // cout << "this_test's addr=" << hex << this_addr << dec << endl;

    assert(find(addrs.begin(), addrs.end(), this_addr) != addrs.end());
}
END_TEST()



static void print_variable(RefPtr<Stab::Variable> var)
{
    cout << hex << var->offset() << dec
         << ' ' << var->name().c_str() << ' ';

    WeakDataTypePtr type = var->type();
    assert(type.ref_ptr());
    cout << type->name()->c_str();

    cout << endl;
}


static void dump_variables(RefPtr<Stab::Block> block)
{
    block->for_each_var(print_variable);
    block->for_each_block(dump_variables);
}


BEGIN_TEST(test_parse, (const char* fileName, addr_t addr))
{
    size_t wordSize = __WORDSIZE;
    // cout << hex << addr << dec << endl;
    NativeTypeSystem types(wordSize);

    /* open STABS for this file */
    Stab::Descriptor desc(fileName);
    desc.init_and_parse(types);

    RefPtr<Stab::CompileUnit> unit = desc.get_compile_unit(addr);
    assert(unit);

    RefPtr<Stab::Function> fun = unit->lookup_function(addr, true);

    assert(fun);
    assert(fun->begin_addr() == addr);

    cout << fun->name().c_str() << endl;

    dump_variables(fun);
}
END_TEST()


/* Helper function that assumes the name of the
   tested variables is v_<type> where <type> is
   the variable's type.

   Removes the v_ prefix, changes all underscores
   to blanks, then compares the string to the
   type name and asserts they are equal.
 */
static void test_variable(RefPtr<Stab::Variable> var)
{
    WeakDataTypePtr type = var->type();
    assert(type.ref_ptr());
    string varName(var->name().c_str());

    cout << varName << " type: " << type->name()->c_str() << endl;

    if (varName.find("v_") == 0)
    {
        string::iterator i = varName.begin();
        for (; i != varName.end(); ++i)
        {
            if (*i == '_')
            {
                *i = ' ';
            }
        }
        // strip the v_ part
        varName = varName.substr(2);
        cout << "comparing: " << varName << " to ";
        cout << type->name()->c_str() << endl;

        assert(varName == type->name()->c_str());
    }
}


static void test_block(RefPtr<Stab::Block> blk)
{
    blk->for_each_var(test_variable);
    blk->for_each_block(test_block);
}


BEGIN_TEST(test_builtin, ())
{
#if 0
    Stab::Descriptor desc("builtin");
    desc.init();

    TypeSystemImpl types;

    Stab::Descriptor::iterator i = desc.begin();
    for (; i != desc.end(); ++i)
    {
        assert(!i->is_null());

        // clog << i->get() << ": " << (*i)->name().c_str();
        (*i)->parse(types);

        vector<RefPtr<Stab::Function> > funcs = (*i)->functions();

        vector<RefPtr<Stab::Function> >::const_iterator f = funcs.begin();

        for (; f != funcs.end(); ++f)
        {
            assert(!f->is_null());
            (*f)->for_each_block(test_block);
        }
    }
 /*
    for (i = desc.begin(); i != desc.end(); ++i)
    {
        assert(!i->is_null());
        clog << i->get() << ": " << (*i)->name().c_str();

        (*i)->parse();
    }
   */
#endif
}
END_TEST()


BEGIN_TEST(test_typedef, ())
{
    static const size_t wordSize = __WORDSIZE;
    NativeTypeSystem types(wordSize);

    Stab::Descriptor desc("typedef");
    desc.init();
    Stab::Descriptor::iterator i = desc.begin();
    for (; i != desc.end(); ++i)
    {
        vector<RefPtr<Stab::Function> > funcs = (*i)->functions();

        // cout << (*i)->name()->c_str();
        // cout << ": " << funcs.size() << " functions\n";

        (*i)->parse(types);

        vector<RefPtr<Stab::Function> >::const_iterator f = funcs.begin();
        for (; f != funcs.end(); ++f)
        {
            // cout << hex << (*f)->begin_addr() << dec << endl;
            (*f)->for_each_block(test_block);
        }
    }
}
END_TEST()



static void test_fwd_var(const RefPtr<Stab::Variable>& var)
{
    assert(var);

    WeakDataTypePtr type = var->type();
    // cout << type.name().c_str() << endl;

    if (ClassType* klass = interface_cast<ClassType*>(type.ref_ptr().get()))
    {
        const size_t n = klass->member_count();
        for (size_t i = 0; i != n; ++i)
        {
            const Member* member = klass->member(i);
            assert(member);

            if (member->name()->is_equal("b_"))
            {
                type = member->type();
                assert(type.ref_ptr());

                /* it is a reference; currently reference types
                   are implemented as pointers. */
                RefPtr<PointerType> ptrType = interface_cast<PointerType>(type.ref_ptr());
                assert(ptrType);

                type = ptrType->pointed_type();
                assert(type.ref_ptr());
                cout << type->name()->c_str() << endl;

                /* the final goal of the test:
                   expect this type to be fully resolved */
                RefPtr<Stab::ForwardType> fwd = interface_cast<Stab::ForwardType>(type.ref_ptr());

                assert(fwd);
                assert(fwd->link());
            }
        }
    }
}


static void test_fwd(RefPtr<Stab::Block> blk)
{
    blk->for_each_var(test_fwd_var);
    blk->for_each_block(test_fwd);
}


BEGIN_TEST(test_ab_forward, ())
{
    static const size_t wordSize = __WORDSIZE;
    NativeTypeSystem types(wordSize);

    Stab::Descriptor desc("abfwd");
    desc.init_and_parse(types);

    Stab::Descriptor::iterator i = desc.begin();
    for (; i != desc.end(); ++i)
    {
        vector<RefPtr<Stab::Function> > funcs = (*i)->functions();
        // cout << (*i)->name()->c_str();
        // cout << ": " << funcs.size() << " function(s)\n";

        vector<RefPtr<Stab::Function> >::const_iterator f = funcs.begin();
        for (; f != funcs.end(); ++f)
        {
            cout << "### " << (*f)->name().c_str() << " ###\n";
            (*f)->for_each_block(test_fwd);
        }
    }
}
END_TEST()



BEGIN_TEST(test_function_type, (const char* fileName))
{
    static const size_t wordSize = __WORDSIZE;
    NativeTypeSystem types(wordSize);

    void (*p_fun)(const char*) = &test_function_type;
    Stab::Descriptor desc(fileName);

    desc.init_and_parse(types);

    addr_t addr = reinterpret_cast<addr_t>(p_fun);
    RefPtr<Stab::CompileUnit> unit = desc.get_compile_unit(addr);

    assert(!unit.is_null());

    RefPtr<Stab::Function> fun = unit->lookup_function(addr, false);
    assert(fun);
    fun->for_each_block(test_block);
}
END_TEST()

struct Fubar
{
    const char* name_;
    Fubar() : name_("fubar") {}
};


void fubar(auto_ptr<Fubar> p)
{
    clog << p->name_ << endl;
    p->name_ = "barfu";

    clog << p->name_ << endl;
}

int main(int argc, char* argv[])
{
//    auto_ptr<Fubar> fp(new Fubar);
//    fubar(fp);

    test_compile_unit_index(argv[0]);
    test_compile_unit_addr(argv[0]);
    test_line_to_addr(argv[0]);

    test_parse(argv[0], (addr_t)&test_compile_unit_index);
    test_parse(argv[0], (addr_t)&main);
    test_parse(argv[0], (addr_t)&test_variable);
    test_parse(argv[0], (addr_t)&test_line_to_addr);

    test_builtin();
    test_typedef();
    test_ab_forward();
    test_parse(argv[0], (addr_t)&fubar);

    test_function_type(argv[0]);

    test_parse(argv[0], (addr_t)&fubar);
    cout << "All tests passed.\n";

    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
