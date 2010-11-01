# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_no_info.sh 205 2007-11-22 03:52:59Z root $

################################################################
#
################################################################
function test_no_info()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
int foo(const char* s)
{
	int x = 0;
	while (*s)	
	{
		x += *s++;
	}
	return x;
}
int main(int argc, char* argv[])
{
	int y = foo(argv[0]);
	return y;
}
	
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( exec a.out )
call test.cancel
call quit
---end---

#compile
rm a.out
build foo.cpp

rm -f $config
run_debugger $@ 
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh
    test_no_info
}

source suffix.sh
