#ifndef SHARED_STRING_H__1176202B_9D50_4388_9ECC_AF35BE648380
#define SHARED_STRING_H__1176202B_9D50_4388_9ECC_AF35BE648380
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

#include "zdk/hsieh.h"
#include "zdk/ref_ptr.h"
#include "zdk/zobject.h"
#include <functional>       // binary_function
#include <iosfwd>

/**
 * A read-only, shared string interface.
 * Q: Why yet another string class?
 * A: to pass it around by interface pointer across
 * interfaces, and share the string across modules
 */
DECLARE_ZDK_INTERFACE_(SharedString, ZObject)
{
    DECLARE_UUID("8e904b59-4404-4159-812a-5df49b48b68a")

    virtual const char* c_str() const = 0;
    virtual size_t length() const = 0;

    virtual bool is_equal(const char*, bool ignoreSpace = false) const = 0;
    virtual bool is_equal2(const SharedString*, bool = false) const = 0;

    virtual bool is_less(const char*) const = 0;

    virtual SharedString* prepend(const char*) const = 0;
    virtual SharedString* append(const char*) const = 0;

    virtual uint32_t hash() const = 0;
};


bool inline ZDK_LOCAL
operator==(const SharedString& lhs, const SharedString& rhs)
{
    return &lhs == &rhs || lhs.is_equal(rhs.c_str());
}


bool inline ZDK_LOCAL
operator!=(const SharedString& lhs, const SharedString& rhs)
{
    return !(lhs == rhs);
}


bool inline ZDK_LOCAL
operator==(const SharedString& lhs, const char* rhs)
{
    return lhs.is_equal(rhs);
}


bool inline ZDK_LOCAL
operator==(const char* lhs, const SharedString& rhs)
{
    return rhs.is_equal(lhs);
}



typedef RefPtr<SharedString> SharedStringPtr;


namespace std
{
    /*  Do not want to override operator < and thus
        change how pointers are being compared, yet
        I can change how SharedStringPtr are ordered
        in a std::set or std::map */

    template<> struct less<SharedStringPtr>
        : public binary_function<SharedStringPtr, SharedStringPtr, bool>
    {
        bool operator()(const SharedStringPtr& lhs,
                        const SharedStringPtr& rhs) const
        {
            if ((&lhs == &rhs) || (lhs.get() == rhs.get()))
            {
                return false;
            }
            return lhs ? (rhs ? lhs->is_less(rhs->c_str()) : false) : rhs;
        }
    };


    /*  For use with hash_map of SharedStringPtr,
     *  similar to the less predicate above
     */
    template<> struct equal_to<SharedStringPtr>
        : public binary_function< SharedStringPtr, SharedStringPtr, bool>
    {
        bool operator()(const SharedStringPtr& lhs,
                        const SharedStringPtr& rhs) const
        {
            if (lhs.get() == rhs.get())
            {
                return true;
            }
            if (lhs)
            {
                return lhs->is_equal2(rhs.get());
            }
            return rhs.is_null();
        }
    };
    
    
    template<> struct hash<SharedStringPtr>
    {
        size_t operator()(const SharedStringPtr& ssp) const
        {
            return SuperFastHash(ssp->c_str(), ssp->length());
        }
    };
}


std::ostream& operator<<(std::ostream& outs, const SharedString*);


#endif // SHARED_STRING_H__1176202B_9D50_4388_9ECC_AF35BE648380
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
