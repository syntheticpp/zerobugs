# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
# WARNING: fails with frame-handlers enabled!
#
function test_throw()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config a.out foo.cpp
cat > ./foo.cpp << '---end---'
// foo.cpp
#include <errno.h>
#include <stdexcept>
#include <iostream>
using namespace std;
struct XYZ
{
	bool crash_;
	double snow_;
	double fun(int x) const;

	explicit XYZ(bool f) : crash_(f)
	{ }
};
double XYZ::fun(int x) const
{
    if (crash_)
    {
        throw runtime_error("boogaboo");
    }
    return x;
}
long long add(long double x, long long y)
{
	return x + y;
}
double bar(XYZ& x)
{
	return x.fun(56.78) + 3.14159;
}
int main()
{
	try 
	{
		bool flag = true;
		XYZ x(flag);
		add((double)-999, 1000LL);
		double y = bar(x);
		return y;
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
}
---end---

################################################################
#write test script for the AutoTest plugin
cat > ./script << '---end---'
echo ##### test_throw #####

call ( break __cxa_throw )
call continue

call ( instruction )
call ( instruction )
call ( instruction )
####call ( test.cancel )
call ( frame 1 )
call ( x )
expect ( 56
)
call quit
---end---

build ${1:-$debug} foo.cpp

ZERO_USE_FRAME_HANDLERS=1 run_debugger --main --break-on-throw ./a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_throw -g
    else
        test_throw -gdwarf-2 $@
        test_throw -gstabs+ $@
    fi
}

source suffix.sh
