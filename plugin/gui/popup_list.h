#ifndef POPUP_LIST_H__3B0B0361_4F19_4C8B_9F1A_4E900D814F14
#define POPUP_LIST_H__3B0B0361_4F19_4C8B_9F1A_4E900D814F14
//
//
// $Id: popup_list.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <set>
#include <string>
#include <boost/shared_ptr.hpp>
#include "gtkmm/window.h"


class TextEntry;

/**
 * A List inside of a popup window, works in conjunction
 * with a TextEntry, for keystroke auto-completion hints
 */
class PopupList : public Gtk::Window
{
public:
    PopupList();

    void set_items(const std::set<std::string>&);

    void set_text_entry(TextEntry*);

    void set_text_and_hide(const std::string&);

private:
    void on_selection();

    void set_position();

    class Impl;
    boost::shared_ptr<Impl> impl_;
    TextEntry* textEntry_;
};

#endif // POPUP_LIST_H__3B0B0361_4F19_4C8B_9F1A_4E900D814F14
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
