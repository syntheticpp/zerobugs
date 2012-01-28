#ifndef INTERPRETER_BOX_H__2119D653_6D3A_4615_A185_EC605B52534D
#define INTERPRETER_BOX_H__2119D653_6D3A_4615_A185_EC605B52534D
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
//
#include "gtkmm/box.h"
#include "gtkmm/events.h"
#include "zdk/interp.h"

class AppSlots;


/**
 * A UI element for interacting with an arbitrary interpreter
 */
class InterpreterBox : public Gtk::VBox
{
public:
    InterpreterBox(AppSlots*, Interpreter*);

    const char* name() const;

private:
    void help(AppSlots*);

private:
    RefPtr<Interpreter> interp_;
    RefPtr<Interpreter::Output> console_;
};

#endif // INTERPRETER_BOX_H__2119D653_6D3A_4615_A185_EC605B52534D
