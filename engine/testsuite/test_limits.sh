# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_limits.sh 205 2007-11-22 03:52:59Z root $
#
################################################################
function test_limits()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
// foo.cpp
#include <iostream>
#include <limits>
using namespace std;
int main()
{
	cout << numeric_limits<long long>::min() << endl;
	cout << numeric_limits<long long>::max() << endl;
	cout << numeric_limits<unsigned long long>::max() << endl;
}
---end---
#write test script for the AutoTest plugin
cat > ./script << '---end---'
call list
call next
call ( -9223372036854775808 - 1 )
expect ( 9223372036854775807
)
call test.cancel
call quit
---end---
#compile source
build ${1:-$debug} $@ foo.cpp
#finally, run the test
run_debugger --main a.out $@
#ZERO_USE_FRAME_HANDLERS=1 run_debugger $@
}
function run()
{
    source common.sh
	test_limits -gdwarf-23
	test_limits -gstabs
	test_limits -gstabs+
}
source suffix.sh
