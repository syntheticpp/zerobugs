# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_sigtrap()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
void do_trap(int flag)
{
    if (flag)
    {
        asm ("int $3");
    }
}
void do_segv(int flag)
{
    if (flag)
    {
        *(int*)0 = 1;
    }
}

int main(int argc, char* argv[])
{
    do_trap(argc == 1);
    do_segv(argc > 1);
    return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'

eip = ( eval /x %eip )
rip = ( eval /x %rip )
call ( show status )
expect ( Thread received SIGTRAP at: $rip )
or ( Thread received SIGTRAP at: $eip )
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp
shift
rm -f $config
source gen_core.sh
run_debugger $@ core


cat > script << '---end---'

eip = ( eval /x %eip )
rip = ( eval /x %rip )
call ( show status )
expect ( SIGSEGV at: $rip )
or ( SIGSEGV at: $eip )
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp
shift

rm -f $config
COREARG=1 source gen_core.sh 
run_debugger $@ core
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_sigtrap -g
    else
        test_sigtrap -gdwarf-2 -g3 $@
        test_sigtrap -gstabs+ $@
    fi
}

source suffix.sh
