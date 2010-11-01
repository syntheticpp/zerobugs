# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_expr_lazy.sh 356 2008-02-19 07:56:19Z root $

################################################################
#
################################################################
function test_expr_lazy()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
int a = 0, b = 0;
int fun_a()
{
	return a = 1;
}
int fun_b()
{
	return b = 1;
}
int main(int argc, char* argv[])
{
	int(*pf)() = &fun_a;
	pf = &fun_b;
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( next )
call ( next )
call ( fun_a() || fun_b() )
expect ( 1
)
call ( a )
expect ( 1
)
call ( b )
expect ( 0
)
call ( 0 && fun_b() )
expect ( 0
)
call ( b )
expect ( 0
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

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
        test_expr_lazy -g
    else
        test_expr_lazy -gdwarf-2 -g3 $@
        test_expr_lazy -gstabs+ $@
    fi
}

source suffix.sh
