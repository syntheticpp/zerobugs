//
// $Id: shared_string.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/stdexcept.h"
#include "zdk/string.h"
#include "zdk/shared_string_impl.h"
#include <iostream>
#include <string.h>

using namespace std;

static uint32_t HASH_INIT = 0;


static inline void* xalloc(size_t nbytes)
{
    return malloc(nbytes);
}

static inline void xfree(void* p, size_t nbytes)
{
    free(p);
}

static inline void xprotect() { }


static inline void check_max_len(size_t len)
{
    if (len > 0x7fffffff)
    {
        throw range_error("max SharedString length exceeded");
    }
}


void SharedStringImpl::init(const char* str, size_t len)
{
    assert(str_ == 0);
    assert(length_ == 0);

    if (len)
    {
        assert(str);

        if (len < sizeof(_small.buf_))
        {
            assert(atom0_ == 0);
            assert(atom1_ == 0);

            memcpy(&_small.buf_, str, len);
            _small.length_ = len;
            small_ = true;

            assert(_small.length_ == len);
            assert(_small.buf_[len] == 0);
        }
        else
        {
            check_max_len(len);

            assert(str);
            str_ = static_cast<char*>(xalloc(len + 1));

            if (str_)
            {
                memcpy(str_, str, len);
                str_[len] = 0;
                length_ = len;

                xprotect();
            }
        }
    }
}


SharedStringImpl::SharedStringImpl(const char* str)
    : small_(false)
    , length_(0)
    , str_(0)
    , hash_(HASH_INIT)
{
    if (str)
    {
        init(str, strlen(str));
    }
}


SharedStringImpl::SharedStringImpl
(
    const char* first,
    const char* last
)
    : small_(false)
    , length_(0)
    , str_(0)
    , hash_(HASH_INIT)
{
    assert(first);
    init(first, distance(first, last));
}


SharedStringImpl::SharedStringImpl(const char* str, size_t len)
    : small_(false)
    , length_(0)
    , str_(0)
    , hash_(HASH_INIT)
{
    assert(str);
    init(str, len);
}


SharedStringImpl::SharedStringImpl(char* s, size_t len, std::nothrow_t)
    : small_(false)
    , length_(len)
    , str_(s)
    , hash_(HASH_INIT)
{
    check_max_len(len);
    assert((s == 0) || (len == 0) || (len == strlen(s)));
}


SharedStringImpl::~SharedStringImpl() throw()
{
    assert(ref_count() == 0);

    if (str_ && !small_)
    {
    #ifdef DEBUG
        memset(str_, 'x', length());
    #endif
        xfree(str_, length() + 1);
    }
}


const char* SharedStringImpl::c_str() const
{
    if (small_)
    {
        return _small.buf_;
    }
    return str_ ? str_ : "";
}


size_t SharedStringImpl::length() const
{
    if (small_)
    {
        return _small.length_;
    }

    if (!length_ && str_)
    {
        size_t len = strlen(str_);
        check_max_len(len);

        length_ = len;
    }

    return length_;
}


bool
SharedStringImpl::is_equal(const char* str, bool ignoreSpace) const
{
    if (str == c_str())
    {
        return true;
    }
    if (str == NULL)
    {
        return !small_ && (str_ == NULL);
    }
    else if (!ignoreSpace)
    {
        return strcmp(c_str(), str) == 0; // assume ASCII
    }
    else
    {
        return strcmp_ignore_space(c_str(), str) == 0;
    }
    return false;
}


bool
SharedStringImpl::is_equal2(const SharedString* str, bool ignspc) const
{
    if (this == str)
    {
        return true;
    }
    if (!str)
    {
        return false;
    }
    //
    // Theoretically I am supposed to be using interface_cast<> here,
    // static_cast<> is faster. The only drawback is that if someone
    // else implements the SharedString interface, this code breaks down.
    //
    const SharedStringImpl* impl = static_cast<const SharedStringImpl*>(str);

    return ((atom0_ == impl->atom0_) && (atom1_ == impl->atom1_))
        || ((hash() == impl->hash()) && is_equal(str->c_str(), ignspc));
}


bool SharedStringImpl::is_less(const char* str) const
{
    assert(str);
    if (this->c_str() == str)
    {
        return false;
    }
    return strcmp(this->c_str(), str) < 0; // assume ASCII
}


SharedString* SharedStringImpl::prepend(const char* str) const
{
    if (!str)
    {
        return const_cast<SharedStringImpl*>(this);
    }

    const size_t len = strlen(str);

    char buf[sizeof (_small.buf_)];
    char* tmp = buf;

    size_t newLength = len + length();

    if (newLength >= sizeof buf)
    {
        check_max_len(newLength);
        tmp = (char*) xalloc(newLength + 1);
    }
    if (tmp)
    {
        memcpy(tmp, str, len);
        memcpy(tmp + len, c_str(), length());

        tmp[newLength] = 0;

        xprotect();
    }
    else
    {
        newLength = 0;
    }
    if (tmp == buf)
    {
        return new SharedStringImpl(buf, newLength);
    }
    return new SharedStringImpl(tmp, newLength, nothrow);
}


SharedString* SharedStringImpl::append(const char* str) const
{
    if (!str)
    {
        return const_cast<SharedStringImpl*>(this);
    }

    const size_t len = strlen(str);
    char buf[sizeof (_small.buf_)];
    char* tmp = buf;

    size_t newLength = len + length();

    if (newLength >= sizeof(buf))
    {
        check_max_len(newLength);
        tmp = (char*) xalloc(newLength + 1);
    }
    if (tmp)
    {
        memcpy(tmp, c_str(), length());
        memcpy(tmp + length(), str, len);

        tmp[newLength] = 0;

        xprotect();
    }
    else
    {
        newLength = 0;
    }
    if (tmp == buf)
    {
        return new SharedStringImpl(buf, newLength);
    }
    return new SharedStringImpl(tmp, newLength, nothrow);
}


uint32_t SharedStringImpl::hash(const char* s)
{
    unsigned long h = HASH_INIT;

    if (s)
    {
        for (; *s; ++s)
        {
            if (!isspace(*s))
            {
                h = 5 * h + *s;
            }
        }
    }
    return uint32_t(h);
}


uint32_t SharedStringImpl::hash() const
{
    if (small_)
    {
        return hash(c_str());
    }
    if (!hash_)
    {
        hash_ = hash(str_);
    }
    return hash_;
}


std::ostream& operator<<(std::ostream& outs, const SharedString* str)
{
    return str ? (outs << str->c_str()) : (outs << "(null SharedString)");
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
