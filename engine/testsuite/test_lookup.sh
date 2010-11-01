# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_lookup.sh 205 2007-11-22 03:52:59Z root $

################################################################
#
################################################################
function test_lookup()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -rf $config a.out foo.cpp
cat > foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
class TestClass
{
    int i_;
public:
    explicit TestClass(int i) : i_(i) { }
    virtual ~TestClass() {}
    virtual int get() const { return i_; }
    virtual int& get() { return i_; }
    virtual int get() volatile { return i_; }
    virtual int get() const volatile { return i_; }
};
int main()
{
    TestClass test(42);
    return test.get();
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call { exec a.out }
call { lookup /c TestClass::get }
expect { 4
}
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

rm -f $config
run_debugger 
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_lookup -g
    else
        test_lookup -gstabs+
        test_lookup -gdwarf-2
    fi
}

source suffix.sh
