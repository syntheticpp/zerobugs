# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_darray()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > hello.d << '---end---'

import std.stdio;

int main()
{
	string greet = "Hello Walter";

	writefln(greet);
	return 0;
}
---end---

cat > script << '---end---'
call ( next )
call ( next )
call ( greet[0] )
expect ( 'H'
)
call ( quit )
---end---

#compile
DMD=`which dmd 2>/dev/null`
echo $DMD
if test -n "$DMD"; then
	$DMD -I/usr/local/src/phobos -g hello.d
	rm -f $config
	shift
	ZERO_D_SUPPORT=1 run_debugger $@ --main hello
	rm hello hello.o
fi
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    test_darray 
}

source suffix.sh
