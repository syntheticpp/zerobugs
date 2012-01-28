# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
# This test is interesting for several reasons:
# 1) passing pointers via registers to function call in x86_64
# 2) Calling a function (strcmp) for which there is no debug info
################################################################
function test_strcmp()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
#include <string.h>
#include <iostream>
static const char buf[] = "Hello World";
using namespace std;
// foo.cpp
int main(int argc, char* argv[])
{
	const char* p = buf;
	cout <<strcmp(p, "Hello") << endl;
	cout <<(void*)strcmp << endl;
	p = "Good Bye";
	strcmp(p, "Hello");
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call next
call next
#call test.cancel
#call ( eval strcmp (p, \"Hello\") )
#expect ( 1
#)
call ( eval strcmp (\"Hello World\", p) )
expect ( 0
)
#call ( eval strcmp (p, \"Hello World!\") )
#expect ( -1
#) or ( 4294967295
#)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp
shift
rm -f $config
#ZERO_DEBUG_INTERP=1 run_debugger $@ --main a.out 
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
        test_strcmp -g
    else
        test_strcmp -gdwarf-2 -g3 $@
        test_strcmp -gstabs+ $@
        test_strcmp -gstabs $@
    fi
}

source suffix.sh
