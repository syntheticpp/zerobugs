//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/zero.h"
#include "stack_view.h"


ui::StackView::StackView(ui::Controller& c)
    : ui::View(c)
{
}


ui::StackView::~StackView() throw()
{
}


RefPtr<Frame> ui::StackView::get_frame(size_t n) const
{
    return stack_->frame(n);
}


void ui::StackView::update(const ui::State& state)
{
    if (state.current_thread())
    {
        stack_ = state.current_thread()->stack_trace();
    }
    else
    {
        stack_.reset();
    }
}


size_t ui::StackView::frame_count() const
{
    return stack_ ? stack_->size() : 0;
}

