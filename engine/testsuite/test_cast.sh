# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
function test_cstyle_cast()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
struct BaseOne { int i; };
struct BaseTwo { char c; };
struct Derived : public BaseOne, public BaseTwo { double d; };

int main()
{
    Derived d;
    d.i = 1;
    d.c = 2;
    d.d = 3.1419;

    BaseOne* pb1 = &d;
    BaseTwo* pb2 = &d;

    return d.i + pb1->i + pb2->c;
}
---end---

#compile source
build ${1:-$debug} foo.cpp

#write test script (processed by the AutoTest plugin)
param="test_cstyle_cast${1:-$debug}"
sed -e "s/PARAM/${param}/g" > ./script << '---end---'
echo # PARAM
call { exec a.out }
call { break main }
call continue
call next
call next
call next
call next
call next
call next
call pb2

#call test.cancel
call { eval (BaseTwo*)&d == pb2 }
expect { 1
}
call quit
---end---

run_debugger
}

################################################################
#
################################################################
function test_const_cast()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
if [ $GCCVER = 2 -a ${1:-$debug} = -gstabs+ ]
    then echo "this test is known not to work with gcc 2.95"; return
fi
rm -f a.out $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
#include <iostream>
using namespace std;
struct B 
{ 
    //virtual ~B() {}
    double s_; 
    B() { clog << "B()\n";}
    B(const B& b) : s_(b.s_) { clog << "B(const B&)\n"; }
};
struct C { double x_; };
struct D { double d_; short y_; };

struct A : public virtual B, public C, public D
{ 
    int i_; 
    int get() volatile const { return i_; }
    void put(int i) { i_ = i; }
};
A a;
int main()
{
    a.put(13);
    a.s_ = 0x42;
    const A& cref = a;
    A* ptr = &a;
    const A* cptr = &a;
    volatile const A* vcptr = &a;
    cout << cptr->get() << endl;
    B* bptr = &a;
    bptr->s_ = 123;
    //C* cptr = &a;
    const A* const_ptr = &a;
    ((B)a).s_ = 101;
    ((B&)a).s_ = 102;
    cout << sizeof(C) << endl;
    cout << sizeof(B) << endl;
    return cref.get() + a.get();
}
---end---

cat > ./script << '---end---'
call next
call next
call next
call next
call next
call next

call ( cptr->get() )
expect ( 13
)
call ( cptr->put(21) )
expect ( invalid cast, try again using C-style cast or reinterpret_cast: static_cast from A const* to A* const
)
call quit
---end---

build ${1:-$debug} foo.cpp
echo $@
run_debugger --main $@ ./a.out 
}


################################################################
#
################################################################
function test_member_cast()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config

cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
struct MyStruct
{
    //MyStruct() : p(0) { }
    void* p;
};
int main()
{
    MyStruct s; 
    s.p = 0;
    return s.p != 0;
}
---end---

cat > ./script << '---end---'
call { exec a.out }
call { break main }
call continue
call next
call next

#call s.p
#expect { NULL
#}

call { (MyStruct*)s.p }
expect { NULL
}
call quit
---end---

#compile source
build ${1:-$debug} foo.cpp

run_debugger
}


################################################################
#
################################################################
function test_user_conversion()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config

cat > ./foo.cpp << '---end---'
#include <math.h> // M_PI
struct XYZ 
{
	long double val_;
	XYZ(double val) : val_(val) { }
	operator long() const { return static_cast<long>(val_); }
};
int main()
{
	XYZ xyz(M_PI);
	int r = static_cast<int>(xyz);
	r = static_cast<unsigned char>(xyz);
	r = static_cast<double>(xyz);
	return r;
}
---end---

cat > ./script << '---end---'
call next
call ( ((XYZ)1.23).val_ )
expect ( 1.23
)
call next
#call test.cancel
call ( static_cast<long>(xyz) )
expect ( 3
)
call ( static_cast<double>(xyz) )
expect ( 3
)
#call test.cancel
call ( static_cast<unsigned char>(xyz) )
expect ( 3
)
call ( (XYZ)(void*)0 )
expect ( An error occurred while casting void* to XYZ
)
call quit
---end---

build ${1:-$debug} foo.cpp

run_debugger --main ./a.out
}

################################################################
#
################################################################
function test_cast()
{
    test_member_cast $@
	test_user_conversion $@
    test_cstyle_cast $@
    
	#todo -- complete writing test
    #test_const_cast $@
}


################################################################
function run()
{
	source common.sh
	
   	#not enough information in stabs, need stabs+
    test_cast -gstabs+
	test_cast -gdwarf-2
}

source suffix.sh

