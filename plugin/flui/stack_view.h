#ifndef STACK_VIEW_H__E43B955D_AB3D_4CFF_B2D7_1C4779EC4EA5
#define STACK_VIEW_H__E43B955D_AB3D_4CFF_B2D7_1C4779EC4EA5
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"

namespace ui
{
    class Controller;


    class StackView : public View
    {
    public:
        explicit StackView(Controller&);

    protected:
        ~StackView() throw();


    private:
    };
}

#endif // STACK_VIEW_H__E43B955D_AB3D_4CFF_B2D7_1C4779EC4EA5

