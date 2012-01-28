# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_shared_ptr()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
echo $config
rm -f $config a.out foo.cpp

cat > ./foo.cpp << '---end---'

#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;

struct BBB
{
	virtual int fun(int x) const = 0;
};
struct CCC : public BBB
{
	int fun(int x) const
	{
		return 42;
	}
};
int main()
{
	boost::shared_ptr<const BBB> pb(new CCC);
	clog << pb->fun(2)<< endl;
}
---end---

################################################################
#write test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_shared_ptr #####
call next
call next
call ( pb->fun(4) )
expect ( 42
)
call quit
---end---

BOOST_INC="-I/usr/local/boost_1_33_1 -I/usr/local/include/boost-1_34"
#compile
build ${1:-$debug} ${BOOST_INC} foo.cpp
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
        test_shared_ptr -g
    else
        test_shared_ptr -gdwarf-2 $@
        test_shared_ptr -gstabs+ $@
#        test_shared_ptr -gstabs $@
    fi
}

source suffix.sh
