#ifndef SELECTIONITEM_H__A93EEF50_B062_4566_AFD4_00AD678BF96E
#define SELECTIONITEM_H__A93EEF50_B062_4566_AFD4_00AD678BF96E
//
// $Id: selectionitem.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if !GTKMM_2
////////////////////////////////////////////////////////////////
// Gtk-- 1.2
 #include <gtk--/widget.h>
 #include <gtk--/listitem.h>

namespace Gtk
{
 typedef ListItem* SelectionItem;
}

#else
////////////////////////////////////////////////////////////////
// Gtkmm 2.x
 #include <algorithm> // for std::swap
 #include <boost/shared_ptr.hpp>
 #include "listitem.h"

 namespace Gtk
 {
/**
 * Wrapper around the user-data associated with a selected
 * item in a list or tree.
 */
 class SelectionItem
 {
 public:
    explicit SelectionItem(ListItem* item) : item_(item)
    {
        assert(item);
    }

    SelectionItem(const SelectionItem& other)
        : item_(other.item_)
    { }

    SelectionItem& operator=(const SelectionItem& other)
    {
        SelectionItem temp(other);
        return this->swap(temp);
    }

    SelectionItem& swap(SelectionItem& other) throw()
    {
        std::swap(item_, other.item_);
        return *this;
    }

  // hack
    operator const ListItem*() const { return item_; }
    operator ListItem*() { return item_; }

    std::string value() const { return item_ ? item_->value() : std::string(); }

    const SelectionItem* operator->() const { return this; }

    void* property_user_data() const { return item_->user_data(); }

 private:
    Gtk::ListItem* item_;
 };
} // namespace Gtk
#endif
#endif // SELECTIONITEM_H__A93EEF50_B062_4566_AFD4_00AD678BF96E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
