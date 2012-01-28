#ifndef MENU_BUILDER_IMPL_H__D7E4FBC8_ECA5_4CDF_B38C_D04D8A68BCC6
#define MENU_BUILDER_IMPL_H__D7E4FBC8_ECA5_4CDF_B38C_D04D8A68BCC6
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

#include <vector>
#include "menu_builder.h"


template<typename T>
class ZDK_LOCAL MenuBuilderImpl : public MenuBuilder<T>
{
    typedef RefPtr<MenuAction<T> > ElemType;
    typedef std::vector<ElemType> MenuVector;
    typedef typename MenuVector::const_iterator const_iterator;

    MenuVector menuVector_;

public:
    void populate(T* obj, Gtk::Menu& menu, MenuClickContext& context)
    {
        const_iterator i = menuVector_.begin();
        for (; i != menuVector_.end(); ++i)
        {
            (*i)->add_to(obj, menu, context);
        }
    }

    void register_menu(const ElemType& elem)
    {
        menuVector_.push_back(elem);
    }

    bool menu_empty() const { return menuVector_.empty(); }
};

#endif // MENU_BUILDER_IMPL_H__D7E4FBC8_ECA5_4CDF_B38C_D04D8A68BCC6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
