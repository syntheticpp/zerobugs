# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
################################################################
# Test step over
################################################################

function test_step_over()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
struct C { double val; C() : val(1.05) {} };
static C _c;
struct B
{
struct A
{
    short s;
    unsigned short us;
    char c;
    unsigned char uc;

    C* next;

    static int fuk;

    A() : s(32767), us(65535), c(127), uc(255), next(0) {}

    int foo()
    {
        return s;
    }
};
};
int B::A::fuk = 42;

int main()
{
    B::A a;
    int res = B::A::fuk;

    a.next = &_c;
    return res + a.foo();
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call { exec a.out }
call { break main }
call continue

call next
call next
call next
call list
call line
expect {
33
}

call quit
---end---

dbgopt=${1:-$debug}
shift

build $dbgopt -DTEST_NAMESPACE foo.cpp
run_debugger $@

build $dbgopt foo.cpp
run_debugger $@
}

function run()
{
    source common.sh

    if [ "$compiler" = icc ]
    then
        test_step_over -g
    else
        test_step_over -gstabs+ $@
        test_step_over -gdwarf-2 $@
    fi
}

source suffix.sh
