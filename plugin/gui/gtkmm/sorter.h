#ifndef SORTER_H__F6767C1F_27BE_4C11_B4FC_B13F384C11C4
#define SORTER_H__F6767C1F_27BE_4C11_B4FC_B13F384C11C4
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include "zdk/export.h"
#include <cstdlib>
#include <string>

namespace detail
{
/**
 * When using gtkmm 2.x, we mimic the old 1.2 behavior:
 * all columns contain strings; in the case of module views,
 * we want to be able to sort by addresses as numbers.
 * Implementing this sorting mechanism is redundant because
 * we could have had columns of numeric types in the model,
 * and use the builtin sorting, but then it would've been more
 * difficult to support the older, gtk-1.2 systems.
 */
template<typename T>
struct ZDK_LOCAL Sorter : public sigc::trackable
{
    T& list_;
    unsigned int column_;
    int base_;

    Sorter(T& list, unsigned column, int base)
        : list_(list)
        , column_(column)
        , base_(base)
    { }

    int compare(const Gtk::TreeModel::iterator& lhs,
                const Gtk::TreeModel::iterator& rhs) const
    {
        const std::string& lstr = (*lhs)[list_.model_column(column_)];
        const std::string& rstr = (*rhs)[list_.model_column(column_)];

        const long lval = strtol(lstr.c_str(), 0, base_);
        const long rval = strtol(rstr.c_str(), 0, base_);

        if (lval < rval)
        {
            return -1;
        }
        if (lval > rval)
        {
            return 1;
        }
        return 0;
    }
};

} // detail


template<typename T>
inline void set_numeric_sort(T& clist, int ncol, int base)
{
    detail::Sorter<T>* sorter(new detail::Sorter<T>(clist, ncol, base));

    Gtk::TreeSortable::SlotCompare sl =
        sigc::mem_fun(*sorter, &detail::Sorter<T>::compare);
    clist.set_sort_func(ncol, sl);
}
#else
template<typename T>
inline void set_numeric_sort(T& list, int ncol, int)
{
    // sorting is not implemented for 1.2
    list.column(ncol).set_passive();
}

#endif // GTKMM_2
#endif // SORTER_H__F6767C1F_27BE_4C11_B4FC_B13F384C11C4
