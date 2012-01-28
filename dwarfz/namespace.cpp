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

#include <assert.h>
#include "private/factory.h"
#include "public/function.h"
#include "public/imported_decl.h"
#include "public/namespace.h"
#include "public/type.h"

using namespace Dwarf;
using namespace boost;

namespace
{
    /**
     * Used by Namespace::functions()
     */
    CLASS NSFunction : public Function
    {
    public:
        typedef Namespace parent_type;

        NSFunction(
            Dwarf_Debug dbg,
            Dwarf_Die die,
            Dwarf_Addr lowPC = 0,
            Dwarf_Addr highPC = 0) : Function(dbg, die, lowPC, highPC)
        { }
    };

    /**
     * For enumerating types in Namespace::types,
     */
    CLASS NSType : public Type
    {
    protected:
        NSType(Dwarf_Debug dbg, Dwarf_Die die) : Type(dbg, die) { }

    public:
        typedef Namespace parent_type;

        DECLARE_CONST_VISITABLE()

        friend class IterationTraits<Type>;
    };
}

namespace Dwarf
{
    template<> struct IterationTraits<NSType>
    {
        typedef shared_ptr<NSType> ptr_type;

        /**
         * Obtain the first element in the list
         */
        static ptr_type first(Dwarf_Debug dbg, Dwarf_Die die);

        /**
         * Get the sibling of same type for a given element
         */
        static void next(ptr_type& elem);
    };
}


Namespace::Namespace(Dwarf_Debug dbg, Dwarf_Die die)
    : Die(dbg, die)
{
}


List<ImportedDecl> Namespace::imported_decls() const
{
    return List<ImportedDecl>(this->dbg(), this->die());
}


bool Namespace::search(Dwarf_Off off) const
{
    if (imports_.empty())
    {
        List<ImportedDecl> imports(dbg(), die());
        for (List<ImportedDecl>::const_iterator i = imports.begin();
            i != imports.end();
            ++i)
        {
            imports_.insert(i->offset());
        }
    }
    return imports_.find(off) != imports_.end();
}


/**
 * @return a list of functions defined within this namespace
 */
const FunList& Namespace::functions() const
{

    if (!funcs_.get())
    {
        funcs_.reset(new FunList);
        List<NSFunction> funcs(dbg(), die());
        List<NSFunction>::const_iterator i = funcs.begin(), end = funcs.end();
        for (; i != end; ++i)
        {
            funcs_->push_back(i);
        }
    }
    assert(funcs_.get());
    return *funcs_;
}


// -------------------------------------------------------------
// Iteration Helpers for reading the types declared within
// a namespace
static Dwarf_Half typeTags[] =
{
    DW_TAG_structure_type,
    DW_TAG_typedef,
};

static const size_t numTags = sizeof(typeTags) / sizeof typeTags[0];

/**
 * Get the first child that matches any of the tags in typeTags
 */
shared_ptr<NSType>
IterationTraits<NSType>::first(Dwarf_Debug dbg, Dwarf_Die die)
{
    shared_ptr<NSType> p;

    Dwarf_Half tag = 0;
    if (Dwarf_Die child = Utils::first_child(dbg, die, typeTags, numTags, &tag))
    {
        shared_ptr<Die> tmp = Factory::instance().create(dbg, child, tag, false);

        if (shared_ptr<Type> type = shared_dynamic_cast<Type>(tmp))
        {
            p = shared_static_cast<NSType>(type);
            // std::clog << p->name() << std::endl;
        }
    }
    return p;
}


/**
 * Get the the first sibling of elem that matches a tag
 * in the typeTags table above.
 */
void
IterationTraits<NSType>::next(shared_ptr<NSType>& elem)
{
    assert(elem);
    Dwarf_Half tag = 0;
    if (elem)
    {
        Dwarf_Debug dbg = elem->dbg();
        Dwarf_Die die = elem->die();
        Dwarf_Die child =
            Utils::next_sibling(dbg, die, typeTags, numTags, &tag);

        shared_ptr<NSType> nextElem;

        if (child)
        {
            if (shared_ptr<Type> type = shared_dynamic_cast<Type>(
                Factory::instance().create(dbg, child, tag, false)))
            {
                nextElem = shared_static_cast<NSType>(type);
                // std::clog <<nextElem->name() << std::endl;
            }
        }
        elem = nextElem;
    }
}
// end Type iteration helpers
// -------------------------------------------------------------


/**
 * @return a list of types defined within this namespace
 */
const TypeList& Namespace::types() const
{
    if (!types_.get())
    {
        types_.reset(new TypeList);

        List<NSType> types(dbg(), die());
        List<NSType>::const_iterator i = types.begin(), end = types.end();
        for (; i != end; ++i)
        {
            types_->push_back(i);
        }
    }
    assert(types_.get());
    return *types_;
}


const VarList& Namespace::variables() const
{
    typedef List<VariableT<Namespace> > NSVarList;

    if (!vars_.get())
    {
        NSVarList vars(dbg(), die());
        vars_.reset(new VarList);

        for (NSVarList::iterator i = vars.begin(); i != vars.end(); ++i)
        {
            //std::string name = this->name();
            //set_name(*i, (name + "::" + i->name()).c_str());

            vars_->push_back(i);
        }
    }
    assert (vars_.get());
    return *vars_;
}


/*
const NamespaceList& Namespace::namespaces() const
{
    if (!vars_)
    {
        List<Namespace> ns(dbg(), die());
        namespaces_.reset(new NamespaceList);

        for (List<Namespace>::iterator i = ns.begin(); i != ns.end(); ++i)
        {
            namespaces_->push_back(i);
        }
    }
    assert (namespaces_);
    return *namespaces_;
}
*/
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
