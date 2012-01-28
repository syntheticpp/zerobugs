# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_anon_struct()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( next )
call ( step )
call ( next )
call ( whatis *s )
expect ( Second
)
call ( quit )
---end---

rm -f a.out $config
CFLAGS="-std=gnu99 -Wall -g" compiler=gcc build ${1:-$debug} check.c
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
        test_anon_struct -g
    else
        test_anon_struct -gdwarf-2 -g3 $@
        test_anon_struct -gstabs+ $@
    fi
}

source suffix.sh
