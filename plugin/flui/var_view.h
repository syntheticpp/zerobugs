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
        , public DebugSymbolEvents
    {
    public:
        explicit VarView(ui::Controller&);

    protected:
        void update(const State&);
        bool notify(DebugSymbol*);
    };
}

#endif // VAR_VIEW_H__0D373DD6_C3ED_49BE_8F01_7FB4D8B72D98

