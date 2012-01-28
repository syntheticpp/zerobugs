#ifndef STOCK_H__0CB8B219_47B2_4E82_A189_3FDFBA323C3A
#define STOCK_H__0CB8B219_47B2_4E82_A189_3FDFBA323C3A
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
 #include <gtkmm/stock.h>
#else
 #include <string>

 namespace Gtk
 {
    struct StockID
    {
        const std::string name_;

        StockID(const char* name) : name_(name ? name : "")
        { }
        operator const std::string& () const
        { return name_; }
    };

    struct BuiltinStockID { };

    namespace Stock
    {
        extern const BuiltinStockID APPLY;
        extern const BuiltinStockID CANCEL;
        extern const BuiltinStockID DIALOG_ERROR;
        extern const BuiltinStockID JUMP_TO;
        extern const BuiltinStockID ZOOM_IN;
        extern const BuiltinStockID GO_FORWARD;
        extern const BuiltinStockID GOTO_LAST;
        extern const BuiltinStockID GOTO_TOP;

        static const char DELETE[] = "_Delete";
        static const char EXECUTE[] = "_Execute";
    };
 }
#endif
#endif // STOCK_H__0CB8B219_47B2_4E82_A189_3FDFBA323C3A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
