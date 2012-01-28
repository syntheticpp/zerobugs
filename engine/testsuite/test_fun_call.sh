# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_function_call()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -rf $config a.out foo.cpp
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#include <stdarg.h>
int fun(double x, double y)
{
     std::clog << x << ", " << y << std::endl;
     return x * y;
}
//note: fails with icc on x86_64
//long double bar(int x)
double bar(int x)
{
    return x * 3.14;
}

int vbar(int x, ...) // test calling a func. with var args.
{
    va_list va;
    va_start(va, x);
	int y = va_arg(va, int);
	std::clog << __func__ << " x=" << x  << ", y=" << y << std::endl;
    x += y;
    va_end(va);
    return x;
}
extern "C" float vbarc(int x, ...)
//extern "C" int vbarc(int x, ...)
//extern "C" double vbarc(int x, ...)
{
    va_list va;
    va_start(va, x);
    double d = va_arg(va, double);
	std::clog << "x=" << x << " d=" << d << std::endl;
    d += x;
    va_end(va);
    return d;
}
namespace X
{
#if (__GNUC__ >= 3)
    struct Interface
#else
    struct __attribute__((com_interface)) Interface
#endif
    {
        virtual int get() = 0;
    };

    class Fubar : public Interface
    {
        int i_;
    public:
        Fubar() : i_(42) {}

        static int compute(char x) { return 42 + x; }

        int get() { return i_; }

        void set(int i) { std::clog << i << "\n"; i_ = i; }
    };
}
struct Baz : public X::Fubar
{
    double x_; 
};
struct Bar : public Baz
{
    void foo();
};
void Bar::foo()
{
    x_ = 3.14;
    set(12345678);
}
int badaboom(bool f)
{
	return f ? 42 : 13;
}
int main()
{
    //using namespace X;
    fun(3.14, 42);

    X::Fubar f;

    f.set(X::Fubar::compute(1));
    f.get() + vbar(1,2) + vbarc(2,2.3);

    Bar baz;
    baz.foo(); 
    return badaboom(0.1);
}
---end---

#write test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_function_call #####
call { exec a.out }
call { break main }
call continue 
call next
call next
call next

call { vbar(2,2) }
expect { 4
}

call { vbarc(1, 2.5) }
expect { 3.5
}

call { fun(1.5, 4) }
expect { 6
}
call { bar(2) }
expect { 6.28
}

call { f.set(123); }
call { f.get(); }
expect { 123
}

call { f.compute(0) }
expect { 42
}

call { X::Fubar::compute(1) }
expect { 43
}
echo static method passed

call { f.get() + X::Fubar::compute(0) }
expect { 165
}
call next
call next
call next

call { baz.set(101) }
call { baz.get() }
expect { 101
}

#call { badaboom(static_cast<bool>(1)) }
#expect { 42
#}
#
# Test that integers are implicitly converted to bool
#
call { badaboom(0) }
expect { 13
}

#
# Test that floats are implicitly converted to bool
#
call { badaboom(1.3) }
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
        test_function_call -g
    else
        test_function_call -gdwarf-2 $@
        test_function_call -gstabs+ $@
    fi
}

source suffix.sh
