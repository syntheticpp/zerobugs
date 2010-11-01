# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_ret_fun.sh 356 2008-02-19 07:56:19Z root $

################################################################
#
################################################################
function test_ret_fun()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
#include <iostream>
using namespace std;
void fun(int& i)
{
	++i;
}
int main(int argc, char* argv[])
{
	int x = argc;
	fun(x);
	cout << x << endl;
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call next
call step
call ret
call list
call line
expect ( 11
) or ( 10
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

rm -f $config
shift
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
        test_ret_fun -g
    else
        test_ret_fun -gdwarf-2 -g3 $@
        test_ret_fun -gstabs+ $@
    fi
}

source suffix.sh
