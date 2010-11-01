# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_ret_obj.sh 356 2008-02-19 07:56:19Z root $

################################################################
#
################################################################
function test_ret_obj()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <iostream>
struct SmallObject
{
	int data_;
//	long x_;
};
struct LargeObject
{
	int mydata_;
	long junk[100];
};
int get_small_value()
{
	return 123;
}
SmallObject get_small_object()
{
	SmallObject small = { 42 };
	return small;
}
LargeObject get_large_object(int x)
{
	LargeObject large = { x };
	return large;
}

int main(int argc, char* argv[])
{
	get_small_object();
	get_large_object(42);

	get_small_value();
	//std::cout << sizeof(SmallObject) << std::endl;
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( next )
f = ( frame )
#r = ( show regs )
call ( get_small_object() )
expect ( {data_=42}
)
#call ( show regs )
#expect ( $r )
call ( frame )
expect ( $f )
call ( get_large_object(123) )
expect ( {mydata_=123, junk[100]={...}}
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} -fpcc-struct-return foo.cpp
shift
rm -f $config
ZERO_MAX_ARRAY=20 run_debugger $@ --main a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_ret_obj -g
    else
        test_ret_obj -gdwarf-2 -g3 $@
        test_ret_obj -gstabs+ $@
    fi
}

source suffix.sh
