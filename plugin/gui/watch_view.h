#ifndef WATCH_VIEW_H__4300E42D_4C13_4EF3_B019_B6FFC8E13659
#define WATCH_VIEW_H__4300E42D_4C13_4EF3_B019_B6FFC8E13659
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

#include "zdk/watch.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "eval_base.h"
#include "variables_view.h"


CLASS WatchView : public EvalBase<WatchView, VariablesView>
{
public:
    explicit WatchView(Debugger&);
    void on_done(const Variant&);

    void save_config();
    void restore(Thread&);

    virtual void display(bool force = false);
    virtual bool update(RefPtr<Thread>);

private:
    virtual bool on_cell_edit_vfunc(CELL_EDIT_PARAM);
    virtual bool on_error(std::string);

private:
    typedef ext::hash_set<std::string> WatchSet;

    // map expressions that could not be evaluated to err msgs
    typedef ext::hash_map<std::string, std::string> ErrMap;

    WatchSet    watchSet_;
    WatchSet    workSet_;
    ErrMap      errMap_;
    std::string current_;   // expression being evaluated
    bool        pending_;
    WeakPtr<WatchList> watchList_;
};

#endif // WATCH_VIEW_H__4300E42D_4C13_4EF3_B019_B6FFC8E13659
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
