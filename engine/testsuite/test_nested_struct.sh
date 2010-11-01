# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_nested_struct.sh 356 2008-02-19 07:56:19Z root $

################################################################
#
################################################################
function test_nested_struct()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call next
call next
call next
call next
call ( emp.age )
expect ( 35
)
call ( emp.first )
expect ( "John"
)
call ( emp.last )
expect ( "Doe"
)
call quit
---end---

#compile
rm a.out
gcc ${1:-$debug} nested_struct.c
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
        test_nested_struct -g
    else
        test_nested_struct -gdwarf-2 -g $@
        #test_nested_struct -gstabs+ $@
    fi
}

source suffix.sh
