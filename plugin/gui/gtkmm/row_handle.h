#ifndef ROW_HANDLE_H__84E0166B_E5B2_4A35_A94F_1AACA05A0A20
#define ROW_HANDLE_H__84E0166B_E5B2_4A35_A94F_1AACA05A0A20
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

#ifdef GTKMM_2
 #include <gtkmm/treeiter.h>
 #include <gtkmm/treepath.h>

 namespace Gtk
 {
    typedef TreeIter RowHandle;
    class TreeRowAdapter;

    int get_row_num(const TreePath& path);
    int get_row_num(const TreeRowAdapter&);
 }
#define INIT_ROW_HANDLE Gtk::RowHandle()
#define ROW_HANDLE_EQUAL(lhs, rhs) Gtk::row_handle_equal(lhs, rhs)

#else
 // Gtk-- 1.2
 enum
 {
    INIT_ROW_HANDLE = -1
 };

 namespace Gtk
 {
    typedef int RowHandle;

    template<typename T>
    inline size_t get_row_num(const T& row)
    {
        return row.get_row_num();
    }

    inline int get_row_num(int nrow)
    {
        return nrow;
    }
 }
#define ROW_HANDLE_EQUAL(lhs, rhs) ((lhs) == (rhs))

#endif
#endif // ROW_HANDLE_H__84E0166B_E5B2_4A35_A94F_1AACA05A0A20
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
