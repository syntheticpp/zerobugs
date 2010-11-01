# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_bitfields.sh 356 2008-02-19 07:56:19Z root $

################################################################
#
################################################################
function test_bitfields()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call next
call next
call next
call next
call ( eval b )
expect ( {flag=1, size=9}
)
call ( eval moreBits )
expect ( {magic=255, data=66}
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} bits.c

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
        test_bitfields -g
    else
        test_bitfields -gdwarf-2 -g3 $@
        test_bitfields -gstabs+ $@
    fi
}

source suffix.sh
