#ifndef VALUE_H__9602A028_2B4C_4034_A6B8_6767C5C0A0E4
#define VALUE_H__9602A028_2B4C_4034_A6B8_6767C5C0A0E4
//
// $Id: value.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/thread_util.h"
#include "zdk/zero.h"


/**
 * Helper for reading values of params, variables
 * and constants from a debugged thread's memory.
 */
template<typename T>
class ZDK_LOCAL Value
{
public:
    Value() : value_(T()) {}

    explicit Value(T value) : value_(value) {}

    T value() const { return value_; }

    /*
     * Read the value from given address in thread's memory
     */
    T read(Thread* thread, addr_t addr)
    {
        assert(thread);

        thread_read(*thread, addr, value_);

        return value();
    }

    T read(const RefPtr<Thread>& thread, addr_t addr)
    {
        return read(thread.get(), addr);
    }

    template<typename U>
    T read(const U& symbol)
    {
        return read(symbol.thread(), symbol.addr());
    }

/**** compiler-generated copy ctor is fine
    Value& operator=(const Value& other)
    {
        value_ = other.value_;
        return *this;
    }
*****/

private:

#if 0
    /* The size of the value, in machine-words */
    enum
    {
        word_count = (sizeof(T) + sizeof(long) - 1) / sizeof(long)
    };

    union
    {
        T value_;
        word_t buf_[word_count];
    };
#else

    T value_;

#endif
};

#endif // VALUE_H__9602A028_2B4C_4034_A6B8_6767C5C0A0E4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
