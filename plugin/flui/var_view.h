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


namespace ui
{
    /**
     * Base class for viewing program variables.
     */
    class VarView
        : public View
        , public DebugSymbolEvents
    {
    public:
        explicit VarView(ui::Controller&);

        void clear(bool resetScope = false);

        DebugSymbol& get_variable(size_t n) const;

        size_t variable_count() const {
            return variables_.size();
        }

        bool is_expanding(size_t row) const {
            return is_expanding(variables_[row].get());
        }

        void expand(size_t row, bool = true);

        bool has_variable_changed(const DebugSymbol&) const;

        /// DebugSymbolEvents interface
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
            DebugSymbol* old ) {
        }

    protected:
        ~VarView() throw();

        BEGIN_INTERFACE_MAP(VarView)
        END_INTERFACE_MAP()

        bool is_same_scope(Symbol*) const;

        /// View interface
        virtual ViewType type() const {
            return VIEW_Vars;
        }

        virtual void update(const State&);

        void update_scope(const State&);

    private:
        // @note: just to be consistent with the fact that this
        // view (and its derived classses) show program variables
        // to the user the Variables name is used here; the
        // debugger engine uses DebugSymbols to model variables;
        // DebugSymbols may model other types of program artifacts,
        // so they are more general. In the context of VarViews
        // it is fair to use them interchangeably
        typedef std::vector<RefPtr<DebugSymbol> > Variables;

        struct Scope;

    private:
        Scope& scope() const;

        RefPtr<Symbol>  current_;
        RefPtr<Scope>   scope_;
        Variables       variables_;
    };
}

#endif // VAR_VIEW_H__0D373DD6_C3ED_49BE_8F01_7FB4D8B72D98

