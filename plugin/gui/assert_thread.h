#ifndef ASSERT_THREAD_H__0310EB6B_014F_4EF7_8045_3CE10D65259A
#define ASSERT_THREAD_H__0310EB6B_014F_4EF7_8045_3CE10D65259A
//
// $Id: assert_thread.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

extern "C"
{
    void assert_ui_thread();

    void assert_main_thread();

    bool is_ui_thread();
}

#endif // ASSERT_THREAD_H__0310EB6B_014F_4EF7_8045_3CE10D65259A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
