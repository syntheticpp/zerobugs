# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_member.sh 205 2007-11-22 03:52:59Z root $
#
# Test that member variables are viewed correctly
################################################################
function test_member_data()
{
echo ----- ${FUNCNAME}${debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
class A
{
    long double d;
public: 
    A() : d(0xffffffffffffffffULL * 1.0) {}

    void func() 
    {
        d /= 2.;
    }
};
int main()
{
    A a;
    a.func();
}
---end---
#compile
build ${debug} foo.cpp

#write test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_member_data #####
call { exec a.out }
call { break main }

call continue
call next
call next
call step
call next
call { print d }
expect { d=1.84467e+19
}
call { eval *this }
expect { {d=1.84467e+19}
}

call { eval d/2.}
expect { 9.22335e+18
}
# FIXME this one fails
#call next
#call { print d }
#expect { d=9.22335e+18
#}

call quit
---end---

#finally, run the test
run_debugger
}

function run()
{
    source common.sh
    debug=-gdwarf-2
    test_member_data

    debug=-gstabs+
    test_member_data
}
source suffix.sh
