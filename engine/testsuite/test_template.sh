# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_template()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
int main(int argc, char* argv[])
{
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call test.cancel
call quit
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
        test_template -g
    else
        test_template -gdwarf-2 -g3 $@
        test_template -gstabs+ $@
    fi
}

source suffix.sh
