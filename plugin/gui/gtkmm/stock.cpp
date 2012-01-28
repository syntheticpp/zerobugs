// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
#include "stock.h"

#ifndef GTKMM_2
namespace Gtk
{
    const BuiltinStockID Stock::APPLY = {};
    const BuiltinStockID Stock::CANCEL = {};
    const BuiltinStockID Stock::DIALOG_ERROR = {};
    const BuiltinStockID Stock::JUMP_TO = {};
    const BuiltinStockID Stock::ZOOM_IN = {};
    const BuiltinStockID Stock::GO_FORWARD = {};
    const BuiltinStockID Stock::GOTO_LAST = {};
    const BuiltinStockID Stock::GOTO_TOP = {};
}
#endif
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
