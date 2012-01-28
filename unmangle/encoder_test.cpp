// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include "encoder.h"

using namespace std;

int main(int argc, char* argv[])
{
    //Mangle::PartialEncoder enc(Mangle::kOldGcc);
    Mangle::PartialEncoder enc;

    cout << enc.run("Foo<char*>") << endl;
    cout << enc.run("Foo<char *>") << endl;
    cout << enc.run("Foo<const char>") << endl;
    cout << enc.run("Foo<const char*>") << endl;
    cout << enc.run("Foo<char const*>") << endl;
    cout << enc.run("Foo<char* const>") << endl;
    // cout << enc.run("Foo<char* const, unsigned int>") << endl;
    cout << enc.run("Blah::Foo::blah") << endl;
    //cout << enc.run("Foo::func") << endl;
    //cout << enc.run("Foo::Foo") << endl;
    //cout << enc.run("Blah::Foo::blah()") << endl;
    //cout << enc.run("Foo::~Foo") << endl;
    cout << enc.run("Foo<Bar, volatile Foo*>::fun") << endl;
    cout << enc.run("Foo<Bar, volatile Foo*>::Foo()") << endl;
    cout << enc.run("Foo<Bar, 42>::func") << endl;
    cout << enc.run("Foo<Bar<Baz, char_traits<char> > >::fun") << endl;

    cout << enc.run("SmartP<const char, const char*, Traits<const char*> >::get") << endl;
    cout << enc.run("SmartP<const char, Traits<const char*>, const char*>::get") << endl;

    cout << enc.run("Foo::operator int() volatile const") << endl;
    cout << enc.run("DebuggerEngine::initialize") << endl;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
