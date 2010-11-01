# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_next.sh 205 2007-11-22 03:52:59Z root $

################################################################
#
################################################################
function test_next()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <iostream>
#include <signal.h>
#include <unistd.h>

using namespace std;


void handle(int signum)
{
	clog << __func__ << ": " << signum << endl;
}

void farg(int argc, char* argv[])
{
	for (--argc, ++argv; argc; --argc, ++argv)
	{
		if (strcmp(*argv, "segv") == 0)
		{
			*(int*)0 = 1;
		}
		else if (strcmp(*argv, "usr1") == 0)
		{
			kill(getpid(), SIGUSR1);
		}
	}
}

int main(int argc, char* argv[])
{
	signal(SIGUSR1, handle);
	signal(SIGSEGV, handle);
	farg(argc, argv);

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
        test_next -g
    else
        test_next -gdwarf-2 -g3 $@
        test_next -gstabs+ $@
    fi
}

source suffix.sh
