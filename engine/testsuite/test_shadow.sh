# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
# Test that foo_ in the derived class "shadows" foo_ in base
################################################################
function test_shadow()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'

#include <iostream>
#include <string>
using namespace std;

class Member
{
    int x_;
public:
    Member() : x_(42) { }
    int x() const { return x_; }
};

class TestBase
{
public:
    int foo_;
    char bar_;

public:
    int foo() const { return foo_; }
    char bar() const { return bar_; }
};

class Test : public TestBase
{
public:
    string foo_;
    Member m_;

public:
    Test() : foo_("blah") { }

    const char* name() const { return foo_.c_str(); }
};


int main()
{
    Test test;

    cout << test.foo_ << endl;
    return test.foo();
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call next
call ( eval test.x_ )
expect ( Test has no member named 'x_'
)
call ( eval test.m_.x_)
expect ( 42
)
call ( eval test.foo_.size() )
expect ( 4
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp
shift
rm -f $config
run_debugger $@ --main a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_shadow -g
    else
        test_shadow -gdwarf-2 -g3 $@
        test_shadow -gstabs+ $@
    fi
}

source suffix.sh
