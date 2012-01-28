#!/bin/sh
# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#

function test_core2()
{
echo ----- ${FUNCNAME}${debug} ----- >> $logfile
rm -f $config foo.cpp a.out
cat > ./foo.cpp << '---end---'
// foo.cpp
#include <pthread.h>
// infinite recursion crash
int crash_me(int x)
{
	return crash_me(x + 1);
}
void* thread(void* p)
{
	crash_me(*(int*)p);
	return 0;
}
int main(int argc, char* argv[])
{
	pthread_t t;
	pthread_create(&t, NULL, thread, &argc);

	pthread_join(t, NULL);
	return 0;
}
---end---
#compile the test program
build ${1:-$debug} foo.cpp -lpthread

ulimit -c unlimited
rm -f core* a.out.core
./a.out
if eval mv core* core
then :
else mv -f a.out.core core
fi

cat > ./script << '---end---'
echo ##### test_core2 #####
call list
call { show regs /all }
call test.cancel

---end---
run_debugger core
}

function run()
{
	source common.sh
	test_core2 -gdwarf-2
	test_core2 -gstabs+
}

source suffix.sh
