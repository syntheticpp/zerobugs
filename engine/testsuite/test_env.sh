# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_env()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <assert.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	const char* v = getenv("__FOOBAR__");
	v = "barfoo";
	setenv("__FOOBAR__", v, 1);
	v = getenv("__FOOBAR__");
	assert(v);
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( next 	)
call ( next 	)
call ( eval v 	)
expect ( NULL
)
call ( next 	)
call ( next 	)
call ( restart 	)
call ( next 	)
call ( next 	)
call ( eval v 	)
expect ( NULL
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp
#rm -f $config
run_debugger --main a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_env -g
    else
        test_env -gdwarf-2 -g3 $@
        test_env -gstabs+ $@
    fi
}

source suffix.sh
