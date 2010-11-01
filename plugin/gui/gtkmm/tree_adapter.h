#ifndef TREE_ADAPTER_H__9FD10072_EC25_4107_93F8_C26947073E30
#define TREE_ADAPTER_H__9FD10072_EC25_4107_93F8_C26947073E30
//
// $Id: tree_adapter.h 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelsort.h>
#include <gtkmm/treeview.h>
#include "column_adapter.h"
#include "flags.h"
#include "row_handle.h"
#include "signal.h"
#include "zdk/export.h"

/**
 * Adapt a TreeView into an old Gtk 1.2 list or tree
 */
template<typename T, typename U>
class ZDK_LOCAL TreeAdapter
    : public Gtk::TreeView
    , protected Gtk::TreeModel::ColumnRecord
{
protected:
    typedef SigC::Signal1<void, int> ColumnClickSignal;

    Glib::RefPtr<T> model_;
    Glib::RefPtr<Gtk::TreeModelSort> sortModel_;


    virtual ~TreeAdapter() { }


    TreeAdapter()
    {
        add_events(Gdk_FLAG(BUTTON_PRESS_MASK));
    }


    virtual Gtk::ColumnAdapter<U> column(unsigned int) = 0;


    /**
     * Add a model column to the view
     * @return the index of the newly added column.
     */
    template<typename V>
    int add_to_view(const std::string& title, Gtk::TreeModelColumn<V>& col)
    {
        Gtk::TreeModel::ColumnRecord::add(col);
        int ncol = this->append_column(title, col);
        assert(ncol);
        --ncol;
        if (Gtk::TreeViewColumn* viewCol = this->get_column(ncol))
        {
            viewCol->set_resizable(true);
            viewCol->set_clickable(true);
            viewCol->signal_clicked().connect(
                sigc::bind(
                    sigc::mem_fun(this, &TreeAdapter::on_click),
                    ncol));
            viewCol->signal_clicked().connect(
                    sigc::bind(columnClick_.slot(), ncol));
        }
        return ncol;
    }


    /**
     * Column-header click handler: sorts the column, and toggle
     * the sort order.
     */
    void on_click(int ncol)
    {
        if (sortModel_)
        {
            int sortCol = 0;
            Gtk::SortType sortOrder = Gtk::SORT_DESCENDING;
            if (sortModel_->get_sort_column_id(sortCol, sortOrder))
            {
                if (sortCol != ncol)
                {
                    sortOrder = Gtk::SORT_ASCENDING;
                }
                else
                {
                    if (sortOrder == Gtk::SORT_DESCENDING)
                    {
                        sortOrder = Gtk::SORT_ASCENDING;
                    }
                    else
                    {
                        sortOrder = Gtk::SORT_DESCENDING;
                    }
                }
            }
            set_sort_column(ncol, sortOrder);
        }
    }

    ColumnClickSignal columnClick_;

    Glib::RefPtr<T> model() { return model_; }

public:
    ColumnClickSignal& signal_click_column() { return columnClick_; }

    Gtk::TreeIter get_iter(const Gtk::TreePath& path) const
    {
        if (sortModel_)
        {
            return model_->get_iter(sortModel_->convert_path_to_child_path(path));
        }
        else
        {
            return model_->get_iter(path);
        }
    }

    /**
     * @note non-const in order to work with older, non-const
     * TreeView::get_path_at_pos
     */
    bool get_selection_info(int x, int y, Gtk::RowHandle* row, int* col)
    {
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* pCol = 0;
        Glib::RefPtr<Gtk::TreeModel> model = model_;
        if (sortModel_)
        {
            model = sortModel_;
        }
        if (row)
        {
            *row = model->children().end();
        }
        if (!get_path_at_pos(x, y, path, pCol, x, y))
        {
            return false;
        }
        if (col)
        {
            const size_t ncol = get_columns().size();
            for (size_t i = 0; i != ncol; ++i)
            {
                if (get_column(i) == pCol)
                {
                    *col = i;
                    break;
                }
            }
        }
        if (Gtk::TreeIter iter = model->get_iter(path))
        {
            if (row)
            {
                *row = iter;
            }
        }
        else
        {
    #ifdef DEBUG
            std::clog << __func__ << " get_iter(): invalid iterator\n";
    #endif
            return false;
        }
        return true;
    }


    void set_sort_column(int col, Gtk::SortType type)
    {
        if (sortModel_)
        {
            sortModel_->set_sort_column(col, type);
            column(col)->set_sort_indicator(true);
            column(col)->set_sort_order(type);
        }
    }


    void set_sort_func(int col, Gtk::TreeSortable::SlotCompare& func)
    {
        if (sortModel_)
        {
            sortModel_->set_sort_func(col, func);
        }
        else
        {
            model_->set_sort_func(col, func);
        }
    }


    void set_selection_mode(Gtk::SelectionMode mode)
    {
        if (Glib::RefPtr<Gtk::TreeSelection> sel = get_selection())
        {
            sel->set_mode(mode);
        }
    }
};

#endif // TREE_ADAPTER_H__9FD10072_EC25_4107_93F8_C26947073E30
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
