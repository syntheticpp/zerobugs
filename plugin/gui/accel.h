#ifndef ACCEL_H__40D14832_386D_47D8_B870_5BD6BA765AB0
#define ACCEL_H__40D14832_386D_47D8_B870_5BD6BA765AB0
//
// $Id: accel.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Accelerator-related utilities, for gtkmm 1.2
//
#include <string>


/**
 * Given a string representation of an accelerator that
 * is suitable for Gtk::AccelGroup::parse, transform it
 * into a string that follows the label convention, i.e.
 * <control> becomes Ctl+, <alt> becomes Alt+, etc.
 */
std::string accel_to_label(const std::string&);


/**
 * Given an accelerator label text, convert it to
 * a string that is parseable by Gtk::AccelGroup::parse
 * i.e. Ctl+ becomes <Ctl>, Alt+ becomes <Alt>, etc
 */
std::string accel_from_label(const std::string&);

#endif // ACCEL_H__40D14832_386D_47D8_B870_5BD6BA765AB0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
