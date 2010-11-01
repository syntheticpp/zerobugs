# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_class_const.sh 205 2007-11-22 03:52:59Z root $

################################################################
#
################################################################
function test_class_const()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
// foo.cpp
struct ABC
{
	static const bool is_ABC = true;
	static const char type = 'A';
	static const int size = 42;
	static const double pi = 3.14;
	enum { FOOBAH = 123 };
	static long foobah;
	long someData_;
};
long ABC::foobah = 100;
int main()
{
	ABC abc;
	abc.someData_ = 123456789;

	return abc.someData_;
}
		
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call test.cancel
call quit
---end---

#compile
rm a.out
build ${1:-$debug} foo.cpp

rm -f $config
run_debugger --main $@ a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_class_const -g
    else
        test_class_const -gdwarf-23 $@
#        test_class_const -gstabs+ $@
    fi
}

source suffix.sh
