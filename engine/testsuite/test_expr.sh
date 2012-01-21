# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_expr.sh 508 2008-05-24 21:32:12Z root $
#
################################################################
# Test expression evaluation
################################################################

function test_expr_eval()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
struct C { double val; C() : val(1.05) {} };
static C _c;
#ifdef TEST_NAMESPACE
namespace B 
{
#else
struct B
{
#endif
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
void foo(int i)
{
	i++;
	std::cout << i << std::endl;
	std::cout << "hello" << std::endl;
}

int main()
{
	B::A a;
	int res = B::A::fuk;
	//A a;
	//return A::fuk;

	a.next = &_c;
	res += a.foo();
	int i = 42;
	foo(i);
	return res;
}
---end---

#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
echo ##### test_expr_eval #####
call { exec a.out }
call { break main }
call continue

call next
call next

call { eval a }
call { eval B::A::fuk }
expect { 42
}
call { eval "-1 << 1" }
expect { -2
}
call { eval a }
expect { {s=32767, us=65535, c=127, uc=255, next=NULL, fuk=42}
} or { {s=32767, us=65535, c=127, uc='H', next=NULL, fuk=42}
} or { {s=32767, us=65535, c=127, uc=255, next=NULL}
}
#'H': PowerPC weirdness, will have to investigate

call { eval "a + a" }
#expect { invalid operands of types B::A and B::A to operator +
expect { operator+ not found in: B::A
}
call { eval "a + 1" }
expect { operator+ not found in: B::A
}
call { eval "a / 2" }
expect {
operator/ not found in: B::A
}
call { eval "a << 1" }
expect {
operator<< not found in: B::A
}
call { eval "a & 1" }
expect {
#---end---
#ARCH=`uname -a | cut -f11 -d' '`
#if [ $ARCH = "x86_64" ]
#	 then echo invalid operands of types B::A and long to operator AND >> script
#	 else echo invalid operands of types B::A and int to operator AND >> script
#fi
#echo "#$ARCH" >> script
#cat >> script << '---end---'
invalid operands of types B::A and int to operator AND
}
call { eval "a.s << 1" }
expect { 65534
}
call { eval "a.us << 1" }
expect { 131070
}
call { eval "++a.us" }
expect { 0
}
call { eval "a.c + a.uc" }
expect { 126
}
call { eval "(&a)->c == a.c" }
expect { 1
}
call { eval "(&a)->c >>= 2" }
expect { 31
}
call { B::A::fuk >>= 3.14 }
#call { eval "5 >>= 3.14" }
expect {
invalid operands of types int and double to operator >>
} or {
invalid operands of types int and complex float to operator >>
}

call { eval a.next->uc }
expect { C*: could not get pointed object
}
call a.prev
call next
call next
call { eval a.next->val }
expect { 1.05
}

call { eval (&a)->val }
expect { B::A has no member named 'val'
}

call step
call step

call { eval next->val }
expect { 1.05
}
call ret
call next
call step
call next
call { eval i }
expect {
42
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
		test_expr_eval -g
	else
		test_expr_eval -gstabs+ $@
		test_expr_eval -gdwarf-2 $@
		test_expr_eval -g $@
		test_expr_eval -ggdb $@
	fi
}

source suffix.sh
