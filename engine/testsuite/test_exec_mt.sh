# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_exec_mt.sh 524 2008-06-13 07:51:22Z root $
#
function test_exec_mt()
{
echo ----- ${FUNCNAME}${debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

void* thread_fun(void* param)
{
	size_t count = (int)param;
	if (count == 3)
	{
		execlp("./bar", "bar", "baz", "foo", 0);
	}
	pthread_kill(pthread_self(), SIGSTOP);
	while(count--)
	{
		std::cout << pthread_self() << ": " << count << std::endl;
	}
}
int main()
{
	pid_t pid = getpid();
	std::cout << "Sleep well: " << pid << std::endl;

	pthread_t t1, t2;
	pthread_create(&t1, NULL, thread_fun, (void*)5);
	pthread_create(&t2, NULL, thread_fun, (void*)10);
	setsid();
	kill(pid, SIGSTOP);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return 0;
}
---end---
cat > ./bar.cpp << '---end---'
// bar.cpp
#include <iostream>
int main(int argc, char* argv[])
{
    for (--argc, ++argv; argc; --argc, ++argv)
    {
        std::cout << *argv << std::endl;
    }
    return 0;
}
---end---

build $@ -o bar bar.cpp 
build $@ foo.cpp -lpthread 

#write test script for the AutoTest plugin
cat > ./script << '---end---'
call test.cancel
---end---
#finally, run the test
#opts="$opts -v"
run_debugger a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    test_exec_mt -gdwarf-2
#    test_exec_mt -gstabs+
}

# run this test standalone
source suffix.sh
