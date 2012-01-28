# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_ptr_call()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
echo $config
rm -f $config a.out foo.cpp

cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#include <math.h>
using namespace std;
template<typename T>
struct P
{
	T* p_;
	P(T* p) : p_(p) { clog << p_ << endl; }
	T* operator->() const { return p_; }
};
template<typename T>
struct Proxy
{
	T* p_;
	Proxy(T* p) : p_(p) { }
	P<T> operator->() const { return p_; }
};
struct AAA
{
	long data_;
	double fun(/*long */double x)
	{
		clog << x << endl;
		return x / 2;
	}
};
struct BBB
{
	virtual int fun(int x) const volatile
	{
		return x * 2;
	}
};
struct CCC : public BBB
{
	int fun(int x) const volatile
	{
		return 42;
	}
};
int main()
{
	char foobar[] = "Foobar";
	AAA* p = new AAA;
	p->data_ = 42;
	p->fun(M_PI);
	P<AAA> sp(p);
	sp->fun(1.);
	Proxy<AAA> pp(p);
	pp->fun(2.);
	delete p;
	void* pv = foobar;
	P<BBB> pb(new BBB);
	Proxy<BBB> pc(new CCC);
	//delete pc;
	clog << pc->fun(2)<< endl;
	return pb->fun(2);
	//return pv > (void*)0xffff;
}
---end---

################################################################
#write test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_ptr_call #####
call ( break foo.cpp:53 )
call continue
call ( p->data_ )

call ( p->fun(.2) )

expect ( 0.1
)

call ( sp->data_ )
expect ( 42
)
call ( sp->fun(4.2) )

expect ( 2.1
)
call ( pp->fun(123) )
expect-i386 ( 61.5
)
call ( pp->data_ )
expect ( 42
#expect-i386 ( 42
)

call next
call next
call next
call line
echo ###########################################################
call ( foobar[3] = 0 )

call ( (char*)pv )
expect ( "Foo"
)
call frame

call ( (char*)pv + 1 )
expect ( "oo"
)

call ( ((char*)pv)[1] )
expect ( 'o'
)
call ( (int)((char*)pv)[1] )
expect ( 111
)
call ( pb->fun(2) )
expect ( 4
)
call next
call next
#call test.cancel
call ( pc->fun(0) )
#expect-i386 ( 42
expect ( 42
)
call quit
---end---

#compile
build ${1:-$debug} -fpcc-struct-return foo.cpp
shift
run_debugger $@ --main ./a.out
#ZERO_DEBUG_INTERP=1 run_debugger $@ --main ./a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_ptr_call -g
    else
        test_ptr_call -gdwarf-2 $@
        test_ptr_call -gstabs+ $@
    fi
}

source suffix.sh
