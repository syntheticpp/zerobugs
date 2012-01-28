#ifndef TEMPORARY_H__1059368586
#define TEMPORARY_H__1059368586
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

/**
 *  Helper class for modifying a value temporarily, and have it
 *  automatically restore when the scope is exited. Uses the
 * "Resource-Acquisition-Is-Initialization" idiom, see Bjarne.
 *  Example:
 *     int some_value = 42;
 *     ...
 *     {
 *          Temporary<int> set_value(some_value, 13);
 *          assert(some_value == 13);
 *          ...
 *          ...
 *     }
 *     assert(some_value == 42);
 */
template<typename T>
class Temporary
{
public:
    typedef T value_type;
    typedef T& reference;

    explicit Temporary(T& ref) : ref_(ref), value_(ref)
    {
    }
    template<typename U>
    Temporary(T& ref, U tempValue) : ref_(ref), value_(ref)
    {
        ref_ = tempValue;
    }
    ~Temporary() throw()
    {
        ref_ = value_;
    }
    operator T () { return value_; }
private:
    // non-copyable, non-assignable
    Temporary(const Temporary&);
    Temporary& operator=(const Temporary&);

    T&  ref_;
    T   value_;
};

#endif // TEMPORARY_H__1059368586
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
