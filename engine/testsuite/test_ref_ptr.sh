# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_ref_ptr()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
echo $config
rm -f $config a.out foo.cpp

cat > ./foo.cpp << '---end---'
#include <iostream>
#include "zdk/ref_ptr.h"
//#include "zdk/ref_counted_impl.h"
#include "zdk/zobject_impl.h"

using namespace std;

struct BBB : public RefCounted
{
	virtual int fun(int x) const = 0;
};
struct CCC : public RefCountedImpl<BBB>
{
	int fun(int x) const
	{
		return 42;
	}
};
int main()
{
	//RefPtr<BBB> pb(new CCC);
	BBB* pb(new CCC);
	clog << pb->fun(2)<< endl;
}
---end---

################################################################
#write test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_ref_ptr #####
call next
call next

call ( pb->fun(4) )
expect ( 42
)
call quit
---end---

#where to search for boost
BOOST_INC="-I/usr/local/boost_1_33_1 -I/usr/local/include/boost-1_34"
#compile
build ${1:-$debug} ${BOOST_INC} -I../../zdk/include -L../../lib foo.cpp -lzdk -lpthread

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
        test_ref_ptr -g
    else
        test_ref_ptr -gdwarf-2 $@
        test_ref_ptr -gstabs+ $@
    fi
}

source suffix.sh
