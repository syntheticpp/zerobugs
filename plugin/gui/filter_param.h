#ifndef FILTER_PARAM_H__48DC2B31_3F3A_433D_8EFC_1EDE3FDE0C09
#define FILTER_PARAM_H__48DC2B31_3F3A_433D_8EFC_1EDE3FDE0C09
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

namespace Gtk
{
    class Box;
}


/**
 * Model a control that corresponds to a DataFilter parameter.
 * @see zdk/data_filter.h
 */
struct DataFilterParam
{
    virtual ~DataFilterParam() { }

    virtual void add_to(Gtk::Box&) = 0;

    virtual void apply() = 0;

    virtual const char* name() const = 0;

    virtual bool has_changed() const = 0;
};


#endif // FILTER_PARAM_H__48DC2B31_3F3A_433D_8EFC_1EDE3FDE0C09
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
