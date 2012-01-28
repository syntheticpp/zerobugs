# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_static()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
struct X;
struct Y
{
	static X x;
	int n;
};
struct X
{
	Y y;
};
X Y::x;
int main(int argc, char* argv[])
{
	X x;
	x.y.n = 42;
	return 0;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( next )
call ( next )
call ( eval x )
expect ( {y={n=42}}
)
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

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
        test_static -g
    else
        test_static -gdwarf-2 -g3 $@
        test_static -gstabs+ $@
    fi
}

source suffix.sh
