# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function TEMPLATE()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

rm -f $config
run_debugger $@ 
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        TEMPLATE -g
    else
        TEMPLATE -gdwarf-2 $@
        TEMPLATE -gstabs+ $@
    fi
}

source suffix.sh
