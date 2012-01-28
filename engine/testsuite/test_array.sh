# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_array()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
int ga[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

int main(int argc, char* argv[])
{
	long a[300];
	long* p = &a[0];
	for (int i = 0; i != 300; ++i)
	{
		a[i] = i * 2;	
	}
	int x = 42;
	return x;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( ga[9] )
expect ( 10
)
call ( ga[10] )
expect ( array subscript is out of range
)
call ( break foo.cpp:11 )
call ( continue )
call ( a[200] )
expect ( 400
)
call ( a[3.14f] )
expect ( invalid type float for array subscript
)
or ( invalid type __float32 for array subscript
)
call ( p[100] )
expect ( 200
)
call ( p[-5] )
expect ( array subscript is out of range
)
call ( ga[1:1][0] )
expect ( 2
)
call ( ga[1:0][0] )
expect ( 2
)
call ( ga[:5][6] )
expect ( array subscript is out of range
)
call ( ga[5:][5] )
expect ( array subscript is out of range
)
call ( x[3] )
expect ( base operand of [] is not pointer or array
)
call ( &a[4] - &a[1] )
expect ( 3
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

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
        test_array -g
    else
        test_array -gstabs+ $@
        test_array -gdwarf-2 -g3 $@
    fi
}

source suffix.sh
