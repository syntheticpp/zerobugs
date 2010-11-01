#ifndef SWITCHABLE_H__3C72CC32_6B85_4270_84D5_D11F7998F730
#define SWITCHABLE_H__3C72CC32_6B85_4270_84D5_D11F7998F730
//
// $Id: switchable.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/unknown2.h"

/**
 * Allows an action, object, etc. to be enabled/disabled.
 * Used for enabling/activating breakpoint actions;
 * provides support for activationg after a number of hits,
 * and or upon evaluating an expression to true
 */
DECLARE_ZDK_INTERFACE_(Switchable, Unknown2)
{
    DECLARE_UUID("e28a8dab-57bb-4182-9be0-5eea1029d582")

    virtual bool is_enabled() const volatile = 0;

    /**
     * Decrements internal count, and returns new value.
     */
    virtual void enable() volatile = 0;

    /**
     * Decrements internal count, and returns new value.
     * If counter reaches zero, the object is disabled.
     */
    virtual void disable() volatile = 0;

    /**
     * The counter may be used for activating a switchable
     * object after a certain condition (a breakpoint was
     * hit, for example) happens a number of times.
     */
    virtual unsigned long counter() const = 0;

    virtual void set_counter(unsigned long) = 0;

    virtual unsigned long activation_counter() const = 0;

    /**
     * Specifies how many times an object must be hit before
     * it is activated. If autoReset is true, the counter is set
     * back to zero once the object is activated; the object needs
     * to be hit another hitCount times before it is activated
     * again. Otherwise, the counter is left unchanged, and the
     * object is activated by all subsequent hits.
     */
    virtual void set_activation_counter(
        unsigned long hitCount,
        bool autoReset) = 0;

    virtual bool auto_reset() const = 0;

    /**
     * @return an expression that, when evaluated to true,
     * activates the object
     */
    virtual const char* activation_expr() const = 0;

    /**
     * sets an expression that, when evaluated to true,
     * activates the object
     */
    virtual void set_activation_expr(const char*) = 0;
};
#endif // SWITCHABLE_H__3C72CC32_6B85_4270_84D5_D11F7998F730
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
