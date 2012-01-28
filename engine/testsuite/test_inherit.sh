# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

function test_inherit()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
if [ $GCCVER = 2 ]
then echo "this test is known to fail with gcc 2.95"; return
fi

echo $config
rm -f $config a.out foo.cpp

cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#include <typeinfo>
#include <string.h>
using namespace std;
class Interface 
{
protected:
	virtual ~Interface() { }
public:
	virtual unsigned fun() const = 0;
};
struct A : public Interface
{
	virtual int bar(int i) const = 0;
	virtual unsigned fun() const = 0;
	int a_;
};
template<typename T> struct Impl : public T
{
	virtual unsigned fun() const { return 1234; }
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
struct B : public C, public Helper<true, A>::Base
{
	B() { }
	int bar(int i) const { return i + 3; }
};
namespace XYZ
{
	struct Foo : public A
	{
		double data_;
		Foo() : data_(3.14159) { }
		unsigned fun() const
		{
			return strlen(typeid(*this).name());	
		}
		virtual int bar(int i) const 
		{
			return data_ + i;
		}
	};
}
void fun(const A* p, int)
{
	cout << p->fun() << endl;
	delete p;
}
int main()
{
	B* p = new B;
	p->c_ = 123;
	p->a_ = 456;
	fun(p, 42);
//	cout << "offsetof a=" << offsetof(B, a_) << endl;	
//	cout << "offsetof c=" << offsetof(B, c_) << endl;	
	XYZ::Foo* q = new XYZ::Foo;
	fun(q, 123);
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
#call ret
echo --- calling p->fun() ---  
#call test.cancel

call ( p->fun() )
#expect ( "aloha"
expect ( 1234
)
#call test.cancel
call ( p=>c_ )
expect ( 123
)
#call test.cancel
call ret
call ( p->c_ )
expect ( 123
)
call continue
call next
#call test.cancel
#call ( p->data_ )
#expect ( 3.14159
#)
call quit
---end---

#compile
build ${1:-$debug} foo.cpp
shift
run_debugger $@ --main ./a.out
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
