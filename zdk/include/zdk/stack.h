#ifndef STACK_H__03BCA316_0211_4596_BAE2_CB269B6607E0
#define STACK_H__03BCA316_0211_4596_BAE2_CB269B6607E0
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

#include "zdk/symbol.h"
#include "zdk/zobject.h"


/**
 * Models a stack frame. Stack frames are set up upon
 * entering a function call.
 */
DECLARE_ZDK_INTERFACE_(Frame, ZObject)
{
    DECLARE_UUID("c0aec75f-7678-40d9-9c10-622b502b8789")

    virtual Platform::addr_t program_count() const = 0;

    virtual Platform::addr_t frame_pointer() const = 0;

    virtual Platform::addr_t stack_pointer() const = 0;

    /**
     * The symbol of the function owning this stack frame
     * @param reserved, must always be NULL
     */
    virtual Symbol* function(Symbol* = NULL) const = 0;

    /**
     * Return this frame's index within a stack trace,
     * zero being the top-most frame.
     */
    virtual size_t index() const = 0;

    virtual void set_user_object(const char* name, ZObject*) = 0;

    /**
     * Get user object by name.
     * @return pointer to ref-counted object, or NULL
     */
    virtual ZObject* get_user_object(const char*) const = 0;

    /**
     * Used when fake frames are inserted into the stack trace
     */
    virtual Platform::addr_t real_program_count() const = 0;

    /**
     * @return true if signal trampoline
     */
    virtual bool is_signal_handler() const = 0;

    /**
     * @return true if the information in this frame
     * was produced by a FrameHandler
     */
    virtual bool is_from_frame_handler() const = 0;
};


/**
 * A snapshot of the debugged program's stack.
 * Modelled as a collection of frames.
 */
DECLARE_ZDK_INTERFACE_(StackTrace, ZObject)
{
    DECLARE_UUID("a0142f8b-ba40-4ee0-ba9c-a0b7d91cf7d7")

    /**
     * Get the depth (number of frames) in this stack trace.
     */
    virtual size_t size() const = 0;

    /**
     * @return false if maximum size was limited by the user
     * @see Thread::stack_trace
     */
    virtual bool is_complete() const = 0;

    /**
     * Get frame at given position; zero is the topmost frame.
     */
    virtual Frame* frame(size_t index) const = 0;

    /**
     * Make the frame given by index the current selection;
     * the selected frame can be used as a context by the
     * application for operations such as displaying the local
     * variables, etc.
     */
    virtual void select_frame(size_t index) = 0;

    virtual Frame* selection() const = 0;

    /**
     * @return a duplicate
     */
    virtual StackTrace* copy() const = 0;

    /**
     * how many frames were produced by a frame handler?
     */
    virtual size_t frame_handler_frame_count() const = 0;
};


#endif // STACK_H__03BCA316_0211_4596_BAE2_CB269B6607E0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
