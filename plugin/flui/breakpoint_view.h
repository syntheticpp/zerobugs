#ifndef BREAKPOINT_VIEW_H__5EE8A2E6_36FC_4DE8_AC43_68F746EC69E9
#define BREAKPOINT_VIEW_H__5EE8A2E6_36FC_4DE8_AC43_68F746EC69E9
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"
#include <vector>

namespace ui
{
    typedef RefPtr<BreakPoint> BreakPointPtr;
    typedef std::vector<BreakPointPtr> BreakPoints;


    class BreakPointView : public View
    {
    public:
        explicit BreakPointView(Controller&);

        ViewType type() const {
            return VIEW_BreakPoints;
        }

    protected:
        ~BreakPointView() throw();

        void update(const ui::State&);

    private:
        BreakPoints     breakpoints_;
    };
}

#endif // BREAKPOINT_VIEW_H__5EE8A2E6_36FC_4DE8_AC43_68F746EC69E9
