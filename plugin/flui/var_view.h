#ifndef VAR_VIEW_H__0D373DD6_C3ED_49BE_8F01_7FB4D8B72D98
#define VAR_VIEW_H__0D373DD6_C3ED_49BE_8F01_7FB4D8B72D98
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/debug_sym.h"
#include "symkey.h"
#include "view.h"
#include <map>
#include <vector>
#include <set>


namespace ui
{
    /**
     * Base class for viewing program variables.
     */
    class VarView 
        : public    View
        , protected DebugSymbolEvents
    {
    public:
        explicit VarView(ui::Controller&);

        DebugSymbol& get_symbol(size_t n) const;

        size_t symbol_count() const {
            return symbols_.size();
        }

        bool is_expanding(size_t row) const {
            return is_expanding(symbols_[row].get());
        }

        void expand(size_t row, bool = true);

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
        virtual bool is_expanding(DebugSymbol*) const;

        /** 
         * Readers call this method to determine what numeric base
         * should be used for the representation of integer values.
         */
        virtual int numeric_base(const DebugSymbol*) const {
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
        virtual ViewType type() const {
            return VIEW_Vars;
        }
        
        virtual void update(const State&);

    protected:
        typedef std::vector<RefPtr<DebugSymbol> > Symbols;
        typedef std::set<SymKey> Expands;

        Symbols     symbols_;
        Expands     expands_;
    };
}

#endif // VAR_VIEW_H__0D373DD6_C3ED_49BE_8F01_7FB4D8B72D98

