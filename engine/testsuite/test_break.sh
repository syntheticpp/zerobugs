# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_break.sh 356 2008-02-19 07:56:19Z root $

################################################################
#
################################################################
function test_break()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config a.out foo.cpp
cat > foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
class MyClass
{
    double x_;
public:
    explicit MyClass(double x) : x_(x) { }
    double fun(long y) volatile; 
	virtual int fred() volatile const
	{
		return 45;
	}
};    
double MyClass::fun(long y) volatile 
{ 
    return x_ + y + fred(); 
}
int main(int argc, char* argv[])
{
    MyClass a(3.1419);
    a.fun(2);
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call { exec a.out }

echo Setting breakpoint at MyClass::fun... 
call { break MyClass::fun }
echo Resuming...

call continue
echo Breakpoint hit. 
call next

call { eval y }
expect { 2
}

call { eval x_ }
expect { 3.1419
}
echo Setting breakpoint at MyClass::fred...
#call { show breakpoints }
call { break MyClass::fred }
echo Resuming...
call continue

echo Executing a.out...
#call { exec a.out }
call { restart }
call { show breakpoints }

call { continue }

#call test.cancel
call { next }
call { eval y }
expect { 2
}
call quit
---end---

#compile
rm -f a.out
build ${1:-$debug} foo.cpp
shift
#because unrecognized command line args are ignored, 
#it is ok to pass the entire command line to the debugger
ZERO_SAVE_STATE=1 run_debugger $@
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_break -g
    else
        ZERO_DEBUG_MAPS=1 test_break -gstabs $@
        test_break -gstabs+ $@
        test_break -gdwarf-2 $@
    fi
}

source suffix.sh
