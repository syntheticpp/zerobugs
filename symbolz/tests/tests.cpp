// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <exception>
#include <iostream>

#include "bench/test.h"
#include "zdk/shared_string_impl.h"
#include "zdk/symbol.h"
#include "zdk/symbol_map.h"
#include "zdk/symbol_table.h"
#include "zdk/zobject_scope.h"
#include "dharma/canonical_path.h"
#include "elfz/public/binary.h"
#include "typez/public/type_system.h"

#include "symbolz/private/symbol_table_impl.h"
// <test_source_line>
#include "public/symbol_table_events.h"
#include "stabz/public/descriptor.h"
#include "stabz/public/compile_unit.h"
// </test_source_line>


using namespace std;


extern "C" char* demangle_d(char* s) { return s; }

static SymbolTableEvents& events()
{
    static RefPtr<SymbolTableEvents> events(SymbolTableEvents::create());
    return *events;
}


/**
 * Enumerate the symbol tables, and check that the file
 * name is set correctly
 */
BEGIN_TEST(test_simple, (const char* filename))
{
    ELF::Binary elf(filename);

    RefPtr<SymbolTable> table =
        SymbolTableImpl::read_tables(NULL, // no process
                                     shared_string(filename),
                                     elf, events(), 0);
    for (; !table.is_null(); table = table->next())
    {
        cout << table->name()->c_str() << " size=" << table->size() << endl << endl;
        size_t count = table->enum_symbols(0, 0, SymbolTable::LKUP_UNMAPPED);
        cout << "enum_symbols() = " << count << endl;

        assert(table->filename()->is_equal(filename));
        // FIXME
        // assert(table->size() == count);
    }
}
END_TEST()


/* just a dummy overload */
void test_simple()
{
}


/*  Lookup the function above in our own symbol table,
    by demangled and mangled name. Check the address,
    and distinguish from an overloaded function. */
static void test_name_lookup(const char* filename);


class SymbolEvents : public EnumCallback<Symbol*>
{
    void notify(Symbol* sym)
    {
        assert(sym);

        if (sym->demangled_name(false)->is_equal("test_simple"))
        {
    #if __GNUC__ < 3
            assert(sym->name()->is_equal("test_simple__FPCc")
                || sym->name()->is_equal("test_simple__Fv"));
    #endif
        }
        else
        {
//          assert((void*)sym->addr() == &test_name_lookup);
            cout << hex << sym->addr();
            cout << " val=" << sym->value() << dec << endl;
        }
    }
};


/**
 * Lookup the function above in our own symbol table,
 * by demangled and mangled name. Check the address,
 * and distinguish from an overloaded function.
 */
BEGIN_TEST(test_name_lookup, (const char* filename))
{
    ELF::Binary elf(filename);
    RefPtr<SymbolTable> table =
        SymbolTableImpl::read_tables(NULL, shared_string(filename), elf, events(), 0);

    for (; !table.is_null(); table = table->next())
    {
        if (!table->is_dynamic())
        {
            SymbolEvents events;

            int count = table->enum_symbols("test_simple", &events, SymbolTable::LKUP_UNMAPPED);

            cout << count << " match(es)\n" << flush;
            assert(count);

        /*#if __GNUC__ < 3
            assert(table->enum_symbols("test_simple__FPCc") == 1);
            assert(table->enum_symbols("test_simple__Fv") == 1);

            assert(table->enum_symbols("test_name_lookup", &events) == 1);
        #endif */
        }

    }
}
END_TEST()


/**
 * Lookup this function by address, then compare names
 */
BEGIN_TEST(test_addr_lookup, (const char* filename))
{
    ELF::Binary elf(filename);
    RefPtr<SymbolTable> table =
        SymbolTableImpl::read_tables(0, shared_string(filename), elf, events(), 0);

    for (; !table.is_null(); table = table->next())
    {
        if (table->is_dynamic())
        {
            continue;
        }
        cout << filename << ": " << hex << table->addr() << dec << endl;

        const addr_t addr = reinterpret_cast<addr_t>(&test_addr_lookup);
        RefPtr<Symbol> sym = table->lookup_symbol(addr);
        // table->lookup(addr + 10);

        cout << (void*) addr << " " << (void*)sym->addr();
        cout <<"=" << sym->demangled_name(false) << endl;

        assert(sym->demangled_name(false)->is_equal("test_addr_lookup"));
    }
}
END_TEST()


class Translator : public EnumCallback<addr_t>
{
public:
    Translator(SymbolTable& table, size_t line)
        : table_(table), line_(line)
    {
    }

private:
    void notify(addr_t addr)
    {
        RefPtr<Symbol> sym = table_.lookup_symbol(addr);

        assert(!sym.is_null());

        cout << hex << sym->value() << dec
             << ": "<< sym->demangled_name()
             << " " << sym->file() << ':' << sym->line() << endl;
        cout << "expecting: " << line_ << endl;
        assert(sym->line() == line_);
    }

    SymbolTable& table_;
    size_t line_;
};


/**
 * Extends SymbolTableEvents -- "connects" the addr_to_line()
 * callback to the stabz library to retrieve source line info.
 */
class SourceLineEvents : public SymbolTableEvents
                       , EnumCallback2<SharedString*, size_t>
{
public:
    SourceLineEvents() : wordSize_(__WORDSIZE), types_(wordSize_) { }

    ~SourceLineEvents() throw() {}

    static RefPtr<SourceLineEvents> create()
    { return new ZObjectImpl<SourceLineEvents>(); }

private:
    const size_t wordSize_;
    NativeTypeSystem types_;
    RefPtr<SharedString> file_;
    size_t line_;

    void notify(BreakPoint*, addr_t)
    {
    }

    void notify(BreakPoint*, const SymbolTable*)
    {
    }

    void notify(SharedString* file, size_t line)
    {
        file_ = file, line_ = line;
    }

    /* Translate address into line and source file name */
    size_t addr_to_line(
        const SymbolTable&      tbl,
        addr_t                  addr,
        RefPtr<SharedString>&   sourceFileName)
    {
        line_ = 0;

        if (desc_.is_null() || &desc_->name() != tbl.filename())
        {
            desc_ = new Stab::Descriptor(*tbl.filename());
            desc_->init_and_parse(types_);
        }

        RefPtr<Stab::CompileUnit> unit = desc_->get_compile_unit(addr);

        if (unit.is_null())
        {
            cout << tbl.filename()->c_str();
            cout << ": unit not found for " << hex << addr << dec << endl;
        }
        else
        {
            cout << tbl.filename()->c_str();
            // cout << ": looking up nearest line...\n";

            if (unit->addr_to_line(addr, NULL, this))
            {
                sourceFileName = file_;
            }
        }

        return line_;
    }


    /*  Enumerate the addresses of code generated for
        the given filename and source line. */
    size_t line_to_addr(
        const SymbolTable&      tbl,
        RefPtr<SharedString>    file,
        size_t                  line,
        EnumCallback<addr_t>*  observer)
    {
        vector<addr_t> addrs = desc_->line_to_addr(file, line);

        cout << addrs.size() << " address(es)\n";
        if (observer)
        {
            vector<addr_t>::const_iterator i = addrs.begin();
            for (; i != addrs.end(); ++i)
            {
                observer->notify(*i);
            }
        }

        return addrs.size();
    }

private:
    RefPtr<Stab::Descriptor> desc_;
};


/**
 * Integration test for libsymbols/libstabz: retrieve
 * line info from the stabs section.
 *
 * @note REQUIRES -gstabs+
 */
BEGIN_TEST(test_source_line, (const char* filename))
{
    RefPtr<SourceLineEvents> events = SourceLineEvents::create();

    ELF::Binary elf(filename);
    RefPtr<SymbolTable> table =
        SymbolTableImpl::read_tables(0, shared_string(filename), elf, *events, 0);

    for (; !table.is_null(); table = table->next())
    {
        if (table->is_dynamic())
        {
            continue;
        }
        addr_t addr = reinterpret_cast<addr_t>(&test_source_line);
        RefPtr<Symbol> sym = table->lookup_symbol(addr);


        // cout << "------------------------------------------\n";
        cout << hex << sym->addr() << dec << ": ";
        cout << sym->demangled_name(false) << " in ";
        cout << sym->file() << ':' << sym->line() << endl;
        cout << "expected: " << __line__ << endl;
        assert(sym->line() == __line__);

        sym = table->lookup_symbol(addr + 10);

        // cout << "------------------------------------------\n";
        cout << hex << sym->addr() << dec << ": ";
        cout << sym->demangled_name(false) << " in ";

        cout << sym->file() << ':' << sym->line() << endl;

        /* Test enum_addresses_by_line -- todo: make separate test */
        Translator xlat(*table, __line__);

        table->enum_addresses_by_line(
            shared_string(canonical_path(__FILE__)).get(), __line__, &xlat);
    }
}
END_TEST()


class SymbolPrinter : public EnumCallback<Symbol*>
{
    void notify(Symbol* sym)
    {
        // cout << sym->demangled_name() << endl;
        cout << sym->name() << " " << sym->demangled_name(false) << endl;

        //cout << (void*)sym->addr() << endl;

        // cout << sym->file()->c_str() << ':' << sym->line() << endl;
    }
};


BEGIN_TEST(test_symbol_map, ())
{
#if 0
    SourceLineEvents    tableEvents;
    SymbolPrinter       symbolEvents;

    RefPtr<SymbolMap> smap = symbol_map(getpid(), tableEvents);

    smap->enum_symbols("strcpy", &symbolEvents);

    addr_t addr = reinterpret_cast<addr_t>(&test_symbol_map);
    cout << "lookup addr=" << hex << addr << dec << endl;

    RefPtr<Symbol> sym = smap->lookup(addr);
    cout << "lookup complete\n";

    assert(!sym.is_null());
    assert(sym->virtual_mem_addr() == addr);

    cout << sym->demangled_name()<< ": "
         << sym->file()->c_str() << ':' << sym->line() << endl;
    assert(sym->line() != 0);
/*
    addr = reinterpret_cast<addr_t>(&strcpy);
    sym = smap->lookup(addr);
    assert(!sym.is_null());
    assert(sym->virtual_mem_addr() == addr);

    cout << sym->demangled_name()<< ": "
         << sym->file()->c_str() << ':' << sym->line() << endl; */

    // smap->lookup(0xc0000001);
#endif
}
END_TEST()


char bigus[100];
static char* const dickus = bigus;

BEGIN_TEST(test_globals, (const char* filename))
{
    SymbolPrinter symbolEvents;

    ELF::Binary elf(filename);
    RefPtr<SymbolTable> table =
        SymbolTableImpl::read_tables(NULL, // no process
                                    shared_string(filename), elf, events(), 0);

    for (; table; table = table->next())
    {
        cout << table->name()->c_str() << endl;
        assert(table->filename()->is_equal(filename));

        table->enum_symbols("bigus", &symbolEvents, SymbolTable::LKUP_UNMAPPED);
    }
}
END_TEST()




int main(int argc, char* argv[])
{
#if 1
    // cout << "address of main=" << (void*)(&main) << endl;
    int ac = argc;
    char** av = argv;
    for (; ac; ++av, --ac)
    {
        test_simple(*av);
    }

    test_name_lookup(argv[0]);
    test_addr_lookup(argv[0]);
// #if __GNUC__ < 3
    // GCC 3 uses DWARF by default
    //test_source_line(argv[0]);
// #endif
    test_symbol_map();
    test_globals(argv[0]);
#endif
    cout << "Tests passed.\n";
    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
