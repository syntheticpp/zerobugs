# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_inh.sh 205 2007-11-22 03:52:59Z root $

################################################################
#
################################################################
function test_inherit()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
echo $config
rm -f $config abc a.cpp

cat > ./a.cpp << '---end---'
////////////////////////////////////////////////////////////////
// a.cpp
#include <iostream>
using namespace std;
class Interface 
{
protected:
	virtual ~Interface() { }
public:
	virtual const char* fun() const = 0;
};
struct A : public Interface
{
	virtual int bar(int i) const = 0;
	int a_;
};
template<typename T> struct Impl : public T
{
	virtual const char* fun() const { return "aloha"; }
};
template<bool, typename T> struct Helper
{
	struct Base : public T
	{
		virtual void barfoo() { }
	};
};
template<typename T> struct Helper<true, T>
{
	struct Base : public Impl<T>
	{
		virtual ~Base() { }
		
		virtual void foobar() { }
	};
};
struct C
{
	long c_;
};
template<typename T>
struct Wrap : public T
{
	int wrap_;
};
struct B : public C, public Helper<true, Wrap<A> >::Base
{
	virtual ~B() {}
	B() { }
	int bar(int i) const { return i + 3; }
};
void fun(const A* p, int)
{
	cout << p->fun() << endl;
	delete p;
}
int main()
{
	//B x;
	B* p = new B;
	p->a_ = 456;
	fun(p, 42);
//	cout << "offsetof a=" << offsetof(B, a_) << endl;	
//	cout << "offsetof c=" << offsetof(B, c_) << endl;	
	return 0;
}
---end---

################################################################
#write test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_inherit #####
call ( break fun )
call continue
call next
call list
call ( p->fun() )
expect ( "aloha"
)
call test.cancel
call ( p->c_ )
expect ( 123
)
#call test.cancel
call quit
---end---

#compile
build ${1:-$debug} a.cpp -o abc

run_debugger $@ --main ./abc
rm abc
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_inherit -g
    else
        test_inherit -gdwarf-2 $@
        test_inherit -gstabs+ $@
    fi
}

source suffix.sh
