#ifndef COLUMN_ADAPTER_H__3CCD3AEA_A5F4_449A_9D94_1C93D766AF65
#define COLUMN_ADAPTER_H__3CCD3AEA_A5F4_449A_9D94_1C93D766AF65
//
// $Id: column_adapter.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <gtkmm/treeview.h>
#include "zdk/export.h"

namespace Gtk
{
    template<typename T>
    class ZDK_LOCAL ColumnAdapter
    {
    private:
        T* view_;
        unsigned int n_;

    public:
        ColumnAdapter(T& view, unsigned int n) : view_(&view), n_(n)
        {
            if (Gtk::TreeViewColumn* col = view.get_column(n_))
            {
                col->set_clickable(true);
                col->set_expand(false);
            }
        }
        ColumnAdapter(const ColumnAdapter& other)
            : view_(other.view_)
            , n_(other.n_)
        {
        }
        ColumnAdapter& operator=(const ColumnAdapter& other)
        {
            ColumnAdapter tmp(other);
            return this->swap(tmp);
        }
        void set_width(unsigned int w)
        {
            assert(view_);
            if (Gtk::TreeViewColumn* col = view_->get_column(n_))
            {
                col->set_min_width(w);
            }
        }
        void set_fixed_width(unsigned int w)
        {
            assert(view_);
            if (Gtk::TreeViewColumn* col = view_->get_column(n_))
            {
                col->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
                col->set_expand(false);
                col->set_fixed_width(w);
            }
        }
        unsigned int get_width() const
        {
            if (Gtk::TreeViewColumn* col = view_->get_column(n_))
            {
                return col->get_width();
            }
            return 0;
        }
        void set_passive()
        {
            assert(view_);
            if (Gtk::TreeViewColumn* col = view_->get_column(n_))
            {
                col->set_clickable(false);
            }
        }

        void set_sizing(Gtk::TreeViewColumnSizing type)
        {
            if (Gtk::TreeViewColumn* col = view_->get_column(n_))
            {
                col->set_sizing(type);
            }
        }

        ColumnAdapter& swap(ColumnAdapter& other) throw()
        {
            std::swap(view_, other.view_);
            std::swap(n_, other.n_);
            return *this;
        }
        Gtk::TreeViewColumn* operator->() const
        {
            assert(view_);
            return view_->get_column(n_);
        }
    };
}
#endif // COLUMN_ADAPTER_H__3CCD3AEA_A5F4_449A_9D94_1C93D766AF65
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
