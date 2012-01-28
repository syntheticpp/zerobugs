# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_dl_core()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
extern "C" void do_something()
{
}
void crash()
{
	*(int*)0 = 1;
}

#if 0 //__GNUC__ >= 4
void __attribute__((constructor)) initialize()
#else
extern "C" void _init()
#endif
{
	try
	{
		crash();
	}
	catch (...)
	{
	}
}
---end---
cat > bar.cpp << '---end---'
// bar.cpp
#include <dlfcn.h>

extern "C" void do_something();
int main()
{
#if 0
	void* handle = dlopen("foo_so", RTLD_LAZY);
	if (handle)
	{
		dlclose(handle);
	}
#endif
	do_something();
	return 0;
}
---end---
cat > script << '---end---'
call ( quit )

---end---

cat > auto.py << '---end---'
import zero

def on_event(event):
	thread=event.thread()
	for frame in thread.stack_trace().frames():
		if frame.index() == 2:
			sym=frame.function()
			#assert sym.demangle() == 'call_init'
			print sym.filename()
			assert sym.filename().find("/ld-") > -1
	return 0

---end---
#compile

build ${1:-$debug} -nostdlib -fPIC -shared foo.cpp -o libfoo.so

#build ${1:-$debug} -ldl bar.cpp 
build ${1:-$debug} -L. bar.cpp -lfoo

rm -f $config

rm -rf core.*
source gen_core.sh
run_debugger --no-trace-fork core --py-run=auto.py
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_dl_core -g
    else
        test_dl_core -gdwarf-2 $@
        test_dl_core -gstabs+ $@
    fi
}

source suffix.sh
