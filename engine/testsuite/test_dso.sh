# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_dso.sh 398 2008-03-12 07:05:17Z root $
#
# Testing two things:
# 1)expression evaluation of an object that is defined
# in a DSO (dwarf location expression should yield correct value)
# 2)a breakpoint inside the DSO is restored correctly
#
function test_dso()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
cat > ./bar.cpp << '---end---'
////////////////////////////////////////////////////////////////
// bar.cpp
#include <math.h>
#include <cstdlib>
#include <iostream>
class Foo
{
protected:
	virtual ~Foo() { }
	Foo() : d_(M_PI){ }
	virtual int boom(bool) const = 0;
	double d_;
};
class Bar : public Foo
{
	int bar_;
public:
	explicit Bar(int bar);
	virtual int boom(bool crash) const 
	{ 
		if (crash)
			*(int*)0 = 1;
		return bar_; 
	}
};
Bar::Bar(int bar) : bar_(bar)
{
}
int bar(bool crash)
{
	//std::cout << __func__ << std::endl;
	Bar b(42);
	return b.boom(crash);
}
---end---
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <unistd.h>
#include <iostream>
extern int bar(bool);
static bool do_foo = false;
int foo()
{
	sleep(5);
	std::clog << __func__ << " returning\n";
	return 25;
}
class CallBar 
{
public:
	virtual int bar() const 
	{
		return bar();
	}
};
	
int main(int argc, char* argv[], char* env[])
{
	while (env && *env) ++env;
	if (do_foo)
	{
		foo();
	}
	int result = bar(argc > 1);
	//CallBar* foo = new CallBar;
	//int result = foo->bar();
	return result;
}
---end---
build ${1:-$debug} -shared bar.cpp -fPIC -o libbar.so
build ${1:-$debug} foo.cpp -L. -lbar
shift

#write test script for the AutoTest plugin
cat > ./script << '---end---'
#
# Test that evaluating a function call correctly
# interrupts a pending system call:
#
#call ( do_foo = true )
#call ( continue timer 900 )
#call ( bar(0) )
#expect ( 42
#)
call ( break /all bar )
call ( continue )
call ( break main )

call ( next )
call ( next )
call ( eval b.bar_ )
expect ( 42
)

call ( detach /y )
#
# Breakpoint in function bar should be persisted
#
call ( exec a.out )
call ( continue )
#expect to break in function bar
call next
call next
call ( eval b.bar_ )
expect ( 42
)
call continue
#run until program exits, then do it again
#to see if the breakpoint was saved allright

call ( exec a.out )
call continue
#expect to break in function bar
call next
call next
call ( eval b.bar_ )
expect ( 42
)

call quit
---end---
ZERO_SAVE_STATE=1 run_debugger ./a.out --main $@
touch libbar.so
ZERO_SAVE_STATE=1 run_debugger ./a.out --main $@
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_dso -g $@
    else
        #test_dso -gstabs+ $@
        #test_dso -gstabs $@
        test_dso -ggdb $@
    fi
}

# run this test standalone
source suffix.sh
