# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_rtti2.sh 205 2007-11-22 03:52:59Z root $

################################################################
#
################################################################
function test_rtti_2()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
struct Base
{
	virtual int fun() const = 0;
};
struct Base2
{
	virtual int bar() const { return 0; }
};
template<typename T>
struct Impl : public T
{
	virtual int fun() const { return 42; }
};
struct Fred : Impl<Base>
{
	virtual int fun() const { return 123; }
	virtual int bar() const { return 456; }
	int fred_;
};
struct Barney : public Base2, public Fred
{
	int fun() { return 456; }
};
const void* foo(const Base& b)
{
	return &b + 1;
}
int main()
{
	Fred f;
	Barney b;
	foo(b);
	return 0;
}

---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call next
call next
call step
call next
call list
call test.cancel
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

rm -f $config
run_debugger $@ --main a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_rtti_2 -g
    else
        test_rtti_2 -gdwarf-2 $@
        test_rtti_2 -gstabs+ $@
    fi
}

source suffix.sh
