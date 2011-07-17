# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_double_reg.sh 656 2009-10-26 03:42:56Z root $

################################################################
# Test usage of double FPU registers in DWARF expressions
################################################################
function test_double_reg()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
double sum(double x, double y, int n = 0)
{
    double result = x + y;
    for (int i = 0; i != n; ++i)
    {
		result += sum(x, y);
    }
    return result;
}
double (*pf)(double, double, int);
int main(int argc, char* argv[])
{
    double result = sum(3.14, 1.1);
    pf = sum;
    result = (*pf)(result, .0, 0);
    return result;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( break foo.cpp:4 )

call ( continue )
call ( x )
expect ( 3.14
)
call ( y )
expect ( 1.1
)
call continue
call ( x )
expect ( 4.24
)
# TODO: FIXME
#call ( y )
#expect ( 0
#)
call continue
call quit
---end---

#compile
rm a.out
build ${1:-$debug} -O2 -fno-inline -Winline foo.cpp

rm -f $config
run_debugger --main a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_double_reg -g
    else
        test_double_reg -gdwarf-2 -g3 $@
    fi
}

source suffix.sh
