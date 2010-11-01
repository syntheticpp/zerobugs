# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_cond_break.sh 717 2010-10-20 06:17:11Z root $

################################################################
#
################################################################
function test_cond_break()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
int fun(int x)
{
	int ret = x;
	++ret;
	return ret;
}
int main(int argc, char* argv[])
{
	for (int i = 0; i != 15; ++i)
	{
		fun(i);
	}
	return 0;
}
---end---
cat > auto.py << '---end---'
import zero

proc = None


def my_break(thread, breakpoint):
	file = breakpoint.symbol().filename()
	for addr in zero.debugger().line_to_addr(file, 5):
		#print ">>>>>",hex(addr)
		thread.set_breakpoint(addr)
		
def on_process(process, thread):
	for sym in process.symbols().lookup('main'):
		process.set_breakpoint(sym.addr(), my_break)

def on_new_breakpoint_action(num, bpnt, action):
	action.set_condition("x == 10")

#def on_event(event):
	#print event.type()
	#if event.type() == zero.BreakPoint:
	#	return True
	
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( x )
expect ( 10
)
call ( quit )
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

rm -f $config
run_debugger a.out --py-run=auto.py
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_cond_break -g
    else
        test_cond_break -gdwarf-2 -g3 $@
        test_cond_break -gstabs+ $@
    fi
}

source suffix.sh
