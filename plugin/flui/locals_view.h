#ifndef LOCALS_VIEW_H__3B4111E8_2CE1_4A40_AF98_65DAC23BFAE8
#define LOCALS_VIEW_H__3B4111E8_2CE1_4A40_AF98_65DAC23BFAE8
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "var_view.h"


namespace ui
{
    /**
     * Local Variables view.
     */
    class LocalsView : public VarView
    {
    public:
        explicit LocalsView(ui::Controller&);

    protected:
        // View interface
        void update(const State&);

    private:
    };
}

#endif // LOCALS_VIEW_H__3B4111E8_2CE1_4A40_AF98_65DAC23BFAE8

