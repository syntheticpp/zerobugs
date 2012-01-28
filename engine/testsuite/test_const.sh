# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_const()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'


int main()
{
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( eval 0x80000000 )
expect ( -2147483648
)
call ( eval 0x80000000L )
expect-x64 ( 2147483648
)
call ( eval 0x80000000L )
expect-i386 ( -2147483648
)
call ( eval 0x80000000LL )
expect ( 2147483648
)
call ( eval 0x80000000U )
expect ( 2147483648
)
call ( eval 0xf0000000 )
expect ( -268435456
)
call ( eval 0xf0000000u )
expect ( 4026531840
)
call ( eval 0x0F0000000 )
expect ( 4026531840
)
call (	quit	)
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp
shift
rm -f $config
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
        test_const -g
    else
        test_const -gdwarf-2 -g3 $@
        test_const -gstabs+ $@
    fi
}

source suffix.sh
