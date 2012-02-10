//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flcallback.h"
#include "flpack_layout.h"
#include "flview.h"


////////////////////////////////////////////////////////////////
void insert(Fl_Widget& w, ui::LayoutCallback& cb)
{
    auto& callback = dynamic_cast<Callback&>(cb);

    callback.set_widget(&w);
    callback.insert();
}


////////////////////////////////////////////////////////////////
ui::Layout::CallbackPtr
FlPackLayout::make_callback(ui::ViewType type)
{
    ui::Layout::CallbackPtr callback;

    switch (type)
    {
    case ui::VIEW_Code:
        callback.reset(new ::Callback(code_, code_->x(), code_->y(), code_->w(), code_->h()));
        break;

    case ui::VIEW_Vars:
        callback.reset(new ::Callback(bottom_, bottom_->x(), bottom_->y(), bottom_->w(), bottom_->h() - 30));
        break;

    case ui::VIEW_Stack:
        break;

    case ui::VIEW_Threads:
        break;

    default:
        break;
    }

    return callback;
}

