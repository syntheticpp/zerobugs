# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_virt_call2.sh 205 2007-11-22 03:52:59Z root $

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
struct Base2
{
    virtual int fun(const Derived*, double x) 
    {
        return 0;
    }
    virtual int fun(const Derived* d, int x) 
	{
		return -1;
	}
};
struct Derived2 : public Base2
{   
    virtual int fun(const Derived* pd, double x)
    {
        return pd ? pd->fun(x) : x;
    }
  	virtual int fun(const Derived*, int x) 
	{
		return -1;
	}
};
int main()
{
    Derived d;
    Base base;
    d.x = 456; base.x = 123;
    d.fun();
    Base* pbase = &d;
    pbase->fun();
    d.fun(.5);
    Derived2 d2; 
    Base2* pb = &d2;
    pb->fun(&d, 3.14); pb->fun(&d, 10);
    // pb->fun(&base,100);
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

call { pbase->fun(); }
expect { 42
}

call next
call next
call next
call next
call next

call { pb->fun(&d, pbase->fun()) }
expect { -1
}

call next
call { b.fun(); }
expect { 42
}

call test.cancel
call { pb->fun(&d, 100) }
expect { -1
}
call { pb->fun(&d, 100.) }
expect { 10
}
call quit
---end---

#compile
build ${1:-$debug} foo.cpp

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
        test_virt_call -gstabs+
        test_virt_call -gdwarf-2
    fi
}

source suffix.sh
