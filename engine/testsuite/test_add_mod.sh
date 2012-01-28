# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
#
function test_add_mod()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
cat > ./bar.cpp << '---end---'
////////////////////////////////////////////////////////////////
// bar.cpp
extern "C"
{
int bar(int cookie)
{
	return cookie == 42 ? 0 : -1;
}
}
---end---

cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <dlfcn.h>

int main()
{
	int result = -1;
	if (void* dso = dlopen("libbar.so", RTLD_LAZY))
	{
		int (*bar)(int) = (int(*)(int)) dlsym(dso, "bar");
		if (bar)
		{
			result = bar(42);
		}
		dlclose(dso);
	}
	return result;
}
---end---

build ${1:-$debug} -shared -fPIC bar.cpp -o libbar.so
if test "`uname`" == "FreeBSD"; then
  build ${1:-$debug} foo.cpp -L.
else
  build ${1:-$debug} foo.cpp -L. -ldl
fi
#write test script for the AutoTest plugin
cat > ./script << '---end---'
call ( addmod libbar.so )

call ( break bar )
call ( continue )
call ( next )

call ( eval cookie )
expect ( 42
)
call ( quit )
---end---

run_debugger ./a.out --main 
}


################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_add_mod -g $@
    else
        test_add_mod -gstabs+ $@
        test_add_mod -gstabs $@
        #test_add_mod -gdwarf-23 $@
        #test_add_mod -g $@
    fi
}

# run this test standalone
source suffix.sh
