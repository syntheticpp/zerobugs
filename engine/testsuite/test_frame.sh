# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_frame.sh 432 2008-04-04 09:28:11Z root $
#
function test_frame()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

string bar(const char* s)
{
	return s;
}
string fun(const char* s, char c)
{
	if (const char* p = strchr(s, c))
	{
		return bar(p);
	}
	return s;
}
int main()
{
	fun("Hello World", 'W');
	return 0;
}
---end---

build ${1:-$debug} $@ foo.cpp

################################################################
#write test script for the AutoTest plugin
cat > ./script << '---end---'
call next
call step
call next
call next
call step
call next
#call test.cancel
call ( eval s )
expect ( "World"
)
call ( frame 1 )
call ( eval s )
expect ( "Hello World"
)
call quit
---end---
#finally, run the test
run_debugger --main ./a.out 
#run_debugger --main ./a.out --dwarf-debug
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
		test_frame -g
    else
        test_frame -gdwarf-2 
        test_frame -gstabs+
    fi
}

# run this test standalone
source suffix.sh
