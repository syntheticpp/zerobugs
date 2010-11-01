//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#ifndef DEBUG
 #define DEBUG
#endif
#include <assert.h>
#include <iostream>
#include "zdk/ref_counted_impl.h"
#include "zdk/weak_ptr.h"

using namespace std;


class Test : public RefCountedImpl<>
{
    static size_t instances_;

    Test(const Test&);
    Test& operator=(const Test&);

public:
    Test() { ++instances_; }
    ~Test() throw() { --instances_; }

    static size_t instances() { return instances_; }

    const void* self() const { return this; }
};

size_t Test::instances_ = 0;


void test_deletion()
{
    assert(Test::instances() == 0);
    {
        RefPtr<Test> ptr(new Test);
        assert(Test::instances() == 1);
    }
    assert(Test::instances() == 0);
}



void test_detach()
{
    assert(Test::instances() == 0);
    RefPtr<Test> ptr(new Test);

    {
        RefPtr<Test> ptr2(ptr.detach());
        assert(Test::instances() == 1);
        assert(!ptr);
    }
    assert(Test::instances() == 0);
}


void test_operators()
{
    RefPtr<Test> ptr(new Test);
    assert(ptr->self() == ptr.get());

    WeakPtr<Test> weak_ptr(ptr);
    assert(weak_ptr.ref_ptr()->self() == ptr.get());
}


void test_weak_ptr()
{
    RefPtr<Test> ptr(new Test);
    WeakPtr<Test> weak_ptr(ptr);
    assert(weak_ptr.ref_count() == 2);
    {
        WeakPtr<Test> tmp(ptr);
        assert(weak_ptr.ref_count() == 3);
    }
    assert(weak_ptr.ref_count() == 2);
    assert(Test::instances() == 1);
    assert(weak_ptr.ref_count() == 2);
    assert(weak_ptr.ref_ptr() == ptr);

    ptr.reset();
    assert(!weak_ptr.ref_ptr());

    WeakPtr<Test> weak;
    {
        Test test2;
        weak = &test2;
        assert(weak.ref_count() == 2);
    }
    assert(weak.ref_count() == 1);
    assert(weak.ref_ptr().get() == 0);
}


class Parent : public RefCountedImpl<>
{
public:
    class Child : public RefCountedImpl<>
    {
        WeakPtr<Parent> parent_;

    public:
        Child(Parent& parent) : parent_(&parent)
        {
        }

        int test() const
        {
            if (RefPtr<Parent> parent = parent_.ref_ptr())
            {
                return parent->test();
            }
            return 0;
        }
    };

    virtual int test() const { return 42; }

    RefPtr<Child> make_child() { return new Child(*this); }
};


void test_parent_child()
{
    RefPtr<Parent> parent(new Parent);
    assert(parent->make_child()->test() == 42);

    //Parent anotherParent;
    //assert(anotherParent.make_child()->test() == 42);
}


void test_lazy()
{
    RefPtr<Test> p1 = new Test;
    {
        RefPtr<Test> p2 = p1;
    }
    WeakPtr<Test> wp = p1;
}


int main()
{
    cout << "sizeof(RefCountedImpl<>)=" << sizeof(RefCountedImpl<>) << endl;
    cout << "sizeof(atomic_t)=" << sizeof(atomic_t) << endl;
    cout << "sizeof(SupportsWeakRef<>)=" << sizeof (SupportsWeakRef<Unknown2>) << endl;

    assert(WeakPtr<Test>() == WeakPtr<Test>());

    test_detach();
    test_deletion();
    test_operators();
    test_weak_ptr();
    test_parent_child();
    test_lazy();

    assert(Test::instances() == 0);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
