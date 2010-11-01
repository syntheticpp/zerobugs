# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_longjmp.sh 205 2007-11-22 03:52:59Z root $

################################################################
#
################################################################
function test_longjmp()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <iostream>
#include <setjmp.h>

using namespace std;
static jmp_buf env;

int main(int argc, char* argv[])
{
	if (setjmp(env))
	{
		cout << "hello again\n";
	}
	else
	{
		cout << "hello\n";
		longjmp(env, 42);
	}
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call test.cancel
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

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
        test_longjmp -g
    else
        test_longjmp -gdwarf-2 -g3 $@
        test_longjmp -gstabs+ $@
    fi
}

source suffix.sh
