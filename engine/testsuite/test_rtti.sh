# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_rtti.sh 356 2008-02-19 07:56:19Z root $
################################################################
#
################################################################
function test_rtti_discovery()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
sed -e "s/VIRT_A/$1/g; s/VIRT_B/$2/g" > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#define EMPTY
#include <iostream>
#if (__GNUC__ < 3) && defined(_CXX_V3_COMPAT)
 #define VTABLE __attribute__((com_interface))
#else
 #define VTABLE
#endif
struct VTABLE BaseOne { double long x; virtual ~BaseOne() {} };
struct VTABLE BaseTwo { int y; virtual ~BaseTwo() {} };
struct VTABLE BaseMiddle { short z; };
struct Derived : VIRT_A public BaseOne
               , public BaseMiddle
               , VIRT_B public BaseTwo 
{
	Derived() : x_(42) { }
    int x_; 
};
void f(BaseOne* p)
{
    std::clog << "p=" <<(void*)p << std::endl;
    p->x = 1;
}
void g(BaseTwo* p)
{
    p->y = 2;
}
int main()
{
    Derived d;
    d.x = 2;
    d.y = 1;
    f(&d);
    g(&d);
}
---end---

#write test script (processed by the AutoTest plugin)
#cat > ./script << '---end---'
sed -e "s/VIRT_A/$1/g; s/VIRT_B/$2/g; s/DFMT/$debug/g" > ./script << '---end---'
echo test_rtti_descovery
echo BaseOne=VIRT_A BaseTwo=VIRT_B debug_flags=DFMT
call { exec a.out }
call { break main }

call continue
call next
call next
call next
call next

call step
call next

call { eval p=>y }
expect { 1
}
call ret
---end---

CXXVER=`"$compiler" -v 2>&1 | tail -1 | cut -f3 -d' '`
echo "# gcc $CXXVER" >> script

case "$CXXVER" in
 3.3.5);;
 3.4.*);;
 4.*);;
 *) echo "call next" >> script;;
esac

cat >> script << '---end---'
#call { test.cancel }
call step
call step

call next
call { eval p=>x }

expect { 1
}
call quit
---end---

build ${debug} foo.cpp
run_debugger

#rm a.out
#build ${debug} -D_CXX_V3_COMPAT foo.cpp
#run_debugger
}


################################################################
#
################################################################
function test_rtti()
{
	rm -f $config
    
    echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile

    echo "####################################" >> $logfile
    echo test_rtti_discovery, no virtual base >> $logfile
    test_rtti_discovery EMPTY EMPTY $@

    echo "#######################################" >> $logfile
    echo test_rtti_discovery, first base virtual >> $logfile
    test_rtti_discovery virtual EMPTY $@

    echo "########################################" >> $logfile
    echo test_rtti_discovery, second base virtual >> $logfile
    test_rtti_discovery EMPTY virtual $@
    
    echo "########################################" >> $logfile
    echo test_rtti_discovery, both bases virtual >> $logfile
    test_rtti_discovery virtual virtual $@
}


################################################################
#
################################################################
function test_rtti_with_templates()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#if (__GNUC__ < 3) && defined(COM)
 #define VTABLE __attribute__((com_interface))
#else
 #define VTABLE
#endif
struct VTABLE BaseOne 
{
    virtual ~BaseOne() {};
    virtual void foo() = 0; 
    int x; 
};
struct VTABLE BaseTwo 
{   
    virtual ~BaseTwo() {};
    virtual void bar() = 0; 
    int y;
};
template<typename T> class FooImpl : public T { void foo() {} };
template<typename T> class BarImpl : public T { void bar() {} };
struct Derived : /* VIRT_A */ public FooImpl<BaseOne>
               , /* VIRT_B */ public BarImpl<BaseTwo>
{
};
void f(BaseOne* p)
{
    p->x = 1;
}
void g(BaseTwo* p)
{
    p->y = 2;
}
int main()
{
    Derived d;
    d.x = 2;
    d.y = 1;
    f(&d);
    g(&d);
}
---end---
build ${1:-$debug} foo.cpp
shift

#write test script for the AutoTest plugin
cat > script << '---end---'
call { exec a.out }
call { break main }
call continue
call next
call next
call next
call next
call step
call next

call { eval p=>y }
expect { 1
}
call ret
---end---
# FIXME: add support in the AutoTest plugin for conditional calls 
CXXVER=`"$compiler" -v 2>&1 | tail -1 | cut -f3 -d' '`

echo "# gcc $CXXVER" >> script
case "$CXXVER" in
 3.3.5);;
 3.4.*);;
 4.*);;
 *) echo "call next" >> script;;
esac

cat >> script << '---end---'
call step
call step
call next
#call ( test.cancel )

call { eval p=>x }
expect { 1
}
call quit
---end---

#finally, run the test
run_debugger  $@
}


function run()
{
    source common.sh
	debug=-gdwarf-2
	test_rtti_discovery virtual virtual

	debug=-gstabs+
	test_rtti_discovery virtual virtual

	debug=-gstabs+
    test_rtti 
    test_rtti_with_templates 

	debug=-gdwarf-2
    test_rtti
    test_rtti_with_templates
}

source suffix.sh
