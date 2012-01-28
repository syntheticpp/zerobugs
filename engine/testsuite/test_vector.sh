# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_vector()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
#include <vector>

int main()
{
    typedef std::pair<int,char const*> Pair;
    typedef std::vector<Pair> Vector;

    Vector test;

    test.push_back( Pair( 1, "one" ));
    test.push_back( Pair( 2, "two" ));
    test.push_back( Pair( 3, "three" ));
    test.push_back( Pair( 4, "four" ));

    return 0;
}
---end---


#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call next
call next
call ( test[0] )
expect ( {first=1, second="one"}
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp
shift

rm -f $config
#run_debugger --main a.out --py-run=${path}/plugin/.zero.py
run_debugger --main a.out --py-run=${path}/plugin/init.py
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_vector -g
    else
        test_vector -gdwarf-2 -g3 $@
        test_vector -gstabs+ $@
    fi
}

source suffix.sh
