# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_static_const.sh 521 2008-06-13 01:51:19Z root $

################################################################
#
################################################################
function test_static_const()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
#include <iostream>
#include <string.h>
using namespace std;
namespace 
{
	static const double pi = 3.14;

	struct XXX
	{
		int i;
		char c;
		double d;
		XXX() { }
		static const long VER = 5;
	};
}
struct ZZZ
{
	static const long VER = 6;
	int x;
};

class AAA
{
	static const int MINOR = 3;
	static int REVISION;
	char name[10];

public:
	static const int MAJOR = 2;

	explicit AAA(const char* n) 
	{ memset(name, 0, sizeof(name));
	  strncpy(name, n, sizeof name);
	}

	virtual int version(int* minor = 0) const
	{
		if (minor)
			*minor = MINOR;
		return MAJOR;
	}
};
int AAA::REVISION = 4;
class BBB : public AAA
{
	double data;
public:
	BBB() : AAA("BBB"), data(1.1618) { }

	static const char* blah() { return "blah"; }
};

int qqq = -1;

int main(int argc, char* argv[])
{
	struct YYY
	{
		long xxx;
	};
	AAA aaa("test static const");
	cout << aaa.MAJOR << endl;
	XXX xxx /* = { 1, 2, 3. } */;
	YYY yyy = { 42 };
	cout << yyy.xxx << endl;
	cout << XXX::VER << endl;
	AAA* bbb = new BBB();	
	static_cast<BBB*>(bbb)->blah();
	cout << aaa.version() << endl;
	cout << bbb->version() << endl;
	delete bbb;
	//ZZZ zzz;
	return qqq + ZZZ::VER;
}
---end---

#test script (processed by the AutoTest plugin)
cat > script << '---end---'
#call test.cancel
#call ( eval XXX::VER )
#expect ( 5
#)
#FIXME
#call ( eval VER )
#expect ( debug symbol not found: VER
#)
call ( eval ZZZ::VER )
expect ( 6
)
call ( eval AAA::MAJOR )
expect ( 2
)
call ( eval AAA::MINOR )
expect ( 3
)
call ( eval AAA::REVISION )
expect ( 4
)
call ( eval MAJOR )
expect ( debug symbol not found: MAJOR
)
call ( eval REVISION )
expect ( debug symbol not found: REVISION
)

#call ( eval blah )
#expect ( debug symbol not found: blah
#expect ( BBB::blah()
#)

call ( BBB::blah() )
expect ( "blah"
)
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
        test_static_const -g
    else
        test_static_const -gdwarf-2 -g3 $@
        #test_static_const -gstabs+ $@
    fi
}

source suffix.sh
