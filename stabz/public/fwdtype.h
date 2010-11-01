#ifndef FWDTYPE_H__B166575C_DD7D_435E_8183_17F6824B3D33
#define FWDTYPE_H__B166575C_DD7D_435E_8183_17F6824B3D33
//
// $Id: fwdtype.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/observer_impl.h"
#include "typez/public/types.h"
#include "stabz/public/type_id.h"


namespace Stab
{
    /**
     * Forward types are place holders for types that
     * have not been fully defined yet. Indirects to
     * another type.
     */
    class ForwardType : public IndirectType
    {
    public:
        DECLARE_UUID("a010d23c-7473-4137-91b2-633ace5934b5")

        BEGIN_INTERFACE_MAP(ForwardType)
            INTERFACE_ENTRY(ForwardType)
            INTERFACE_ENTRY_INHERIT(IndirectType)
            INTERFACE_ENTRY_DELEGATE(link())
        END_INTERFACE_MAP()

        ForwardType(SharedString* name, const TypeID& id);

        ~ForwardType() throw() {}

        /**
         * link this type to target type.
         */
        bool link(DataType* target);

        /**
         * @return the type this type forwards to, or null
         */
        DataType* link() const;

        const TypeID& typeID() const { return typeID_; }

        bitsize_t bit_size() const;

        void describe(int) const;

        void on_state_change(Subject*);

    protected:
        SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

        bool is_fundamental() const;

        // void attach(Observer*);

        // void set_name(SharedString&);

        size_t parse(const char*, Unknown2*) const;

    private:
        WeakPtr<DataType> link_;
        RefPtr<Observer> observ_;
        TypeID typeID_;
    };
}

#endif // FWDTYPE_H__B166575C_DD7D_435E_8183_17F6824B3D33
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
