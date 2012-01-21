# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_expr.sh 508 2008-05-24 21:32:12Z root $
#
################################################################
# Test expression evaluation
################################################################

function test_expr_dwarf3()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>

void foo(int i)
{
	i++;
	std::cout << i << std::endl;
	std::cout << "hello" << std::endl;
}

int main()
{
	int res = 100;
	int i = 42;
	foo(i);
	return res;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
echo ##### test_expr_dwarf3 #####
call { exec a.out }
call { break main }
call continue

call next
call next
call next
call step
call next
call ( eval i )
expect ( 42
)
call quit

---end---

dbgopt=${1:-$debug}
shift

#build $dbgopt -DTEST_NAMESPACE foo.cpp
#run_debugger $@

build $dbgopt foo.cpp
run_debugger $@
}

function run()
{
	source common.sh

	if [ "$compiler" = icc ]
	then
		:
	else
		test_expr_dwarf3 -gdwarf-3 $@
	fi
}

source suffix.sh
