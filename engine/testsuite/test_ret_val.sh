# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
# Test return values, walk up and down the stack
################################################################
function test_ret_val()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
// foo.cpp
#include <assert.h>
#include <math.h>
int foo(int i, int j = 0)
{
	return i + 1 + j;
}
long long bar(long long i, double trouble)
{
	long long x = i + trouble;
	int y = foo(x);
	assert(y == 46);
	if (y>trouble)
	{
		return ++i;
	}
	return trouble;
}
int main()
{
	int r = foo(41);
	bar(42, M_PI);
	return r;
}
---end---
#write test script for the AutoTest plugin
cat > ./script << '---end---'
call { exec a.out }
call { break main }
# need to specify LL,
call { eval 181462966779LL }
expect { 181462966779
}
call { eval 181462966779 }
expect-x64 { 181462966779
}
call { eval 181462966779 }
expect-i386 { integer constant is too large for 'long' type
}
call continue 
call next
call step

call next
call ret
call { print /ret }
expect { foo=42
}
#call next
call step
#call test.cancel
call next
call next
call step
call next

call { eval i }
expect { 45
}


call { up }
call { eval i }
expect { 42
}

call { eval foo(123, 0) }
expect { 124
}

call down
call ret

call { print /ret }
expect { foo=46
}

call { eval i }
expect { 42
}

call quit
---end---
#compile source
build ${1:-$debug} foo.cpp
shift
#finally, run the test
run_debugger $@
#ZERO_USE_FRAME_HANDLERS=1 run_debugger $@
}
function run()
{
    source common.sh
	#test_ret_val -fomit-frame-pointer -gdwarf-2
	test_ret_val -gdwarf-2 -g3 $@
	test_ret_val -gstabs
	test_ret_val -gstabs+
}
source suffix.sh
