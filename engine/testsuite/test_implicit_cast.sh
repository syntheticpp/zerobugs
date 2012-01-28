# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_implicit_cast()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <assert.h>
#include <iostream>
struct Foo
{
	Foo(const Foo&);

	Foo() : val_(0) { }
	long val_; 
};
Foo::Foo(const Foo& that) : val_(that.val_) 
{ std::cout << "Foo copy ctor\n";  }
struct Bar
{
	// Foo foo_;
	//Foo& operator()() { return foo_; }

	// operator Foo&() { return *(Foo*)this; }
	operator const Foo&() const { return *(Foo*)this; }
};
Foo* foobar(Foo& foo)
{
	return &foo;
}
const Foo* barfoo(const Foo& foo)
{
	return &foo;
}
const Foo* fred(double eps, Foo foo)
{
	return eps <= .5 ? &foo : 0;
}
int main()
{
	Bar bar;
	Foo foo;
	//foo.val_ = 42;
	foobar(foo);
	barfoo(bar);
	assert(fred(.0, foo) != &foo);
	//fred(.0, bar);
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call next
call list

#call test.cancel

const_addr = ( (const Foo*)&bar )
call ( eval foobar(bar) )
expect ( invalid cast, try again using C-style cast or reinterpret_cast: static_cast from Bar to Foo& const
) 
or ( invalid cast, try again using C-style cast or reinterpret_cast: static_cast from Bar to Foo&
)
call ( eval barfoo(bar) )
expect ( $const_addr )
call quit
---end---

#compile
rm a.out
build ${1:-$debug} -fno-elide-constructors foo.cpp
#build ${1:-$debug} foo.cpp

rm -f $config
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
        test_implicit_cast -g
    else
        test_implicit_cast -gdwarf-2 -g3 $@
        test_implicit_cast -gstabs+ -g3 $@
    fi
}

source suffix.sh
