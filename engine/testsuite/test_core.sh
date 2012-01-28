# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#

function test_core()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
void crash(double val)
{
    for (int i=0; i != 42; ++i)
        *(int*)3 = val;
}
struct ABC
{
	int foo_;

	virtual int baz() const
	{
    #if defined(__i386__) || defined(__x86_64__)
        asm ("mov $123, %ebx");
    #endif
		if (foo_)
			crash(3.1419);
		return 100;
	}
};
int main()
{
	ABC abc;
	abc.foo_ = 42;
	abc.baz();
	return 0;
}
---end---
#compile the test program
build ${1:-$debug} foo.cpp -lm 
shift

source gen_core.sh

cat > ./script << '---end---'
echo ##### test_core #####
call { handle 11 ignore }
call { loadcore core }

call { eval val }
expect { 3.1419
}
call { eval ++i }
expect {
cannot modify variables in core file
}
call next
expect {
#operation not available for core files
thread is not runnable
}

call instruction
expect {
#set_single_step_mode: operation not available for core files
thread is not runnable
}
call { break main }
expect {
thread is not runnable
}
call { show regs /all }

call up

call { %r 5 }
expect-x64 { 0x7b
}
call { %r 0 }
expect-i386 { 0x7b
}

### test that regs are being read correctly from the corefile,
### let's try ebx really quick:
call { %ebx }
#todo: %ebx should work on x86_64!!!
expect-i386 { 123
}
call { %rbx }
expect-x64 { 123
}
call { eval this->foo_ }
expect { 42
}
call continue
call quit

---end---
run_debugger $@
}

function run()
{
    source common.sh
    test_core -gstabs+ $@
    test_core -gdwarf-2 $@
}

source suffix.sh
