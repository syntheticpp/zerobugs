# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
# Test that the environment of a crashed process is read correctly
# from core dumps
#
function test_core_env()
{
#test requires python plugin
if [ -f "../../plugin/zpython.so" ]
then
	:
else
	return 0
fi
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -rf core.*
cat > foo.cpp << '---end---'
// foo.cpp
void crash_me()
{
	*(int*)0 = 1;
}
int main()
{
	crash_me();
}
---end---

cat > script << '---end---'
call ( where )
call ( quit )

---end---

cat > auto.py << '---end---'
import zero

def on_process(process, thread):
	print '-----',process.name(),'-----'
	env=process.environ()
	for name in env:
		print name,'=',env[name]
	assert env['FOOZY'] == 'bear'
	assert env['WOOZY'] == 'was=he'

---end---

#compile
rm -f a.out core*
build ${1:-$debug} foo.cpp

ulimit -c unlimited
LS_COLORS= FOOZY=bear WOOZY="was=he" ./a.out foozy
mv core* core 2>/dev/null || mv -f a.out.core core 2>/dev/null || true

rm -f $config
run_debugger core --py-exec=auto.py 
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_core_env -g
    else
        test_core_env -gdwarf-2 $@
        test_core_env -gstabs+ $@
        test_core_env -gstabs $@
    fi
}

source suffix.sh
