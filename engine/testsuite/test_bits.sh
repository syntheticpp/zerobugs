# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$

################################################################
#
################################################################
function test_bits()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config a.out foo.cpp
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
struct Foo
{
	bool flip : 1;
	bool flop : 2;
	signed char flab : 2;
};
struct Package
{
	float balls[2];
	long tool : 12;
	short form : 5;
};
struct Package2 : public Package
{
	long foo : 8;
	int bar : 12;
};
int main()
{
	Foo foo;
	foo.flip = true;
	foo.flop = false;
	Package package;
	package.balls[0] = package.balls[1] = 3.1419;
	package.tool = 10;
	package.form = 5;
	foo.flab = 2;
	Package2 pkg2;
	pkg2.foo = 'x';
	package.tool = 100000000000ULL;
	std::cout << package.tool << std::endl;
	return package.tool + foo.flip + foo.flop;
}
---end---
#test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_bits #####
call { exec a.out }
#call test.cancel
call { break main }
call continue 
call next
call next
call next
call { foo.flip }

expect { 1
}
call { foo.flop }
expect { 0
}
call next
call next
call next
call { eval package.tool }

expect { 10
}
call { eval package.form }
expect { 5
}
call { next }
call { eval foo.flab }
expect { -2
}
##### test modifying #####
call { eval foo.flab = -1 }
expect { -1
}
call { foo.flab }
expect { -1
}
call { eval foo.flop = 1 }
call { foo.flop }
expect { 1
}
call { eval foo.flip = 0 }
call { foo.flip }
expect { 0
}
call { eval foo.flip = 1 }
call { foo.flip }
expect { 1
}
call { eval package.tool = 11 }
call { eval package.tool }
expect { 11
}
#call { eval package.tool = 100000000000ULL }
call { next }
call { next }
call { eval package.tool }
expect { -2048
}

call quit
---end---

#compile
build ${1:-$debug} foo.cpp 

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
		test_bits -g
	else
		test_bits -gdwarf-2
	 	test_bits -gstabs+
		#test_bits -gstabs
	fi
}

# run this test standalone
source suffix.sh
