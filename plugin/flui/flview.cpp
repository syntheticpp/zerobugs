//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "const.h"
#include "flcallback.h"
#include "flpack_layout.h"
#include "flview.h"

using namespace ui;


////////////////////////////////////////////////////////////////
void insert(Fl_Widget& w, LayoutCallback& cb)
{
    auto& callback = dynamic_cast<Callback&>(cb);

    callback.set_widget(&w);
    callback.insert();
}


////////////////////////////////////////////////////////////////
Layout::CallbackPtr
FlPackLayout::make_callback(ViewType type)
{
    Layout::CallbackPtr callback;

    switch (type)
    {
    case VIEW_Code:
        callback.reset(new ::Callback(code_, code_->x(), code_->y(),
            code_->w(), code_->h()));
        break;

    case VIEW_Vars:
        callback.reset(new ::Callback(bottomL_, bottomL_->x(),bottomL_->y(),
            bottomL_->w(), bottomL_->h() - Const::tab_label_height));
        break;

    case VIEW_Stack:
        callback.reset(new ::Callback(bottomR_, bottomR_->x(), bottomR_->y(),
            bottomR_->w(), bottomR_->h() - Const::tab_label_height));
        break;

    case VIEW_Threads:
        break;

    default:
        break;
    }

    return callback;
}

