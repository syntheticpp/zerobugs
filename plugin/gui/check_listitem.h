#ifndef CHECK_LISTITEM_H__21880987_7871_49C6_96D5_56E76891A6D6
#define CHECK_LISTITEM_H__21880987_7871_49C6_96D5_56E76891A6D6
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
//
#include <memory>
#include "gtkmm/listitem.h"
#include "gtkmm/selectionitem.h"
#include "zdk/ref_ptr.h"
#include "zdk/zobject.h"


namespace Gtk
{
    class CheckButton;
}


/**
 * A list item that has a check button
 */
class ZDK_LOCAL CheckListItem : public Gtk::ListItem
{
public:
    CheckListItem(const std::string& label, bool checked);

    CheckListItem(const CheckListItem&);

    virtual ~CheckListItem();

    /* emitted when check button is toggled */
    SigC::Signal0<void> toggled;

    /**
     * Associate a reference-counted object with this item.
     */
    void set_data(const RefPtr<ZObject>&);

    RefPtr<ZObject> data() const { return data_; }

    bool is_checked() const;

    void check(bool);

    bool has_check_button() const;

    void hide_check_button();

protected:
    void grab_focus_impl() {}

private:
    RefPtr<ZObject>     data_;

#ifdef GTKMM_2
    bool                checked_;
#else
    std::auto_ptr<Gtk::CheckButton> cbtn_;
#endif
};


/**
 * Return true if the item is a CheckListItem and the
 * check button at the left is active
 */
bool ZDK_LOCAL is_checked(const Gtk::ListItem*);

bool ZDK_LOCAL has_check_button(const Gtk::ListItem*);

/**
 * If the list item is a CheckListItem, them set the
 * active state of the check button
 */
void ZDK_LOCAL check(Gtk::ListItem*, bool);

#endif // CHECK_LISTITEM_H__21880987_7871_49C6_96D5_56E76891A6D6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
