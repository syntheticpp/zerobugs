#ifndef BREAKPOINT_VIEW_H__5EE8A2E6_36FC_4DE8_AC43_68F746EC69E9
#define BREAKPOINT_VIEW_H__5EE8A2E6_36FC_4DE8_AC43_68F746EC69E9
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/weak_ptr.h"
#include "view.h"
#include <vector>

class BreakPointAction;


namespace ui
{
    struct UserBreakPoint
    {
        RefPtr<BreakPoint>          bpoint;
        RefPtr<BreakPointAction>    action;
    };

    typedef RefPtr<BreakPoint> BreakPointPtr;
    typedef std::vector<UserBreakPoint> BreakPoints;


    class BreakPointView
        : public View
        , EnumCallback<BreakPointAction*>
    {
    public:
        explicit BreakPointView(Controller&);

        ViewType type() const {
            return VIEW_BreakPoints;
        }

        size_t size() const {
            return breakpoints_.size();
        }

        BreakPoints::const_iterator begin() const {
            return breakpoints_.begin();
        }

        BreakPoints::const_iterator end() const {
            return breakpoints_.end();
        }

        UserBreakPoint operator[](size_t n) const {
            return breakpoints_[n];
        }

    protected:
        ~BreakPointView() throw();

        void update(const ui::State&);

        virtual void update_breakpoint(BreakPoint&);

        // EnumCallback interface
        void notify(BreakPointAction*);

    private:
        // valid during update_breakpoint
        WeakPtr<BreakPoint> current_;

        BreakPoints         breakpoints_;
    };
}

#endif // BREAKPOINT_VIEW_H__5EE8A2E6_36FC_4DE8_AC43_68F746EC69E9

