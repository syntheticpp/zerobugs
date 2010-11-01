#ifndef OBSERVER_H__503E2538_4E91_4EA3_9283_FA2932839CD8
#define OBSERVER_H__503E2538_4E91_4EA3_9283_FA2932839CD8
//
// $Id: observer.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zobject.h"


struct Subject;

DECLARE_ZDK_INTERFACE_(Observer, ZObject)
{
    DECLARE_UUID("6f1df56c-26d2-457f-bd32-6d9e600eec61")

    virtual void on_state_change(Subject*) = 0;
};


DECLARE_ZDK_INTERFACE_(Subject, ZObject)
{
    DECLARE_UUID("bec6a296-8e98-40c2-847c-daa6ca75927f")

    virtual bool attach(Observer*) = 0;

    virtual bool detach(Observer*) = 0;

    virtual void notify_state_change() = 0;

};

#endif // OBSERVER_H__503E2538_4E91_4EA3_9283_FA2932839CD8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
