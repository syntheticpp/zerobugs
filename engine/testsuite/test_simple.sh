# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_simple()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <iostream>
void fun(int x)
{
	std::cout << x << std::endl;
}
int main(int argc, char* argv[]) 
{
	std::cout << "&argc=" << &argc << std::endl;
	int n = argc;
	fun(++n);
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
        test_simple -g
    else
        test_simple -gdwarf-2 $@
        test_simple -gstabs+ $@
    fi
}

source suffix.sh
