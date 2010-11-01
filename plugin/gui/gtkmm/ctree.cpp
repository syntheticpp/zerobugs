//
// $Id: ctree.cpp 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include <stdexcept>
#include <gtk/gtksignal.h>
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "ctree.h"
#include "flags.h"


using namespace std;


Gtk::TreeElement::TreeElement(const vector<string>& columns)
    : columns_(columns)
{
}


void Gtk::TreeElement::init(const char* col[], size_t n)
{
    assert(col);
    for (unsigned int i = 0; i != n; ++i)
    {
        assert(col[i]);
        columns_.push_back(col[i]);
    }
    assert(columns_.size() == n);
}


Gtk::CTree::CTree(const Glib::SArray& columnTitles)
    : expanding_(false)
    , selection_(this)
{
    // foreground color is in the 4th hidden column
    const int fgColNum = columnTitles.size() + 3;

    Glib::SArray::const_iterator i = columnTitles.begin();
    for (; i != columnTitles.end(); ++i)
    {
        boost::shared_ptr<ModelColumn> col(new ModelColumn());
        modelColumns_.push_back(col);

        const int n = add_to_view(*i, *col);
        if (Gtk::TreeViewColumn* viewCol = get_column(n))
        {
            if (CellRenderer* r = get_column_cell_renderer(n))
            {
                viewCol->add_attribute(*r, "foreground", fgColNum);
            }
            viewCol->set_resizable(true);
            viewCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
        }
    }
    Gtk::TreeModel::ColumnRecord::add(userData_);
    Gtk::TreeModel::ColumnRecord::add(expandPending_);
    Gtk::TreeModel::ColumnRecord::add(selectable_);
    Gtk::TreeModel::ColumnRecord::add(foreground_);

    model_ = Gtk::TreeStore::create(*this);
    set_model(model_);
    //sortModel_ = TreeModelSort::create(model_);
    //set_model(sortModel_);
    //set_fixed_height_mode(true);
}


Gtk::CTree::ModelColumn& Gtk::CTree::model_column(unsigned int n)
{
    return *modelColumns_[n];
}


void Gtk::CTree::clear()
{
    assert(model_);
    if (model_)
    {
        model_->clear();
        assert(model_->children().empty());
    }
}


void Gtk::CTree::on_row_expanded(const TreeModel::iterator& i,
                                 const TreeModel::Path&)
{
    if (!expanding_)
    {
        signal_tree_expand_.emit(TreeRowAdapter(*this, *i));
    }
}


void Gtk::CTree::on_row_collapsed(const TreeModel::iterator& i,
                                  const TreeModel::Path&)
{
    //Gtk::TreeModel::Path path = model()->get_path(i);
    //collapse_row(path);
    signal_tree_collapse_(TreeRowAdapter(*this, *i));
}


void Gtk::CTree::on_cursor_changed()
{
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column = NULL;

    get_cursor(path, column);
    if (Gtk::TreeIter iter = model_->get_iter(path))
    {
        int ncol = 0; // todo: column index
        signal_selection_changed_(TreeRowAdapter(*this, *iter), ncol);
    }
}



////////////////////////////////////////////////////////////////
Gtk::TreeRowListAdapter Gtk::CTree::rows()
{
    assert(model_);
    return Gtk::TreeRowListAdapter(*this, 0);
}


/**
 * @todo make it work with sortModel_
 */
Gtk::TreeRowAdapter Gtk::CTree::row(int nrow)
{
    assert(model_);
    TreeIter i = model_->children().begin();
    for (int j = 0; i != model_->children().end(); ++i, ++j)
    {
        if (j == nrow)
        {
            break;
        }
    }
    return Gtk::TreeRowAdapter(*this, i);
}


/**
 *  @todo make it work with sortModel_
 */
Gtk::TreeRowAdapter Gtk::CTree::row(const Gtk::TreePath& path)
{
    Gtk::TreeIter iter = model()->get_iter(path);
    assert(iter);
    return Gtk::TreeRowAdapter(*this, iter);
}


Gtk::ColumnAdapter<Gtk::CTree> Gtk::CTree::column(unsigned int n)
{
    return ColumnAdapter<CTree>(*this, n);
}


// TreeRowListAdapter
Gtk::TreeRowListAdapter::TreeRowListAdapter(CTree& tree, const TreeRow* row)
  : tree_(&tree)
  , row_(row)
  , children_(tree.model()->children())
{
    if (row)
    {
        assert (*row);
    }
}


/*
Gtk::TreeRowAdapter Gtk::TreeRowListAdapter::operator[](unsigned int n)
{
    assert(tree_);
    if (row_)
    {
        assert(*row_);
        return TreeRowAdapter(*tree_, row_->children()[n]);
    }
    return TreeRowAdapter(*tree_, children_[n]);
}
*/


void Gtk::TreeRowListAdapter::push_back(const TreeElement& elem)
{
    push_back_(elem);
}


void Gtk::TreeRowListAdapter::push_back(const vector<string>& elem)
{
    push_back_(elem);
}


template<typename U>
void Gtk::TreeRowListAdapter::push_back_(const U& elem)
{
    assert(elem.size());
    assert(tree_->model());

    // there are 4 hidden columns
    assert((int)elem.size() + 4 == tree_->model()->get_n_columns());

    TreeModel::iterator iter;
    if (row_)
    {
        assert(*row_);
        iter = tree_->model()->append(row_->children());
    }
    else
    {
        iter = tree_->model()->append();
        children_ = tree_->model()->children();
    }
    if (!iter)
    {
        throw runtime_error(__func__);
    }
    TreeRow row = *iter;

    // make it selectable by default
    row.set_value(tree_->selectable(), true);

    if (row_ && row_->get_value(tree_->expand_pending()))
    {
        Temporary<bool> temp(tree_->expanding_, true);
        if (tree_->expand_row(tree_->model()->get_path(*row_), false))
        {
            row_->set_value(tree_->expand_pending(), false);
        }
    }

    for (size_t i = 0; i != elem.size(); ++i)
    {
        row.set_value(i, elem[i]);
    }

    assert((*iter).get_value(tree_->user_data()) == 0);
}


// TreeRowAdapter
Gtk::TreeRowAdapter::TreeRowAdapter()
    : tree_(0)
    , iter_()
{
}


Gtk::TreeRowAdapter::TreeRowAdapter
(
    CTree& tree, TreeNodeChildren::iterator iter
)
 : tree_(&tree)
 , iter_(iter)
{
    if (!iter)
    {
        throw runtime_error("invalid iterator in TreeRowAdapter");
    }
}


Gtk::TreeRowAdapter::TreeRowAdapter(const TreeRowAdapter& other)
 : tree_(other.tree_)
 , iter_(other.iter_)
{
    assert(iter_);
    assert(*iter_);
}


void Gtk::TreeRowAdapter::set_foreground(const Gdk_Color& color)
{
    assert(iter_);
    (*iter_).set_value(tree_->foreground(), color.name());
}


void Gtk::TreeRowAdapter::expand()
{
    if (iter_)
    {
        Gtk::TreeModel::Path path = tree_->get_path(iter_);

        //guard against emitting the "expanded" signal, since
        //the node expansion is programmatical
        Temporary<bool> temp(tree_->expanding_, true);

        if (!tree_->expand_row(path, false))
        {
            iter_->set_value(tree_->expand_pending(), true);
            assert(iter_->get_value(tree_->expand_pending()));
        }
    }
}


Gtk::TreePath Gtk::TreeRowAdapter::get_path() const
{
    return tree_->get_path(iter_);
}


int Gtk::get_row_num(const Gtk::TreeRowAdapter& row)
{
    return get_row_num(row.get_path());
}


void Gtk::TreeRowAdapter::swap(TreeRowAdapter& other) throw()
{
    std::swap(tree_, other.tree_);
    std::swap(iter_, other.iter_);

    assert(iter_);
    assert(*iter_);
}


Gtk::TreeRowAdapter&
Gtk::TreeRowAdapter::operator=(const TreeRowAdapter& other)
{
    TreeRowAdapter temp(other);
    swap(temp);
    assert(iter_);
    assert(*iter_);

    return *this;
}


void* Gtk::TreeRowAdapter::get_data() const
{
    return iter_ ? (*iter_).get_value(tree_->user_data()).get() : NULL;
}


static void nodelete(void*) { }

void Gtk::TreeRowAdapter::set_data(void* data)
{
    assert(iter_);
    if (iter_)
    {
        (*iter_).set_value(tree_->user_data(), boost::shared_ptr<void>(data, nodelete));
    }
}


Gtk::TreeRowListAdapter Gtk::TreeRowAdapter::subtree() const
{
    assert(iter_);
    assert(*iter_);
    return Gtk::TreeRowListAdapter(*CHKPTR(tree_), const_cast<TreeRow*>(&*iter_));
}


Gtk::TreeCellAdapter Gtk::TreeRowAdapter::operator[](unsigned int n) const
{
    assert(tree_);
    if (iter_)
    {
        return Gtk::TreeCellAdapter(*iter_, tree_->model_column(n));
    }
    return Gtk::TreeCellAdapter();
}


bool Gtk::TreeRowAdapter::get_selectable() const
{
    assert(iter_);

    return iter_ ? (*iter_).get_value(tree_->selectable()) : false;
}


void Gtk::TreeRowAdapter::set_selectable(bool selectable)
{
    assert(iter_);
    if (iter_)
    {
        (*iter_).set_value(tree_->selectable(), selectable);
    }
}


void Gtk::TreeRowAdapter::select()
{
    if (get_selectable())
    {
        assert(iter_);
        tree_->selection().select(iter_);
        tree_->signal_tree_select_row().emit(*this, 0);
    }
}


bool
Gtk::operator!=(const Gtk::TreeRowAdapter& lhs, const Gtk::TreeRowAdapter& rhs)
{
    return !lhs.equal(rhs);
}


////////////////////////////////////////////////////////////////
// TreeRowListAdapter
Gtk::TreeRowListAdapter::TreeRowListAdapter(
    const TreeRowListAdapter& other)
    : tree_(other.tree_)
    , row_(other.row_)
    , children_(other.children_)
{
}


Gtk::TreeRowListAdapter&
Gtk::TreeRowListAdapter::operator=(const TreeRowListAdapter& other)
{
    TreeRowListAdapter tmp(other);
    this->swap(tmp);
    children_ = tmp.children_; // hack :(
    return *this;
}


void Gtk::TreeRowListAdapter::swap(TreeRowListAdapter& other) throw()
{
    std::swap(tree_, other.tree_);
    std::swap(row_, other.row_);
    // children_.swap(other.children_);
}


Gtk::TreeRowAdapter Gtk::TreeRowListAdapter::front() const
{
    assert(tree_);
    if (row_)
    {
        assert(*row_);
    }
    const TreeNodeChildren& children = row_ ? row_->children() : children_;
    assert(!children.empty());

    TreeNodeChildren::iterator i = children.begin();
    assert(i);
    return Gtk::TreeRowAdapter(*tree_, *i);
}


Gtk::TreeRowAdapter Gtk::TreeRowListAdapter::back() const
{
    assert(tree_);
    if (row_)
    {
        assert(*row_);
    }
    const TreeNodeChildren& children = row_ ? row_->children() : children_;
    assert(!children.empty());

    TreeNodeChildren::iterator i = children.end();
    --i;
    assert(i);
    return Gtk::TreeRowAdapter(*tree_, *i);
}


Gtk::TreeRowListAdapter::iterator
Gtk::TreeRowListAdapter::erase(iterator iter)
{
    assert(tree_);
    assert(iter);
    return tree_->model()->erase(iter);
}


Gtk::TreeRowListAdapter::iterator
Gtk::TreeRowListAdapter::begin()
{
    if (row_)
    {
        return row_->children().begin();
    }
    assert(tree_);
    children_ = tree_->model()->children();
    return children_.begin();
}


Gtk::TreeRowListAdapter::iterator Gtk::TreeRowListAdapter::end()
{
    if (row_)
    {
        return row_->children().end();
    }
    return children_.end();
}


Gtk::TreeRowListAdapter::const_iterator
Gtk::TreeRowListAdapter::begin() const
{
    if (row_)
    {
        return row_->children().begin();
    }
    assert(tree_);
    const_cast<TreeNodeChildren&>(children_) = tree_->model()->children();
    return children_.begin();
}


Gtk::TreeRowListAdapter::const_iterator
Gtk::TreeRowListAdapter::end() const
{
    if (row_)
    {
        return row_->children().end();
    }
    return children_.end();
}


bool Gtk::TreeRowListAdapter::empty() const
{
    return row_ ? row_->children().empty() : children_.empty();
}


size_t Gtk::TreeRowListAdapter::size() const
{
    return row_ ? row_->children().size() : children_.size();
}


////////////////////////////////////////////////////////////////
// TreeCellAdapter
Gtk::TreeCellAdapter::TreeCellAdapter(
    const TreeRow& row, const TreeModelColumn<string>& col)
 : proxy_(new proxy_type(row, col))
{
}


string Gtk::TreeCellAdapter::get_text() const
{
    return proxy_ ? *proxy_ : string();
}


void Gtk::TreeCellAdapter::set_text(const string& text)
{
    if (proxy_)
    {
        *proxy_ = text;
    }
}


////////////////////////////////////////////////////////////////
// TreeSelectionAdapter
Gtk::TreeSelectionAdapter::TreeSelectionAdapter(CTree* tree) : tree_(tree)
{
}


bool Gtk::TreeSelectionAdapter::empty() const
{
    if (!sel_ && tree_)
    {
        sel_ = tree_->get_selection();
    }
    if (!sel_)
    {
        return true;
    }
    return (sel_->count_selected_rows() == 0);
}


Gtk::TreeRowAdapter Gtk::TreeSelectionAdapter::get_selected_row()
{
    if (empty())
    {
        throw out_of_range(__func__);
    }
    assert(tree_);
    assert(sel_);

    assert(sel_->get_mode() == Gtk::SELECTION_SINGLE);
    TreeModel::iterator iter = sel_->get_selected();
    return TreeRowAdapter(*tree_, iter);
}


Gtk::TreeSelectionAdapter::const_iterator
Gtk::TreeSelectionAdapter::begin() const
{
    if (!sel_ && tree_)
    {
        sel_ = tree_->get_selection();
    }
    assert(sel_);
    TreeSelection::ListHandle_Path path = sel_->get_selected_rows();
    return path.begin();
}


void Gtk::TreeSelectionAdapter::select(const Gtk::TreeModel::iterator& iter)
{
    if (!sel_ && tree_)
    {
        sel_ = tree_->get_selection();
    }
    if (sel_)
    {
        sel_->select(iter);
    }
}


void Gtk::TreeSelectionAdapter::select(
    const Gtk::TreeModel::iterator& first,
    const Gtk::TreeModel::iterator& last)
{
    if (!sel_ && tree_)
    {
        sel_ = tree_->get_selection();
    }
    if (sel_)
    {
        Glib::RefPtr<TreeModel> model = sel_->get_model();
        sel_->select(model->get_path(first), model->get_path(last));
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
