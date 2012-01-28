#ifndef CLIST_H__C1D9B5A5_0503_4C7D_9072_7B2D56C5172A
#define CLIST_H__C1D9B5A5_0503_4C7D_9072_7B2D56C5172A
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

#include "row_handle.h"
#ifndef GTKMM_2
 #include <gtk--/clist.h>
 namespace Gtk
 {
    template<typename I, typename L>
    inline CList::Row get_list_row(I i, const L&)
    {
        return CList::Row(*i);
    }

    inline CList::Row get_row(CList&, CList::Row& row)
    {
        return row;
    }
 }
#else
 #include <boost/shared_ptr.hpp>
 #include <gtkmm/liststore.h>
 #include <gtkmm/treeview.h>
 #include "column_adapter.h"
 #include "tree_adapter.h"

 namespace Gtk
 {
    class CList;
    class RowAdapter;

    /**
     * Adapt the Gtk::ListStore to mimic gtk-- 1.2 RowList
     * @note another possible approach is to collapse this
     * class into CList, since in the old gtkmm-1.2 world,
     * the widget was both the model and the view.
     */
    class ZDK_LOCAL ListAdapter : public Gtk::ListStore
    {
    public:
        explicit ListAdapter(CList& list);

        RowAdapter front() const;

        RowAdapter back() const;

        void push_back(const std::vector<std::string>&);

        operator Children() const { return children(); }

        const Gtk::TreeModelColumn<std::string>& operator[](size_t) const;

        RowAdapter operator[](TreeIter iter);

        void remove_row(size_t r);

        bool empty() const;

    private:
        CList& list_;
    };

/******************************************************************************/
    class ZDK_LOCAL ListSelectionAdapter
    {
    public:
        typedef std::vector<TreePath>::const_iterator const_iterator;

        explicit ListSelectionAdapter(CList& list);

        size_t size() const;

        bool empty() const { return size() == 0; }

        const_iterator begin() const;
        const_iterator end() const;

    private:
        CList& list_;
        mutable Glib::RefPtr<const Gtk::TreeView::Selection> sel_;
        mutable std::vector<TreePath> cont_;
    };

/******************************************************************************/
    class ZDK_LOCAL CellAdapter
    {
        TreeValueProxy<std::string> proxy_;

        static const TreeModelColumn<std::string>&
            get_column(TreeModel& model, size_t n)
        {
            return dynamic_cast<ListAdapter&>(model)[n];
        }

    public:
        CellAdapter(TreeModel& model, TreeRow& row, size_t n);

        std::string get_text() const { return proxy_; }

        void set_text(const std::string& text) { proxy_ = text; }
    };

/******************************************************************************/
    class ZDK_LOCAL RowAdapter : public Gtk::TreeRow
    {
    public:
        RowAdapter(const Gtk::TreePath&, CList&);

        RowAdapter(Gtk::TreeIter& row, CList&);

        CellAdapter operator[](size_t i);

        void set_data(void*);

        void* get_data() const;

        void set_background(const Gdk::Color&);

        void set_foreground(const Gdk::Color&);

        //void expand();

        //bool get_selectable() const;

        void set_selectable(bool);

        void select();

    private:
        Gtk::CList& list_;
    };


    /**
     * Adapt the gtkmm-2.x TreeView into a gtkmm-1.2 CList
     */
    class ZDK_LOCAL CList : public TreeAdapter<ListAdapter, CList>
    {
    public:
        typedef ListAdapter::Children RowList;
        typedef RowAdapter Row;
        typedef CellAdapter Cell;
        typedef ListSelectionAdapter SelectionList;
        typedef SigC::Signal3<
            void, RowHandle, int, GdkEvent*> SelectionChangedSignal;

        friend class ListAdapter;
        friend class RowAdapter;

        explicit CList(const Glib::SArray& titles);

        ListAdapter& rows();

        Row row(RowHandle handle)
        {
            return Row(handle, *this);
        }

        SelectionList selection();

        void clear() { rows().clear(); }

        void remove_row(size_t index);

        ColumnAdapter<CList> column(unsigned int n)
        {
            return ColumnAdapter<CList>(*this, n);
        }

        TreeModelColumn<std::string>& model_column(unsigned int n)
        {
            return *columns_[n];
        }

        TreeModelColumn<void*> user_data() { return userData_; }

        size_t size() const { return model_->children().size(); }
        bool empty() const { return model_->children().empty(); }

        SelectionChangedSignal& signal_select_row()
        {
            return selectRow_;
        }

        Glib::RefPtr<ListStore> model() const { return model_; }

        TreeNodeChildren::iterator get_row(TreeNodeChildren::iterator) const;

    private:
        void on_cursor_changed();

        // in the old CList, all columns store strings
        typedef TreeModelColumn<std::string> Column;

        std::vector<boost::shared_ptr<Column> > columns_;

        // hidden column, for per-row user data
        TreeModelColumn<void*> userData_;

        SelectionChangedSignal selectRow_;
    };


    inline void ZDK_LOCAL ListAdapter::remove_row(size_t r)
    {
        list_.remove_row(r);
    }


    template<typename I, typename L>
    ZDK_LOCAL RowAdapter get_list_row(I i, L& list)
    {
        return RowAdapter(i, list);
    }


    inline CellAdapter::CellAdapter(TreeModel& model, TreeRow& row, size_t n)
        : proxy_(row[get_column(model, n)])
    { }


    inline RowAdapter ZDK_LOCAL
    get_row(CList& list, TreeNodeChildren::iterator iter)
    {
        assert(iter);
        iter = list.get_row(iter);
        return RowAdapter(iter, list);
    }
 }
#endif // GTKMM_2
#endif // CLIST_H__C1D9B5A5_0503_4C7D_9072_7B2D56C5172A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
