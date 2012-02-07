// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "test.h"
#include "testhdr.h"
#include "dharma/canonical_path.h"
#include "public/compile_unit.h"
#include "public/debug.h"
#include "public/init.h"
#include "public/namespace.h"
#include "public/line_events.h"
#include "public/type.h"
#include "zdk/shared_string_impl.h"


using namespace Dwarf;
using namespace std;

struct Forward;
void test_forward(const char* self, Forward*);


// test_namespaces
namespace Fred
{
    struct Foobar { int x_; };

    namespace Barney
    {
        struct Foo { int y_; };

        using Fred::Foobar;
    }
    using namespace ABC;
}

BEGIN_TEST(test_unit_lookup_type, (const char* self))
{
    Dwarf_Addr addr = reinterpret_cast<Dwarf_Addr>(&test_unit_lookup_type);
    Debug dbg(self);
    shared_ptr<CompileUnit> unit = dbg.lookup_unit(addr);
    assert(unit);

    shared_ptr<Type> type = unit->lookup_type("Fred::Foobar");
    assert(type);
    assert(type->is_complete());

    type = dbg.lookup_type("Fred::Foobar", 1);
    assert(type);
    assert(type->is_complete());
}
END_TEST()


BEGIN_TEST(test_namespace, (const char* self))
{
    Debug dbg(self);
    const Debug::UnitList& units = dbg.units();
    Debug::UnitList::const_iterator u = units.begin();
    for (; u != units.end(); ++u)
    {
        //cout << (*u)->name() << endl;
        List<Namespace> ns = (*u)->namespaces();
        List<Namespace>::iterator i = ns.begin();
        for (; i != ns.end(); ++i)
        {
            //cout << "  namespace " << i->name() << endl;
            if (strcmp(i->name(), "Fred") == 0)
            {
                shared_ptr<Type> type = ((*u)->lookup_type("Fred::Foobar"));
                assert(type);
                // assert(type->is_complete());
            }
        }
    }
}
END_TEST()


BEGIN_TEST(test_namespace2, (const char* self))
{
    Debug dbg(self);
    shared_ptr<Type> type = dbg.lookup_type("Fred::Foobar", 1);
    assert(type);
    assert(type->is_complete());
}
END_TEST()


BEGIN_TEST(test_nested_namespace, (const char* self))
{
    Debug dbg(self);
    shared_ptr<Type> type = dbg.lookup_type("Fred::Barney::Foo", 1);
    assert(type);
    assert(type->is_complete());
/* todo
    type = dbg.lookup_type("Fred::Barney::Foobar");
    assert(type);
    assert(type->is_complete()); */
}
END_TEST()


namespace
{
    struct LineEvents : public SrcLineEvents
    {
        vector<Dwarf_Addr> addr_;

        bool on_srcline(SharedString* file,
                        Dwarf_Unsigned line,
                        Dwarf_Addr addr)
        {
            cout << file->c_str() << ":" << line << " " << hex << addr << dec << endl;
            addr_.push_back(addr);
            return true;
        }

        void on_done() { }
    };
}

/*
static bool
progress(void* ptr, const char* msg, double percent)
{
    std::cout << "\033[K" << msg << "\r";
    return true;
}
*/

BEGIN_TEST(test_line_info, (const char* self))
{
    Debug debug(self);

    LineEvents events;
    RefPtr<SharedString> file(shared_string(canonical_path(__FILE__)));
    // line-to-addr:
    debug.line_to_addr(file.get(), __LINE__, events);
    debug.line_to_addr(file.get(), 0, events);
    vector<Dwarf_Addr> addresses = events.addr_;
    assert(!addresses.empty());

    // addr-to-line:
    vector<Dwarf_Addr>::const_iterator i = addresses.begin();
    for (; i != addresses.end(); ++i)
    {
        cout << "addr=" << hex << *i << dec << endl;
        shared_ptr<CompileUnit> unit = debug.lookup_unit(*i);
        assert(unit);
        cout << unit->name() << " [" << hex << unit->low_pc();
        cout << "-" << unit->high_pc() << "]\n" << dec;
        assert(file->is_equal(unit->name()));
        assert(strcmp(unit->short_path(), __FILE__) == 0);
        Dwarf_Addr addr = 0;
        assert (unit->addr_to_line(*i, &addr, &events));
        cout << "Addr=" << hex << addr << dec << endl;
        assert(addr);
        assert(addr == *i);
    }
}
END_TEST()


struct Forward
{
    char name[256];
};

using namespace ABC;

int main(int argc, char* argv[])
{
    Fred::Barney::Foobar foobar = { 123 };
    //Fred::Foobar foobar = { 123 };
    Fred::Foobar malabar = { 0 };
    Fred::Barney::Foo foo = { 456 };
    Forward f = { "forward" };
    Fred::XYZ<long> xyz(42);

    init();

    test_forward(argv[0], &f);
    test_unit_lookup_type(argv[0]);
    test_namespace(argv[0]);
    test_namespace2(argv[0]);
    test_nested_namespace(argv[0]);
    test_line_info(argv[0]);

    return foobar.x_  + (long_type)foo.y_ + malabar.x_;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
