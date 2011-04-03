#ifndef CTREE_H__8FF213C1_C868_472D_BB63_62D8B6C724D4
#define CTREE_H__8FF213C1_C868_472D_BB63_62D8B6C724D4
//
// $Id: ctree.h 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include "base.h"
#include "row_handle.h"

#if !defined (GTKMM_2)
 #include <gtk--/ctree.h>

/*** Gtkmm-1.2 ***/
 namespace Gtk
 {
     void inline clear_rows(CTree& tree)
     {
        static_cast<CList&>(tree).rows().clear();
     }

     inline CTree::Row
     get_row(const CTree::RowList&, const CTree::Row& row)
     {
        return row;
     }

    inline CTree::Row
    get_selected_row(CTree_Helpers::SelectionList& sel)
    {
        assert(!sel.empty());
        return *sel.begin();
    }
 }
#else
/*** Gtkmm-2.4 ***/
 #include <string>
 #include <vector>
 #include <boost/shared_ptr.hpp>
 #include <gtkmm/treestore.h>
 #include <gtkmm/treeview.h>
 #include "color.h"
 #include "column_adapter.h"
 #include "tree_adapter.h"

/* Fake the old gtkmm-1.2 CTree class */
 namespace Gtk
 {
    enum LineStyle
    {
        CTREE_LINES_NONE =      Gtk::TREE_VIEW_GRID_LINES_NONE,
        CTREE_LINES_DOTTED =    Gtk::TREE_VIEW_GRID_LINES_VERTICAL,
        CTREE_LINES_SOLID =     Gtk::TREE_VIEW_GRID_LINES_BOTH
    };


    static inline bool
    row_handle_equal(const RowHandle& lhs, const RowHandle& rhs)
    {
        if (!lhs)
        {
            return !rhs;
        }
        else if (!rhs)
        {
            return false;
        }

        return lhs.get_stamp() == rhs.get_stamp() && lhs.equal(rhs);
    }

/**************************************************************/
    class ZDK_LOCAL TreeElement
    {
    public:
        explicit TreeElement(const std::vector<std::string>&);

        template<size_t n>
        explicit TreeElement(const char* (&col) [n])
        {
            init(col, n);
        }

        size_t size() const { return columns_.size(); }

        const std::string& operator[](size_t n) const
        {
            return columns_[n];
        }

    private:
        void init(const char* [], size_t);

        std::vector<std::string> columns_;
    };


    class CTree;
    class TreeCellAdapter;
    class TreeRowListAdapter;


/**************************************************************/
    class ZDK_LOCAL TreeRowAdapter
    {
    public:
        typedef TreeCellAdapter Cell;

        TreeRowAdapter();

        TreeRowAdapter(CTree&, TreeNodeChildren::iterator);

        TreeRowAdapter(const TreeRowAdapter&);

        TreeRowAdapter& operator=(const TreeRowAdapter&);

        TreeRowListAdapter subtree() const;

        void set_foreground(const Gdk_Color&);

        // not implemented
        // void set_background(const Gdk_Color&);
        // void set_foreground(const std::string&);

        void expand();

        bool get_selectable() const;

        void set_selectable(bool);

        void select();

        Cell operator[](unsigned int) const;

        void set_data(void*);

        void* get_data() const;

        void swap(TreeRowAdapter&) throw();

        bool equal(const TreeRowAdapter& other) const
        {
            if (iter_)
            {
                return iter_->equal(other.iter_);
            }
            return !other.iter_;
        }

        TreePath get_path() const;

        TreeNodeChildren::iterator iter() { return iter_; }

    private:
        CTree* tree_;
        TreeNodeChildren::iterator iter_;
    };


/**************************************************************/
    class ZDK_LOCAL TreeCellAdapter
    {
    public:
        std::string get_text() const;

        void set_text(const std::string& text);

        TreeCellAdapter() { }

        TreeCellAdapter(const TreeRow&, const TreeModelColumn<std::string>&);

    private:
        typedef TreeValueProxy<std::string> proxy_type;

        boost::shared_ptr<proxy_type> proxy_;
    };


/**************************************************************/
    class ZDK_LOCAL TreeRowListAdapter
    {
    public:
        typedef TreeRowAdapter Row;
        typedef TreeNodeChildren::iterator iterator;
        typedef TreeNodeChildren::const_iterator const_iterator;

        TreeRowListAdapter(CTree&, const TreeRow*);
        TreeRowListAdapter(const TreeRowListAdapter&);
        TreeRowListAdapter& operator=(const TreeRowListAdapter&);

        Row front() const;
        Row back() const;

        // const Row& operator[](unsigned int) const;
        // Row operator[](unsigned int);

        // do not worry about the semantics here not making much
        // sense -- this is just to support legacy Gtk-- 1.2 code
        Row operator[](RowHandle handle)
        {
            assert(tree_);
            return Row(*tree_, handle);
        }
        void push_back(const std::vector<std::string>& elem);
        void push_back(const Gtk::TreeElement& elem);

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

        iterator erase(iterator);
        iterator erase(Row& row) { return erase(row.iter()); }

        CTree& get_ctree() const { assert(tree_); return *tree_; }

        bool empty() const;

        size_t size() const;

    private:
        void swap(TreeRowListAdapter&) throw();

        template<typename U> void push_back_(const U& elem);

    private:
        CTree*              tree_;
        const TreeRow*      row_;
        TreeNodeChildren    children_;
    };


/**************************************************************/
    class ZDK_LOCAL TreeSelectionAdapter
    {
    public:
        typedef TreeSelection::ListHandle_Path::iterator iterator;
        typedef TreeSelection::ListHandle_Path::const_iterator
            const_iterator;

        explicit TreeSelectionAdapter(CTree* tree);

        const_iterator begin() const;
        const_iterator end() const;
        // iterator begin();
        // iterator end();

        bool empty() const;

        TreeRowAdapter get_selected_row();

        void select(const TreeModel::iterator& iter);

        void select(const TreeModel::iterator& first,
                    const TreeModel::iterator& last);

    private:
        CTree* tree_;

        mutable Glib::RefPtr<Gtk::TreeSelection> sel_;
    };


    /**********************************************************
     * Simulate the old gtkmm-1.2 CTree widget with a gtkmm-2.4
     * TreeView
     * @note
     * SORTING DOES NOT WORK YET Do not attempt to instantiate
     * the sort model
     */
    class ZDK_LOCAL CTree : public TreeAdapter<TreeStore, CTree>
    {
        typedef TreeAdapter<TreeStore, CTree> Base;
        friend class TreeRowListAdapter;

    public:
        typedef TreeCellAdapter Cell;
        typedef TreeRowAdapter Row;
        typedef TreeRowListAdapter RowList;
        typedef TreeElement Element;

        typedef ColumnAdapter<CTree> Column;

        typedef TreeSelectionAdapter SelectionList;

        typedef SigC::Signal2<void, Row, int> SignalSelectionChanged;

        // in the old CTree, all columns store strings
        typedef TreeModelColumn<std::string> ModelColumn;

        using TreeAdapter<TreeStore, CTree>::model;

        explicit CTree(const Glib::SArray& viewColumnTitles);

        SigC::Signal1<void, Row> signal_tree_expand()
        { return signal_tree_expand_; }

        SigC::Signal1<void, Row> signal_tree_collapse()
        { return signal_tree_collapse_; }

        SignalSelectionChanged signal_tree_select_row()
        { return signal_selection_changed_; }

        TreeSelectionAdapter& selection() { return selection_; }

        void clear();

        RowList rows();

        Row row(RowHandle hrow) { return TreeRowAdapter(*this, hrow); }
        Row row(int nrow);

        Row row(const Gtk::TreePath& path);

        Column column(unsigned int);

        template<typename Iter>
        Gtk::TreePath get_path(const Iter& iter)
        {
            return model()->get_path(iter);
        }

        TreeModelColumn<std::string>& model_column(unsigned int);

        const TreeModelColumn<bool>& expand_pending()
        { return expandPending_; }

        const TreeModelColumn<boost::shared_ptr<void> >& user_data()
        { return userData_; }

        const TreeModelColumn<bool>& selectable()
        { return selectable_; }

        const TreeModelColumn<std::string>& foreground()
        { return foreground_; }

        bool expanding_;

        void on_cursor_changed();

        void set_line_style(LineStyle style)
        { 
            set_grid_lines(static_cast<TreeViewGridLines>(style));
        }

        TreeModel::Children children()
        {
            // todo: worry about the sortModel_ here?
            return model()->children();
        }

    private:
        void on_row_expanded(const TreeModel::iterator&,
                             const TreeModel::Path&);

        void on_row_collapsed(const TreeModel::iterator&,
                              const TreeModel::Path&);

        std::vector<boost::shared_ptr<ModelColumn> > modelColumns_;

        // hidden column, for per-row user data
        TreeModelColumn<boost::shared_ptr<void> > userData_;
        TreeModelColumn<bool> expandPending_;
        TreeModelColumn<bool> selectable_;
        TreeModelColumn<std::string> foreground_;

        SigC::Signal1<void, Row> signal_tree_expand_;
        SigC::Signal1<void, Row> signal_tree_collapse_;
        SignalSelectionChanged signal_selection_changed_;

        TreeSelectionAdapter selection_;
    };


/**************************************************************/
    namespace CTree_Helpers
    {
        typedef CTree::SelectionList SelectionList;
        typedef CTree::RowList RowList;
    };

    void inline ZDK_LOCAL clear_rows(CTree& tree) { tree.clear(); }

    bool ZDK_LOCAL operator!=(const TreeRowAdapter&, const TreeRowAdapter&);

    inline TreeRowAdapter ZDK_LOCAL
    get_row(const TreeRowListAdapter& rows, TreeNodeChildren::iterator iter)
    {
        assert(iter);
        return TreeRowAdapter(rows.get_ctree(), iter);
    }

    inline TreeRowAdapter ZDK_LOCAL
    get_selected_row(CTree_Helpers::SelectionList& sel)
    {
        return sel.get_selected_row();
    }
}
#endif
#endif // CTREE_H__8FF213C1_C868_472D_BB63_62D8B6C724D4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
