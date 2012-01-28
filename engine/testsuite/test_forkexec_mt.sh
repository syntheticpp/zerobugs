# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
function test_forkexec_mt()
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
    execlp("./bar", "bar", "fubar", 0);
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
int main(int argc, char* argv[])
{
    std::clog << "&argc=" << &argc << std::endl;
    for (--argc, ++argv; argc; --argc, ++argv)
    {
        std::clog << *argv << std::endl;
    }
    return 0;
}
---end---
build $debug $@ foo.cpp -o foo -lpthread
build $debug $@ -fno-omit-frame-pointer -O0 bar.cpp -o bar -lpthread

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
run_debugger --main foo 
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh
    test_forkexec_mt -gstabs+ 
    test_forkexec_mt -gdwarf-2
}

# run this test standalone
source suffix.sh
