# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_forkexec.sh 524 2008-06-13 07:51:22Z root $
#
function test_forkexec()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f foo bar $config
cat > ./foo.cpp << '---end---'
// foo.cpp
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
void child()
{
	execlp("./bar", "bar", "fubar", NULL);
}
int main()
{
	pid_t pid = getpid();
	std::cout << "pid=" << pid << std::endl;

	if ((pid = fork()) == 0)
	{
		child();
	}
	else
	{
		std::cout << pid << std::endl;	
		waitpid(pid, 0, 0);
	}
	return 0;
}
---end---
cat > ./bar.cpp << '---end---'
// bar.cpp
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char* argv[])
{
	std::clog << "&argc=" << &argc << std::endl;
	for (--argc, ++argv; argc; --argc, ++argv)
	{
		std::clog << *argv << std::endl;
	}

	pid_t pid = fork();
	if (pid == 0)
	{
		kill(getgid(), SIGSTOP);
	}
	else
	{
		waitpid(pid, 0, 0);
	}
	return 0;
}
---end---
build $debug $@ foo.cpp -o foo
build $debug $@ -fno-omit-frame-pointer -O0 bar.cpp -o bar

#write test script for the AutoTest plugin
cat > ./script << '---end---'

addr = { eval (int)&child }
call { break child }
call { continue }
call { clear $addr }
call { step }
call { show threads /count }
expect { 2
}
call { continue }
call { break main }
call { continue }
call { step }
call { eval argc }
expect { 2
}

call { eval argv[1] }
expect { "fubar"
}
call { quit }

---end---

#finally, run the test
run_debugger --main --trace-fork foo 
}

################################################################
# Run this test standalone
################################################################
function run()
{
	source common.sh
	test_forkexec -gstabs+ 
	test_forkexec -gdwarf-2
}

# run this test standalone
source suffix.sh
