//
// $Id: bool_filter_param.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "gtkmm/box.h"
#include "gtkmm/checkbutton.h"
#include "bool_filter_param.h"
#include "options.h"


BoolFilterParam::BoolFilterParam(const char* name,
                                 const char* label,
                                 Properties* prop,
                                 bool value)
    : prop_(prop)
    , name_(name)
    , iniValue_(value)
    , btn_(manage (new_check_button(label)))
{
    if (prop)
    {
        iniValue_ = prop->get_word(name, value);
    }
    btn_->set_active(iniValue_);
}



void BoolFilterParam::add_to(Gtk::Box& box)
{
    box.pack_start(*btn_, false, false);
}



void BoolFilterParam::apply()
{
    if (RefPtr<Properties> prop = prop_.ref_ptr())
    {
        prop->set_word(name_.c_str(), btn_->get_active());
    }
}


bool BoolFilterParam::has_changed() const
{
    return iniValue_ != btn_->get_active();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
