# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_expr_opt.sh 356 2008-02-19 07:56:19Z root $

################################################################
#
################################################################
function test_expr_opt()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
#include <iostream>


void foo (int x)
{
    x++;
    std::cout << x << std::endl;
    std::cout << &foo << std::endl;
}

int main(int argc, char* argv[])
{
    int j = 42;
    foo(j);

    return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( next )
call ( step )
call ( next )
call ( eval x )
expect ( 43
)
call quit
---end---

#compile the sample
rm a.out
build ${1:-$debug} -O2 foo.cpp

rm -f $config
shift
run_debugger $@ --main a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_expr_opt -g
    else
        test_expr_opt -ggdb $@
    fi
}

source suffix.sh
