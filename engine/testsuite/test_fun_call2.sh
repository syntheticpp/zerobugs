# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_fun_call2.sh 205 2007-11-22 03:52:59Z root $
#
################################################################
function test_fun_call2()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config a.out foo.cpp
cat > ./foo.cpp << '---end---'
// foo.cpp
#include <errno.h>
struct XYZ
{
	bool crash_;
	double snow_;
	double fun(int x) const
	{
		if (crash_)
		{
			*(int*)x = 1234;
		}
		return x;
	}
	explicit XYZ(bool f) : crash_(f), snow_(123.)
	{ }
	operator double() const { return snow_; }

};
inline long long add(long double x, long long y)
{
	return x + y;
}
inline long add2(long x, long y)
{
	return x + y;
}
int foo(XYZ& xoxo, double d) 
//int foo(XYZ& xoxo, long d)
{
	long n = d;
	return xoxo.fun(((long)&n, d));
}
int bar(XYZ& x)
{
	return foo(x, 56.78);
}
int main()
{
	bool flag = true;
	XYZ x(flag);
	add((double)-999, 1000LL);
	bar(x);
	add2(1, x);
	return (long)&bar;
}
---end---

################################################################
#write test script for the AutoTest plugin
cat > ./script << '---end---'
echo ##### test_fun_call2 #####
call test.cancel
call quit
---end---

build ${1:-$debug} -O -finline-functions foo.cpp
#build ${1:-$debug} -finline-functions foo.cpp
#build ${1:-$debug} -funit-at-a-time foo.cpp
ZERO_INLINE_HEURISTICS=1 ZERO_USE_FRAME_HANDLERS=1 run_debugger --main ./a.out $@
#run_debugger --main ./a.out $@
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_fun_call2 -g
    else
        test_fun_call2 -gdwarf-2 $@
#       test_fun_call2 -gstabs+ $@
    fi
}

source suffix.sh
