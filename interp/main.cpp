// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: main.cpp 714 2010-10-17 10:03:52Z root $
//
// TEST PROGRAM
//
#include <math.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include "interp.h"
#include "zdk/expr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/type_system.h"
#include "zdk/variant_util.h"
#include "typez/public/types.h"
#include "bench/test.h"
#include "test_context.h"
#include "test_debug_symbol.h"

using namespace std;


////////////////////////////////////////////////////////////////
// Create some data types and variables
BEGIN_TEST(populate_context,(TestContext& context))
{
// --- build some data types
    RefPtr<SharedString> typeNameInt = shared_string("int");
    RefPtr<SharedString> typeNameUInt = shared_string("unsigned int");
    RefPtr<SharedString> typeNameInt64 = shared_string("long long");
    RefPtr<SharedString> typeNameUInt64 = shared_string("unsigned long long");
    RefPtr<SharedString> typeNameShort = shared_string("short");
    RefPtr<SharedString> typeNameUShort = shared_string("unsigned short");
    RefPtr<SharedString> typeNameClassA = shared_string("ClassA");
    RefPtr<SharedString> typeNameClassB = shared_string("ClassB");

    RefPtr<SharedString> typeNameBits = shared_string("bits");
    RefPtr<SharedString> typeNameDouble = shared_string("double");

    RefPtr<DataType> typeInt = context.type_system().get_int_type(
        typeNameInt.get(), 32, true);

    RefPtr<DataType> typeUInt = context.type_system().get_int_type(
        typeNameUInt.get(), 32, false);

    RefPtr<DataType> typeShort = context.type_system().get_int_type(
        typeNameShort.get(), 16, true);

    RefPtr<DataType> typeUShort = context.type_system().get_int_type(
        typeNameShort.get(), 16, false);

    RefPtr<DataType> typeField = context.type_system().get_int_type(
        typeNameBits.get(), 7, true);

    RefPtr<DataType> typeInt64 = context.type_system().get_int_type(
        typeNameInt64.get(), 64, true);

    RefPtr<DataType> typeUInt64 = context.type_system().get_int_type(
        typeNameUInt64.get(), 64, false);

    RefPtr<DataType> typeDouble = context.type_system().get_float_type(
        typeNameDouble.get(), sizeof(double));

   // pointer to double
    RefPtr<DataType> typePtrDouble =
        context.type_system().get_pointer_type(typeDouble.get());

    // reference to int
    RefPtr<DataType> typeRefInt =
        context.type_system().get_reference_type(typeInt.get());

// -- a couple of classes (B derived from A)
    RefPtr<ClassTypeImpl> typeClassA =
        new ClassTypeImpl(&context.type_system(),
                          typeNameClassA.get(), sizeof(int));
    RefPtr<ClassTypeImpl> typeClassB =
        new ClassTypeImpl(&context.type_system(),
                          typeNameClassB.get(),
                          2 * sizeof(int));

    RefPtr<SharedString> memA = shared_string("i_");
    RefPtr<SharedString> memB = shared_string("j_");

    typeClassA->add_member(memA, NULL, 0, typeInt->bit_size(), *typeInt);
    typeClassB->add_member(memB, NULL, 0, typeInt->bit_size(), *typeInt);

    typeClassB->add_base(*typeClassA, 0, ACCESS_PUBLIC, false);

// --- add some variables
    context.add_debug_symbol(new TestDebugSymbol(*typeInt, "i", "1"));
    context.add_debug_symbol(new TestDebugSymbol(*typeInt, "j", "1"));
    context.add_debug_symbol(new TestDebugSymbol(*typeInt, "m", "-1"));
    context.add_debug_symbol(new TestDebugSymbol(*typeUInt, "k", "2"));
    context.add_debug_symbol(new TestDebugSymbol(*typeInt64, "x", "123"));
    context.add_debug_symbol(new TestDebugSymbol(*typeUInt64, "y", "456"));
    context.add_debug_symbol(new TestDebugSymbol(*typeField, "bits", "2"));
    context.add_debug_symbol(new TestDebugSymbol(*typeDouble, "pi", "3.1419"));

    context.add_debug_symbol(new TestDebugSymbol(*typeClassB, "b", "0x40001234"));
    context.add_debug_symbol(new TestDebugSymbol(*typeShort, "s", "1"));
    context.add_debug_symbol(new TestDebugSymbol(*typeShort, "xs", "0xffff"));
    context.add_debug_symbol(new TestDebugSymbol(*typeUShort, "us", "1"));

    {
        RefPtr<TestDebugSymbol> ptr =
            new TestDebugSymbol(*typePtrDouble, "dptr", "0x40001000");

        RefPtr<TestDebugSymbol> sym = new TestDebugSymbol(*typeDouble, "", "1.23");

        ptr->add_child(sym.get());
        context.add_debug_symbol(ptr);
    }
    {
        // add a reference symbol
        RefPtr<TestDebugSymbol> ref =
            new TestDebugSymbol(*typeRefInt, "iref", "0x40001000");
        RefPtr<TestDebugSymbol> sym = new TestDebugSymbol(*typeInt, "i", "123");

        ref->add_child(sym.get());
        context.add_debug_symbol(ref);
    }

    {
        std::ostringstream tmp;
        tmp << std::numeric_limits<int64_t>::max();
        context.add_debug_symbol(
            new TestDebugSymbol(*typeInt64, "max_int64", tmp.str().c_str()));
    }
    {
        std::ostringstream tmp;
        tmp << std::numeric_limits<uint64_t>::max();
        context.add_debug_symbol(
            new TestDebugSymbol(*typeUInt64, "max_uint64", tmp.str().c_str()));
    }
}
END_TEST()


class TestExprEvents : public SubjectImpl<ExprEvents>
{
    RefPtr<Variant> var_;
    ostream& output_;

    bool on_done(Variant* var, bool*, DebugSymbolEvents*)
    {
        var_ = var;
        return false;
    }

    void on_error(const char* errmsg) { output_ << endl << errmsg; }

    void on_warning(const char* errmsg) { output_ << endl << errmsg; }

    bool on_event(Thread*, addr_t) { return false; }

    void on_call(addr_t, Symbol*) { }

    ExprEvents* clone() const
    { return new TestExprEvents(*this); }

public:
    explicit TestExprEvents(ostream& output) : output_(output) {}
    TestExprEvents(const TestExprEvents& that)
        : var_(that.var_)
        , output_(that.output_)
    { }

    RefPtr<Variant> var() const { return var_; }
};


////////////////////////////////////////////////////////////////
RefPtr<Variant>
evaluate(Context& ctxt, const char* expr, const char* expect = 0)
{
    cout << "------------------------\n";
    cout << "evaluating: " << expr << endl;

    istringstream input(expr);

    RefPtr<Interp> interp = Interp::create(ctxt, input);

    RefPtr<TestExprEvents> events = new TestExprEvents(interp->output_stream());

    interp->run(events.get());

    string output = interp->output();
    cout << output << endl;

    if (expect)
    {
        if (output != expect)
        {
            cerr << "expected: " << expect << endl;
            abort();
        }
    }
    return events->var();
}


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_int_additive_expr,(Context& context))
{
    RefPtr<Variant> v;

    ostringstream expr;
    expr << numeric_limits<unsigned long long>::max() << "ULL";
    v = evaluate(context, expr.str().c_str());
    assert(v.get());

    v = evaluate(context, "i + 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 2);

    v = evaluate(context, "i + j");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 2);

    v = evaluate(context, "i - j");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 0);

    v = evaluate(context, "i + (i + j) + j");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 4);

    v = evaluate(context, "i + bits");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 3);

    // test int64 stuff
    v = evaluate(context, "x + i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT64);
    assert(v->int64() == 124);

    v = evaluate(context, "y + i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT64);
    assert(v->uint64() == 457);

    v = evaluate(context, "y + x");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT64);
    assert(v->uint64() == 579);

    v = evaluate(context, "x + x");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT64);
    assert(v->int64() == 246);

    v = evaluate(context, "x - y");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT64);
    assert(v->uint64() == (uint64_t)-333);
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_float_expr,(Context& context))
{
    RefPtr<Variant> v;

    {
        ostringstream expr;
        expr << numeric_limits<long double>::min() << "L";
        v = evaluate(context, expr.str().c_str());
        assert(v.get());
    }
    {
        ostringstream expr;
        expr << numeric_limits<double>::max();
        v = evaluate(context, expr.str().c_str());
        assert(v.get());
    }
    {
        ostringstream expr;
        expr << numeric_limits<double>::max() << "f";
        v = evaluate(context, expr.str().c_str());
        assert(v.get());
    }

    v = evaluate(context, "pi");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 3.1419);

    v = evaluate(context, "pi + .01");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 3.1519);

    v = evaluate(context, "pi - i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 2.1419);

    v = evaluate(context, "i - pi");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == -2.1419);

    v = evaluate(context, "i + pi");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 4.1419);

    v = evaluate(context, "pi + pi");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(fabs(v->long_double() - 3.1419 * 2) < numeric_limits<double>::epsilon());
}
END_TEST()

////////////////////////////////////////////////////////////////
BEGIN_TEST( test_operator_addr,(Context& context))
{
    RefPtr<Variant> v = evaluate(context, "&i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_POINTER);
    assert(v->pointer() == 0x40001234);

    v = evaluate(context, "&i - 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_POINTER);
    assert(v->pointer() == 0x40001230);

    v = evaluate(context, "&i + 2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_POINTER);
    assert(v->pointer() == 0x4000123c);

    // expect 'non l-value in unary &'
    v = evaluate(context, "&(i + 2)");
    assert(v.is_null());

    // expect 'non l-value in unary &'
    v = evaluate(context, "&(i--)");
    assert(v.is_null());

    v = evaluate(context, "&(++i)");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_POINTER);
    assert(v->pointer() == 0x40001234);
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_comparisons,(Context& context))
{
    // integer to integer
    RefPtr<Variant> v = evaluate(context, "1 == 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "1 != 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 0);

    v = evaluate(context, "i > 0");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "0 < i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "i >= 0");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "0 <= i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "0 != i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    // integer to float
    v = evaluate(context, "1.0 == 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "1.0 != 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 0);

    v = evaluate(context, "i > 0.1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "0.3 < i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "i >= 0.2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, ".5 <= i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "0.123 != i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "1.3 > i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "i <= 1.2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "1.5 >= i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "i != 1.23");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_postfix,(Context& context))
{
    RefPtr<Variant> v;

    v = evaluate(context, "i++");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 2);

    v = evaluate(context, "i--");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 2);

    v = evaluate(context, "i--");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "++i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "++pi");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 4.1419);

    v = evaluate(context, "pi--");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 4.1419);
}
END_TEST()


////////////////////////////////////////////////////////////////
void test_postfix_member(Context& context)
{
/*
    RefPtr<Variant> v;
    v = evaluate(context, "b.j_");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    v = evaluate(context, "(&b)->j_");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
 */
}


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_multiplicative_expr,(Context& context))
{
    RefPtr<Variant> v;

    v = evaluate(context, "i * k");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT32);
    assert(v->int64() == 2);

    v = evaluate(context, "m * k");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT32);
    assert(v->uint64() == 4294967294U);

    v = evaluate(context, "k * m");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT32);
    assert(v->uint64() == 4294967294U);

    v = evaluate(context, "-1 * m");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "-1 * k");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT32);
    assert(v->uint64() == 4294967294U);

    v = evaluate(context, "8 % 3");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 2);

    v = evaluate(context, "k * 3.1419");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 6.2838);

    v = evaluate(context, "3.1419 * 2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 6.2838);

    v = evaluate(context, "3.14 / 2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 1.57);

    v = evaluate(context, "3.14 % 2");
    assert(v.is_null());

    v = evaluate(context, "4 / 2.0");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 2);
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_bitwise_expr,(Context& context))
{
    RefPtr<Variant> v;

    v = evaluate(context, "s << 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 2);

    v = evaluate(context, "xs << 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 0xfffe);

    v = evaluate(context, "s << -1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == -2147483648LL);

    v = evaluate(context, "us << -1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT32);
    assert(v->int64() == 2147483648LL);

    v = evaluate(context, "max_int64 << 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT64);
    assert(v->int64() == (int64_t)0xfffffffffffffffeLL);

    v = evaluate(context, "max_uint64 << 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT64);
    assert(v->uint64() == 0xfffffffffffffffeULL);

    v = evaluate(context, "0x5 & 0x4");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 0x4);

    v = evaluate(context, "~1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == -2);

   /*
    v = evaluate(context, "(unsigned) ~1");
    assert(v.get());
    //assert(v->type_tag() == Variant::VT_UINT32);
    assert(v->type_tag() == Variant::VT_ULONG);
    assert(v->int64() == (unsigned)-2);
*/
    v = evaluate(context, "~(char)1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT8);
    assert(v->int64() == -2);
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_assignment,(Context& context))
{
    RefPtr<Variant> v;

    v = evaluate(context, "i += 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 2);

    v = evaluate(context, "i /= 2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "i *= 3");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 3);

    v = evaluate(context, "i %= 2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    v = evaluate(context, "i <<= 2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 4);

    v = evaluate(context, "i >>= 2");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 1);

    // expect 'invalid operands of types double and int to operator <<'
    v = evaluate(context, "pi <<= 1");
    assert(v.is_null());

    v = evaluate(context, "pi += i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 4.1419);

    v = evaluate(context, "pi -= i");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 3.1419);

    evaluate(context, "pi -= (i + foo) - 2");
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_type_spec,(Context& context))
{
    RefPtr<Variant> v;
    evaluate(context, "(int char)",
        "(int char\nmore than one type specified");
    evaluate(context, "(char int)",
        "(char int\nmore than one type specified");
    evaluate(context, "(short char)i",
        "(short char\nshort specifier invalid for char");
    evaluate(context, "(char short)i",
        "(char short\nshort specifier invalid for char");
    evaluate(context, "(long char)i",
        "(long char\nlong specifier invalid for char");
    evaluate(context, "(char long)i",
        "(char long\nlong specifier invalid for char");

    evaluate(context, "(short long)i",
        "(short long)i\nlong and short cannot be specified together");
    evaluate(context, "(long short)i",
        "(long short)i\nlong and short cannot be specified together");
    evaluate(context, "(short int long)i",
        "(short int long)i\nlong and short cannot be specified together");
    evaluate(context, "(int long short)i",
        "(int long short)i\nlong and short cannot be specified together");

    evaluate(context, "(short double)i",
        "(short double\nshort, signed or unsigned invalid for non-int");

    evaluate(context, "(double short)i",
        "(double short)i\nshort, signed or unsigned invalid for non-int");

    evaluate(context, "(long double)i");
    evaluate(context, "(double long)i");

    evaluate(context, "(long long)i");
    evaluate(context, "(long long long)i");

    evaluate(context, "(short short)i");

    evaluate(context, "(char unsigned)i");
    evaluate(context, "(unsigned char)i");

    evaluate(context, "(unsigned float)i");
    evaluate(context, "(float unsigned)i");

    evaluate(context, "(short void)i");
    evaluate(context, "(void short)i");

    evaluate(context, "(unsigned void)i");
    evaluate(context, "(void unsigned)i");

    evaluate(context, "(int32_t)i");
    v = evaluate(context, "(uint32_t)m");
    assert(v.get());
    cout << "v->type_tag() = " << v->type_tag() << endl;
    assert(v->type_tag() == Variant::VT_UINT32);
    assert(v->int64() == (unsigned int)-1);

    v = evaluate(context, "(unsigned short)-1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_UINT16);
    assert(v->int64() == 65535);

    evaluate(context, "(short int32_t)i",
        "(short int32_t)\nmore than one type specified");
/*
    evaluate(context, "(class int32_t)i",
        "(class int32_t\nint is not a class nor a struct");
    evaluate(context, "(struct uint32_t)i",
        "(struct uint32_t\nunsigned int is not a class nor a struct");
    evaluate(context, "(union int32_t)i",
        "(union int32_t\nint is not a union");
 */
    evaluate(context, "(short)i");
    evaluate(context, "(unsigned short)i");
    evaluate(context, "(short unsigned)i");
    evaluate(context, "(short signed)i");
    evaluate(context, "(signed short)i");
    evaluate(context, "(unsigned char)i");
    evaluate(context, "(char unsigned)i");
    evaluate(context, "(char signed)i");
    evaluate(context, "(signed char)i");
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST( test_pointer_arithmetic,(Context& context))
{
    RefPtr<Variant> v;

    v = evaluate(context, "dptr + 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_POINTER);
    assert(v->pointer() == 0x40001008);

    v = evaluate(context, "1 - dptr", "1 - dptr\n"
        "invalid operands of types int and double* to operator -");
    assert(v.is_null());

    v = evaluate(context, "*dptr");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_DOUBLE);
    assert(v->long_double() == 1.23);

    v = evaluate(context, "iref + 1");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == 124);

    v = evaluate(context, "1 - iref");
    assert(v.get());
    assert(v->type_tag() == Variant::VT_INT32);
    assert(v->int64() == -122);

    v = evaluate(context, "*iref");
    assert(v.is_null());
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST(test_cast,(Context& context))
{
    evaluate(context, "static_cast<double>(i)");
}
END_TEST()


////////////////////////////////////////////////////////////////
BEGIN_TEST(test_abstract_decl,(Context& context))
{
// todo: elaborate
    evaluate(context, "(int (*))0");
    evaluate(context, "(int (*)())0");
    evaluate(context, "(int (*)()())0");
    evaluate(context, "*(int *)0");
    evaluate(context, "(int **)0");
    evaluate(context, "(int ***)0");
    evaluate(context, "*(int *)&pi");
    evaluate(context, "(int *)&pi");

    evaluate(context, "(int (*)[])0");
    evaluate(context, "(int (*)[]())0");
}
END_TEST()


////////////////////////////////////////////////////////////////
int main()
{
#if defined(DEBUG_OBJECT_LEAKS)
     ObjectManager::instance();
#endif
    try
    {
        RefPtr<TestContext> context = new TestContext;

        populate_context(*context);

        test_cast(*context);

        test_pointer_arithmetic(*context);

        test_int_additive_expr(*context);

        test_float_expr(*context);

        test_operator_addr(*context);

        test_comparisons(*context);

        test_postfix(*context);
        test_postfix_member(*context);

        test_multiplicative_expr(*context);

        test_bitwise_expr(*context);

        test_assignment(*context);

        test_type_spec(*context);

        //FIXME
        //test_abstract_decl(*context);
/* todo: test_fully_qualified_names
        evaluate(*context, "foo::bar");
        evaluate(*context, "::foo::bar");
        evaluate(*context, "::foo::bar::baz");
        evaluate(*context, "::foo");
 */
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
