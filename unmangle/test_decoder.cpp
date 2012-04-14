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

#ifdef NDEBUG
 #undef NDEBUG
#endif
#include "config.h"
#include <assert.h>
#include <iostream>
//#define CPLUS_DEMANGLE_COMPAT
#include "config.h"
#ifdef USE_SHORT_STD_SUBST
 //#undef USE_SHORT_STD_SUBST
#endif
#include "decoder.h"
#include "encoder.h"

using namespace std;

static void test(const char* mangledName, const char* decoded)
{
    Decoder<char> d(mangledName, 0, UNMANGLE_DEFAULT);

    if (char* result = d.parse())
    {
        cout << '\"' << result << '\"' << endl;
        if (strcmp(result, decoded) != 0)
        {
            cout << "\nInput: " << mangledName << endl;
            cout << "------------------------------------------\n";
            cout << "Expected: " << decoded << endl << endl;
            cout << "Got     : " << result << endl;
         /*
            cout << "------------------------------------------\n";
            size_t i = 0;
            for (; decoded[i] == result[i]; ++i)
            {
            }
            cout << "EXPECTED: " << decoded + i << endl;
            cout << "GOT     : " << result + i << endl;
         */
            assert(false);
        }
        free(result);
#if 0 // test the PartialEncoder as well ...
        Mangle::PartialEncoder enc;

        string mangled = enc.run(decoded);
        assert(strncmp(mangled.c_str(), mangledName, mangled.size()) == 0);
#endif
    }
    else
    {
        cout << "EXPECTED: " << decoded << endl;
        assert(false);
    }
}
void test_operators()
{
    struct
    {
        string mangled;
        string demangled;
    } oper[] =
    {
        { "nw", "operator new" },
        { "na", "operator new[]" },
        { "dl", "operator delete" },
        { "da", "operator delete[]" },
        { "ps", "operator+" },
        { "ng", "operator-" },
        { "ad", "operator&" },
        { "de", "operator*" },
        { "co", "operator~" },
        { "pl", "operator+" },
        { "mi", "operator-" },
        { "ml", "operator*" },
        { "dv", "operator/" },
        { "rm", "operator%" },
        { "an", "operator&" },
        { "or", "operator|" },
        { "eo", "operator^" },
        { "aS", "operator=" },
        { "pL", "operator+=" },
        { "mI", "operator-=" },
        { "mL", "operator*=" },
        { "dV", "operator/=" },
        { "rM", "operator%=" },
        { "aN", "operator&=" },
        { "oR", "operator|=" },
        { "eO", "operator^=" },
        { "ls", "operator<<" },
        { "rs", "operator>>" },
        { "lS", "operator<<=" },
        { "rS", "operator>>=" },
        { "eq", "operator==" },
        { "ne", "operator!=" },
        { "lt", "operator<" },
        { "gt", "operator>" },
        { "le", "operator<=" },
        { "ge", "operator>=" },
        { "nt", "operator!" },
        { "aa", "operator&&" },
        { "oo", "operator||" },
        { "pp", "operator++" },
        { "mm", "operator--" },
        { "cm", "operator," },
        { "pm", "operator->*" },
        { "pt", "operator->" },
        { "cl", "operator()" },
        { "ix", "operator[]" },
        { "qu", "operator?", },
        { "st", "" },
        { "sz", "operator sizeof" },
    };
    for (size_t i = 0; i != sizeof(oper)/sizeof(oper[0]); ++i)
    {
        string mangled = "_Z" + oper[i].mangled;
        test(mangled.c_str(), oper[i].demangled.c_str());
    }
}
int main()
{
  try
  {
//    test_operators();
#ifdef CPLUS_DEMANGLE_COMPAT
goto _skip;
#endif
    test("_Z2Ab", "Ab");
    //test("_ZSt2Ab", "std::Ab");
    test("_ZNSt2AbE", "std::Ab");
    test("_Z3fooKPv", "foo(void* const)");
    test("_Z3fooPKv", "foo(void const*)");
    //test("_ZNSt3FooC2Ev", "std::Foo::Foo()");
    test("_ZNSt3FooC1Ev", "std::Foo::Foo()");
    //test("_Z7DerivedC1v", "Derived::Derived()");
    test("_ZN7DerivedC1Ev", "Derived::Derived()");
    //test("_ZN7DerivedD0Ev", "Derived::~Derived()");
    test("_ZN7DerivedD1Ev", "Derived::~Derived()");
    //test("_ZNSt3FooC2Eif", "std::Foo::Foo(int, float)");
    test("_ZNSt3FooC1Eif", "std::Foo::Foo(int, float)");
    test("_Z3funcz", "fun(char, ...)");
    test("_Z3fooPv", "foo(void*)");
    test("_Z3fooPPv", "foo(void**)");
    test("_Z3fooPKPVv", "foo(void volatile* const*)");
    test("_Z3fooPFviE", "foo(void (*)(int))");
    test("_Z3fooKPFviE", "foo(void (* const)(int))");
    test("_Z3fun1A", "fun(A)");
    test("_Z3funKPN1EE", "fun(E* const)");
    test("_Z1fRKiS_v", "f(int const&, int const)");
    test("_ZN3FooaSERKS_", "Foo::operator=(Foo const&)");

    // template ctor
    test("_ZN1AC1IiEEv", "A::A<int>()");
    test("_ZN1AC1IfEEv", "A::A<float>()");
    test("_ZN1AC1IdEEv", "A::A<double>()");
    test("_ZN1AC1IgEEv", "A::A<__float128>()");
    test("_ZN1AC1IcEEv", "A::A<char>()");
    test("_ZN1AC1IPcEEv", "A::A<char*>()");

    test("_Z1f3FooIiEc", "f(Foo<int>, char)");
    test("_Z1f3FooIiES_", "f(Foo<int>, Foo)");
    test("_Z1f3FooIiES0_", "f(Foo<int>, Foo<int>)");
    test("_Z1f3FooIiES_IcE", "f(Foo<int>, Foo<char>)");
    test("_Z1f3FooIiES_IcES1_", "f(Foo<int>, Foo<char>, Foo<char>)");

    test("_ZNVK3FoocviEv", "Foo::operator int() volatile const");
    test("_Z3FunKi", "Fun(int const)");

    test("_ZN1A1BC1IiEERKT_", "A::B::B<int>(int const&)");
    test("_ZN1A1BC1IiEERKS0_", "A::B::B<int>(A::B const&)");
    test("_ZNK4MetaI1TE3getEv", "Meta<T>::get() const");
    test("_Z3funIiEiT_", "int fun<int>(int)");
    test("_Z3funIiPcEiT0_", "int fun<int, char*>(char*)");
    test("_Z1FPKvS_", "F(void const*, void const)");
    test("_Z1FPKvS0_", "F(void const*, void const*)");

    test("_ZN1FI3FooS_IiEE1fEv", "F<Foo, F<int> >::f()");
    test("_ZN1FI3FooS0_IiEE1fEv", "F<Foo, Foo<int> >::f()");
    test("_ZN1FI3FooS_IiEE1fES_IcE", "F<Foo, F<int> >::f(F<char>)");
    test("_Z1fIiiEvT0_", "void f<int, int>(int)");
    test("_Z1fIiiEvT0_", "void f<int, int>(int)");
    test("_Z3foo5Hello5WorldS0_S_", "foo(Hello, World, World, Hello)");
    test("_Z3funI3FooE3BarIT_EPS2_", "Bar<Foo> fun<Foo>(Foo*)");

    test("_Z1FIiLl2EE", "F<int, (long)2>");
    test("_Z1FI1TIiELl2EE", "F<T<int>, (long)2>");

    test("_Z1FILl2E1FIiES1_E", "F<(long)2, F<int>, F<int> >");
    test("_Z1FILl2E1FIiES0_E", "F<(long)2, F<int>, F>");
    test("_Z1FILl2E1FIiES1_E", "F<(long)2, F<int>, F<int> >");
    test("_Z1FILl2ES_IiES0_E", "F<(long)2, F<int>, F<int> >");
    test("_Z1FILl2EN1F1fIiEES2_E", "F<(long)2, F::f<int>, F::f<int> >");
    test("_Z1FILl2EN1F1fIiEES1_E", "F<(long)2, F::f<int>, F::f>");

    test("_ZN1FILl2ES_IiES0_E1FET1_S2_v",
         "F<(long)2, F<int>, F<int> >::F(F<int>, F<int>)");
    test("_ZN1FILl2E1FIiES1_E1FET1_S1_v",
         "F<(long)2, F<int>, F<int> >::F(F<int>, F<int>)");

    test("_Z1AIi1BIicELb0EE", "A<int, B<int, char>, (bool)0>");
    test("_Z1AIi1BI1CIiPcEELb0EE", "A<int, B<C<int, char*> >, (bool)0>");

    // template arg
    test("_Z3fooIxE", "foo<long long>");
    test("_Z3fooILf1.2EE", "foo<(float)[1.2]>");
    test("_Z3fooILd1.2EE", "foo<(double)[1.2]>");
    test("_Z3fooILg1.2EE", "foo<(__float128)[1.2]>");
    // array type
    test("_Z1fA20_iv", "f(int[20])");
    test("_Z1fA_iv", "f(int[])");

    test("_Z14interface_castI7IntType8DataType7RefBaseIS1_EE6RefPtrIT_S2_IS5_EES4_IT0_T1_E",
         "RefPtr<IntType, RefBase<IntType> > interface_cast<IntType, DataType, RefBase<DataType> >(RefPtr<DataType, RefBase<DataType> >)");

    test("_Z1FILZ1aEE", "F<a>");
    test("_Z1FILZ1fvEE", "F<f()>");
    test("_Z1FILZ1fIiEvvEE", "F<void f<int>()>");

    test("_Z1FILZ1fIiEvvES_E", "F<void f<int>(), F>");

    test("_ZdlPvS_", "operator delete(void*, void*)");

    // local name
    test("_ZGVZ10bool_falsevE5False", "guard variable for bool_false()::False");
    test("_ZZ1FKP1AvES_", "F(A* const)::A");

    test("_Z1FIiZ1AE2fuE3fooiS0_", "foo F<int, A::fu>(int, A::fu)");

    test("_ZNK5FubarILi2EE3funEv", "Fubar<(int)2>::fun() const");

    test("_ZltI7OStringIcSt11char_traitsIcESt23__malloc_alloc_templateILi0EEEEbRK9SubstringIT_NS7_10value_typeES1_IS8_EESC_",
        "bool operator< <OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> > >(Substring<OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >, OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >::value_type, std::char_traits<OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >::value_type> > const&, Substring<OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >, OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >::value_type, std::char_traits<OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >::value_type> > const&)");

    test("_ZNSaISsEC1Ev",
#ifdef USE_SHORT_STD_SUBST
        "std::allocator<std::string>::allocator()");
#else
        "std::allocator<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >::allocator()");
#endif
    //test("_ZNSbIcEC1Ev", "std::basic_string<char>::basic_string()");

    test("_ZSt21_Rb_tree_rotate_rightPSt18_Rb_tree_node_baseRS0_",
        "std::_Rb_tree_rotate_right(std::_Rb_tree_node_base*, std::_Rb_tree_node_base*&)");

    // special names
    test("_ZTI10BaseMiddle", "typeinfo for BaseMiddle");
    test("_ZTS10BaseMiddle", "typeinfo name for BaseMiddle");
    test("_ZTT10BaseMiddle", "VTT for BaseMiddle");
    test("_ZTV7BaseOne", "vtable for BaseOne");
    test("_ZTv0_n12_N7DerivedD0Ev",
        "virtual thunk [v:0,-12] to Derived::~Derived()");
    test("_ZThn12_N7DerivedD1Ev",
        "non-virtual thunk [nv:-12] to Derived::~Derived()");
    test("_ZTcv0_n12_hn12_N7Derived1fEvv",
         "covariant return thunk [v:0,-12] [nv:-12] to Derived::f()");
    test("_ZTchn10_v4_n12_N7Derived1fEvv",
        "covariant return thunk [nv:-10] [v:4,-12] to Derived::f()");

    // expression
    test("_Z1BIXdvplL1JELi1ELi2EEE", "B<(((J))+((int)1))/((int)2)>");
    test("_Z1BIXngL1JEEE", "B<-((J))>");
    test("_Z1BIXcoL1i1EEE", "B<~((i)1)>");
    test("_Z1BIXstlEE", "B<sizeof(long)>");
    test("_Z1BIXsr3Foo3BarEE", "B<Foo::Bar>");
    test("_Z1BIXsr3Foo3BarIiEEE", "B<Foo::Bar<int> >");
    test("_Z1BIXqust3FooLi0ELi1EEE", "B<sizeof(Foo)?((int)0):((int)1)>");
    test("_Z1FIXpLL1AELi1EEE", "F<((A))+=((int)1)>");
    test("_Z1FIXszLlEEE", "F<sizeof((long))>");

    // pointer to member
    test("_Z1FM1Fiv", "F(int F::*)");
    test("_Z1FMK1Fiv", "F(int F const::*)");
    test("_Z1FKM1Fiv", "F(int F::* const)");
    test("_Z1FM1AFvvEv", "F(void (A::*)())");

    test("_Z1F1A1AS_S0_v", "F(A, A, A, A)");

    test("_ZN7DecoderIcE6OutputC1Ev", "Decoder<char>::Output::Output()");
    test("_ZN7DecoderIcE6Output6appendISsEEvRKT_",
#ifdef USE_SHORT_STD_SUBST
        "void Decoder<char>::Output::append<std::string>(std::string const&)");
#else
        "void Decoder<char>::Output::append<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >(std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)");
#endif
    test("_ZNSt11_Deque_baseI1ASaI1AEE15_M_create_nodesEPP1AS3_",
        "std::_Deque_base<A, std::allocator<A> >::_M_create_nodes(A**, std::_Deque_base<A, std::allocator<A> >)");
    test("_ZNSaI9SubstringI7OStringIcSt11char_traitsIcESt23__malloc_alloc_templateILi0EEEcS2_EED1Ev",
        "std::allocator<Substring<OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >, char, std::char_traits<char> > >::~allocator()");

    test("_ZSt4swapIjEvRT_S1_", "void std::swap<unsigned int>(unsigned int&, unsigned int&)");

    test("_ZStmiI1AR1AP1AS0_S1_ENSt15_Deque_iteratorIT_T0_T1_E15difference_typeERKS6_RKS2_IS3_T2_T3_E",
        "std::_Deque_iterator<A, A&, A*>::difference_type std::operator-<A, A&, A*, A, A>(A const&, A&<A, A, A> const&)");
    test("_ZStmiISsRSsPSsS0_S1_ENSt15_Deque_iteratorIT_T0_T1_E15difference_typeERKS6_RKS2_IS3_T2_T3_E",
        "std::_Deque_iterator<std::string, std::string&, std::string*>::difference_type std::operator-<std::string, std::string&, std::string*, std::string&, std::string*>(std::_Deque_iterator<std::string, std::string&, std::string*> const&, std::_Deque_iterator<std::string, std::string&, std::string*> const&)");
#ifdef TEST_BROKEN_ABI
    test("_ZN9__gnu_cxxmiIPK9SubstringI7OStringIcSt11char_traitsIcESt23__malloc_alloc_templateILi0EEEcS4_ESA_SB_EEE",
    "__gnu_cxx::operator-<Substring<OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >, char, std::char_traits<char> > const*, Substring<OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >, char, std::char_traits<char> > const*, Substring<OString<char, std::char_traits<char>, std::__malloc_alloc_template<(int)0> >, char, std::char_traits<char> > const*>");

#endif // TEST_BROKEN_ABI

    test("_ZNK5FubarI3BarI3FooIiESaIS2_EESaIS4_EE4funcEv",
        "Fubar<Bar<Foo<int>, std::allocator<Foo<int> > >, "
        "std::allocator<Bar<Foo<int>, std::allocator<Foo<int> > > > >::func() const");

    test("_ZNK3FooISt6vectorSaIS0_IcSaIcEEEE4funcEv",
        "Foo<std::vector, std::allocator<std::vector<char, std::allocator<char> > > >::func() const");

    test("_ZN10DynamicLib6importIFP6PluginPK4UUIDEEEbPKcR9ImportPtrIT_E",
        "bool DynamicLib::import<Plugin* ()(UUID const*)>(char const*, ImportPtr<Plugin* ()(UUID const*)>&)");

    test("_ZN11auto_handleIP12Disassembler15ImportPtrTraitsIS1_E23ref_counted_base_handleIS1_S3_N10DynamicLib7CounterEEEC2IPS5_EES1_T_",
    "auto_handle<Disassembler*, ImportPtrTraits<Disassembler*>, ref_counted_base_handle<Disassembler*, ImportPtrTraits<Disassembler*>, DynamicLib::Counter> >::auto_handle<DynamicLib*>(Disassembler*, DynamicLib*)");

    test("_ZN11auto_handleIPFP6PluginPK4UUIDE15ImportPtrTraitsIS6_E23ref_counted_base_handleIS6_S8_N10DynamicLib7CounterEEE4swapERSD_",
    "auto_handle<Plugin* (*)(UUID const*), ImportPtrTraits<Plugin* (*)(UUID const*)>, ref_counted_base_handle<Plugin* (*)(UUID const*), ImportPtrTraits<Plugin* (*)(UUID const*)>, DynamicLib::Counter> >::swap(auto_handle<Plugin* (*)(UUID const*), ImportPtrTraits<Plugin* (*)(UUID const*)>, ref_counted_base_handle<Plugin* (*)(UUID const*), ImportPtrTraits<Plugin* (*)(UUID const*)>, DynamicLib::Counter> >&)");

    test("_ZN1TI3FooEcv1UEv", "T<Foo>::operator U()");
    test("_ZN12DebuggerBase18register_interfaceEPK4UUIDPFP7ZObjectvE",
         "DebuggerBase::register_interface(UUID const*, ZObject* (*)())");

    test("_ZNK5boost10shared_ptrI10DynamicLibEcvMS2_PS1_Ev",
        "boost::shared_ptr<DynamicLib>::operator DynamicLib* boost::shared_ptr<DynamicLib>::*() const");

    test("_ZN15TypeFactoryImpl12get_int_typeIcEE6RefPtrI8DataType7RefBaseIS2_EEPT_PKc",
        "RefPtr<DataType, RefBase<DataType> > TypeFactoryImpl::get_int_type<char>(char*, char const*)");

    test("_ZN5boost6detail26sp_enable_shared_from_thisEPVKvS2_RKNS0_12shared_countE",
        "boost::detail::sp_enable_shared_from_this(void volatile const*, void volatile const*, boost::detail::shared_count const&)");

    test("_ZSteqIcSt11char_traitsIcESaIcEEbRKSbIT_T0_T1_ES8_",
        "bool std::operator==<char, std::char_traits<char>, std::allocator<char> >(std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)");

    test("_ZSt4swapISt14_Rb_tree_colorEvRT_S2_",
        "void std::swap<std::_Rb_tree_color>(std::_Rb_tree_color&, std::_Rb_tree_color&)");

    test("_ZSt9transformISt17_Rb_tree_iteratorISt4pairIKi6RefPtrI6Thread7RefBaseIS4_EEERKS8_PS9_EN9__gnu_cxx17__normal_iteratorIPS7_St6vectorIS7_SaIS7_EEEENSD_9select2ndIS8_EEET0_T_SN_SM_T1_",
        "__gnu_cxx::__normal_iterator<RefPtr<Thread, RefBase<Thread> >*, std::vector<RefPtr<Thread, RefBase<Thread> >, std::allocator<RefPtr<Thread, RefBase<Thread> > > > > std::transform<std::_Rb_tree_iterator<std::pair<int const, RefPtr<Thread, RefBase<Thread> > >, std::pair<int const, RefPtr<Thread, RefBase<Thread> > > const&, std::pair<int const, RefPtr<Thread, RefBase<Thread> > > const*>, __gnu_cxx::__normal_iterator<RefPtr<Thread, RefBase<Thread> >*, std::vector<RefPtr<Thread, RefBase<Thread> >, std::allocator<RefPtr<Thread, RefBase<Thread> > > > >, __gnu_cxx::select2nd<std::pair<int const, RefPtr<Thread, RefBase<Thread> > > > >(std::_Rb_tree_iterator<std::pair<int const, RefPtr<Thread, RefBase<Thread> > >, std::pair<int const, RefPtr<Thread, RefBase<Thread> > > const&, std::pair<int const, RefPtr<Thread, RefBase<Thread> > > const*>, std::_Rb_tree_iterator<std::pair<int const, RefPtr<Thread, RefBase<Thread> > >, std::pair<int const, RefPtr<Thread, RefBase<Thread> > > const&, std::pair<int const, RefPtr<Thread, RefBase<Thread> > > const*>, __gnu_cxx::__normal_iterator<RefPtr<Thread, RefBase<Thread> >*, std::vector<RefPtr<Thread, RefBase<Thread> >, std::allocator<RefPtr<Thread, RefBase<Thread> > > > >, __gnu_cxx::select2nd<std::pair<int const, RefPtr<Thread, RefBase<Thread> > > >)");

    test("_ZN3Gtk16EmitProxySignal2IviiNS_8EditableE12_GtkEditableLi3EXadL24gtk_editable_delete_textEEEclEii",
    "Gtk::EmitProxySignal2<void, int, int, Gtk::Editable, _GtkEditable, (int)3, &((gtk_editable_delete_text))>::operator()(int, int)");

    test("_ZN3Gtk16EmitProxySignal0IvNS_6ObjectE10_GtkObjectLi0EXadL_Z20gtkmm_object_destroyPS2_EEE4emitEv",
    "Gtk::EmitProxySignal0<void, Gtk::Object, _GtkObject, (int)0, &(gtkmm_object_destroy(_GtkObject*))>::emit()");

    test("_ZN14DebuggerEngine15for_each_pluginIRN34_GLOBAL__N_symbol_events.cppIf7Omb13OnSymbolTableEEET_S4_",
    "(anonymous namespace)::OnSymbolTable& DebuggerEngine::for_each_plugin<(anonymous namespace)::OnSymbolTable&>((anonymous namespace)::OnSymbolTable&)");
    test("_GLOBAL__I__ZlsRSoRK4UUID",
        "global constructors keyed to operator<<(std::ostream&, UUID const&)");

    test("_ZN6RefPtrIN36_GLOBAL__N_cpu_state_saver.cpp2YrWmb4REGSE7RefBaseIS1_EED1Ev",
        "RefPtr<(anonymous namespace)::REGS, RefBase<(anonymous namespace)::REGS> >::~RefPtr()");

    test("_Z10back_traceRKSt6vectorIlSaIlEEjjljRSt5dequeI9FrameInfoSaIS5_EE",
        "back_trace(std::vector<long, std::allocator<long> > const&, "
        "unsigned int, unsigned int, long, unsigned int, "
        "std::deque<FrameInfo, std::allocator<FrameInfo> >&)");
    test("_ZN9__gnu_cxx13unary_composeISt9binder1stISt10mem_fun1_tIv12Enum"
        "CallbackIP6PluginvES5_EESt19const_mem_fun_ref_tIP14DebuggerPlugin9ImportPtr"
        "ISA_EEEC1ERKS8_RKSE_",
        "__gnu_cxx::unary_compose<std::binder1st<std::mem_fun1_t<void, "
        "EnumCallback<Plugin*, void>, Plugin*> >, std::const_mem_fun_ref_t"
        "<DebuggerPlugin*, ImportPtr<DebuggerPlugin> > >::unary_compose"
        "(std::binder1st<std::mem_fun1_t<void, EnumCallback<Plugin*, void>, Plugin*> > "
        "const&, std::const_mem_fun_ref_t<DebuggerPlugin*, ImportPtr<DebuggerPlugin> > const&)");
    test("_ZN9__gnu_cxx13unary_composeISt9binder1stISt10mem_fun1_tIv12EnumCallback"
        "IP6PluginvES5_EESt19const_mem_fun_ref_tIP14DebuggerPlugin9ImportPtr"
        "ISA_EEEC1ERKS8_RKSE_",
        "__gnu_cxx::unary_compose<std::binder1st<std::mem_fun1_t<void, EnumCallback<"
        "Plugin*, void>, Plugin*> >, std::const_mem_fun_ref_t<DebuggerPlugin*, "
        "ImportPtr<DebuggerPlugin> > >::unary_compose(std::binder1st<std::mem_fun1_t<"
        "void, EnumCallback<Plugin*, void>, Plugin*> > const&, std::const_mem_fun_ref_t<"
        "DebuggerPlugin*, ImportPtr<DebuggerPlugin> > const&)");

    test("_ZN5boostneINS_14token_iteratorINS_14char_separatorIcSt11char_traitsIcEEEN9__gnu_cxx17__normal_iteratorIPKcSsEESsEESsNS_21forward_traversal_tagERKSsiSB_SsSC_SE_iEENS_3mpl6apply2INS_6detail12always_bool2ET_T4_E4typeERKNS_15iterator_facadeISJ_T0_T1_T2_T3_EERKNSN_ISK_T5_T6_T7_T8_EE",
    "boost::mpl::apply2<boost::detail::always_bool2, boost::token_iterator<boost::char_separator<char, std::char_traits<char> >, __gnu_cxx::__normal_iterator<char const*, std::string>, std::string>, boost::token_iterator<boost::char_separator<char, std::char_traits<char> >, __gnu_cxx::__normal_iterator<char const*, std::string>, std::string> >::type boost::operator!=<boost::token_iterator<boost::char_separator<char, std::char_traits<char> >, __gnu_cxx::__normal_iterator<char const*, std::string>, std::string>, std::string, boost::forward_traversal_tag, std::string const&, int, boost::token_iterator<boost::char_separator<char, std::char_traits<char> >, __gnu_cxx::__normal_iterator<char const*, std::string>, std::string>, std::string, boost::forward_traversal_tag, std::string const&, int>(boost::iterator_facade<boost::token_iterator<boost::char_separator<char, std::char_traits<char> >, __gnu_cxx::__normal_iterator<char const*, std::string>, std::string>, std::string, boost::forward_traversal_tag, std::string const&, int> const&, boost::iterator_facade<boost::token_iterator<boost::char_separator<char, std::char_traits<char> >, __gnu_cxx::__normal_iterator<char const*, std::string>, std::string>, std::string, boost::forward_traversal_tag, std::string const&, int> const&)");

    test("_ZN3funIiEE3bazIcEi", "baz<char> fun<int>(int)");
    test("_ZN3funIiEE3bazI3fooIcEEi", "baz<foo<char> > fun<int>(int)");
    test("_ZN3funI3fooIcEEE3bazI3fooIcEES1_", "baz<foo<char> > fun<foo<char> >(foo<char>)");
    test("_ZN3funI3fooIcEEE3bazI3fooIcEES5_",
        "baz<foo<char> > fun<foo<char> >(baz<foo<char> >)");

    test("_ZN3Gtk57_GLOBAL__N__ZN3Gtk5CTreeC2ERKNS_14SArray_Helpers6SArrayEi28tree_select_row_convert_funcEPN4SigC9Callback3IvNS_13CTree_Helpers3RowEiP9_GtkCTreeEEP13_GtkCTreeNodeiS6_",
        "Gtk::(anonymous namespace)::tree_select_row_convert_func(SigC::Callback3<void, Gtk::CTree_Helpers::Row, int, _GtkCTree*>*, _GtkCTreeNode*, int, _GtkCTree*)");

    test("_ZN1X5Fubar3setEi\0", "X::Fubar::set(int)");

    test("_Z3funRA3_K1T", "fun(T const (&) [3])");

    test("_ZN3Gtk11TreeElementC1ILj3EEERAT__PKc",
        "Gtk::TreeElement::TreeElement<(unsigned int)3>(char const* (&) [(unsigned int)3])");

    test("_ZNSt8functionIFvvEEC1IZN16FlBreakPointViewC1ERN2ui10ControllerEiiiiEUlvE_EET_NSt9enable_ifIXntsrSt11is_integralIS8_E5valueENS1_8_UselessEE4typeE",
        "std::function<void ()>::function<FlBreakPointView::FlBreakPointView(ui::Controller&, int, int, int, int)::{lambda()#1}>(FlBreakPointView::FlBreakPointView(ui::Controller&, int, int, int, int)::{lambda()#1}, std::enable_if<!(std::is_integral<FlBreakPointView::FlBreakPointView(ui::Controller&, int, int, int, int)::{lambda()#1}>::value), std::function<void ()>::_Useless>::type)");



#ifdef CPLUS_DEMANGLE_COMPAT
_skip:
    test("_ZN5boost14char_separatorIcSt11char_traitsIcEEclIN9__gnu_cxx17__normal_iteratorIPKcSsEESsEEbRT_SA_RT0_",
    "bool boost::char_separator<char, std::char_traits<char> >::operator()<__gnu_cxx::__normal_iterator<char const*, std::string>, std::string>(__gnu_cxx::__normal_iterator<char const*, std::string>&, __gnu_cxx::__normal_iterator<char const*, std::string>, std::string&)");
    test("_ZN5boost22escaped_list_separatorIcSt11char_traitsIcEE9do_escapeIN9__gnu_cxx17__normal_iteratorIPKcSsEESsEEvRT_SA_RT0_",
    "void boost::escaped_list_separator<char, std::char_traits<char> >::do_escape<__gnu_cxx::__normal_iterator<char const*, std::string>, std::string>(__gnu_cxx::__normal_iterator<char const*, std::string>&, __gnu_cxx::__normal_iterator<char const*, std::string>, std::string&)");
 #endif
    //test("_ZZNK7DecoderIcE6Output6substrEjjE19__PRETTY_FUNCTION__",
    //  "Decoder<char>::Output::substr(unsigned, unsigned)::__PRETTY_FUNCTION__ const");

    // todo: error cases
    // test("_ZN1fIiS0_E1gEvv", "_ZN1fIiS0_E1gEvv");

    //error
    //test("_ZN3FooaSERKS123", "Foo::operator=(Foo const&)");

    test("_ZN5boost6detail8function15trivial_managerIPFvvEE3getENS1_11any_pointerENS1_30functor_manager_operation_typeE",
        "boost::detail::function::trivial_manager<void (*)()>::get(boost::detail::function::any_pointer, boost::detail::function::functor_manager_operation_type)");

    test("_ZN5boost6detail8function15trivial_managerINS_6python7objects119_GLOBAL__N__usr_src_build_710630_i386_BUILD_boost_1_33_1_libs_python_build_.._src_object_function.cpp_00000000_E803402111bind_returnEE3getENS1_11any_pointerENS1_30functor_manager_operation_typeE",
    "boost::detail::function::trivial_manager<boost::python::objects::(anonymous namespace)::bind_return>::get(boost::detail::function::any_pointer, boost::detail::function::functor_manager_operation_type)");
    test("_ZL16test_addr_lookupPKc", "test_addr_lookup(char const*)");
  }
  catch (const std::exception& e)
  {
      cerr << e.what() << endl;
  }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
