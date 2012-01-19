# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_virt_call.sh 356 2008-02-19 07:56:19Z root $

################################################################
#
################################################################
function test_virt_call()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
struct Base
{
	virtual int bar() const { return 99; }
    virtual int fun() const { return 13; }
	int x;
};
struct Derived : public Base
{
	double fun(double x) const { return x * .1; }
    virtual int fun() const { return 42; }
	virtual double fun(int x, int y) const { return x * y * .1; }
};
int main()
{
    Derived d;
    Base base;
    d.x = 123;
    d.fun();
    Base* pbase = &d;
    base.fun(); pbase->fun();
    d.fun(.5);
    Base& b = d;
    return b.fun();
}
---end---

cat > ./script << '---end---'
echo ##### test_virtual_call #####
call { exec a.out }
call { break main }
call continue 
call next
call next
call next
call next
call next
call next

call { base.fun(); }
expect { 13
}

#call test.cancel
call { pbase->fun(); }
expect { 42
}

call next
call next
call next

call { b.fun(); }
expect { 42
}
call quit
---end---

#compile
build ${1:-$debug} foo.cpp
shift
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
        test_virt_call -g $@
    else
        test_virt_call -gdwarf-2
        test_virt_call -ggdb
        test_virt_call -gstabs+ 
    fi
}

source suffix.sh
