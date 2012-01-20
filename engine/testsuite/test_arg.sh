# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_arg.sh 666 2009-11-09 06:06:18Z root $
#
function test_arg()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
/**
 * This is interesting for x86-64: make sure all params are
 * passed correctly (by registers)
 */
int foo(int a1, long a2, unsigned a3, long a4, short a5, int a6)
{
	return a1 / 2 + a2 * 2 + a3 + a4 + 5 + a6;
}
struct Bar
{
	long a;
	double b;
	int c;
	Bar() : a(1), b(2),c(3) { }
	//Bar(const Bar& other) : a(other.a), b(other.b) { }
};

long double bar(Bar b, long x = 0)
//float bar(Bar b, long x = 0)
{
	//std::cout << b.a << ", " << b.b << ", " << x << std::endl;
	return b.a + b.b + x;
}

int main(int argc, char* argv[])
{
	std::cout << argc << std::endl;

	Bar b;
	std::clog << sizeof(b) << std::endl;
	int x = 2;
	x = foo(x, x / 2, 3, 4, 5, 6);
	return bar(b);
}
---end---

build ${1:-$debug} $@ foo.cpp

#write test script for the AutoTest plugin
cat > ./script << '---end---'

call ( next )
call ( eval argc )
expect ( 2
)
call ( eval argv[1] )
expect ( "1"
)

call ( next )
call ( next )
call ( next )

call ( bar(b, 2) )
expect ( 5
)
call ( break foo )
call ( foo(1, 2, 3, 4, 5, 6) )
call ( next )

call ( a1 )
expect ( 1
)
call ( a2 )
expect ( 2
)
call ( a3 )
expect ( 3
)
call ( a4 )
expect ( 4
)
call ( a5 )
expect ( 5
)
call ( a6 )
expect ( 6
)
call ( %rdi )
expect-x64 ( 1
)
call ( %rsi )
expect-x64 ( 2
)
call ( %rdx )
expect-x64 ( 3
)
call ( %rcx )
expect-x64 ( 4
)
call ( %r8 )
expect-x64 ( 5
)
call ( %r9 )
expect-x64 ( 6
)
call ( quit )
---end---

ZERO_START_MAIN=1 run_debugger ./a.out -- 1
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
        test_arg -gdwarf-2
        test_arg -gstabs+
    fi
}

# run this test standalone
source suffix.sh
