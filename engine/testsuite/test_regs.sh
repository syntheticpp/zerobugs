# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_regs.sh 205 2007-11-22 03:52:59Z root $
#
function test_regs()
{
echo ----- ${FUNCNAME}${debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>

long double fun(double d, long long x)
{
	std::clog << __func__ << ": " << d << std::endl;
	double r = rand();
	time_t t = time(0);
	r /= t;

	d *= M_PI;
	x -= t;
	return pow(d, x);
}
int main()
{
	time_t t = time(0);
	srand(t);
	long double r = rand();
	r = fun(r, t);
	return 0;
}
---end---

build $@ foo.cpp

################################################################
#write test script for the AutoTest plugin
################################################################
cat > ./script << '---end---'

call next
call next
call next
call next
# Test that registers are restored correctly by the
# expression evaluator
regs = { show regs /all }

# call a function inside the debugged program
call { fun((double)M_PI, 42) }
call { show regs /all }
expect { $regs }

# chain two calls
call { fun(fun((double)M_PI, 42), 50) }
call { show regs /all }
expect { $regs }

call quit
---end---

#finally, run the test
run_debugger --main ./a.out
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
        test_regs -gdwarf-23
    fi
}

# run this test standalone
source suffix.sh
