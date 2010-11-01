// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "zdk/shared_string_impl.h"
#include "zdk/ref_ptr.h"


static void test_compare_small_strings()
{
    RefPtr<SharedString> s1 = shared_string("foo");

    assert(s1->length() == 3);

    RefPtr<SharedString> s2 = shared_string(std::string("foo"));
    RefPtr<SharedString> s3 = SharedStringImpl::take_ownership(strdup("foo"));

    assert(s1->is_equal2(s1.get()));

    assert(s1->is_equal2(s2.get()));
    assert(s2->is_equal2(s3.get()));
    assert(s1->is_equal2(s3.get()));

    assert(strcmp(s1->c_str(), "foo") == 0);
}


static void test_compare_strings()
{
    RefPtr<SharedString> s1 = shared_string("__fubaristic__");
    RefPtr<SharedString> s2 = shared_string(std::string("__fubaristic__"));
    RefPtr<SharedString> s3 = shared_string(std::string("__foobaristic__"));

    assert(s1->is_equal2(s2.get()));
    assert(!s1->is_equal2(s3.get()));
}


static void test_append()
{
    RefPtr<SharedString> s1 = shared_string("foo");
    RefPtr<SharedString> s2 = s1->append("bar");

    RefPtr<SharedString> s3 = shared_string("foobar");
    RefPtr<SharedString> s4 = s3->append("-supercalifragilistic");

    assert(s2->is_equal2(s3.get()));
    assert(s4->is_equal("foobar-supercalifragilistic"));
}


static void test_prepend()
{
    RefPtr<SharedString> s1 = shared_string("bar");
    RefPtr<SharedString> s2 = s1->prepend("foo");

    RefPtr<SharedString> s3 = shared_string("foobar");
    RefPtr<SharedString> s4 = s3->prepend("supercalifragilistic-");

    assert(s2->is_equal2(s3.get()));
    assert(s4->is_equal("supercalifragilistic-foobar"));
}


static void test_empty()
{
    RefPtr<SharedString> s(SharedStringImpl::create());
    assert(s->length() == 0);
}


int main()
{
    std::cout << sizeof (RefCountedImpl<SharedString>)  << std::endl;
    std::cout << sizeof (ZObjectImpl<SharedString>)  << std::endl;
    std::cout << sizeof (SharedStringImpl)  << std::endl;
    //assert(sizeof (SharedStringImpl) == 16 + sizeof (ZObjectImpl<SharedString>));
    std::cout << sizeof (SharedStringImpl) << ", " << sizeof (ZObjectImpl<SharedString>) << std::endl;
    test_compare_strings();
    test_compare_small_strings();

    test_append();
    test_prepend();

    test_empty();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
