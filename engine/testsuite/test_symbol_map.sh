# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_symbol_map.sh 414 2008-03-23 22:52:57Z root $

ubuntu_msg="
*******************************************************************\n
In Maverick Meerkat (10.10) Ubuntu introduced a patch to disallow\n
ptracing of non-child processes by non-root users - only a\n
process which is a parent of another process can ptrace it for\n
normal users - whilst root can still ptrace every process.\n
\n
Disable this restriction by executing:\n
\n
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope\n
*******************************************************************\n
"

################################################################
#
################################################################
function test_symbol_map()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
if test `cat /proc/sys/kernel/yama/ptrace_scope` = 1
	then echo -e $ubuntu_msg; return 1
fi

cat > foo.cpp << '---end---'
// foo.cpp
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	if (const char* abi = getenv("LD_ASSUME_KERNEL"))
	{
		std::cout << "abi=" << abi << std::endl;
	}
	sleep(10);
	std::cout << "hello " << argv[0] << std::endl;
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'

call ( show modules /unloaded )
expect (
)
call quit
---end---

function aout_pid()
{
	ps -ef | grep a.out | grep -v grep | grep -v defunct | \
		sed -r -e 's/[ ]+/ /g' | cut -d' ' -f2
}
#compile
killall -9 a.out
rm a.out
build ${1:-$debug} foo.cpp
shift
./a.out&
PID=`aout_pid`
echo $PID
rm -f $config

if (${path}/bin/zero $PID $opts); then :
	else return 1
fi

kill -9 $PID 2>/dev/null  || true
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_symbol_map -g
    else
        test_symbol_map -gdwarf-2 -g3 $@
        test_symbol_map -gstabs+ $@
    fi
}

source suffix.sh
