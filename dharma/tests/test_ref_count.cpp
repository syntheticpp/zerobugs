
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "test_common.h"
#include <iostream>
#include "zdk/ref_counted_impl.h"
#include "zdk/ref_ptr.h"

struct SampleInterface : public RefCounted
{
    virtual void run() const = 0;
};


class SampleInterfaceImpl : public RefCountedImpl<SampleInterface>
{
public:
    SampleInterfaceImpl() { std::cout << "hello\n"; }
    ~SampleInterfaceImpl() throw() { std::cout << "goodbye\n"; }

private:
    void run() const
    { std::cout << "Hello Universe" << std::endl; }
};


int main()
{
    RefPtr<SampleInterface> rp1(new SampleInterfaceImpl());
    std::cout << rp1->ref_count() << std::endl;
    assert(rp1->ref_count() == 1);

    RefPtr<SampleInterfaceImpl> rp2 = dynamic_cast<SampleInterfaceImpl*>((SampleInterface*)rp1.get());
    assert(rp1->ref_count() == 2);
    RefPtr<SampleInterface> rp4;

    rp4 = rp2;
    assert(rp1->ref_count() == 3);

    {
        RefPtr<SampleInterface> rp3 = rp1;
        assert(rp1->ref_count() == 4);
        rp3->run();
    }
    assert(rp1->ref_count() == 3);
    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
