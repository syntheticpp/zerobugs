# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: $

################################################################
#
################################################################
function test_longjmp()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <setjmp.h>

static jmp_buf jmp;

void fun()
{
	longjmp(jmp, 1);
}

int main(int argc, char* argv[])
{
	if (setjmp(jmp) == 0)
	{
		fun();
	}
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call { break fun }
call continue
call next
call next
call list
call line
expect { 15
}
call quit
---end---

#compile
rm -f a.out
build ${1:-$debug} foo.cpp

rm -f $config
#run_debugger $@ --main a.out segv
run_debugger $@ --main a.out usr1
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
