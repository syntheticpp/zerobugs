#ifndef BOOL_FILTER_PARAM_H__C70F008A_541C_4E74_9BE6_CB2502AA33C1
#define BOOL_FILTER_PARAM_H__C70F008A_541C_4E74_9BE6_CB2502AA33C1
//
// $Id: bool_filter_param.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string>
#include "zdk/properties.h"
#include "zdk/weak_ptr.h"
#include "gtkmm/base.h"
#include "filter_param.h"


namespace Gtk
{
    class CheckButton;
}


class ZDK_LOCAL BoolFilterParam : public Gtk::Base, public DataFilterParam
{
public:
    BoolFilterParam(const char* name,
                    const char* label,
                    Properties*,
                    bool defaultValue);

    void add_to(Gtk::Box& box);

    void apply();

    const char* name() const { return name_.c_str(); }

    bool has_changed() const;

private:
    WeakPtr<Properties> prop_;
    std::string name_;
    bool iniValue_;
    Gtk::CheckButton* btn_;
};

#endif // BOOL_FILTER_PARAM_H__C70F008A_541C_4E74_9BE6_CB2502AA33C1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
