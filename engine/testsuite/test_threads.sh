# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_threads.sh 645 2009-08-13 07:35:55Z root $
#
function test_threads()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>

#define SLEEP_USEC 500
using namespace std;

typedef std::vector<int> IntVect;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static IntVect vect;
static bool allProducersDone = false;

void debug_hook(const char* func)
{
}
void* producer(void* p)
{
	const int n = *(int*)p;
	for (int i = 0; i < n; ++i)
	{
		int val = (RAND_MAX * double(i)) / (rand() + .1);
		if (pthread_mutex_lock(&mutex) == 0)
		{
			vect.push_back(val);
			usleep(SLEEP_USEC);
			debug_hook(__func__);
			pthread_mutex_unlock(&mutex);
		}
	}
	cout << __func__ << ": done\n";
	if (pthread_mutex_lock(&mutex) == 0)
	{
		vect.push_back(0);
		allProducersDone = true;
		pthread_mutex_unlock(&mutex);
	}

	return 0;
}
void* consumer(void* p)
{
	const int n = *(int*)p;
	for (int i = 0; i < n; ++i)
	{
		if (pthread_mutex_lock(&mutex) == 0)
		{
			if (!vect.empty())
			{
				int val = vect.back();
				vect.pop_back();

				if (allProducersDone)
				{
					i = n;
				}
			}
			pthread_mutex_unlock(&mutex);
			//cout << __func__ << ": " << val << endl;
			usleep(10);
		}
	}
	cout << __func__ << ": done\n";
	return 0;
}
int main(int argc, char* argv[])
{
	pthread_t p1, p2, p3, p4, p5, p6, c1, c2;
	int n = 100;
	
	srand(time(NULL));

	pthread_create(&p1, NULL, producer, &n);

	//pthread_create(&p2, NULL, producer, &n);
	//pthread_create(&p3, NULL, producer, &n);
	//pthread_create(&p4, NULL, producer, &n);
	//pthread_create(&p5, NULL, producer, &n);
	//pthread_create(&p6, NULL, producer, &n);
	n /= 2;
	pthread_create(&c1, NULL, consumer, &n);
	//pthread_create(&c2, NULL, consumer, &n);
	pthread_join(p1, NULL);

	//pthread_join(p2, NULL);
	//pthread_join(p3, NULL);
	//pthread_join(p4, NULL);
	//pthread_join(p5, NULL);
	//pthread_join(p6, NULL);
	pthread_join(c1, NULL);
	//pthread_join(c2, NULL);
	return 0;
}
---end---
echo "############# ${2:-dynamic} ################"
build ${1:-$debug} ${2:-} foo.cpp -lpthread

#write test script for the AutoTest plugin
cat > ./script << ---end---
addr = ( /x (long)&debug_hook )
call ( break debug_hook )
call ( continue			)
call ( show threads /count )
#may get 4 on 2.4 kernels
expect ( 3
) or ( 4
) or ( 2
)
call ( next 			)
call ( eval func 		)
expect ( "producer"
)
call next
call next
#call test.cancel
call ( eval i )
expect ( 0
)
call ( clear \$addr )
call ( continue		)
call ( quit			)

---end---

#run_debugger ./a.out --main --log-verbose $@
run_debugger ./a.out --main
}

function test_threads_static()
{
	test_threads $1 -static
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
        #test_threads -gdwarf-2
        #test_threads -gstabs+
        #test_threads_static -gdwarf-2
        test_threads_static -gstabs+
    fi
}

# run this test standalone
source suffix.sh
