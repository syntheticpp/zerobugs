# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_segv_2.sh 656 2009-10-26 03:42:56Z root $
#
function test_segv_2()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <signal.h>
#include <stdlib.h>
#include <string.h>
int signum = 0;

void my_handler(int sig)
{
	signum = sig;
	abort();
}
void my_handler2(int sig, siginfo_t*, void*)
{
	signum = sig;
	abort();
}
void crash(double val)
{
    for (int i=0; i != 42; ++i)
        *(int*)3 = val;
}
struct ABC
{
	int foo_;

	virtual int baz() const
	{
		if (foo_)
			crash(3.1419);
		return 100;
	}
};
int main()
{
#if 1
	signal(SIGSEGV, my_handler);
#else
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = my_handler;
	sa.sa_sigaction = my_handler2;
	// sa.sa_flags = SA_SIGINFO;

	sigaction(SIGSEGV, &sa, 0);
#endif
	ABC abc;
	abc.foo_ = 42;
	abc.baz();
	return 0;
}
---end---
#compile the test program
build ${1:-$debug} foo.cpp -lm
shift
#ulimit -c unlimited
#rm -f core* a.out.core
#./a.out
#if eval mv core* core
#	then :
#	else mv -f a.out.core core
#fi

cat > ./script << '---end---'
echo ##### test_segv_2 #####
#call { handle 11 ignore }
#call { loadcore core }
#call next
call { exec a.out }
call { continue }

#call { continue }
#call { where }

#call { frame signal }
#call { up }
call { eval val }
expect { 3.1419
}

call quit

---end---
run_debugger --main $@
}

function run()
{
    source common.sh
    test_segv_2 -gdwarf-2 $@
    #test_segv_2 -gstabs $@
    #test_segv_2 -gstabs+ $@
}

source suffix.sh
