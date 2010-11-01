# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_recursive.sh 205 2007-11-22 03:52:59Z root $
# 
# Test that the 'ret' command (which translates internally into
# a Thread::step_until_current_func_returns call) works properly
# with recursive functions.
#
function test_recursive()
{
#test requires python
if [ -f "../../plugin/zpython.so" ]
then
	:
else
	return 0
fi
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'

#include <iostream>

int recursive(int x)
{
	if (x > 0)
	{
		std::cout << __func__ << ": " << x << std::endl;
		return x + recursive(--x);
	}
	return 0;
}

int main()
{
	int n = recursive(3);
	return n;
}

---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'

call ( continue )
call ( ret )
call ( ret )
call ( ret )

call ( quit )

---end---

cat > auto.py << '---end---'
#
# this is the python script part of the test
# 
import zero

def on_breakpoint(thread, breakpoint):
	thread.step(zero.Step.OverStatement)
	thread.step(zero.Step.OverStatement)
	thread.step(zero.Step.OverStatement)
	thread.step(zero.Step.Statement)
	return True


def on_process(process, thread):
	for sym in thread.symbols().lookup('recursive'):
		thread.set_breakpoint(sym.addr(), on_breakpoint)

depth = 5

def on_event(event):
	global depth
	print event.type()
	if event.type() == zero.DebugEvent.Type.CallReturned:
		depth = depth - 1
		print depth,event.thread().stack_trace().size()
		assert event.thread().stack_trace().size() == depth
	return False

---end---

build ${1:-$debug} foo.cpp

rm -f $config
run_debugger a.out --main --py-run=auto.py
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_recursive -g
    else
        test_recursive -gstabs+ $@
        test_recursive -gstabs $@
        test_recursive -gdwarf-2 $@
    fi
}

source suffix.sh
