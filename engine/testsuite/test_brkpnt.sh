# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_brkpnt.sh 319 2008-01-28 09:44:09Z root $

################################################################
#
################################################################
function test_brkpnt()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
int sqr(int i)
{
	return i * i;
}
int main(int argc, char* argv[])
{
	sqr(argc);
	return 0;
}
---end---

cat > auto.py << '---end---'
import zero

removed = False
inserted = False

def on_breakpoint_inserted(sym, type):
	print type, "Breakpoint inserted at %x" % sym.addr()
	global inserted
	inserted = True

def on_breakpoint_deleted(sym, type):
	print type, "Breakpoint removed from %x" % sym.addr()
	global removed
	removed = True

def on_process_detach(process):
	if process:
		print "process %d detached" % process.pid()
	global inserted
	global removed
	assert inserted
	assert removed
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
addr = ( /x (long)&sqr )
call ( break sqr )
call ( clear $addr )
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

rm -f $config
run_debugger --main a.out --py-run=auto.py
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_brkpnt -g
    else
        test_brkpnt -gdwarf-2 $@
        test_brkpnt -gstabs+ $@
    fi
}

source suffix.sh
