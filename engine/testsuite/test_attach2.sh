# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_attach2.sh 205 2007-11-22 03:52:59Z root $
#
function test_attach2()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config a.out
cat > ./bar.cpp << '---end---'
int fred(int i)
{
	i *= 42;
	return i;
}
---end---
build ${1:-$debug} -fPIC -shared bar.cpp -o libfred.so

cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
//#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
int fred(int);
int bar(int i, void* handle)
{
	//if (handle)
	//	dlclose(handle);
	return fred(i);
}
int main(int argc, char* argv[])
{
	std::clog << "My PID is: " << getpid() << std::endl;
	sleep(3);
	void* handle = 0; // = dlopen("libcrypt.so", RTLD_LAZY);
	//std::clog << "handle=" << handle << std::endl;
	return bar(argc, handle);
}
---end---

mkdir -p fred
mv libfred.so fred
build ${1:-$debug} -Lfred foo.cpp -lfred

#write test script for the AutoTest plugin
cat > ./script << '---end---'
call ( break bar )
echo Breakpoint set at function bar()
call continue
call ( break fred )
echo Breakpoint set at function fred()
call continue
call next
call next
call ( eval i )
expect ( 42
)
call quit

---end---

function aout_pid()
{
	ps -ef | grep a.out | grep -v grep | grep -v defunct | \
		sed -r -e 's/[ ]+/ /g' | cut -d' ' -f2
}


killall -9 a.out
cd ..
LD_LIBRARY_PATH=testsuite/fred testsuite/a.out&
# The SED part compresses white spaces 
ps -ef | grep a.out | grep -v grep | sed -r -e 's/[ ]+/ /g'
PID=`aout_pid`
echo pid=\"$PID\"
cd testsuite
run_debugger $PID 

killall -9 a.out
cd fred
LD_RUN_PATH=. ../a.out&
#LD_LIBRARY_PATH=. ../a.out&
#ps -ef | grep a.out
PID=`aout_pid`
echo pid=\"$PID\"
cd ..
run_debugger $PID 

}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    test_attach2 -gstabs $@
    test_attach2 -gstabs+ $@
    test_attach2 -gdwarf-2 $@
    test_attach2 -gdwarf-2 $@ -feliminate-dwarf2-dups
}

# run this test standalone
source suffix.sh

rm -rf fred
