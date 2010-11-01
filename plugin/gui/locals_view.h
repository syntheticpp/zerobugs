#ifndef LOCALS_VIEW_H__BC855668_36A6_4624_A51F_C8960FD7694E
#define LOCALS_VIEW_H__BC855668_36A6_4624_A51F_C8960FD7694E
//
// $Id: locals_view.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "variables_view.h"


/**
 * Displays the local variables and parameters (symbols in the
 * current scope of the current stack frame) using a CTree.
 * @note LOCALS is a bit of a misnomer: this widget also displays
 * global variables; in other words, it displays any variable that
 * is visible from within the current scope.
 */
class ZDK_LOCAL LocalsView : public VariablesView
{
public:
    explicit LocalsView(Debugger&);

    virtual bool update(RefPtr<Thread>);

    Symbol* function_in_scope() const { return fun_.get(); }

private:
    virtual void clear_data(bool keepExpand = false) throw();

    RefPtr<Symbol> fun_;
};

#endif // LOCALS_VIEW_H__BC855668_36A6_4624_A51F_C8960FD7694E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
