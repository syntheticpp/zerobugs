#ifndef VAR_VIEW_H__0D373DD6_C3ED_49BE_8F01_7FB4D8B72D98
#define VAR_VIEW_H__0D373DD6_C3ED_49BE_8F01_7FB4D8B72D98
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/debug_sym.h"
#include "view.h"


namespace ui
{
    /**
     * Base class for viewing program variables.
     */
    class VarView 
        : public View
        , protected DebugSymbolEvents
    {
    public:
        explicit VarView(ui::Controller&);

    protected:
        ~VarView() throw();

        BEGIN_INTERFACE_MAP(VarView)
        END_INTERFACE_MAP()

        // === DebugSymbolEvents interface ===
        bool notify(DebugSymbol*);

        /** 
         * Symbols that correspond to aggregate objects such as
         * class instances or arrays may be expanded, so that the
         * user can inspect their sub-parts. This method is called
         * by the reader implementations to determine if the client
         * wants such an aggregate object to be expanded or not.
         */
        virtual bool is_expanding(DebugSymbol*) const
        {
            return false;
        }

        /** 
         * Readers call this method to determine what numeric base
         * should be used for the representation of integer values.
         */
        virtual int numeric_base(const DebugSymbol*) const
        {
            return 0;
        }

        /** 
         * A change in the symbol has occurred (name, type, address * etc.)
         * A pointer to the old values is passed in.
         */
        virtual void symbol_change (   
            DebugSymbol* newSym,
            DebugSymbol* old )
        { }

        // === View interface === 
        virtual ViewType type() const
        {
            return VIEW_Vars;
        }
    };
}

#endif // VAR_VIEW_H__0D373DD6_C3ED_49BE_8F01_7FB4D8B72D98

