#ifndef SHARED_STRING_IMPL_H__0AD27CA9_ABB2_46AF_A0FE_EB5077C0F6B1
#define SHARED_STRING_IMPL_H__0AD27CA9_ABB2_46AF_A0FE_EB5077C0F6B1
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

#include <assert.h>
#include <new>                  // std::nothrow_t
#include "zdk/fobject.h"
#include "zdk/shared_string.h"
#include "zdk/zobject_impl.h"


CLASS SharedStringImpl : public ZObjectImpl<SharedString>
{
public:
BEGIN_INTERFACE_MAP(SharedString)
    INTERFACE_ENTRY(SharedString)
    INTERFACE_ENTRY(ZObject)
END_INTERFACE_MAP()

    virtual ~SharedStringImpl() throw();

    static SharedString* take_ownership(char* s, size_t len = 0) throw()
        { return new SharedStringImpl(s, len, std::nothrow); }
    static SharedString* create(const char* str = 0)
        { return new SharedStringImpl(str); }

    static SharedString* create(char* str,
                                size_t len,
                                std::nothrow_t nothrow)
        { return new SharedStringImpl(str, len, nothrow); }

    static SharedString* create(const char* s1, const char* s2)
        { return new SharedStringImpl(s1, s2); }

    static SharedString* create(const char* str, size_t len)
        { return new SharedStringImpl(str, len); }

    static uint32_t hash(const char*);

private:
    //non-copyable, non-assignable
    SharedStringImpl(const SharedStringImpl&);
    SharedStringImpl& operator=(const SharedStringImpl&);

    /**
     * Ctor makes a copy of the passed string
     */
    explicit SharedStringImpl(const char*);
    SharedStringImpl(char* s, size_t len, std::nothrow_t);
    SharedStringImpl(const char*, const char*);
    SharedStringImpl(const char*, size_t length);

    virtual const char* c_str() const;

    virtual size_t length() const;

    virtual bool is_equal(const char*, bool) const;

    virtual bool is_equal2(const SharedString*, bool) const;

    virtual bool is_less(const char*) const;

    virtual SharedString* prepend(const char*) const;

    virtual SharedString* append(const char*) const;

    virtual uint32_t hash() const;

    void init(const char*, size_t len);

private:
    union
    {
        struct __attribute__((packed))
        {
            mutable bool small_ : 1;
            mutable unsigned int length_ : 31;
            char*  str_;
        #if (__WORDSIZE == 32)
            mutable uint64_t hash_;
        #elif (__WORDSIZE == 64)
            mutable uint32_t hash_;
        #endif
        };

        struct __attribute__((packed))
        {
            mutable bool small_ : 1;
            mutable uint8_t length_ : 7;
            char buf_[15];
        } _small;

        struct
        {
            uint64_t atom0_;
            uint64_t atom1_;
        };
    };
};


template<typename T> inline ZDK_LOCAL
RefPtr<SharedString> shared_string(const T& s)
{
    return SharedStringImpl::create(s.c_str(), s.length());
}

template<> RefPtr<SharedString>
shared_string<SharedString>(const SharedString&);

inline ZDK_LOCAL RefPtr<SharedString> shared_string(const char* str)
{
    return SharedStringImpl::create(str);
}

inline ZDK_LOCAL RefPtr<SharedString> shared_string(const unsigned char* str)
{
    return SharedStringImpl::create(reinterpret_cast<const char*>(str));
}

inline ZDK_LOCAL RefPtr<SharedString> shared_string(const char* str, size_t len)
{
    return SharedStringImpl::create(str, len);
}

#endif // SHARED_STRING_IMPL_H__0AD27CA9_ABB2_46AF_A0FE_EB5077C0F6B1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
