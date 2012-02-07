//
// $Id$
//
// Sample usage of dwarfz
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <algorithm>
#include <exception>
#include <functional>
#include <iostream>
#include "array_type.h"
#include "base_type.h"
#include "class_type.h"
#include "compile_unit.h"
#include "const_type.h"
#include "function.h"
#include "debug.h"
#include "enum_type.h"
#include "init.h"
#include "location.h"
#include "member.h"
#include "parameter.h"
#include "pointer_type.h"
#include "subroutine_type.h"
#include "typedef.h"
#include "variable.h"
#include "volatile_type.h"

using namespace std;


/// Helper class, uses the visitor pattern
/// to print the type of a given datum.
class TypeVisitor
    : public BaseVisitor
    , public Visitor<Dwarf::ArrayType>
    , public Visitor<Dwarf::BaseType>
    , public Visitor<Dwarf::KlassType>
    , public Visitor<Dwarf::ConstType>
    , public Visitor<Dwarf::EnumType>
    , public Visitor<Dwarf::PointerType>
    , public Visitor<Dwarf::SubroutineType>
    , public Visitor<Dwarf::Typedef>
    , public Visitor<Dwarf::UnionType>
    , public Visitor<Dwarf::VolatileType>
{
    void visit(const Dwarf::ArrayType& type)
    {
        assert(type.elem_type());
        cout << type.elem_type()->name();

        Dwarf::List<Dwarf::Dimension> dim = type.dimensions();

        Dwarf::List<Dwarf::Dimension>::const_iterator i = dim.begin();
        for (; i != dim.end(); ++i)
        {
            cout << '[' << i->size() << ']';
        }
    }

    void visit(const Dwarf::BaseType& type)
    { cout << type.name(); }

    void visit(const Dwarf::ConstType& type)
    {
        std::shared_ptr<Dwarf::Type> pt = type.type();
        if (pt)
        {
            cout << "const ";
            pt->accept(this);
        }
    }

    void visit(const Dwarf::VolatileType& type)
    {
        std::shared_ptr<Dwarf::Type> pt = type.type();
        if (pt)
        {
            cout << "volatile ";
            pt->accept(this);
        }
    }

    void visit(const Dwarf::KlassType& type)
    {
        cout << type.name();
/*
        Dwarf::List<Dwarf::Member> mem = type.members();

        Dwarf::List<Dwarf::Member>::const_iterator
            i = mem.begin(), end = mem.end();

        for (; i != end; ++i)
        {
            cout << " " << i->name() << endl;
            i->byte_size();
        }
 */
    }

    void visit(const Dwarf::UnionType& type)
    {
        cout << type.name();
/*
        Dwarf::List<Dwarf::Member> mem = type.members();

        Dwarf::List<Dwarf::Member>::const_iterator
            i = mem.begin(), end = mem.end();

        for (; i != end; ++i)
        {
            cout << " " << i->name() << endl;
            i->byte_size();
        }
 */
    }

    void visit(const Dwarf::PointerType& type)
    {
        std::shared_ptr<Dwarf::Type> pt = type.type();
        if (pt)
        {
            cout << "pointer to ";
            pt->accept(this);
        }
    }

    void visit(const Dwarf::SubroutineType& type)
    {
        cout << "subroutine " << type.name();
    }

    void visit(const Dwarf::Typedef& type)
    {
        cout << "typedef " << type.name();
    }

    void visit(const Dwarf::EnumType& type)
    {
        cout << "enum " << type.name();
    }
};


/**
 * Print data objects to standard output
 */
struct PrintDatum
    : public binary_function<Dwarf::Datum, size_t, void>
{
    void operator()(const Dwarf::Datum& datum, size_t depth) const
    {
        const string indent(depth, ' ');
        cout << indent << "<datum ";

        if (const char* name = datum.name())
        {
            cout << "name=" << name << " ";
        }

        cout << "type=";
        TypeVisitor v;

        std::shared_ptr<Dwarf::Type> type(datum.type());
        if (type)
        {
            type->accept(&v);
        }

        // cout << "loc=0x" << hex << datum.loc()->eval() << dec;
        cout << "/>\n";
    }
};


/* This function object prints the variables
 * scoped within a given lexical block
 */
class PrintBlock : public unary_function<Dwarf::Block, void>
{
    size_t indent_;

public:
    explicit PrintBlock(size_t indent) : indent_(indent) {}

    void operator()(const Dwarf::Block& block) const
    {
        using namespace Dwarf;

        start_tag("<", block.name());

        cout << " low_pc=0x" << hex << block.low_pc();
        cout << " high_pc=0x" << block.high_pc() << dec << ">\n";

        /* print variables within this block */
        List<VariableT<Block> > vars = block.variables();
        for_each(vars.begin(), vars.end(),
                bind2nd(PrintDatum(), 4 + indent_));

        Dwarf::List<Dwarf::Block> blocks = block.blocks();
        for_each(blocks.begin(), blocks.end(),
                PrintBlock(4 + indent_));

        start_tag("</", block.name());
        cout << ">\n";
    }

private:
    void start_tag(const char* tag, const char* name) const
    {
        const string indent(indent_, ' ');

        cout << indent << tag;
        if (name)
        {
            cout << name;
        }
        else
        {
            cout << "BLOCK";
        }
    }
};


/// Print the function's name, its parameters, and local variables
void print_func(const Dwarf::Function& func)
{
    using namespace Dwarf;

    if (const char* name = func.name())
    {
        cout << "  <function name=" << name << ">\n";

        cout << "    frame_base=0x"
             << hex << func.frame_base() << dec << endl;

        if (std::shared_ptr<Location> loc = func.loc())
        {
            //cout << "    addr=0x"
            //     << hex << loc->eval(0) << dec << endl;
            cout << "    loc=" << loc.get() << endl;
        }
        if (!func.params().empty())
        {
            cout << "    <param>\n";

            List<Parameter> params = func.params();
            for_each(params.begin(), params.end(),
                     bind2nd(PrintDatum(), 8));
            cout << "    </param>\n";
        }

        List<VariableT<Block> > vars = func.variables();
        for_each(vars.begin(), vars.end(), bind2nd(PrintDatum(), 4));


        Dwarf::List<Dwarf::Block> blocks = func.blocks();
        for_each(blocks.begin(), blocks.end(), PrintBlock(4));


        cout << "  </function>\n"; // closing tag
    }
}


/// Iterates thru the functions defined in the given compilation unit.
void print_unit(const Dwarf::CompileUnit& unit)
{
    cout << "<" << unit.name() << ">\n";

    Dwarf::List<Dwarf::Function> funcs = unit.functions();
    for_each(funcs.begin(), funcs.end(), print_func);

    cout << "</" << unit.name() << ">\n";
}


/// Iterates thru the compilation units that make a binary file
void print_file(const char* filename)
{
    Dwarf::Debug dbg(filename);

    Dwarf::List<Dwarf::CompileUnit> units = dbg.units();
    for_each(units.begin(), units.end(), print_unit);
}


int main(int argc, char* argv[])
{
    Dwarf::init();

    for (--argc, ++argv; argc; --argc, ++argv)
    {
        try
        {
            print_file(*argv);
        }
        catch (const exception& e)
        {
            cerr << *argv << ": " << e.what() << endl;
        }
        catch (...)
        {
            cerr << *argv << ": unknown exception\n";
        }
    }
    cout << endl;
    cout.flush();
    return 0;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
