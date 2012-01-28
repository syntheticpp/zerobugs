#ifndef ADAPT_H__637B7F5C_A58A_46E8_AC58_EA42DA9E3398
#define ADAPT_H__637B7F5C_A58A_46E8_AC58_EA42DA9E3398
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

#include "zdk/config.h"
#include "zdk/data_type.h"
#include "zdk/shared_string.h"
#include "zdk/symbol_map.h"
#include "generic/visitor.h"
#include "handle.h"
#include "reader.h"
#include "type_map.h"

class ClassTypeImpl;    // typez/public/types.h


namespace Dwarf
{
    class ArrayType;
    class AssocArrayType;
    class BaseType;
    class ConstType;
    class Delegate;
    class DynArrayType;
    class EnumType;
    class Function;
    class KlassType;
    class Parameter;
    class PointerType;
    class PtrToMemberType;
    class Reader;
    class SubroutineType;
    class Typedef;
    class UnionType;
    class VolatileType;

    template<typename T> class List;

    enum Depth
    {
        ADAPT_FULL,
        ADAPT_SHALLOW,
        ADAPT_VERY_SHALLOW
    };

    /**
     * Adapts a Dwarf::Type into a zdk DataType;
     * uses the Acyclic ConstVisitor pattern.
     * @note the dwarfz C++ wrapper for libdwarf was
     * originaly written as a standalone library, which
     * does not know about the zdk object model.
     */
    CLASS TypeAdapter
        : public BaseVisitor
        , public ConstVisitor<Dwarf::ArrayType>
        , public ConstVisitor<Dwarf::AssocArrayType>
        , public ConstVisitor<Dwarf::Delegate>
        , public ConstVisitor<Dwarf::DynArrayType>
        , public ConstVisitor<Dwarf::BaseType>
        , public ConstVisitor<Dwarf::KlassType>
        , public ConstVisitor<Dwarf::ConstType>
        , public ConstVisitor<Dwarf::EnumType>
        , public ConstVisitor<Dwarf::PointerType>
        , public ConstVisitor<Dwarf::PtrToMemberType>
        , public ConstVisitor<Dwarf::UnionType>
        , public ConstVisitor<Dwarf::SubroutineType>
        , public ConstVisitor<Dwarf::Typedef>
        , public ConstVisitor<Dwarf::VolatileType>
    {
        void visit(const Dwarf::ArrayType&);
        void visit(const Dwarf::AssocArrayType&);
        void visit(const Dwarf::Delegate&);
        void visit(const Dwarf::DynArrayType&);
        void visit(const Dwarf::BaseType&);
        void visit(const Dwarf::KlassType&);
        void visit(const Dwarf::ConstType&);
        void visit(const Dwarf::EnumType&);
        void visit(const Dwarf::PointerType&);
        void visit(const Dwarf::PtrToMemberType&);
        void visit(const Dwarf::SubroutineType&);
        void visit(const Dwarf::Typedef&);
        void visit(const Dwarf::UnionType&);
        void visit(const Dwarf::VolatileType&);

    public:
        TypeAdapter(Reader*, const RefPtr<Thread>&, addr_t frameBase, TypeMap&);

        /**
         * @return the adapted type
         */
        RefPtr<DataType> type() const { return type_; }

        RefPtr<DataType> apply(boost::shared_ptr<Dwarf::Type>);

        /**
         * @return the ZDK DataType object that corresponds to
         * a Dwarf::Function object. The function type is determined
         * based on the function's return and argument types.
         * An optional parameter indicates whether the function
         * is a method of a class
         */
        RefPtr<DataType> get_type(const Dwarf::Function&, const KlassType* = NULL);

        /**
         * Set the value for the "shallow" flag, which is used
         * when adapting structs and classes. For performance
         * reasons, the TypeAdapter may not descend into the
         * children of a struct or class -- rather, if the "shallow"
         * flag is set, it produces a proxy object for the adapted
         * class type; the proxy will trigger a "deep" adaptation
         * the first time the member data needs to be read.
         */
        void set_depth(Depth depth) { depth_ = depth; }

        /**
         * Given a forward-declared type (for which is_incomplete returns
         * true), attempt to locate the full definition.
         * @return the resolved type, or NULL
         */
        boost::shared_ptr<Dwarf::Type> resolve(const Dwarf::Type&);

        /**
         * Force type name to given value.
         */
        void set_class_name(const RefPtr<SharedString>& name)
        {
            typeName_ = name;
        }

        void set_base_addr(addr_t addr) { baseAddr_ = addr; }

    private:
        RefPtr<DataType> get_fun_type(
            const Dwarf::Die&,
            const boost::shared_ptr<Dwarf::Type>&,
            const Dwarf::List<Dwarf::Parameter>&,
            const Dwarf::KlassType* = NULL);

        RefPtr<DataType> get_fun_type(
            const Dwarf::Die&,
            const boost::shared_ptr<Dwarf::Type>&,
            const Dwarf::Function::ParamList&,
            const Dwarf::KlassType* = NULL);

        void add_methods(const Dwarf::KlassType&, ClassTypeImpl&);

        boost::shared_ptr<Type> lookup_type(const Dwarf::Type& type,
                                            const RefPtr<SharedString>& module,
                                            bool byCtor = false) const;

        TypeSystem& type_system();

        /**
         * helper lookup, implemented here so that
         * the compiler may inline it if necessary
         */
        boost::shared_ptr<Dwarf::Type>
        lookup_type_in_all_modules( RefPtr<Process>,
                                    const Type& what,
                                    bool byCtor = false
                                  ) const;

    private:
        Reader* const           reader_;
        RefPtr<Thread>          thread_;
        addr_t                  baseAddr_;  // frame base
        off_t                   modAdjust_;
        addr_t                  pc_;        // program count
        TypeMap&                typeMap_;
        RefPtr<DataType>        type_;      // adapted result
        Depth                   depth_;
        TypeAdapter*            context_;
        RefPtr<SharedString>    modName_;
        RefPtr<SharedString>    typeName_;
    };
}

#endif // ADAPT_H__637B7F5C_A58A_46E8_AC58_EA42DA9E3398
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
