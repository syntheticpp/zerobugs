#ifndef STACK_VIEW_H__E43B955D_AB3D_4CFF_B2D7_1C4779EC4EA5
#define STACK_VIEW_H__E43B955D_AB3D_4CFF_B2D7_1C4779EC4EA5
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/zerofwd.h"
#include "view.h"


namespace ui
{
    class Controller;


    class StackView : public View
    {
    public:
        typedef std::vector<RefPtr<Frame> > Frames;

        explicit StackView(Controller&);

        size_t frame_count() const;

        RefPtr<Frame> get_frame(size_t n) const;

        void select_frame(size_t);

    protected:
        ~StackView() throw();

        virtual void update(const ui::State&);

        virtual ViewType type() const {
            return VIEW_Stack;
        }

    private:
        RefPtr<StackTrace>   stack_;
    };
}

#endif // STACK_VIEW_H__E43B955D_AB3D_4CFF_B2D7_1C4779EC4EA5

