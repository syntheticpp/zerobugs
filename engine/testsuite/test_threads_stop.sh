# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
#
function test_threads_stop()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > ./foo.cpp << '---end---'
// foo.cpp

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <wait.h>

#include <vector>
#include <iostream>

using namespace std;

static size_t depth = 5;

static vector<pthread_t> threads;
static bool done = false;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pid_t get_thread_id()
{
    return syscall(__NR_gettid);

}


void* thread_entry(void* data)
{
    clog << getpid() << ", tid=" << get_thread_id() << ": depth=" << depth << ", data=" << data << endl;

    if  (--depth)
    {
        pthread_t t;

        if (pthread_create(&t, NULL, thread_entry, data) == 0)
        {
            if (pthread_mutex_lock(&mutex) == 0)
            {
                threads.push_back(t);
                pthread_mutex_unlock(&mutex);
            }
        }
    }
    while (!done)
    {
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    thread_entry(NULL);

	for (size_t i = 0; i != threads.size(); ++i)
	{
		pthread_join(threads[i], NULL);
	}
	return 0;
}
---end---
build ${1:-$debug} ${2:-} foo.cpp -lpthread

#write test script for the AutoTest plugin
cat > ./script << ---end---
call ( yield 3 )
call ( eval done )
expect ( false
)
call ( eval done=true )
call ( continue )
call ( quit )

---end---

run_debugger ./a.out --main --syscall-trace
}

function test_threads_stop_static()
{
	test_threads_stop $1 -static
}


################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
       	echo todo: icc test?
    else
        test_threads_stop -gdwarf-2
        test_threads_stop -gstabs+
        test_threads_stop_static -gdwarf-2
        test_threads_stop_static -gstabs+
    fi
}

# run this test standalone
source suffix.sh

