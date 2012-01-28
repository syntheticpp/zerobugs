# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
################################################################
function test_no_sym()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

static char* code = NULL;

void mess_up_stack()
{
	void* a[0];
	a[1] = code;
}

int main()
{
	if (posix_memalign((void**)&code, 4096, 4096) == 0)
	{
		memset(code, 0x90, sizeof (long));
		mprotect(code, 4096, PROT_READ | PROT_EXEC);
		mess_up_stack();
	}
}
---end---

cat > script << '---end---'
call ( test.cancel )
call ( quit )

---end---

#compile

build ${1:-$debug} foo.cpp

rm -f $config
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
        test_no_sym -g
    else
        test_no_sym -gdwarf-2 $@
        #test_no_sym -gstabs+ $@
    fi
}

source suffix.sh
