# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_ctor.sh 205 2007-11-22 03:52:59Z root $
#
################################################################
# Test expression evaluation
################################################################

function test_ctor()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
#include <iostream>
namespace Bar 
{
	struct Foo
	{
		Foo() { std::cout << "Foo::Foo\n"; }
	};
}
int main()
{
	Bar::Foo foo;
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call test.cancel
call next
call ( eval Foo() )
call quit
---end---

build ${1:-$debug} foo.cpp
run_debugger $@ --main ./a.out
}

function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then
        test_ctor -g
    else
        test_ctor -gdwarf-2 $@
        test_ctor -gstabs+ $@
    fi
}

source suffix.sh
