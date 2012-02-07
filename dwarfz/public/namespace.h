#ifndef NAMESPACE_H__EC26FCA4_B34D_433A_9CA8_00D8C361AB8F
#define NAMESPACE_H__EC26FCA4_B34D_433A_9CA8_00D8C361AB8F
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
//
#include <memory>
#include <set>
#include <dwarf.h>
#include "die.h"
#include "funfwd.h"
#include "list.h"
#include "variable.h"


namespace Dwarf
{
    class CompileUnit;
    class ImportedDecl;
    class Namespace;

    typedef std::vector<std::shared_ptr<Namespace> > NamespaceList;


    CLASS Namespace : public Die
    {
    public:
        enum { TAG = DW_TAG_namespace };
        typedef Namespace parent_type;

        Namespace(Dwarf_Debug dbg, Dwarf_Die die);

        virtual ~Namespace() throw() { }

        List<ImportedDecl> imported_decls() const;

        bool search(Dwarf_Off) const;

        const FunList& functions() const;

        const TypeList& types() const;

        const VarList& variables() const;

        //const NamespaceList& namespaces() const;
        List<Namespace> ns_list() const
        { return List<Namespace>(this->dbg(), this->die()); }

    private:
        mutable std::set<Dwarf_Off> imports_;
        mutable std::auto_ptr<FunList> funcs_;
        mutable std::auto_ptr<TypeList> types_;
        mutable std::auto_ptr<VarList> vars_;
    };


    /**
     * Allow for creating List<Namespace> as children of
     * objects of type T
     */
    template<typename T>
        CLASS NamespaceT : public Namespace
    {
    public:
        typedef T parent_type;

    protected:
        NamespaceT (Dwarf_Debug dbg, Dwarf_Die die)
            : NamespaceT(dbg, die)
        { }

    public:
        ~NamespaceT() throw() { }
    };
}
#endif // NAMESPACE_H__EC26FCA4_B34D_433A_9CA8_00D8C361AB8F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
