# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_interface.sh 432 2008-04-04 09:28:11Z root $

################################################################
# Test RTTI with template implementation of an interface
################################################################
function test_interface()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
if [ $GCCVER = 2 ]
then echo "this test is known to fail with gcc 2.95"; return
fi

echo $config
rm -f $config a.out foo.cpp

cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#include <typeinfo>
using namespace std;
class Interface 
{
protected:
	virtual ~Interface() { }
};
struct Abstract : public Interface
{
	virtual const char* fun() const = 0;
	virtual int bar() const = 0;
};
struct Detail
{
	int value_;
	Detail() : value_(0) {}
	virtual void set_value(int value) { value_ = value; }
};
template<typename T, typename B = Detail>
struct Impl : public T, public B
{
	const char* fun() const { return typeid(T).name(); }
	int bar() const { return this->value_; }
};
const char* fun(const Abstract& a)
{
	const char* result = a.fun();
	clog << result << endl;
	return result;
}
int main()
{
	Impl<Abstract> impl;
	impl.set_value(42);
	fun(impl);
	return 0;
}
---end---

################################################################
#write test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_interface #####
call ( break fun )
call continue
call next

#call ( a.value_ )

call ( (&a)=>value_ )
expect ( 42
)
call quit
---end---

#compile
build ${1:-$debug} foo.cpp
shift
run_debugger --main ./a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_interface -g
    else
        test_interface -gdwarf-2 $@
        test_interface -gstabs+ $@
    fi
}

source suffix.sh
