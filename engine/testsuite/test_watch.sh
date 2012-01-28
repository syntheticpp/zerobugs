# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_watchpoint()
{
rm -f a.out $config
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
int foo(int& x)
{
    std::clog << __func__ << ": &x=" << &x << std::endl;
    return x = 2;
}
int baz()
{
    int x = 1;
    std::clog << __func__ << ": &x=" << &x << std::endl;
    return 40 + foo(x);
}
int bar()
{
    int x = 0;
    std::clog << __func__ << ": &x=" << &x << std::endl;
    return ++x;
}
int main()
{
   	baz();
    return bar();
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call { run a.out }
call { break baz }
call { continue }
call { next }
call { next }

call { watch x }
call { continue }
call { eval x }
expect { 2
}
call quit
---end---

#compile
build ${1:-$debug} foo.cpp
shift
rm -f $config
run_debugger $@
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_watchpoint -g
    else
        test_watchpoint -gdwarf-2 $@
        test_watchpoint -gstabs+ $@
    fi
}

source suffix.sh
