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
// Simulate a Gtkmm-1.2 List widget, using the Gtkmm-2.4 TreeView
// -- for ease of porting over from the old GTK
//
#include "list.h"
#include "listitem.h"
#include "selectionitem.h"
#include "zdk/check_ptr.h"
#include "../check_listitem.h" // hack
#include <gtkmm/treeselection.h>
#ifdef DEBUG
 #include <iostream>
 using namespace std;
#endif



Gtk::List::List()
{
    set_headers_visible(false);

    Gtk::TreeModel::ColumnRecord::add(column_);
    Gtk::TreeModel::ColumnRecord::add(userData_);
    Gtk::TreeModel::ColumnRecord::add(active_);
    Gtk::TreeModel::ColumnRecord::add(visible_);

    listStore_ = Gtk::ListStore::create(*this);

    // <CheckListItem support>
    Gtk::CellRendererToggle* render =
        Gtk::manage( new Gtk::CellRendererToggle() );
    append_column("", *render);
    if (Gtk::TreeViewColumn* col = get_column(0))
    {
        col->add_attribute(render->property_active(), active_);
        col->add_attribute(render->property_visible(), visible_);
    }
    render->signal_toggled().connect(
        sigc::mem_fun(*this, &Gtk::List::on_cell_toggled));
    // </CheckListItem support>

    append_column("", column_);// add column to view

    set_model(listStore_);
    selection_.reset(new Gtk::SelectionList(*this));
    if (get_selection())
    {
        get_selection()->signal_changed().connect(
            sigc::mem_fun(this, &Gtk::List::on_selection_changed));
    }
}


void Gtk::List::on_selection_changed()
{
    selection_changed_impl();
    selection_changed_();
}


bool Gtk::List::on_button_press_event(GdkEventButton* event)
{
    bool result = true;
    if (event->button != 3)
    {
        result = TreeView::on_button_press_event(event);
    }
    // In the Gtk-1.2 world, each item is a widget contained
    // within the list, and each gets its own button event --
    // we need to simulate the same behavior and pass the event
    // to the appropriate item.
    Gtk::ListItem* item = NULL;

    // 1. find which item is under the mouse
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* col = NULL;
    int x = static_cast<int>(event->x);
    int y = static_cast<int>(event->y);

    if (!get_path_at_pos(x, y, path, col, x, y))
    {
        return result;
    }
    if (Gtk::TreeIter iter = CHKPTR(listStore_)->get_iter(path))
    {
        const Gtk::TreeRow& row = *iter;
        if (void* data = row.get_value(userData_))
        {
            item = reinterpret_cast<Gtk::ListItem*>(data);
        }
    }

    // 2. pass it the event
    if (item)
    {
        item->on_button_press_event(event);
    }

    return result;
}


void Gtk::List::add(Gtk::ListItem& item)
{
    Gtk::TreeModel::iterator iter = CHKPTR(listStore_)->append();
    Gtk::TreeModel::Row row = *iter;

    row.set_value(column_, item.value());
    row.set_value(userData_, &item);

    if (CheckListItem* c = dynamic_cast<CheckListItem*>(&item))
    {
        row.set_value(visible_, true);
        row.set_value(active_, c->is_checked());
    }

    if (item.is_managed_())
    {
        item.index_ = items_.size();
        items_.push_back(boost::shared_ptr<ListItem>(&item));
    }
}


Gtk::SelectionList& Gtk::List::selection()
{
    assert(selection_.get());
    return *selection_;
}


void Gtk::List::set_selection_mode(Gtk::SelectionMode mode)
{
    Glib::RefPtr<Gtk::TreeSelection> sel = get_selection();
    if (sel)
    {
        sel->set_mode(mode);
    }
}


Gtk::ItemList& Gtk::List::get_item_list() const
{
    if (itemList_.get() == 0)
    {
        assert(listStore_);
        itemList_.reset(new ItemList(const_cast<List&>(*this)));
    }
    assert(itemList_.get());
    return *itemList_;
}


int Gtk::List::child_position(const Gtk::ListItem& item) const
{
    return item.index();
}


const Gtk::SelectionList& Gtk::List::selection() const
{
    assert(selection_.get());
    return *selection_;
}


void Gtk::List::on_cell_toggled(const Glib::ustring& path)
{
    // Get the model row that has been toggled.
    Gtk::TreeIter iter = CHKPTR(listStore_)->get_iter(Gtk::TreeModel::Path(path));
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;

        CheckListItem* item =
            dynamic_cast<CheckListItem*>(row.get_value(userData_));

        assert(item);
        if (item)
        {
            bool flag = item->is_checked();
            flag ^= true;
            item->check(flag);

            row[active_] = flag;
        }
    }
}

/*
void Gtk::List::clear_items(int, int)
{
    std::clog << __func__ << ": not implemented\n";
    throw std::logic_error("not implemented");
}
*/


void Gtk::List::remove_items(Gtk::SelectionList::iterator first,
                             Gtk::SelectionList::iterator last)
{
    Items::iterator j = items_.begin();

    Gtk::TreeNodeChildren list = CHKPTR(listStore_)->children();
    Gtk::TreeIter i = list.begin();
    for (; first != last && i != list.end(); )
    {
        Gtk::ListItem* item = (*i).get_value(userData_);
        if (item == *first)
        {
            i = CHKPTR(listStore_)->erase(i);
            j = items_.erase(j);
            ++first;
        }
        else
        {
            ++i, ++j;
        }
    }
}

/*
void Gtk::List::select_item(int n)
{
    select(CHKPTR(listStore_)->children()[n]);
}


void Gtk::List::deselect_item(int n)
{
    deselect(CHKPTR(listStore_)->children()[n]);
}
*/

void Gtk::List::select(const Gtk::TreeRow& row)
{
    Glib::RefPtr<Gtk::TreeView::Selection> sel = get_selection();
    if (sel)
    {
        scroll_to_row(CHKPTR(listStore_)->get_path(row));

        sel->select(row);
        //assert(sel->is_selected(row));
    }
}


void Gtk::List::deselect(const Gtk::TreeRow& row)
{
    Glib::RefPtr<Gtk::TreeView::Selection> sel = get_selection();
    if (sel)
    {
        sel->unselect(row);
    }
}


void Gtk::List::select_all()
{
    Glib::RefPtr<Gtk::TreeView::Selection> sel = get_selection();
    if (sel)
    {
        sel->select_all();
    }
}


void Gtk::List::unselect_all()
{
    Glib::RefPtr<Gtk::TreeView::Selection> sel = get_selection();
    if (sel)
    {
        sel->unselect_all();
    }
}


Gtk::TreeModel::const_iterator
Gtk::List::get_iter(const Gtk::TreePath& path) const
{
    return CHKPTR(listStore_)->get_iter(path);
}


// SelectionList
Gtk::SelectionList::SelectionList(Gtk::List& list) : list_(list)
{
}


size_t Gtk::SelectionList::size() const
{
    Glib::RefPtr<const Gtk::TreeView::Selection> sel = list_.get_selection();
    if (!sel)
    {
        return 0;
    }
    return sel->count_selected_rows();
}


Gtk::TreeModel::iterator Gtk::SelectionList::get_selected()
{
    Gtk::TreeModel::iterator iter;

    Glib::RefPtr<Gtk::TreeView::Selection> sel = list_.get_selection();
    if (sel)
    {
        iter = sel->get_selected();
    }
    return iter;
}


std::vector<Gtk::SelectionItem>& Gtk::SelectionList::items_impl() const
{
    items_.clear();
    Glib::RefPtr<Gtk::TreeView::Selection> sel = list_.get_selection();
    if (sel)
    {
        typedef Gtk::TreeSelection::ListHandle_Path ListHandle_Path;

        ListHandle_Path rows = sel->get_selected_rows();

        ListHandle_Path::const_iterator i = rows.begin();
        for (; i != rows.end(); ++i)
        {
            const TreeModelColumn<Gtk::ListItem*>& modelCol =
                list_.user_data_col();

            assert(list_.get_iter(*i));

            ListItem* item = list_.get_iter(*i)->get_value(modelCol);
            if (item)
            {
                items_.push_back(SelectionItem(item));
            }
        }
    }

    return items_;
}


Gtk::ItemList::ItemList(List& list) : list_(list)
{
}


void Gtk::ItemList::clear()
{
    CHKPTR(list_.listStore_)->clear();
    list_.items_.clear();
}


Gtk::TreeModel::Children Gtk::ItemList::children()
{
    return CHKPTR(list_.listStore_)->children();
}


Gtk::TreeModel::Children Gtk::ItemList::children() const
{
    return CHKPTR(list_.listStore_)->children();
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
