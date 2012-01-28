# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_io()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
#include <iostream>

class MyStream
{
public:
	MyStream& operator<<(const char* msg) 
	{
		std::cout << msg;
	}
};
int main()
{
	MyStream ms;

	ms << "foo;" << "bar\n";
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call ( next )
call ( ms << \"foo\n\" )
call ( test.cancel )
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

rm -f $config
run_debugger --main a.out $@
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_io -g
    else
        test_io -gdwarf-2 $@
        test_io -gstabs+ $@
    fi
}

source suffix.sh
