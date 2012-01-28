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

#include <cassert>
#include <iostream>
#include "clist.h"
#include "flags.h"

using namespace std;


Gtk::CList::CList(const Glib::SArray& titles)
{
    Glib::SArray::const_iterator i = titles.begin();
    for (; i != titles.end(); ++i)
    {
        boost::shared_ptr<Column> col(new Column());
        columns_.push_back(col);
        add_to_view(*i, *col);
    }
    Gtk::TreeModel::ColumnRecord::add(userData_);
    model_ = Glib::RefPtr<ListAdapter>(new ListAdapter(*this));
    sortModel_ = TreeModelSort::create(model_);
    set_model(sortModel_);
}


Gtk::ListAdapter& Gtk::CList::rows()
{
    assert(model_);
    return *(model_.operator->());
}


Gtk::ListSelectionAdapter Gtk::CList::selection()
{
    return ListSelectionAdapter(*this);
}


void Gtk::CList::remove_row(size_t index)
{
    Glib::RefPtr<Gtk::TreeModel> treeModel = model_;
    if (sortModel_)
    {
        treeModel = sortModel_;
    }

    Gtk::TreeNodeChildren list = treeModel->children();
    Gtk::TreeIter i = list.begin();
    for (size_t n = 0; i != list.end(); ++n)
    {
        if (n == index)
        {
            if (sortModel_)
            {
                i = sortModel_->convert_iter_to_child_iter(i);
            }
            i = model_->erase(i);
            break;
        }
        else
        {
            ++i;
        }
    }
}



Gtk::TreeNodeChildren::iterator
Gtk::CList::get_row(Gtk::TreeNodeChildren::iterator i) const
{
    if (sortModel_)
    {
        i = sortModel_->convert_child_iter_to_iter(i);
    }
    return i;
}


void Gtk::CList::on_cursor_changed()
{
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column = NULL;

    get_cursor(path, column);

    if (Gtk::TreeIter iter = model_->get_iter(path))
    {
        selectRow_(iter, 0, NULL);
    }
}


////////////////////////////////////////////////////////////////
// ListSelectionAdapter
Gtk::ListSelectionAdapter::ListSelectionAdapter(CList& list)
    : list_(list)
{
}


size_t Gtk::ListSelectionAdapter::size() const
{
    if (!sel_)
    {
        sel_ = list_.get_selection();
    }
    if (!sel_)
    {
        return 0;
    }
    return sel_->count_selected_rows();
}


Gtk::ListSelectionAdapter::const_iterator
Gtk::ListSelectionAdapter::begin() const
{
    if (!sel_)
    {
        sel_ = list_.get_selection();
    }
    assert(sel_);

    if (cont_.empty())
    {
        cont_ = sel_->get_selected_rows();
    }
    return cont_.begin();
}


Gtk::ListSelectionAdapter::const_iterator
Gtk::ListSelectionAdapter::end() const
{
    if (!sel_)
    {
        sel_ = list_.get_selection();
    }
    assert(sel_);
    if (cont_.empty())
    {
        cont_ = sel_->get_selected_rows();
    }
    return cont_.end();
}


////////////////////////////////////////////////////////////////
// ListAdapter
Gtk::ListAdapter::ListAdapter(CList& list)
    : Gtk::ListStore(list)
    , list_(list)
{
}


void Gtk::ListAdapter::push_back(const vector<string>& elem)
{
    assert(elem.size() == list_.columns_.size());

    iterator iter = append();

    Row row = *iter;
    for (size_t i = 0; i != elem.size(); ++i)
    {
        row.set_value(i, elem[i]);
    }
}

/*
Gtk::RowAdapter Gtk::ListAdapter::front() const
{
    assert(!children().empty());
    TreeIter iter = children().begin();
    assert(iter);

    return RowAdapter(iter, list_);
}
*/


Gtk::RowAdapter Gtk::ListAdapter::back() const
{
    assert(!children().empty());
    TreeIter iter = children().end();
    if (!children().empty())
    {
        --iter;
        assert(iter);
    }
    return RowAdapter(iter, list_);
}


bool Gtk::ListAdapter::empty() const
{
    return children().empty();
}


const Gtk::TreeModelColumn<string>&
Gtk::ListAdapter::operator[](size_t n) const
{
    return *list_.columns_[n];
}


Gtk::RowAdapter Gtk::ListAdapter::operator[](TreeIter iter)
{
    return RowAdapter(iter, list_);
}


////////////////////////////////////////////////////////////////
// RowAdapter
Gtk::RowAdapter::RowAdapter(TreeIter& iter, CList& list)
    : ListAdapter::Row(*iter)
    , list_(list)
{
}


Gtk::RowAdapter::RowAdapter(const TreePath& path, CList& list)
    : ListAdapter::Row(*list_.model_->get_iter(path))
    , list_(list)
{
}


void Gtk::RowAdapter::set_data(void* data)
{
    set_value(list_.user_data(), data);
}


void* Gtk::RowAdapter::get_data() const
{
    return get_value(list_.user_data());
}


void Gtk::RowAdapter::set_selectable(bool)
{
    // todo
}


void Gtk::RowAdapter::set_background(const Gdk::Color& color)
{
    // todo
}


void Gtk::RowAdapter::select()
{
    if (Glib::RefPtr<Gtk::TreeSelection> sel = list_.get_selection())
    {
        sel->select(*this);
        list_.signal_select_row().emit(*this, 0, 0);
    }
}


Gtk::CellAdapter Gtk::RowAdapter::operator[](size_t i)
{
    //return CellAdapter(*model_, *this, i);
    //fixme: causes an assertion to fail when the list is re-sorted
    return CellAdapter(*list_.model().operator->(), *this, i);
}


// FIXME: okay for CList, broken for CTree
int Gtk::get_row_num(const Gtk::TreePath& path)
{
    int nrow = 0;
    const size_t depth = path.get_depth();
    assert(path.get_indices().size() == depth);

    for (size_t i = 0; i != depth; ++i)
    {
        nrow += path[i];
    }
    // clog << __func__ << ": " << nrow << endl;

    return nrow;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
