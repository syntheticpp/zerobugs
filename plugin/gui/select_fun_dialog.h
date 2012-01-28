#ifndef SELECT_FUN_DIALOG_H__02EF88AD_22C9_4511_B776_E2769C43691A
#define SELECT_FUN_DIALOG_H__02EF88AD_22C9_4511_B776_E2769C43691A
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
//
#include <sstream>
#include "dharma/symbol_util.h"
#include "select_dialog.h"

/**
 * A dialog for selecting functions from a list.
 */
class SelectFunDialog : public SelectDialog
{
public:
    template<typename I> SelectFunDialog
    (
        I first, I last,
        const char* msg,
        const char* title = "Select Function"
    ) : SelectDialog(btn_ok_cancel, title, msg)
    {
        for (; first != last; ++first)
        {
            std::ostringstream s;

            const addr_t addr = (*first)->addr();
            print_symbol(s, addr, *first, true);

            add_item(s.str(), first->get());
        }
    }
};

#endif // SELECT_FUN_DIALOG_H__02EF88AD_22C9_4511_B776_E2769C43691A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
