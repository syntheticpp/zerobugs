# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
function test_attach()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
killall -9 a.out 2>/dev/null
sleep 1
rm -rf .zero a.out
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
void wake_up(int param)
{
	std::cout << "Good morning" << std::endl;
}
void* thread_fun(void* param)
{
	size_t count = (long)param;
	usleep(1000);
	//pthread_kill(pthread_self(), SIGSTOP);
	while(count--)
	{
		std::cout << pthread_self() << ": " << count << std::endl;
		usleep(1000);
	}
}
int main()
{
	pid_t pid = getpid();
	std::cout << "Sleep well: " << pid << std::endl;

	pthread_t t1, t2;
	pthread_create(&t1, NULL, thread_fun, (void*)450);
	pthread_create(&t2, NULL, thread_fun, (void*)500);
	setsid();
	kill(pid, SIGSTOP);

	wake_up(42);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return 0;
}
---end---

build ${1:-$debug} foo.cpp -lpthread
shift

#write test script for the AutoTest plugin
cat > ./script << '---end---'
#call { break wake_up }
call { show threads /count }
expect { 3
}
#call test.cancel
#call param
#expect { 42
#}
#call test.cancel
#call detach
call quit

---end---
function aout_pid()
{
	ps -ef | grep a.out | grep -v grep | grep -v defunct | \
		sed -r -e 's/[ ]+/ /g' | cut -d' ' -f2
}

./a.out&
PID=`aout_pid`
echo pid=$PID
if [ -n "$PID" ]
then
	#run_debugger $PID
	${path}/bin/zero $PID $opts
else
	echo "No such process";
	return 1
fi
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    #test_attach -gstabs $@
    #test_attach -gstabs+ $@
    test_attach -gdwarf-2 $@
}

# run this test standalone
source suffix.sh
