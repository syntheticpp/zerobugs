#ifndef VIEWS_FACTORY_H__9BB76B7C_ECE0_45E0_9940_49B40E22AFBF
#define VIEWS_FACTORY_H__9BB76B7C_ECE0_45E0_9940_49B40E22AFBF
//
// $Id: views_factory.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/shared_ptr.hpp>
#include "zdk/unknown2.h"


namespace Gtk
{
    class Widget;
}

struct View
{
    virtual ~View() { }

    virtual Gtk::Widget* widget() = 0;
};
typedef boost::shared_ptr<View> ViewPtr;


DECLARE_INTERFACE(ViewsFactory)
{
    virtual ~ViewsFactory() { }

    virtual ViewPtr create_program_view() const = 0;

    virtual ViewPtr create_registers_view() const = 0;

    virtual ViewPtr create_stack_view() const = 0;

    virtual ViewPtr create_threads_view() const = 0;

    virtual ViewPtr create_variables_view() const = 0;
};

#endif // VIEWS_FACTORY_H__9BB76B7C_ECE0_45E0_9940_49B40E22AFBF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
