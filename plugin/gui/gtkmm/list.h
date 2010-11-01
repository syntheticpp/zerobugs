#ifndef LIST_H__4E6C4596_2EB1_4E8B_83A0_6E23D8472B4D
#define LIST_H__4E6C4596_2EB1_4E8B_83A0_6E23D8472B4D
//
// $Id: list.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "listitem.h"

#ifndef GTKMM_2

// Gtk-- 1.2
 #include "selectionitem.h"
 #include <vector>
 #include <gtk--/list.h>

 inline std::vector<Gtk::SelectionItem>
 Gtk_get_selection_items(const Gtk::List::SelectionList& list)
 {
     std::vector<Gtk::SelectionItem> result(list.begin(), list.end());
     return result;
 }
/* todo
 inline void Gtk_select_all(const Gtk::List& list)
 {
 }

 inline void Gtk_unselect_all(const Gtk::List& list)
 {
 }
*/
#else

////////////////////////////////////////////////////////////////
 #include <memory>
 #include <vector>
 #include <boost/shared_ptr.hpp>
 #include <gtkmm/liststore.h>
 #include <gtkmm/treeview.h>
 #include "selectionitem.h"

/**
 * Fake the old Gtk-- 1.2 List widget with a TreeView
 */
 namespace Gtk
 {
    class List;
    class ItemList;


    class SelectionList
    {
    public:
        typedef std::vector<SelectionItem>::iterator iterator;
        typedef std::vector<SelectionItem>::const_iterator const_iterator;

        explicit SelectionList(List&);

        bool empty() const { return size() == 0; }

        size_t size() const;

        TreeNodeChildren::iterator get_selected();

        const std::vector<SelectionItem>& items() const
        { return items_impl(); }

        iterator begin() { return items_impl().begin(); }

        iterator end() { return items_impl().end(); }

        //const_iterator begin() const;
        //const_iterator end() const;

    private:
        std::vector<SelectionItem>& items_impl() const;

        List& list_;
        mutable std::vector<SelectionItem> items_;
    };



    /**
     * Mimic the Gtk-1.2 List Widget -- in the old paradigm, the
     * list was both the view and the model, so it makes sense to
     * aggregate them all together.
     */
    class List : public TreeView
               , private Gtk::TreeModel::ColumnRecord
    {
        friend class ItemList;

        void add(Gtk::Widget&) {}

    public:
        typedef Gtk::SelectionList SelectionList;
        typedef Gtk::ItemList ItemList;
        // adapted items
        typedef std::vector<boost::shared_ptr<Gtk::ListItem> > Items;

        List();

        SigC::Signal0<void> signal_selection_changed()
        { return selection_changed_; }

        const SelectionList& selection() const;

        SelectionList& selection();

        void set_selection_mode(Gtk::SelectionMode mode);

        /**
         * @note purposely masks
         * virtual void add(Gtk::Widget&)
         */
        void add(Gtk::ListItem&);

        ItemList& items() { return get_item_list(); }

        const ItemList& items() const { return get_item_list(); }

        // void clear_items(int, int);

        void select_item(int);

        void deselect_item(int);

        void select(const Gtk::TreeRow&);

        void deselect(const Gtk::TreeRow&);

        void select_all();
        void unselect_all();

        void on_selection_changed();

        void remove_items(SelectionList::iterator,
                          SelectionList::iterator);

        // -- used by SelectionList
        const TreeModelColumn<Gtk::ListItem*>& user_data_col()
        { return userData_; }

        Gtk::TreeModel::const_iterator get_iter(const Gtk::TreePath&) const;

        // using the template-method pattern
        virtual void selection_changed_impl() { }

        bool on_button_press_event(GdkEventButton* event);

        int child_position(const ListItem&) const;

    private:
        ItemList& get_item_list() const;

        // the old List widget has just one column
        TreeModelColumn<std::string> column_;

        //<support for CheckListItem>
        TreeModelColumn<bool> active_;
        TreeModelColumn<bool> visible_;

        void on_cell_toggled(const Glib::ustring&);
        //</support for CheckListItem>

        TreeModelColumn<Gtk::ListItem*> userData_; // hidden column

        // list store model
        Glib::RefPtr<ListStore> listStore_;

        std::auto_ptr<SelectionList> selection_;

        SigC::Signal0<void> selection_changed_;

        mutable std::auto_ptr<ItemList> itemList_;
        Items items_; //adapted items
    };


    /**
     * A list of items
     */
    class ItemList
    {
        ItemList(const ItemList&);
        ItemList& operator=(const ItemList&);

        List& list_;

        friend ItemList& List::get_item_list() const;

    protected:
        explicit ItemList(List&);

        Gtk::TreeModel::Children children();
        Gtk::TreeModel::Children children() const;

    public:
        typedef TreeNodeChildren::iterator iterator;
        typedef TreeNodeChildren::iterator const_iterator;

        size_t size() const { return children().size(); }
        bool empty() const { return children().empty(); }

        void clear();

        iterator begin() { return children().begin(); }
        iterator end() { return children().end(); }

        const_iterator begin() const { return children().begin(); }

        const_iterator end() const { return children().end(); }
    };


    inline void Gtk::List::select_item(int n)
    {
        select(listStore_->children()[n]);
    }


    inline void Gtk::List::deselect_item(int n)
    {
        deselect(listStore_->children()[n]);
    }
 } // namespace Gtk

 inline std::vector<Gtk::SelectionItem>
 Gtk_get_selection_items(const Gtk::List::SelectionList& list)
 {
    return list.items();
 }

 inline void Gtk_select_all(Gtk::List& list)
 {
    list.select_all();
 }

 inline void Gtk_unselect_all(Gtk::List& list)
 {
    list.unselect_all();
 }
#endif // GTKMM_2
#endif // LIST_H__4E6C4596_2EB1_4E8B_83A0_6E23D8472B4D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
