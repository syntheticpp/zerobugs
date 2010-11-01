# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_global_bp.sh 205 2007-11-22 03:52:59Z root $

################################################################
#
################################################################
function test_global_bp()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <pthread.h>
#include <unistd.h>

void do_something()
{
	for (int i = 0; i != 100; ++i)
	{
		usleep(1000);
	}
}

void* thread_proc(void*)
{
	do_something();
	return 0;
}

void do_nothing()
{
}

int main()
{
	pthread_t t1, t2;

	pthread_create(&t1, 0, thread_proc, 0);
	pthread_join(t1, 0);

	do_nothing();

	pthread_create(&t2, 0, thread_proc, 0);
	pthread_join(t2, 0);
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'

call ( break do_nothing )
call ( break thread_proc )
call ( continue )

call ( break do_something )
call ( continue )

call ( show threads )
call ( show breakpoints )

call ( continue )
call ( show threads )
call ( show breakpoints )

call ( continue )
call ( show threads )
call ( show breakpoints )

call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp -lpthread

rm -f $config
run_debugger --main a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_global_bp -g
    else
        test_global_bp -gdwarf-2 $@
        #test_global_bp -gstabs+ $@
    fi
}

source suffix.sh
