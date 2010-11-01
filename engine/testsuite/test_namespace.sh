# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_namespace.sh 356 2008-02-19 07:56:19Z root $
#

function test_namespace()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <memory>
using namespace std;
namespace Fred
{
	struct A
	{
		short x_;
		static int fred;
	};
}
int Fred::A::fred = 123;

int main(int argc, char* argv[])
{
	Fred::A a = { 123 };
	auto_ptr<Fred::A> pa(new Fred::A);
	pa->x_ = 42;
	return a.x_;
}
---end---
build ${1:-$debug} foo.cpp
shift
#write test script for the AutoTest plugin
cat > ./script << '---end---'
call next
call next
call next
call next
call ( eval a.x_ )
expect ( 123
)
call ( eval pa->x_ )
expect ( 42
)
call ( eval Fred::A::fred )
expect ( 123
)
#call test.cancel
call quit
---end---

run_debugger ./a.out --main $@
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
        test_namespace -gstabs+
        test_namespace -gdwarf-23
    fi
}

# run this test standalone
source suffix.sh
