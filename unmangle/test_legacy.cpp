//
// $Id: test_legacy.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#ifdef NDEBUG
 #undef NDEBUG
#endif
#include <assert.h>
#include <exception>
#include <iostream>
#include <string>
#define CPLUS_DEMANGLE_COMPAT
//#include "config.h"
//#ifdef USE_SHORT_STD_SUBST
 //#undef USE_SHORT_STD_SUBST
//#endif
#include "legacy.h"

using namespace std;

static void test(
    const char* mangledName,
    const char* decoded,
    bool tor = false,
    int flags = 0)
{
    legacy::Decoder<char> d(mangledName, flags);
    size_t len = 0;
    if (char* result = tor ? d.parse_global_ctor_dtor(NULL, &len)
                           : d.parse(NULL, &len))
    {
        assert(strlen(result) == len);
        cout << '\"' << result << '\"' << endl;
        if (strcmp(result, decoded) != 0)
        {
            cout << "\nInput: " << mangledName << endl;
            cout << "------------------------------------------\n";
            cout << "Expected: " << decoded << endl << endl;
            cout << "Got     : " << result << endl;
#if 1
            cout << "------------------------------------------\n";
            size_t i = 0;
            for (; decoded[i] == result[i]; ++i)
            {
            }
            cout << "EXPECTED: " << decoded + i << endl;
            cout << "GOT     : " << result + i << endl;
#endif
            assert(false);
        }
        free(result);
    }
    else if (decoded)
    {
        cout << "Expected: " << decoded << endl;
        assert(false);
    }
}
int main()
{
    try
    {
        test("_Dmain", NULL);
        test("", NULL);
        test("__vt_10bad_typeid", "bad_typeid virtual table");
        test("always_noconv__CQ24_STLt7codecvt3ZcZcZ11__mbstate_t",
            "_STL::codecvt<char, char, __mbstate_t>::always_noconv(void) const");
        test("widen__CQ24_STLt9basic_ios2ZwZQ24_STLt11char_traits1Zwc",
             "_STL::basic_ios<wchar_t, _STL::char_traits<wchar_t> >::widen(char) const");
        test("assign__Q24_STLt11char_traits1ZcRcRCc",
             "_STL::char_traits<char>::assign(char&, char const&)");
        test("allocate__Q24_STLt13__debug_alloc1ZQ24_STLt12__node_alloc2b0i0Ui",
             "_STL::__debug_alloc<_STL::__node_alloc<false, 0> >::allocate(unsigned int)");
        test("distance__H1ZPCc_4_STLRCX01T1_Q34_STLt15iterator_traits1ZX0115difference_type",
            "_STL::iterator_traits<char const*>::difference_type _STL::distance<char const*>"
            "(char const* const&, char const* const&)");
        test("distance__H1ZPCc_RCX01T0_Q34_STLt15iterator_traits1ZX0115difference_type",
            "_STL::iterator_traits<char const*>::difference_type distance<char const*>"
            "(char const* const&, char const* const&)");
        test("foo__FUiPv", "foo(unsigned int, void*)");
        test("__10bad_typeid", "bad_typeid::bad_typeid(void)");
        test("_._10bad_typeid", "bad_typeid::~bad_typeid(void)");
        test("_4_STL.cerr", "_STL::cerr");
        test("_Assert__Q24_STLt18__stl_debug_engine1ZbPCcT1i",
            "_STL::__stl_debug_engine<bool>::_Assert(char const*, char const*, int)");

        // array args
        test("foo__3FooA_i", "Foo::foo(int [])");
        test("foo__3BarA10_i", "Bar::foo(int [10])");

        test("foo__H1Zi_", "foo<int>(void)");
        test("foo__H1Zi", "foo<int>(void)");

        test("compare__CQ24_STLt7collate1ZcPCcN31",
            "_STL::collate<char>::compare(char const*, "
            "char const*, char const*, char const*) const");

        test("__H1Zb_t9Temporary1ZbRbX00",
            //"Temporary<bool>::Temporary<bool>(bool&, bool)");
            "Temporary<bool>::Temporary(bool&, bool)");

        test("sp_enable_shared_from_this__Q25boost6detailPCVvT1RCQ35boost6detail12shared_count",
            //"boost::detail::sp_enable_shared_from_this(void const volatile*, "
            //"void const volatile*, boost::detail::shared_count const&)");
            "boost::detail::sp_enable_shared_from_this(void volatile const*, void volatile const*, "
            "boost::detail::shared_count const&)");

        test("__tf10bad_typeid", "bad_typeid type_info function");
        test("__ti10bad_typeid", "bad_typeid type_info node");
        test("__tfc", "char type_info function");
        test("__tiUs", "unsigned short type_info node");

        test("__thunk_12__._Q24_STLt14basic_iostream2ZcZQ24_STLt11char_traits1Zc",
            "virtual function thunk (delta:-12) for _STL::basic_iostream<char, "
            "_STL::char_traits<char> >::~basic_iostream(void)");

        test("__adjust_float_buffer__4_STLPcT1c",
            "_STL::__adjust_float_buffer(char*, char*, char)");

        test("append__Q2t7Decoder3ZcUi_32_Ui_512_6OutputUi",
            "Decoder<char, 32, 512>::Output::append(unsigned int)");

        test("__apl__Q24_STLt12basic_string3ZcZQ24_STLt11char_traits1ZcZQ24_STLt9allocator1Zcc",
            "_STL::basic_string<char, _STL::char_traits<char>, _STL::allocator<char> >::"
            "operator+=(char)");
        test("_Q24_STLt9money_get2ZcZPCc.id", "_STL::money_get<char, char const*>::id");

        test("_Q24_STLt15_Integer_limits5Zcc_m128_c_127_im1b1.digits",
            "_STL::_Integer_limits<char, (char)-128, (char)127, -1, true>::digits");
        test("_Q24_STLt15_Integer_limits5Zss_m32768_s_32767_im1b1.digits",
            "_STL::_Integer_limits<short, -32768, 32767, -1, true>::digits");

        test("builtin_table__t7Decoder3ZcUi_32_Ui_512_",
             "Decoder<char, 32, 512>::builtin_table(void)");

        test("append__H1ZQ2t7Decoder3ZcUi_32_Ui_512_6Output_Q2t7Decoder3ZcUi_32_Ui_512_6OutputRCX00_v",
            "void Decoder<char, 32, 512>::Output::append<Decoder<char, 32, 512>::Output>("
            "Decoder<char, 32, 512>::Output const&)");

        test("do_in__CQ24_STLt7codecvt3ZcZcZ11__mbstate_tR11__mbstate_tPCcT2RPCcPcT5RPc",
            "_STL::codecvt<char, char, __mbstate_t>::do_in(__mbstate_t&, char const*, char const*, "
            "char const*&, char*, char*, char*&) const");

        test("__ls__Q24_STLt13basic_ostream2ZwZQ24_STLt11char_traits1ZwPFRQ24_STL8ios_base_RQ24_STL8ios_base",
            "_STL::basic_ostream<wchar_t, _STL::char_traits<wchar_t> >::operator<<"
            "(_STL::ios_base& (*)(_STL::ios_base&))");

        test("__eq__H1ZQ24nutst6string4ZcZQ24_STLt11char_traits1ZcZQ24nuts13default_allocUi_"
            "1024__4nutsRCQ24nutst9substring3ZX01ZQ2X0110value_typeZQ24_STLt11char_traits1ZQ2"
            "X0110value_typeT1_b",
            "bool nuts::operator==<nuts::string<char, _STL::char_traits<char>, nuts::default_alloc, 1024> >("
            "nuts::substring<nuts::string<char, _STL::char_traits<char>, nuts::default_alloc, 1024>, "
            "nuts::string<char, _STL::char_traits<char>, nuts::default_alloc, 1024>::value_type, "
            "_STL::char_traits<nuts::string<char, _STL::char_traits<char>, nuts::default_alloc, 1024>"
            "::value_type> > const&, nuts::substring<nuts::string<char, _STL::char_traits<char>, "
            "nuts::default_alloc, 1024>, nuts::string<char, _STL::char_traits<char>, nuts::default_alloc, "
            "1024>::value_type, _STL::char_traits<nuts::string<char, _STL::char_traits<char>, nuts::default_alloc, "
            "1024>::value_type> > const&)");

        test("__18BreakPointBaseImplRCV18BreakPointBaseImplR6Thread",
            "BreakPointBaseImpl::BreakPointBaseImpl(BreakPointBaseImpl volatile const&, Thread&)");

        test("__nw__FUiPv", "operator new(unsigned int, void*)");

        test("__H1Z11ProfileInfo_Q25boostt10shared_ptr1Z11ProfileInfoPX00",
            //"boost::shared_ptr<ProfileInfo>::shared_ptr<ProfileInfo>(ProfileInfo*)");
            "boost::shared_ptr<ProfileInfo>::shared_ptr(ProfileInfo*)");

        // conversion operator
        test("__opPCc__t9Temporary1ZPCc", "Temporary<char const*>::operator char const*(void)");

        test("__Q2t7Decoder3ZcUi_32_Ui_512_6Output", "Decoder<char, 32, 512>::Output::Output(void)");
        test("_._Q2t7Decoder3ZcUi_32_Ui_512_6Output", "Decoder<char, 32, 512>::Output::~Output(void)");

        // nested vtable
        test("__vt_Q24_STLt13basic_fstream2ZcZQ24_STLt11char_traits1Zc."
                "Q24_STLt9basic_ios2ZcZQ24_STLt11char_traits1Zc",
            "_STL::basic_fstream<char, _STL::char_traits<char> >::"
                "_STL::basic_ios<char, _STL::char_traits<char> > virtual table");

        test("_GLOBAL_.D.unmangle", "global destructors keyed to unmangle", true);

        test("_GLOBAL_.D.do_decimal_point__CQ24_STLt8numpunct1Zc",
            "global destructors keyed to _STL::numpunct<char>::do_decimal_point(void) const",
            true);

        test("_GLOBAL_.D.__Q24_STL12_Locale_implPCc",
            "global destructors keyed to _STL::_Locale_impl::_Locale_impl(char const*)",
            true);

        test("equal__H1Z3foo_3bar", "bar::equal<foo>(void)");
        //test("equal__H1Z3foo_3bar_", "bar::equal<foo>()");
        test("equal__H1Z3foo_C3bar", "bar::equal<foo>(void) const");
        //test("equal__H1Z3foo_C3bar_", "bar::equal<foo>() const");

        test("equal__H1Z3foo_C3bar_b", "bool bar::equal<foo>() const");

        // template template parameter
        test("foo__H1z13Foo_3Bar", "Bar::foo<template <Foo> class>(void)");
        test("foo__H1z23Fooi3Bar_3Fun", "Fun::foo<template <Foo, int> class Bar>(void)");

        test("__cl__H3ZvZQ35boost4_mfit3mf35ZvZ10MainWindowZQ24_STLt12basic_string3ZcZQ24_STLt11char_traits"
                "1ZcZQ24_STLt9allocator1ZcZUiZPQ24_STLt6vector2Zt6RefPtr2Z11DebugSymbolZt7RefBase1Z11DebugSymbol"
                "ZQ24_STLt9allocator1Zt6RefPtr2Z11DebugSymbolZt7RefBase1Z11DebugSymbolZQ35boost3_bi5list0_Q35"
                "boost3_bit5list44ZQ35boost3_bit5value1ZP10MainWindowZQ35boost3_bit5value1ZQ24_STLt12basic_string"
                "3ZcZQ24_STLt11char_traits1ZcZQ24_STLt9allocator1ZcZQ35boost3_bit5value1ZUiZQ35boost3_bit5value1ZPQ24"
                "_STLt6vector2Zt6RefPtr2Z11DebugSymbolZt7RefBase1Z11DebugSymbolZQ24_STLt9allocator1Zt6RefPtr2Z11"
                "DebugSymbolZt7RefBase1Z11DebugSymbolGQ35boost3_bit4type1ZX00X10RX20_X00",
            "void boost::_bi::list4<boost::_bi::value<MainWindow*>, boost::_bi::value<_STL::basic_string<char, "
                "_STL::char_traits<char>, _STL::allocator<char> > >, boost::_bi::value<unsigned int>, "
                "boost::_bi::value<_STL::vector<RefPtr<DebugSymbol, RefBase<DebugSymbol> >, _STL::allocator<"
                "RefPtr<DebugSymbol, RefBase<DebugSymbol> > > >*> >::operator()<void, boost::_mfi::mf3<void, "
                "MainWindow, _STL::basic_string<char, _STL::char_traits<char>, _STL::allocator<char> >, "
                "unsigned int, _STL::vector<RefPtr<DebugSymbol, RefBase<DebugSymbol> >, _STL::allocator<"
                "RefPtr<DebugSymbol, RefBase<DebugSymbol> > > >*>, boost::_bi::list0>(boost::_bi::type<void>, "
                "boost::_mfi::mf3<void, MainWindow, _STL::basic_string<char, _STL::char_traits<char>, "
                "_STL::allocator<char> >, unsigned int, _STL::vector<RefPtr<DebugSymbol, RefBase<DebugSymbol> >, "
                "_STL::allocator<RefPtr<DebugSymbol, RefBase<DebugSymbol> > > >*>, boost::_bi::list0&)");

        test("equal__H1ZQ24nutst6string4ZcZQ24_STLt11char_traits1ZcZQ24nuts13default_allocUi_1024"
                "__CQ24nutst9substring3ZQ24nutst6string4ZcZQ24_STLt11char_traits1ZcZQ24nuts13default_alloc"
                "Ui_1024_ZcZQ24_STLt11char_traits1ZcRCQ24nutst9substring3ZX00ZcZQ24_STLt11char_traits1Zc_b",
            "bool nuts::substring<nuts::string<char, _STL::char_traits<char>, nuts::default_alloc, 1024>, "
                "char, _STL::char_traits<char> >::equal<nuts::string<char, _STL::char_traits<char>, "
                "nuts::default_alloc, 1024> >(nuts::substring<nuts::string<char, _STL::char_traits<char>, "
                "nuts::default_alloc, 1024>, char, _STL::char_traits<char> > const&) const");

        test("for_each_plugin__H1ZQ24_STLt13unary_compose2ZQ24_STLt9binder1st1ZQ24_STLt10mem_fun1_t3ZvZt12"
            "EnumCallback2ZP6PluginZvZP6PluginZQ24_STLt19const_mem_fun_ref_t2ZP14DebuggerPluginZt9ImportPtr"
            "1Z14DebuggerPlugin_14DebuggerEngineX01_X01",
            "_STL::unary_compose<_STL::binder1st<_STL::mem_fun1_t<void, EnumCallback<Plugin*, void>, Plugin*> >, "
                "_STL::const_mem_fun_ref_t<DebuggerPlugin*, ImportPtr<DebuggerPlugin> > > "
                "DebuggerEngine::for_each_plugin<_STL::unary_compose<_STL::binder1st<_STL::mem_fun1_t"
                "<void, EnumCallback<Plugin*, void>, Plugin*> >, _STL::const_mem_fun_ref_t<DebuggerPlugin*, "
                "ImportPtr<DebuggerPlugin> > > >(_STL::unary_compose<_STL::binder1st<_STL::mem_fun1_t<void, "
                "EnumCallback<Plugin*, void>, Plugin*> >, _STL::const_mem_fun_ref_t<DebuggerPlugin*, "
                "ImportPtr<DebuggerPlugin> > >)");
/* todo
        test("__Q314DebuggerEngine37on_attach__14DebuggerEngineR6Thread.0_16RestorerCallbackR14DebuggerEngine",
            "DebuggerEngine::on_attach__14DebuggerEngineR6Thread.0::RestorerCallback::RestorerCallback(DebuggerEngine&)");
 */
/*
        test("_M_copy_unbuffered__H3ZwZQ24_STLt11char_traits1ZwZQ24_STLt19_Constant_unary_fun2ZbZUi"
            "_4_STLPQ24_STLt13basic_istream2ZX01ZX11PQ24_STLt15basic_streambuf2ZX01ZX11T2X21bT5_i",
            "int _STL::_M_copy_unbuffered<wchar_t, _STL::char_traits<wchar_t>, _STL::_Constant_unary_fun"
            "<bool, unsigned int> >(_STL::basic_istream<wchar_t, _STL::char_traits<wchar_t> >*, "
            "_STL::basic_streambuf<wchar_t, _STL::char_traits<wchar_t> >*, _STL::basic_streambuf<"
            "wchar_t, _STL::char_traits<wchar_t> >*, _STL::_Constant_unary_fun<bool, unsigned int>, bool, bool)");
 */
        test("_throw_failure__C3ios", "ios::_throw_failure(void) const");

        test("__rs__7istreamPFR3ios_R3ios", "istream::operator>>(ios& (*)(ios&))");

        // squangle
        test("__9bad_allocRCB0", "bad_alloc::bad_alloc(bad_alloc const&)");
        test("compare__Q24_STLt11char_traits1ZcPCcn1Ui",
            "_STL::char_traits<char>::compare(char const*, char const*, unsigned int)");

        test("__t6RefPtr2ZQ226_GLOBAL_.N.__4ExprP6Interp11MacroEventsZt7RefBase1ZQ226_GLOBAL_.N.__4ExprP6Interp11MacroEventsPQ226_GLOBAL_.N.__4ExprP6Interp11MacroEvents",
            "RefPtr<{anonymous}::MacroEvents, RefBase<{anonymous}::MacroEvents> >::RefPtr({anonymous}::MacroEvents*)");
        test("__t6RefPtr2ZQ226_GLOBAL_.N.__4ExprP6Interp11MacroEventsZt7RefBase1ZQ226_GLOBAL_.N.__4ExprP6Interp11MacroEventsPQ226_GLOBAL_.N.__4ExprP6Interp11MacroEvents",
            "RefPtr<{anonymous}::MacroEvents, RefBase<{anonymous}::MacroEvents> >::RefPtr",
            false, UNMANGLE_NOFUNARGS);
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
