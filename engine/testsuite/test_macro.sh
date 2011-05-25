# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_macro.sh 356 2008-02-19 07:56:19Z root $
#
function test_macro()
{
if [ $GCCVER = 2 ]
 then return
fi
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.h << '---end---'
#define RETURN return
---end---
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define FOOBAR foobar()
#define BARFOO int i=0;
#define RET return i;
#define EMPTY
using namespace std;
int foobar() { return 42; }
int inc(double x) { return x + 1; }
#define INC(a) inc(a)
#define INC1(b) inc(b)
#define INC2 inc

struct Foo
{
	Foo() { }
};
const void* foobar(const Foo& f)
{
	return &f;
}
#define FOOBARX Foo

int main()
{
	BARFOO
	cout << MIN(1, ++i) << endl;
#undef BARFOO
	Foo();
	cout << i << endl;
	cout << INC(inc(.1)) << endl;
	foobar();
	RET
#include "foo.h"
}
---end---

build ${1:-$debug} foo.cpp
shift

#write test script for the AutoTest plugin
cat > ./script << '---end---'
call { eval EMPTY }
expect { 1
}
call { eval RETURN }
expect { debug symbol not found: RETURN
}
call next
call next
call list

#call test.cancel
call { FOOBAR + FOOBAR }
expect { 84
}
call { INC(3.4) }
expect { 4
}

call { MIN(1, ++i) }
expect { 2
}
call { i = 0 }

x = FOOBAR
call { eval FOOBAR }
expect { $x }
call { eval BARFOO }
expect { int i=0;
}
call RET
expect { return i;
}
call { eval __GNUC__ = 3 }
expect { non-lvalue in assignment
}

call { INC(inc(1.)) }
expect { 3
}

call { inc(INC(1.)) }
expect { 3
}

#echo "-------------------------------------------------------"
#call test.cancel
call { INC(INC(1.)) }
expect { 3
}
call next
call { eval BARFOO }
expect { debug symbol not found: BARFOO
}
#call { eval foobar(FOOBARX()) }
call quit
---end---
#finally, run the test
run_debugger --main ./a.out $@
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_macro -g3 $@
    else
        test_macro -g3 $@
        #test_macro -gdwarf-23 $@
    fi
}

# run this test standalone
source suffix.sh
