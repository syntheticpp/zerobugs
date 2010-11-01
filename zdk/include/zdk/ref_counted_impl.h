#ifndef REF_COUNTED_IMPL_H__DA555A9B_D545_4808_AC17_A7E51B4BFBC2
#define REF_COUNTED_IMPL_H__DA555A9B_D545_4808_AC17_A7E51B4BFBC2
//
// $Id: ref_counted_impl.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/ref_counted.h"
#include "zdk/weak_ref_impl.h"

template<typename T, bool>
struct RefCountedTraits
{
    typedef SupportsWeakRef<T> Base;
};
template<typename T>
struct RefCountedTraits<T, false>
{
    typedef T Base;
};
////////////////////////////////////////////////////////////////
//! A stock implementation of RefCounted
/*!
 Usage:
 \code
    class InterfaceImpl : public RefCountedImpl<Interface>
    { ... }
    where Interface derived from RefCounted, or:

    class SomeClass : public RefCountedImpl<> { ... }
 \endcode
 */
template<typename T = RefCounted>
class RefCountedImpl
    : public RefCountedTraits<T, T::supports_weak_ref>::Base
{
    // non-copyable, non-assignable
    RefCountedImpl(const RefCountedImpl&);
    RefCountedImpl& operator=(const RefCountedImpl&);

public:
    RefCountedImpl() { }
    virtual ~RefCountedImpl() throw()
    {
        assert(ref_count() == 0);
    }
    virtual long ref_count() const volatile
    {
        return atomic_read(count_);
    }

protected:
    virtual void inc_ref()
    {
        atomic_inc(count_);
    }
    virtual bool dec_ref_and_test()
    {
        return atomic_dec_and_test(count_);
    }

private:
    mutable atomic_t count_;
};
#endif // REF_COUNTED_IMPL_H__DA555A9B_D545_4808_AC17_A7E51B4BFBC2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
